using ArgParse,Random

function load_text(filename,stopwords)
    content = read(filename, String)
    words = split(content)
    words = [replace(w, r"[[:punct:]]" => "") for w in words]
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
    stopwords = readlines("./stopwords.txt")
    text = load_text("./text_small.txt",stopwords)
    println("text size = ", length(text))
    window_size = 3
    n_negative_samples = 3
    train_data = prepare_train_data(text,window_size,n_negative_samples)
    println("train set size = ",length(train_data))
end

main()