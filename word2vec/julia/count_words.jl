using Test

function load_text(filename)
    content = read(filename, String)
    words = split(content)
    words = [replace(w, r"[[:punct:]]" => "") for w in words]
    words
end

function test2()
    filename = "text_long.txt"
    for _ in 1:10
        @time content = read(filename, String)
        @time words = split(content)
        content=0
        @time c = length(Set(words))
        println("------------------------")
        GC.gc()
    end
end

#     .first
#    [x][x][x][][][]
# [ ][x][x][x][][][]
# [x][ ][x][x][][][]
# [x][x][x][][][][]

function count_words(text::String,first::Int,last::Int)
    cnt::Int = 0

    if first == 1
        prev_is_space = true
    else
        prev_is_space = text[first-1] == ' '
    end

    while first <= last
        if prev_is_space
            if text[first] != ' '
                prev_is_space = false
                cnt += 1
            end
        else
            if text[first] == ' '
                prev_is_space = true
            end
        end
        first += 1
    end
    cnt
end

function count_distinct_words(text::String,first::Int,last::Int)
    hash_value::UInt64 = 5381
    dwords = Dict{UInt64,SubString}()
    if first == 1
        prev_is_space = true
    else
        prev_is_space = text[first-1] == ' '
    end
    function add_s(hash_value::UInt64,s::SubString)
        if !haskey(dwords,hash_value)
            dwords[hash_value] = s
            #println("add_s $hash_value '$s'")
        else
            if dwords[hash_value] != s
                cw = dwords[hash_value]
                println("add_s $hash_value s='$s' cw='$cw'")
            end
            @assert dwords[hash_value] == s
        end
    end
    word_start_idx = first
    isgood = c -> isletter(c) || c == '\''
    while first <= last
        char = text[first]
        if prev_is_space
            if isgood(char)
                prev_is_space = false
                hash_value = (hash_value * 33) + Int(char)
                word_start_idx = first
            end
        else
            if !isgood(char)
                prev_is_space = true
                add_s(hash_value,SubString(text,word_start_idx,first-1))
                hash_value = 5381
            else
                hash_value = (hash_value * 33) + Int(char)
            end
        end
        first += 1
    end
    if !prev_is_space
        L = length(text)
        while first <= L && isgood(text[first])
            hash_value = (hash_value * 33) + Int(text[first])
            first += 1 
        end
        add_s(hash_value,SubString(text,word_start_idx,first-1))
    end
    #length(dwords)
    values(dwords)
end

function test_count_words()
    text = "to jest  testowy    string ala ma kota"
    @test count_words(text,1,3)  == 1
    @test count_words(text,1,4)  == 2
    @test count_words(text,1,11) == 3
    @test count_words(text,2,11) == 2
    @test count_words(text,2,4)  == 1
    @test count_words(text,3,11) == 2
    @test count_words(text,17,length(text)) == 4
end

function test_split()
    N = Threads.nthreads()
    dn = Float32(1000) / N
    ranges = [(floor(Int,dn*i)+1, floor(Int,dn*(i+1))) for i in 0:N-1]
    for (idx,r) in enumerate(ranges)
        println(idx,':',r)
    end
end

function count_words_mt(content,N)
    dn = Float32(length(content)) / N
    ranges = [(floor(Int,dn*i)+1, floor(Int,dn*(i+1))) for i in 0:N-1]
    tasks = [Threads.@spawn count_words(content, a,b) for (a,b) in ranges]
    # Fetch results from all threads
    results = fetch.(tasks)
    sum(results)
end

function test3()
    filename = "text_long.txt"
    println("read file")
    @time content = read(filename, String)
    println("count by length(split)")
    @time n_words = length(split(content))
    N = Threads.nthreads()
    println("count by mt")
    @time n_words_mt = count_words_mt(content,N)
    @test n_words == n_words_mt
end

function test4()
    filename = "text_short.txt"
    println("read file")
    @time content = read(filename, String)
    println("count by length(split)")
    @time n_words = length(Set(split(content)))
    println("count by content |> split |> Set |> length")
    @time n_words_1 = content |> split |> Set |> length
    @assert n_words == n_words_1
    println("count by count_distinct_words")
    @time n_words_2 = count_distinct_words(content,1,length(content))
    println("n_words = $n_words, n_words_2=$n_words_2")
    @assert n_words == n_words_2
end

#run julia -t auto count_words.jl
@show Threads.nthreads()
#test_count_words()
#test_split()
#test3()

#n = count_distinct_words(text,1,length(text))
#@show n
#test4()

filename = "text_long.txt"
println("read file")
content = read(filename, String)
@time s = content |> (X -> split(X, r"[ ,.]+"; keepempty=false)) |> Set
@time s2 = Set(split(content, r"[ ,.]+"; keepempty=false))
@time s1 = count_distinct_words(content,1,length(content))
d = setdiff(s,s1)
println("diff=$d len=$(length(d))")
