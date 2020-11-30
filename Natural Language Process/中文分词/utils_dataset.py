# coding=utf-8

from utils_args import args
from utils_other import line2label, line2char, collate_fn
from utils_model import train_w2v

import os

import torch
from torch.utils.data import Dataset, DataLoader

import gensim
from tqdm import tqdm


class MSRDataSet(Dataset):
    def __init__(self, data_path, w2idx):
        self.label_lines = []  # 所有标签序列
        self.idx_lines = []  # 所有对应的字编号序列
        self.preprocess(data_path, w2idx)  # 预处理 从文件中生成上述序列

    def preprocess(self, data_path, w2idx):
        """对数据进行预处理"""
        char_lines = []
        with open(data_path, "r", encoding="utf-8") as f:
            for line in tqdm(f.readlines()):
                line = line.split()
                if len(line):  # 跳过空句子
                    self.label_lines.append(line2label(line))
                    char_lines.append(line2char(line))
        for line in tqdm(char_lines):
            self.idx_lines.append([w2idx[ch] for ch in line])

    def __getitem__(self, index):
        return self.idx_lines[index], self.label_lines[index]

    def __len__(self):
        return len(self.idx_lines)


