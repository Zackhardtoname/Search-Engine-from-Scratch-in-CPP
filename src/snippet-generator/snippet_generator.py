import pandas as pd
import utilities
import classifiers

length = 2
keywords = ["state", "right"]
f = open("US.txt", "r")
const_us = f.read()

d = {'para': const_us.split("\n")}
df = pd.DataFrame(data=d)

topic_classifier = classifiers.Classifier(df, keywords)
paraDF = topic_classifier.classify()

paraDF[keywords] = utilities.scale(paraDF[keywords])
paraDF = paraDF.sort_values(by='topic_score', ascending=False)
sent = classifiers.extract(paraDF, keywords)

for topic in keywords:
    best = sent[sent['topic'] == topic][:length]
    print(best)
