using ArgParse

function parseCommandLine()
    s = ArgParseSettings()
    @add_arg_table! s begin
        "file"
            help = "filename to parse"
            arg_type = String
            required = true
        "output"
            help = "summary filename"
            arg_type = String
    end
    return parse_args(s)
end

struct Record
    sum::Float32
    cnt::Int32
    vmin::Float32
    vmax::Float32
end

function loadMeasurements(filename::String)
    db = Dict()
    open(filename,"r") do file
        for line in eachline(file)
            if line[1] == '#' continue end
            name,svalue = split(line,";")
            value = parse(Float32,svalue)
            update::Bool = true
            rec = get!(db,name) do 
                update = false
                Record(value,1,value,value)
            end

            if update
                sum = rec.sum + value
                cnt = rec.cnt + 1
                vmin = min(rec.vmin,value)
                vmax = max(rec.vmax,value)
                db[name] = Record(sum,cnt,vmin,vmax)
            end
        end
    end
    db
end

function summarize(db, output_fn)
    for (name,record) in pairs(db)
        average = record.sum / record.cnt
        output_fn("$name,$average")
    end
end

function main()
    cmd_args = parseCommandLine()
    file_path = cmd_args["file"]
    output_path = cmd_args["output"]
    @time db = loadMeasurements(file_path)
    if output_path === nothing
        summarize(db,line -> println(line))
    else
        open(output_path,"w") do file
            summarize(db,line -> write(file,line*'\n'))
        end
    end
end

main()
