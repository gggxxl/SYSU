import torch


def line2label(l):
    """根据分词给每一个字打上标志 0:B（开头）、1:M（中间）、2:E（结束）或3:S（单独）"""
    labels = []
    for word in l:
        if len(word) == 1:
            labels.append(3)
        else:
            labels.append(0)
            for _ in word[1:-1]:
                labels.append(1)
            labels.append(2)
    return labels


def line2char(l):
    """将分词构成的句子拆分为字构成的句子 便于学习字向量和向量化句子"""
    chars = []
    for word in l:
        for ch in word:
            chars.append(ch)
    return chars


def label2split(padlabels, lens):
    """将标志序列转换为分词结果（每个词的开始结束位置）"""
    split = []
    weird = 0
    i = 0
    last = 0
    while i < lens:
        if padlabels[i] == 2:  # B...E
            split.append((last, i))
            last = i + 1
        elif padlabels[i] == 3:  # ...S...
            if last < i:  # 预料之外的情况 比如 BS
                split.append((last, i - 1))
                weird += 1
            split.append((i, i))
            last = i + 1
        elif i == lens - 1:
            split.append((last, i))
        i += 1
    return split, weird


def collate_fn(batch):
    """通过padding实现变长batch 返回padding的batch"""
    lens = torch.tensor([len(dat[0]) for dat in batch])
    idx_batch = [torch.tensor(d[0]) for d in batch]
    label_batch = [torch.tensor(d[1]) for d in batch]
    # 对batch进行padding 即将长度不足的序列用0填充到与batch内最长序列等长
    idx_batch = torch.nn.utils.rnn.pad_sequence(idx_batch, batch_first=True, padding_value=0)
    label_batch = torch.nn.utils.rnn.pad_sequence(label_batch, batch_first=True, padding_value=0)
    return idx_batch, label_batch, lens


def log_sum_exp(smat):
    """对矩阵 (b,n,n) 的每一列：先exp 再sum 再log 输出一个行向量(b,1,n)"""
    vmax = smat.max(dim=1, keepdim=True).values  # 每一列的最大数
    return (smat - vmax).exp().sum(axis=1, keepdim=True).log() + vmax  # 把最大的元素 先减后加 数值稳定


