using ArgParse,Random

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
            default = 2
        "--nneg"
            help = "nubmer of negative samples"
            arg_type = Int
            default = 3
        "--emb_len"
            help = "size of output embedings"
            arg_type = Int
            default = 32
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
    content = read(filename,String)
    words = split(content)
    words = [replace(w,r"[[:punct:]]" => "") for w in words]
    #setdiff(words,stopwords) nice but discards word order
    [w for w in words if w ∉ stopwords]
end

struct Record
    center_word_idx::Int32
    context_word_idx::Int32
    label::Int32
end

function prepare_train_data(text,window_size,n_negative_samples)
    train_data = Record[]
    N = length(text)
    for idx in window_size+1:N-window_size
        context = String[]
        for sub_idx in -window_size:window_size
            if sub_idx != 0
                widx = idx + sub_idx
                push!(train_data, Record(idx,widx,1))
                push!(context,text[widx])
            end
        end
        negative_samples = Set{Int32}()
        while length(negative_samples) < n_negative_samples
            i = rand(1:N)
            if text[i] ∉ context
                push!(negative_samples,i)
            end
        end
        for ni in negative_samples
            push!(train_data, Record(idx,ni,0) )
        end
    end
    train_data
end

function main()
    args = parseCommandLine()
    stopwords = load_stopwords(args["stopwords"])
    println( "stopwords=", join(stopwords[1:10],",") )
    words = load_text(args["input"],stopwords)
    println("words=",words[1:10])

    window_size = 3
    n_negative_samples = 3
    train_data = prepare_train_data(text,window_size,n_negative_samples)
    println("train set size = ",length(train_data))
end

main()
