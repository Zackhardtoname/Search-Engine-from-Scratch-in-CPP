import pickle
import gensim.downloader as api

model_gigaword = api.load("glove-wiki-gigaword-100")

print("Downloading... It could take a while")
with open("./w2v_model.pkl", "wb") as f:
    pickle.dump(model_gigaword, f, pickle.HIGHEST_PROTOCOL)