if __name__ == '__main__':
    """算法debug"""
    import torch
    pad_score =[[[ 0.4440,  0.2396, -0.3698, -0.2865],
         [ 0.4490,  0.2249, -0.3950, -0.2714],
         [ 0.4554,  0.2208, -0.3991, -0.2826],
         [ 0.4297,  0.2109, -0.3776, -0.3089],
         [ 0.2960,  0.2104, -0.2429, -0.4008],
         [ 0.2960,  0.2104, -0.2429, -0.4008]],
                [[0.4440, 0.2396, -0.3698, -0.2865],
                 [0.4490, 0.2249, -0.3950, -0.2714],
                 [0.4554, 0.2208, -0.3991, -0.2826],
                 [0.4297, 0.2109, -0.3776, -0.3089],
                 [0.4297, 0.2109, -0.3776, -0.3089],
                 [0.4297, 0.2109, -0.3776, -0.3089]]
                ]
    transi_from_BOS= [-1.0150,  1.8956, -0.2732,  0.7640]
    transi_to_EOS=[ 1.3646,  0.4115,  1.7182, -0.3772]
    transitions=[[-1.0702e+00,  1.9567e+00,  1.4061e-01,  1.5405e-01],
        [-1.6920e-03,  3.9496e-01,  4.3274e-01,  9.9779e-01],
        [ 1.4537e+00,  1.0329e+00,  9.9909e-01, -1.5993e+00],
        [-3.6438e-01, -7.1596e-01, -2.9975e+00, -2.1059e+00]]
    import math

    ans = [0, 0, 0, 0]
    for i in [0, 1, 2, 3]:
            ans[i] += math.exp(transi_from_BOS[i] + pad_score[0][0][i])
    #print([math.log(ans[i]) for i in [0,1,2,3]])
    ans = [0, 0, 0, 0]
    for i in [0, 1, 2, 3]:
        for j in [0, 1, 2, 3]:
                    ans[j] += math.exp(transi_from_BOS[i] + transitions[i][j] + pad_score[0][0][i] + pad_score[0][1][j])
    #print([math.log(ans[i]) for i in [0,1,2,3]])
    ans = [0, 0, 0, 0]
    for i in [0, 1, 2, 3]:
        for j in [0, 1, 2, 3]:
            for k in [0, 1, 2, 3]:
                    ans[k] += math.exp(transi_from_BOS[i] + transitions[i][j] + transitions[j][k] + pad_score[0][0][i] + pad_score[0][1][j] + pad_score[0][2][k])
    #print([math.log(ans[i]) for i in [0,1,2,3]])
    ans = [0,0,0,0]
    for i in [0,1,2,3]:
        for j in [0,1,2,3]:
            for k in [0,1,2,3]:
                for l in [0,1,2,3]:
                    ans[l] += math.exp(transi_from_BOS[i]+transitions[i][j]+transitions[j][k]+transitions[k][l] + pad_score[0][0][i] + pad_score[0][1][j] + pad_score[0][2][k] + pad_score[0][3][l])
    #print([math.log(ans[i]) for i in [0,1,2,3]])
    ans = 0
    amax = (0,0,0,0)
    smax = -100
    for i in [0, 1, 2, 3]:
        for j in [0, 1, 2, 3]:
            for k in [0, 1, 2, 3]:
                for l in [0, 1, 2, 3]:
                    tmp =transi_from_BOS[i] + transitions[i][j] + transitions[j][k] + transitions[k][l] + transi_to_EOS[l] + pad_score[0][0][i] + pad_score[0][1][j] + pad_score[0][2][k] + pad_score[0][3][l]
                    ans += math.exp(tmp)
                    if tmp > smax:
                        amax=(i,j,k,l)
                        smax =tmp
    print(smax,amax)
    print(math.log(ans))
    ans1=transi_from_BOS[0]
    #print("target={}".format(ans1))
    add=transitions[0][2] + transitions[2][0] + transitions[0][2]
    ans1+=add
    #print("target={:.3f} {:.3f}".format(add, ans1))
    add=pad_score[0][0][0] + pad_score[0][1][2] + pad_score[0][2][0] + pad_score[0][3][2]
    ans1+=add
    #print("target={:.3f} {:.3f}".format(add, ans1))
    add=transi_to_EOS[2]
    ans1+=add
    print("target={:.3f}".format(ans1))

    ans = 0
    amax = (0, 0, 0, 0, 0, 0)
    smax = -100
    for i in [0, 1, 2, 3]:
        for j in [0, 1, 2, 3]:
            for k in [0, 1, 2, 3]:
                for l in [0, 1, 2, 3]:
                    for m in range(4):
                        for n in range(4):
                            tmp = transi_from_BOS[i] + transitions[i][j] + transitions[j][k] + transitions[k][l] + transitions[l][m] +transitions[m][n] +\
                                  transi_to_EOS[n] + pad_score[1][0][i] + pad_score[1][1][j] + pad_score[1][2][k] + pad_score[1][3][l] + pad_score[1][4][m] + pad_score[1][5][n]
                            ans += math.exp(tmp)
                            if tmp > smax:
                                amax = (i, j, k, l, m, n)
                                smax = tmp
    print(smax, amax)
    print(math.log(ans))
    ans1 = transi_from_BOS[0]
    # print("target={}".format(ans1))
    add = transitions[0][2] + transitions[2][0] + transitions[0][2]+ transitions[2][0]+ transitions[0][2]
    ans1 += add
    # print("target={:.3f} {:.3f}".format(add, ans1))
    add = pad_score[1][0][0] + pad_score[1][1][2] + pad_score[1][2][0] + pad_score[1][3][2]+ pad_score[1][4][0] + pad_score[1][5][2]
    ans1 += add
    # print("target={:.3f} {:.3f}".format(add, ans1))
    add = transi_to_EOS[2]
    ans1 += add
    print("target={:.3f}".format(ans1))

    transi_from_BOS=torch.tensor(transi_from_BOS)
    pad_scores = torch.tensor(pad_score).double()
    lens = torch.tensor([4,6])
    transitions = torch.tensor(transitions)
    transi_to_EOS = torch.tensor(transi_to_EOS)
    lens_dim = 6

    pad_target = torch.tensor([[0, 2, 0, 2, 0, 0], [0, 2, 0, 2, 0, 2]])
    len_bool = torch.tensor(range(lens_dim)).view(1, -1) < lens.view(-1, 1)
    len_1_bool = torch.tensor(range(lens_dim)).view(1, -1) < (lens - 1).view(-1, 1)
    # 计算目标labels在当前模型下的预测概率
    target_score = transi_from_BOS[pad_target[:, 0]]  # <BOS> 二元特征 (b,)
    # print(target_score)
    # 二元特征 (b, )
    add = torch.sum(transitions[pad_target[:, :-1], pad_target[:, 1:]] * len_1_bool[:, :-1], dim=1)
    target_score += add
    # print(add,target_score)
    add = torch.sum(pad_scores.view(-1, 4)[range(2 * lens_dim), pad_target.flatten()].view(2, lens_dim) * len_bool,
                    dim=1)  # 一元特征
    target_score += add
    # print(add, target_score)
    add = transi_to_EOS[pad_target[range(2), (lens - 1).flatten()]]  # <EOS> 二元特征
    target_score += add
    print("target_score", target_score)

    all_score = log_sum_exp(transi_from_BOS.view(1, 1, -1).repeat(2, 1, 1) + pad_scores[:, 0, :].unsqueeze(1))  # (b,1,n)
    for i in range(1, lens_dim):  # (b,n) in (len,b,n)
        # (b,n,1)+(b,1,n)+(1,n,n) = (b,n,n) = (b,1,n) b中len大于i的 继续计算 len小于等于i的 保持不变\
        all_score[lens > i] = log_sum_exp(all_score.permute(0, 2, 1) + pad_scores[:, i, :].unsqueeze(1) +
                                          transitions.unsqueeze(0))[lens > i]
    # 从不同状态到<EOS> (b,n,1) （相当于(b,n,n)中的一列) (b,n,1) (b,1,1)
    all_score = log_sum_exp(all_score.permute(0, 2, 1) + transi_to_EOS.view(1, -1, 1)).flatten(start_dim=0)
    print("all_score", all_score)

    backtrace = torch.zeros((lens_dim-1, 2, 4)).long()  # (len,b,n)
    best_score = transi_from_BOS.view(1, 1, -1).repeat(2, 1, 1) + pad_scores[:, 0, :].unsqueeze(1)  # (b,1,n)
    smat = torch.zeros((2, 4, 4)).double()
    for i in range(1, lens_dim):  # (b,n) in (len,b,n)
        # (b,n转移前,n转移后) 对每一个转移后 找一个最好的转移前
        smat[lens > i] = (best_score.permute(0, 2, 1) + pad_scores[:, i, :].unsqueeze(1) + transitions.unsqueeze(0).double())[lens > i]
        # (b,1,n) (b,n)
        best_score, idx = smat.max(dim=1, keepdim=True)
        backtrace[i-1] = idx.squeeze(1)
    best_score = best_score.permute(0, 2, 1) + transi_to_EOS.view(1, -1, 1)  # (b,n,1)
    best_labels = torch.zeros((2, lens_dim)).long()  # (b,len)
    best_labels[range(2), lens - 1] = best_score.argmax(dim=1).flatten().data
    for i in range(lens_dim - 1):  # (len-2)
        best_labels[range(2), lens - 2 - i] = backtrace[lens - 2 - i, range(2), best_labels[range(2), lens - 1 - i].flatten()].data
    print("best_score", best_score)
    print("best_labels", best_labels)
