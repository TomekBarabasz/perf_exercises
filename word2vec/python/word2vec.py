import numpy as np
import string
from tqdm import tqdm
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
    ids_to_words = list(set(text))
    n_words = len(ids_to_words)
    word_ids = {ids_to_words[i]:i for i in range(n_words)}
    text_as_ids = [word_ids[w] for w in text]
    return text_as_ids, ids_to_words

def prepare_train_data(Text, window_size, n_neg_samples, verbose = False):
    # windows_size = 3
    # [0][1][2][3][4][5][6][7]                  [90][91][92][93][94][95][96][97][98][99] [len=100]
    #           | first center word                                      | last center word   
    train_data = []
    n_neg_samples = 2*window_size  #same number of pos and neg samples

    center_words = set()
    context_words = set()
    for center_idx in tqdm(range(window_size, len(Text)-window_size)):
        w = Text[center_idx]
        center_words.add(w)
        ctx_words = [Text[ci] for ci in range(center_idx-window_size, center_idx+window_size+1) if ci != center_idx and Text[ci] != w]        
        context_words.update(ctx_words)
        train_data.extend( [(w,cw,1) for cw in ctx_words] )
        neg_words = []
        while len(neg_words) < n_neg_samples:
            i = np.random.randint(0,len(Text))
            nw = Text[i]
            if nw not in neg_words and nw not in ctx_words and nw != w:
                neg_words.append(nw)
        if verbose:
            print(f'{w} : ctx {ctx_words} neg {neg_words}')
        train_data.extend( [(w,nw,0) for nw in neg_words] )
    common_words = center_words.intersection(context_words)
    len_0 = len(train_data)
    train_data = [(w,cw,label) for (w,cw,label) in train_data if w in common_words and cw in common_words]
    if verbose:
        len_1 = len(train_data)
        dropped = center_words.difference(common_words)
        dropped_ctx = context_words.difference(common_words)
        print(f'dropped {len_0-len_1} rows from train data : center words {dropped} context_words {dropped_ctx}')
    return train_data

def write_debug_output(filename,text,train_data):
    with open(filename,'w') as file:
        file.write(','.join(text))
        file.write('\n\n\n')
        for i,r in enumerate(train_data):
            file.write(f'{i:3d}: {r}\n')

def write_debug_output_i(filename,text,train_data,wdict):
    with open(filename,'w') as file:
        file.write(f'{wdict}' + '\n')
        file.write(','.join([wdict[i] for i in text]))
        file.write('\n\n\n')
        for i,r in enumerate(train_data):
            file.write(f'{i:3d}: ({wdict[r[0]]},{wdict[r[1]]},{r[2]})\n')

def normalize(emb):
    emb /= np.linalg.norm(emb, axis=1, keepdims=True)

def update_embeddings(main_emb, ctx_emb, train_data, learning_rate):
    e = main_emb[ train_data[:,0] ]
    c =  ctx_emb[ train_data[:,1] ]
    dotp = np.sum( e*c, axis=1 )
    def sigmoid(v,scale=1):
        return 1 / (1+np.exp(-scale*v))
    scores = sigmoid(dotp)
    errors = train_data[:,2] - scores
    # updates one for each row of train_data
    updates = (c-e) * errors.reshape(-1,1) * learning_rate 

    # apply accumulated updates to main word embeddngs
    np.add.at(main_emb,train_data[:,0],updates)
    # apply accumulated updates to context word embeddngs
    np.add.at(ctx_emb,train_data[:,1],updates)

    normalize(main_emb)
    normalize(ctx_emb)

def run_word2vec(train_data, n_words, embedding_size, n_iter, learning_rate):
    main_emb = np.random.randn(n_words,embedding_size)
    normalize(main_emb)
    # main_emb = normalize(main_emb, norm='l2') # from sklearn.preprocessing import normalize

    ctx_emb = np.random.randn(n_words,embedding_size)
    normalize(ctx_emb)
    # ctx_emb = normalize(ctx_emb, norm='l2') # from sklearn.preprocessing import normalize

    for _ in tqdm(range(n_iter)):
        update_embeddings(main_emb, ctx_emb, train_data, learning_rate)
    
    return main_emb

