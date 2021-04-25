# Ignore all the pandas warnings as they are not dangerous for us

import pandas as pd
import utilities
import classifiers
import requests
import sys
from bs4 import BeautifulSoup

url = "\n".join(sys.argv[1:])
response = requests.get(url)
document = BeautifulSoup(response.content, "lxml").text

documents = [document]

length = 1
keywords = ["covid", "vaccine"]
document_titles = ["cnn_news_covid.txt", "cnn_news_covid_2.txt"]
# documents = [open(title, "r").read() for title in document_titles]

d = {'content': documents}
df = pd.DataFrame(data=d)

df = utilities.extract_para(df)
topic_classifier = classifiers.Classifier(df, keywords)
paraDF = topic_classifier.classify()

paraDF[keywords] = utilities.scale(paraDF[keywords])
paraDF = paraDF.sort_values(by='topic_score', ascending=False)
sent = classifiers.summarize(paraDF, keywords)

for document_idx in range(len(documents)):
    best = sent[sent['document_idx'] == document_idx][:length]
    # print(f"snippet for {document_titles[document_idx]}:")
    print(". ".join(best["component"]))
