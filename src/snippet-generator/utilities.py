import json
import pandas as pd
import numpy as np
import re


class NumpyEncoder(json.JSONEncoder):
    """ Special json encoder for numpy types """

    def default(self, obj):
        if isinstance(obj, (np.int_, np.intc, np.intp, np.int8,
                            np.int16, np.int32, np.int64, np.uint8,
                            np.uint16, np.uint32, np.uint64)):
            return int(obj)
        elif isinstance(obj, (np.float_, np.float16, np.float32,
                              np.float64)):
            return float(obj)
        elif isinstance(obj, (np.ndarray,)):  #### This is the fix
            return obj.tolist()
        return json.JSONEncoder.default(self, obj)


def is_number(s):
    try:
        float(s)
        return True
    except ValueError:
        return False


def scale(series):
    return (series - series.min()) / (series.max() - series.min()) * 100


def explode(df, lst_cols, fill_value=''):
    # make sure `lst_cols` is a list
    # if lst_cols and not isinstance(lst_cols, list):
    lst_cols = [lst_cols]
    # all columns except `lst_cols`
    idx_cols = df.columns.difference(lst_cols)

    # calculate lengths of lists
    lens = df[lst_cols[0]].str.len()

    if (lens > 0).all():
        # ALL lists in cells aren't empty
        return pd.DataFrame({
            col: np.repeat(df[col].values, lens)
            for col in idx_cols
        }).assign(**{col: np.concatenate(df[col].values) for col in lst_cols}) \
                   .loc[:, df.columns]
    else:
        # at least one list in cells is empty
        return pd.DataFrame({
            col: np.repeat(df[col].values, lens)
            for col in idx_cols
        }).assign(**{col: np.concatenate(df[col].values) for col in lst_cols}) \
                   .append(df.loc[lens == 0, idx_cols]).fillna(fill_value) \
                   .loc[:, df.columns]


def sentence_split(df, column, keep=False, mode='para'):
    """
    Split the values of a column and expand so the new DataFrame has one split
    value per row. Filters rows where the column is missing.

    Params
    ------
    df : pandas.DataFrame
        dataframe with the column to split and expand
    column : str
        the column to split and expand
    sep : str
        the string used to split the column's values
    keep : bool
        whether to retain the presplit value as it's own row

    Returns
    -------
    pandas.DataFrame
        Returns a dataframe with the same columns as `df`.
    """
    indexes = list()
    new_values = list()
    df = df.dropna(subset=[column])

    if mode == 'sentence':  # in essense it takes a list
        for i, presplit in enumerate(df[column].astype(str)):
            values = re.split('.。|？|\?', presplit)

            if keep and len(values) > 1:
                indexes.append(i)
                new_values.append(presplit)

            for value in values:
                indexes.append(i)
                new_values.append(value)
    else:  # in essense it takes a list
        for i, values in enumerate(df[column]):
            for value in values:
                indexes.append(i)
                new_values.append(value)

    new_df = df.iloc[indexes, :].copy()
    new_df[column] = new_values

    return new_df


def extract_para(df):
    """
    Converts a dataframe whose rows are pieces of documents to a new dataframe whose rows are paragraphs from documents
    """
    indexes = []
    new_values = []

    for i, text in enumerate(df['content']):
        split = text.split("\n")
        new_values += split
        indexes += [i] * len(split)

    new_df = df.iloc[indexes, :].copy()
    new_df['para'] = new_values
    del new_df['content']
    new_df.reset_index(level=0, inplace=True)  # make the old indeices a column
    new_df.rename(columns={'index': 'document_idx'}, inplace=True)

    return new_df