def plot_embeddings(emb,wdict,plot_dim):
    if emb.shape[1] > 2:
        from sklearn.decomposition import PCA
        pca = PCA(n_components=plot_dim)
        embxd = pca.fit_transform(emb)
    else:
        embxd = emb
    
    f = plt.figure()
    ax = f.add_subplot(111, projection='3d' if plot_dim==3 else None) 
    if plot_dim == 2:
        ax.scatter( embxd[:,0], embxd[:,1] )
        ax.scatter( 0,0,5,'red' )
    else:
        ax.scatter( embxd[:,0], embxd[:,1], embxd[:,2] )
        ax.scatter( 0,0,0,s=5,c='red' )
    
    for name,e in zip(wdict,embxd):
        ax.text(*e,name)
    plt.show()
    
def main(Args):
    stopwords = load_stopwords(Args.stopwords) if Args.stopwords is not None else []
    t0 = datetime.now()
    text = read_train_text(Args.input,stopwords)
    print(f"loaded {len(text)} words in {datetime.now()-t0}")
    if 0:
        t0 = datetime.now()
        train_data = prepare_train_data(text, Args.window, Args.neg, True)
        print(f"prepared {len(train_data)} training samples in {datetime.now()-t0}")
        write_debug_output('./debug.out',text,train_data)

    itext,wdict = text_2_indices(text)
    print(f"found {len(wdict)} distinct words")
    t0 = datetime.now()
    train_data_i = prepare_train_data(itext, Args.window, Args.neg)
    print(f"prepared {len(train_data_i)} training samples in {datetime.now()-t0}")
    # write_debug_output_i('./debug_i.out',itext,train_data_i,wdict)
    t0 = datetime.now()
    emb = run_word2vec(np.array(train_data_i),len(wdict),Args.emb, Args.iter, Args.lr)
    print(f"done {Args.iter} training iterations in {datetime.now()-t0}")

    if Args.plot != 0:
        plot_embeddings(emb,wdict, Args.plot)
    else:
        print('learned embeddings')
        for i,e in enumerate(emb):
            s = ','.join([f'{x:.3f}' for x in e])
            print( f'{wdict[i]:15} [{s}]')

def test(Args):
    def plot_embedding_vectors():
        from mpl_toolkits.mplot3d import Axes3D
        import matplotlib
        matplotlib.use('Qt5Agg')
        main_emb = np.random.normal( 0, 0.1, (10,3))
        #row_norms = np.sqrt( (main_emb**2).sum(axis=1) ).reshape(-1,1)
        main_emb /= np.linalg.norm(main_emb, axis=1, keepdims=True)
        fig = plt.figure()
        ax = fig.add_subplot(111, projection='3d')
        origin = np.zeros(main_emb.shape)
        x_origin, y_origin, z_origin = origin[:, 0], origin[:, 1], origin[:, 2]

        # Extract u, v, w components (vector components)
        u, v, w = main_emb[:, 0], main_emb[:, 1], main_emb[:, 2]

        # Plot the vectors using quiver
        ax.quiver(x_origin, y_origin, z_origin, u, v, w)
        max_val = np.max(np.abs(main_emb)) * 1.2  # Add some padding for better visualization
        ax.set_xlim([-max_val, max_val])
        ax.set_ylim([-max_val, max_val])
        ax.set_zlim([-max_val, max_val])
        plt.show()

    plot_embedding_vectors()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="word2vec app")
    parser.add_argument("input", type = str, help="input file")
    parser.add_argument("--stopwords", "-s",type = str, help="stopwords file")
    parser.add_argument("--window","-w",    type = int, default=3, help="window[context] size")
    parser.add_argument("--neg","-n",       type = int, default=3, help="number of negative sampls")
    parser.add_argument("--emb","-e",       type = int, default=32, help="embeddings size[dimension]")
    parser.add_argument("--iter","-i",      type = int, default=25, help="number of iterations")
    parser.add_argument("--lr","-l",        type = float, default=0.1, help="learning rate")
    parser.add_argument("--test","-t",      type = int, help="test number to run")
    parser.add_argument("--plot","-p",      type = int, choices=[0,2,3], default=0, help="plot final embeddings")
    Args = parser.parse_args()
    if Args.test is None:
        main(Args)
    else:
        test(Args)
