# coding=utf-8


import torch
from torch.utils.data import Dataset
import numpy as np

from tqdm import tqdm


class MSRDataSet(Dataset):
    def __init__(self):
        self.word2idx = {}
        self.idx2word = {}
        self.sentence = []
        self.read_file("./dataset/split1.txt")
        self.read_file("./dataset/split2.txt")
        # self.read_file("./dataset/tiny.txt")
        # 用词典表示词与唯一编号之间的双射
        self.word2idx['EOS'] = 0
        self.idx2word[0] = 'EOS'
        self.word2idx['pad'] = 1
        self.idx2word[1] = 'pad'
        cnt = 2
        for s in tqdm(self.sentence):
            for w in s:
                if w not in self.word2idx:
                    self.word2idx[w] = cnt
                    self.idx2word[cnt] = w
                    cnt += 1
        # 将句子编号化
        for s in tqdm(self.sentence):
            for i in range(len(s)):
                s[i] = self.word2idx[s[i]]
            s.append(0)  # EOS
        # 根据长度进行排序
        self.sortidx = np.argsort([len(s) for s in self.sentence])

    def read_file(self, path):
        with open(path,  'r', encoding='UTF-8') as f:
            for line in tqdm(f.readlines()):
                l = line.split()
                if len(l) < 2:  # 去掉单词
                    continue
                self.sentence.append(l)

    def idx2seq(self, idx):
        res = ""
        for i in idx:
            res += self.idx2word[i] + " "
        return res

    def __getitem__(self, index):
        return self.sentence[self.sortidx[index]]
        # torch.nn.functional.one_hot(self.sentence[index], num_classes=len(self.word2idx))

    def __len__(self):
        return len(self.sentence)


def collate_fn(batch):
    """通过padding实现变长batch 返回padding的batch"""
    lens = torch.LongTensor([len(dat) for dat in batch])
    idx_batch = [torch.LongTensor(d) for d in batch]
    # 对batch进行padding 即将长度不足的序列用0填充到与batch内最长序列等长
    idx_batch = torch.nn.utils.rnn.pad_sequence(idx_batch, batch_first=True, padding_value=1)
    return idx_batch, lens

if __name__ == "__main__":
    training_set = MSRDataSet()
    print(len(training_set.word2idx))
    print(training_set.idx2seq(training_set[0]))