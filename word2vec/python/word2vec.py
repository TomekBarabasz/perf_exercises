import numpy as np
import pandas as pd
import string
from tqdm import tqdm
from scipy.spatial.distance import cosine
import matplotlib.pyplot as plt
import argparse
from datetime import datetime

def load_stopwords(filename):
    with open(filename,'r') as file:
        return [w for w in file.read().split('\n') if w]

def read_train_text(filename,stopwords):
    with open(filename, encoding='utf-8') as file:
        text = file.read()
    translation_table = str.maketrans('', '',   string.punctuation + 
                                                string.digits +
                                                '”'+'“'+'’'+'\''+'\n')
    text = text.translate(translation_table).lower().split()
    return [w for w in text if w not in stopwords]

def text_2_indices(text):
    word_ids = {w:i for i,w in enumerate(set(text))}
    text_as_ids = [word_ids[w] for w in text]
    return text_as_ids, word_ids

def prepare_train_data(Words, window_size, n_neg_samples):
    # windows_size = 3
    # [0][1][2][3][4][5][6][7]                  [90][91][92][93][94][95][96][97][98][99] [len=100]
    #           | first center word                                      | last center word

    word_dict, iWords = text_2_indices(Words)
    
    train_data = []
    n_neg_samples = 2*window_size  #same number of pos and neg samples
    for center_idx in range(window_size, len(Words)-window_size):
        w = Words[center_idx]
        ctx_words = [Words[ci] for ci in range(center_idx-window_size, center_idx+window_size+1) if ci != center_idx and Words[ci] != w]
        train_data.extend( [(w,cw,1) for cw in ctx_words] )
        neg_words = []
        while len(neg_words) < n_neg_samples:
            i = np.random.randint(0,len(Words))
            nw = Words[i]
            if nw not in neg_words and nw not in ctx_words:
                neg_words.append(nw)
        train_data.extend( [(w,nw,0) for nw in neg_words] )
    return train_data

def write_debug_output(filename,text,train_data):
    with open(filename,'w') as file:
        file.write(','.join(text))
        file.write('\n\n\n')
        for i,r in enumerate(train_data):
            file.write(f'{i:3d}: {r}\n')

def constrain_words(train_data):
    df = pd.DataFrame(columns=['center_word','context_word','label'],data=train_data)

    print('center_words')
    words = set(df.center_word)
    print( words )
    print('context_words')
    cwords = set(df.context_word)
    print( cwords )

    common_words = np.intersect1d(df.context_word, df.center_word)
    all_words = words.union(cwords)
    print('word,is_center,is_context')
    for w in all_words:
        f1 = w in words
        f2 = w in cwords
        f3 = w in common_words
        print(f'{w:15} {f1:5} {f2:5} {f3:5}')

    words = common_words

    df = df[(df.center_word.isin(words)) & (df.context_word.isin(words))].reset_index(drop=True)
    print(df)

def main(Args):
    stopwords = load_stopwords(Args.stopwords) if Args.stopwords is not None else []
    t0 = datetime.now()
    text = read_train_text(Args.input,stopwords)
    print(f"loaded {len(text)} words in {datetime.now()-t0}")
    print( text[:10] )
    t0 = datetime.now()
    train_data = prepare_train_data(text, Args.window, Args.neg)
    print(f"prepared {len(train_data)} training samples in {datetime.now()-t0}")

    write_debug_output('./debug.out',text,train_data)

    constrain_words(train_data)
    

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="word2vec app")
    parser.add_argument("input", type = str, help="input file")
    parser.add_argument("--stopwords", "-s",type = str, help="stopwords file")
    parser.add_argument("--window","-w",    type = int, default=3, help="window[context] size")
    parser.add_argument("--neg","-n",       type = int, default=3, help="number of negative sampls")
    parser.add_argument("--emb","-e",       type = int, default=32, help="embeddings size[dimension]")
    Args = parser.parse_args()
    main(Args)
