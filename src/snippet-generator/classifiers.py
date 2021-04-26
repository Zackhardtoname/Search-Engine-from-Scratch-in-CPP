import pickle
import utilities
import pandas as pd


class Classifier:
    """
    A class that classifies articles into different topics and assigns similarity scores.
    """

    def __init__(self, componentDF, topics):
        self.componentDF = componentDF
        self.topics = topics

        with open("w2v_model.pkl", "rb") as f:
            self.model = pickle.load(f)

    def classify(self):
        """
        regular classify method
        Params
        ------
        None
        Returns
        -------
        a dataframe whose content is ranked
        """
        self.componentDF['words'] = self.componentDF['para'].apply(
            lambda x: [w for w in x.lower().split() if w in self.model.key_to_index])
        self.componentDF = self.componentDF[self.componentDF['words'].apply(
            lambda x: x != [])]

        for topic in self.topics:
            self.componentDF[topic] = self.componentDF['words'].apply(lambda x: self.model.n_similarity(x, topic))
            self.componentDF[topic] = utilities.scale(self.componentDF[topic])

        self.componentDF['topic'] = self.componentDF[self.topics].idxmax(axis=1)
        self.componentDF['topic_score'] = self.componentDF[self.topics].max(axis=1)

        del self.componentDF["words"]

        return self.componentDF


# TODO make it a function of the Classifier class to not to reread the w2v model
def summarize(df=None, topics=None,
              num_components=8, min_num_words=5, max_num_words=40):
    """
    ranks and extracts paragraphs so they contain a specific number of words/sentences.
    """
    with open("w2v_model.pkl", "rb") as f:
        model = pickle.load(f)

    df = df[df['para'].apply(lambda x: len(x) >= min_num_words)]
    componentDF = pd.DataFrame()  # represents the final DF for output
    agg = df.groupby('topic')

    for topic in topics:
        top_paras = agg.get_group(topic).nlargest(num_components, 'topic_score')  # get top paras for the topic
        top_paras.reset_index(level=0, inplace=True)
        top_paras.rename(columns={'index': 'para_idx'}, inplace=True)
        top_sentences = utilities.sentence_split(top_paras, 'para', mode='sentence')  # split into sentences
        top_sentences.rename(columns={'para': 'component'}, inplace=True)
        top_sentences['component'] = top_sentences['component'].apply(lambda x: x.strip(' [],，‘’“”。\''))
        top_sentences.drop_duplicates(subset='component', keep='first', inplace=True)
        top_sentences['words'] = top_sentences['component'].apply(
            lambda x: [w for w in x.lower().split() if w in model.key_to_index])
        top_sentences = top_sentences[top_sentences['words'].apply(
            lambda x: x != [])]
        top_sentences['sentence_score'] = top_sentences['words'].apply(
            lambda x: model.n_similarity(x, topic))
        top_sentences['sent_idx'] = range(len(top_sentences))
        top_sentences = top_sentences.sort_values(by='sentence_score', ascending=False)
        agg_sents = top_sentences.groupby('para_idx')

        for _, para in agg_sents:
            s = para['component'].iloc[0]
            i = 1

            while len(para['component']) > i:
                temp = s + para['component'].iloc[i]

                if len(temp) > max_num_words:
                    break

                s = temp
                i += 1

            para = para.head(i)
            para.sort_values(by=['sent_idx'], inplace=True)
            res = para.groupby(['document_idx', 'topic', 'topic_score'])['component'].apply(
                ''.join).reset_index()
            componentDF = componentDF.append(res)

    return componentDF
