from utils_args import args
from utils_other import line2char, log_sum_exp

import torch
import torch.nn as nn
import gensim
import numpy as np

import os


def train_w2v():
    """训练字向量模型"""
    sentences = []
    with open("./dataset/msr_training.txt", "r", encoding="utf-8") as f:
        for line in f.readlines():
            sentences.append(line2char(line.split()))
    with open("./dataset/msr_test.txt", "r", encoding="utf-8") as f:  # 没有用到测试集的分词信息
        for line in f.readlines():
            sentences.append(line2char(line.split()))
    size = args.char_vector_size
    if not args.pretrain:
        # 不使用预训练词向量
        target = "./pretrain/char2vec" + str(size) + ".txt"
        if not os.path.isfile(target):
            w2v_model = gensim.models.Word2Vec(size=size, window=3, sg=2,
                                               min_count=1, workers=4)  # 每个字都要有字向量 所以最低词频为1
            # 使用训练集训练字向量
            w2v_model.build_vocab(sentences)
            w2v_model.wv.save_word2vec_format(target, binary=False)
            del w2v_model
        return gensim.models.KeyedVectors.load_word2vec_format(target, binary=False)
    else:
        # 使用预训练词向量 300维
        pretrain_source = "./pretrain/" + args.pretrain
        pretrain_target = "./pretrain/char2vec_" + args.pretrain + ".txt"
        if not os.path.isfile(pretrain_target):
            w2v_model = gensim.models.Word2Vec(size=300, window=3, sg=2, min_count=1, workers=4)
            # 使用训练集训练字向量（为了建立字典 即知道需要哪些字的词向量）
            w2v_model.build_vocab(sentences)
            # 与预训练字向量合并（直接使用预训练词向量 没有在当前语料中再次训练）
            w2v_model.intersect_word2vec_format(pretrain_source, binary=False)
            # 保存模型 便于后续使用
            w2v_model.wv.save_word2vec_format(pretrain_target, binary=False)
            print("Model saved.")
            del w2v_model
        return gensim.models.KeyedVectors.load_word2vec_format(pretrain_target, binary=False)


def train_embedding(w2v):
    # 给每个字一个不同的编号
    w2idx = {}
    for i, ch in enumerate(w2v.vocab):
        w2idx[ch] = i + 1  # 编号从1开始 0 作为pad index
    # 使用预训练词向量模型为BiLSTM_CRF的embedding层构建预训练参数
    pretrain_embedding = torch.zeros((len(w2idx) + 1, args.char_vector_size))
    for i, ch in enumerate(w2v.vocab):
        pretrain_embedding[i + 1, :] = torch.tensor(np.require(w2v[ch], requirements=['W']))
    return w2idx, pretrain_embedding


