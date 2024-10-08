using ArgParse,Random,ProgressMeter,LinearAlgebra
using DelimitedFiles #for debug only ?

function parseCommandLine()
    s = ArgParseSettings()
    @add_arg_table! s begin
        "input"
            help = "input filename with text corpora"
            arg_type = String
            required = true
        "output"
            help = "output filename with word embeddings"
            arg_type = String
            default = "./embeddings.txt"
        "--wsize"
            help = "window size in words"
            arg_type = Int
            default = 3
        "--nneg"
            help = "nubmer of negative samples"
            arg_type = Int
            default = 3
        "--emb_len"
            help = "size of output embedings"
            arg_type = Int
            default = 32
        "--lr"
            help = "learning rate"
            arg_type = Float32
            default = 0.1
        "--n_iter"
            help = "number of iterations to run"
            arg_type = Int
            default = 25
        "--stopwords"
            help = "input filename with stopwords"
            arg_type = String
            default = "../data/stopwords.txt"
    end
    return parse_args(s)
end

function load_stopwords(filename)
    return readlines(filename)
end

function load_text(filename,stopwords)
    text = read(filename,String)
    text = replace(text,r"[[:punct:]]" => "")
    words = text |> lowercase |> split
    #setdiff(words,stopwords) nice but discards word order
    [w for w in words if w ∉ stopwords]
end

function text_to_indices(words)
    unique_words = unique(words)
    word_to_id = Dict(w => i for (i,w) in enumerate(unique_words))
    text_i = [word_to_id[w] for w in words]
    text_i,unique_words
end

function prepare_train_data(text,window_size,n_negative_samples=nothing)
    Row = Tuple{Int32,Int32,Int32}
    train_data = Row[]
    N = length(text)
    if n_negative_samples === nothing
        n_negative_samples = 2*window_size
    end
    sizehint!(train_data,N*(n_negative_samples + 2*window_size))
    @showprogress 1 "Preparing train data..." for idx in window_size+1:N-window_size
        w = text[idx]
        context = [text[i] for i in idx-window_size:idx+window_size if i!=idx && text[i] != w]
        for c in context
            push!(train_data,(Int32(w),Int32(c),Int32(1)))
        end

        negative_samples = Int32[] #same as Vector{Int32}()
        while length(negative_samples) < n_negative_samples
            i = rand(1:N)
            cw = text[i]
            if cw ∉ context && cw ∉ negative_samples && cw != w
                push!(negative_samples,cw)
            end
        end
        for nw in negative_samples
            push!(train_data, (Int32(w),Int32(nw),Int32(0)) )
        end
    end
    train_data
end

function convert_to_2darray(train_data)
    hcat(collect.(train_data)...)'    # use transpose to get row-size structure
end

function filter_train_data(train_data)
    ws  = BitSet(train_data[:,1])
    cws = BitSet(train_data[:,2])
    common = intersect(ws,cws)
    #[r for r in train_data if r[1] ∈ common && r[2] ∈ common]
    filtered = filter(row -> row[1] ∈ common && row[2] ∈ common, eachrow(train_data))
    reduce(vcat,filtered)
end

function dump_training_data(training_data,unique_words, filename)
    open(filename,"w") do file
        for uw in enumerate(unique_words)
            writedlm(file,[uw]," ")
        end
        for r in training_data
            writedlm(file,[r]," ")
        end
    end
end

function normalize(emb)
    #in place per row normalization 
    emb .= emb ./ norm.(eachrow(emb))
end

function update_embeddings(main_emb,ctx_emb,train_data,learning_rate)
    e = main_emb[ train_data[:,1] ]
    c =  ctx_emb[ train_data[:,2] ]
    dotp = dot.(eachrow(e),eachrow(c))
    sigmoid(x) = 1 / (1+exp(-x))
    scores = sigmoid.(dotp)
    errors = train_data[:,3] - scores
end

function run_word2vec(train_data, n_words, embedding_size, n_iter, learning_rate)
    main_emb = randn(n_words,embedding_size)
    normalize(main_emb)
    ctx_emb  = randn(n_words,embedding_size)
    normalize(ctx_emb)
    @showprogress 1 "Running word2vec..." for _ in 1:n_iter
        update_embeddings(main_emb,ctx_emb,train_data,learning_rate)
    end
end

function main()
    args = parseCommandLine()
    stopwords = load_stopwords(args["stopwords"])
    #println( "stopwords=", join(stopwords[1:10],",") )
    @time text = load_text(args["input"],stopwords)
    #println("words=",words[1:10])
    text_i,unique_words = text_to_indices(text)
    println("loaded $(length(text)) words, $(length(unique_words)) unique")
    window_size = args["wsize"]
    prepare_train_data(text_i[1:10],1)
    println("preparing training data")
    @time train_data = prepare_train_data(text_i,window_size)
    println("convert to 2d array")
    @time train_data = convert_to_2darray(train_data)
    println("filtering training data")
    @time train_data = filter_train_data(train_data)
    #dump_training_data(train_data,unique_words, "./debug-dump.out")
    println("train set size = ",length(train_data))
    #run_word2vec(train_data,length(unique_words),args["emb_len"], args["n_iter"], args["lr"])
end

main()
