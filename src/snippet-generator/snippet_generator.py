import pandas as pd
import utilities
import classifiers

length = 2
keywords = ["state", "right"]
f = open("US.txt", "r")
const_us = f.read()

f = open("japan.txt", "r")
const_japan = f.read()
documents = [const_us, const_japan]
d = {'content': documents}
# d = {'para': const_us.split("\n")}
df = pd.DataFrame(data=d)

df = utilities.extract_para(df)
topic_classifier = classifiers.Classifier(df, keywords)
paraDF = topic_classifier.classify()

paraDF[keywords] = utilities.scale(paraDF[keywords])
paraDF = paraDF.sort_values(by='topic_score', ascending=False)
sent = classifiers.extract(paraDF, keywords)

for document_idx in range(len(documents)):
    best = sent[sent['document_idx'] == document_idx][:length]
    print(best)