class BiLSTM_CRF(nn.Module):
    def __init__(self, device, pretrain_embedding):
        super(BiLSTM_CRF, self).__init__()
        self.vector_dim = args.char_vector_size  # 词向量维数
        self.hidden_dim = args.char_vector_size  # LSTM隐藏层总维数（两个方向）
        self.label_dim = 4  # 标签个数
        self.device = device  # CPU或GPU
        # 是否使用预训练词向量
        if args.pretrain:
            self.embedding = nn.Embedding.from_pretrained(pretrain_embedding,
                                                          freeze=args.fix_embedding,  # 是否固定词向量
                                                          padding_idx=0)
        else:
            self.embedding = nn.Embedding(pretrain_embedding.shape[0],
                                          pretrain_embedding.shape[1],
                                          padding_idx=0)
        # 词嵌入--LSTM--CRF
        self.lstm = nn.LSTM(self.vector_dim,
                            self.hidden_dim // 2,
                            num_layers=1,
                            bidirectional=True,
                            batch_first=True)
        self.hidden2label = nn.Linear(self.hidden_dim, self.label_dim)
        self.transitions = nn.Parameter(torch.randn(self.label_dim, self.label_dim))
        self.transi_from_BOS = nn.Parameter(torch.randn(self.label_dim))
        self.transi_to_EOS = nn.Parameter(torch.randn(self.label_dim))

    def pad_lstm_pad(self, pad_seq, lens):
        """从padded的batch获得padded的LSTM和hidden2label输出的发射分数batch"""
        features = self.embedding(pad_seq)  # embedding
        packed_seq = torch.nn.utils.rnn.pack_padded_sequence(features,
                                                             lens,
                                                             batch_first=True,
                                                             enforce_sorted=False)  # pack
        pack_lstm_out, _ = self.lstm(packed_seq)  # BiLSTM
        pad_lstm_out, _ = torch.nn.utils.rnn.pad_packed_sequence(pack_lstm_out, batch_first=True)  # pad
        pad_scores = self.hidden2label(pad_lstm_out)  # 发射分数 (b,len,n)
        return pad_scores

    def criterion(self, pad_seq, pad_target, lens):
        """负对数似然 loss 真实labels在模型中的预测概率相对其他预测的越大 loss越低"""
        # 获得lstm的输出（整个seq） 即序列中每个字所对应的每个label的可能性
        pad_scores = self.pad_lstm_pad(pad_seq, lens)
        batch_dim, lens_dim, _ = pad_scores.shape
        # lens的bool矩阵 (b,len) 表示pad_scores中哪些不是pad值
        len_bool = (torch.tensor(range(lens_dim)).view(1, -1) < lens.view(-1, 1)).to(self.device)
        len_1_bool = (torch.tensor(range(lens_dim)).view(1, -1) < (lens - 1).view(-1, 1)).to(self.device)
        # 计算目标labels在当前模型下的预测概率
        target_score = self.transi_from_BOS[pad_target[:, 0]]  # <BOS> 二元特征 (b,)
        target_score += torch.sum(self.transitions[pad_target[:, :-1], pad_target[:, 1:]]
                                  * len_1_bool[:, :-1], dim=1)  # 二元特征 (b, )
        target_score += torch.sum(pad_scores.view(-1, self.label_dim)[range(batch_dim * lens_dim),
                            pad_target.flatten()].view(batch_dim, lens_dim) * len_bool, dim=1)  # 一元特征
        target_score += self.transi_to_EOS[pad_target[range(batch_dim), (lens-1).flatten()]]  # <EOS> 二元特征
        # 计算所有预测的总分 （注意维度）
        # 从<BOS>到不同状态 (b,1,n)相当于(b,n,n)中的一行
        all_score = log_sum_exp(self.transi_from_BOS.view(1, 1, -1).repeat(batch_dim, 1, 1)
                                + pad_scores[:, 0, :].unsqueeze(1))
        for i in range(1, lens_dim):
            # (b,n,1)+(b,1,n)+(1,n,n) = (b,n,n) = (b,1,n) b中len大于i的 继续计算 len小于等于i的 保持不变
            all_score[lens > i] = log_sum_exp(all_score.permute(0, 2, 1) + pad_scores[:, i, :].unsqueeze(1)
                                              + self.transitions.unsqueeze(0))[lens > i]
        # 从不同状态到<EOS> (b,n,1) （相当于(b,n,n)中的一列) (b,n,1) (b,1,1)
        all_score = log_sum_exp(all_score.permute(0, 2, 1)
                                + self.transi_to_EOS.view(1, -1, 1)).flatten(start_dim=0)
        return torch.mean(all_score - target_score)  # (b)

    def forward(self, pad_seq, lens):
        """用模型来预测标签 取最大预测概率者"""
        pad_scores = self.pad_lstm_pad(pad_seq, lens)
        batch_dim, lens_dim, _ = pad_scores.shape
        # 使用动态规划 记录到达当前时刻每个状态的最佳分数和路径
        # backtrace：记录每个时刻到达每个状态的最高分路径的上一个状态是什么
        backtrace = torch.zeros((lens_dim - 1, batch_dim, self.label_dim)).long().to(self.device)  # (len,b,n)
        # best_score：记录到达当前时刻的每个状态的最高分
        best_score = (self.transi_from_BOS.view(1, 1, -1).repeat(batch_dim, 1, 1)
                      + pad_scores[:, 0, :].unsqueeze(1))  # (b,1,n)
        # smat：到达从每一状态到达下一时刻的每一状态的分数 用于计算这一步的best_score和backtrace
        smat = torch.zeros((batch_dim, self.label_dim, self.label_dim)).double().to(self.device)
        for i in range(1, lens_dim):
            # (b,n转移前,n转移后) 对每一个转移后 找一个最好的转移前
            smat[lens > i] = ((best_score.permute(0, 2, 1) + pad_scores[:, i, :].unsqueeze(1)
                               + self.transitions.unsqueeze(0))[lens > i])
            best_score, idx = smat.max(dim=1, keepdim=True)  # (b,1,n) (b,n)
            backtrace[i - 1] = idx.squeeze(1)
        best_score = best_score.permute(0, 2, 1) + self.transi_to_EOS.view(1, -1, 1)  # (b,n,1)
        # 从最后分数最高的状态开始根据backtrace记录的最好转移前状态倒推 得到所有路径中最高分数的一条路径
        best_labels = torch.zeros((batch_dim, lens_dim)).long().to(self.device)  # (b,len)
        best_labels[range(batch_dim), lens - 1] = best_score.argmax(dim=1).flatten().data
        for i in range(lens_dim - 1):  # (len-2)
            best_labels[range(batch_dim),
                        lens - 2 - i] = backtrace[lens - 2 - i,
                                                  range(batch_dim),
                                                  best_labels[range(batch_dim), lens - 1 - i].flatten()].data
        len_bool = (torch.tensor(range(lens_dim)).view(1, -1) < lens.view(-1, 1)).to(self.device)
        # 返回最优路径
        return best_labels * len_bool
