import random,argparse
from datetime import datetime
from shutil import copyfile
from numpy.random import normal

def load_data(filename):
    data = {}
    cnt_rows = 0
    with open(Args.input,'r') as input:
        for line in input:
            if line[0]=='#': continue
            name,svalue = line.split(';')
            fvalue = float(svalue)
            if name not in data:
                data[name] = (1,fvalue,fvalue,fvalue)
            else:
                cnt,fsum,fmin,fmax = data[name]
                data[name] = (cnt+1,fsum+fvalue,min(fmin,fvalue),max(fmax,fvalue))
            cnt_rows += 1
    return data,cnt_rows

def main(Args):
    t0 = datetime.now()
    db,cnt_rows = load_data(Args.input)
    t1 = datetime.now()
    print(f'loaded {cnt_rows} in {t1-t0}')
    
    if Args.g:
        rows_todo = Args.rows - len(db)
        if rows_todo < 0:
            print(f'input file already has {cnt_rows} rows')
            return
        copyfile(Args.input, Args.o)
        keys = list(db.keys())
        out = open(Args.o,'a')
        for _ in range(rows_todo):
            k = random.choice(keys)
            f,mi,ma = db[k]
            f1 = f + normal(f,Args.dev)
            out.write(f'{k};{f1}\n')

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="1brc app")
    parser.add_argument("input", type = str, help="input file")
    parser.add_argument("-o",    type = str, help="output file")
    parser.add_argument("-rows", type = int, help="total rows to create")
    parser.add_argument("-dev",  type = float, help="variation to add")
    parser.add_argument("-g",    action='store_true', help="input file generation")
    Args = parser.parse_args()

    main(Args)
