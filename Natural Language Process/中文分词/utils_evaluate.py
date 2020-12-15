from utils_args import args
from utils_other import label2split, collate_fn
from utils_model import train_embedding, train_w2v

import torch
from torch.utils.data import DataLoader
from tqdm import tqdm


def evaluate(device, model, testSet, running_id, save=False):
    model.eval()
    s_res = []
    f1 = precision = recall = 0.0
    total = 0
    with torch.no_grad():
        loader_dict = dict(shuffle=False, batch_size=args.batch_size, pin_memory=True, num_workers=4,
                           collate_fn=collate_fn)
        test_loader = DataLoader(testSet, **loader_dict)
        for pack in tqdm(test_loader):
            pad_seq = pack[0].to(device).long()
            target_label = pack[1].to(device).long()
            lens = pack[2]
            best_labels = model(pad_seq, lens)
            # best_split_bool = (best_labels == 2) | (best_labels == 3)
            for c, t, l in zip(best_labels, target_label, lens):
                cs, _ = label2split(c, l)
                if save:
                    s_res.append(cs[:])
                ts, _ = label2split(t, l)
                cs, ts = set(cs), set(ts)
                predict = len(cs)
                ground_truth = len(ts)
                correct = len(cs.intersection(ts))
                precision_i = correct / predict
                recall_i = correct / ground_truth
                if precision_i + recall_i != 0:
                    f1 += 2 * precision_i * recall_i / (precision_i + recall_i)
                precision += precision_i
                recall += recall_i
                total += 1
        precision = precision / total
        recall = recall / total
        f1 = f1 / total
        if save:
            res = []
            with open('./dataset/msr_test_gold.txt', "r", encoding="utf-8") as source:  # msr_test.txt有漏字 不要用
                i = 0
                for line in source.readlines():
                    line_split = line.split()
                    line = ""
                    for ls in line_split:  # 去掉分词的空格
                        line += ls
                    if len(line) == 0:
                        continue
                    res_i = ""
                    for s in s_res[i]:  # 根据模型的分词产生空格
                        res_i += line[s[0]: s[1]+1] + "  "
                    res.append(res_i)
                    i += 1
                with open('./save/' + running_id + "_res_{}.txt".format(f1),
                          'a+', encoding='utf-8') as target:
                    for r in res:
                        target.write(r + '\n')
        return precision, recall, f1


# def evaluate(device, model, testSet, running_id, cmp_set):
#     """对分好词的文件进行评估"""
#     model.eval()
#     s_res = []
#     f1 = precision = recall = 0.0
#     total = 0
#     with torch.no_grad():
#         loader_dict = dict(shuffle=False, batch_size=args.batch_size, pin_memory=True, num_workers=4,
#                            collate_fn=collate_fn)
#         cmp_loader = DataLoader(cmp_set, **loader_dict)
#         test_loader = DataLoader(testSet, **loader_dict)
#         for pack1, pack2 in zip(cmp_loader, test_loader):
#             cmp_label = pack1[1].to(device).long()
#             target_label = pack2[1].to(device).long()
#             lens = pack2[2]
#             for c, t, l in zip(cmp_label, target_label, lens):
#                 cs, weird = label2split(c, l)
#                 s_res.append(cs[:])
#                 ts, _ = label2split(t, l)
#                 cs, ts = set(cs), set(ts)
#                 predict = len(cs)
#                 ground_truth = len(ts)
#                 correct = len(cs.intersection(ts))
#                 precision_i = correct / predict
#                 recall_i = correct / ground_truth
#                 if precision_i + recall_i != 0:
#                     f1 += 2 * precision_i * recall_i / (precision_i + recall_i)
#                 precision += precision_i
#                 recall += recall_i
#                 total += 1
#         precision = precision / total
#         recall = recall / total
#         f1 = f1 / total
#         res = []
#         with open('./dataset/msr_test_gold.txt', "r", encoding="utf-8") as source:
#             i = 0
#             for line in source.readlines():
#                 line_split = line.split()
#                 line = ""
#                 for ls in line_split:  # 去掉分词的空格
#                     line += ls
#                 if len(line) == 0:
#                     continue
#                 res_i = ""
#                 for s in s_res[i]:  # 根据模型的分词产生空格
#                     res_i += line[s[0]: s[1] + 1] + "  "
#                 res.append(res_i)
#                 i += 1
#             with open('./save/' + running_id + "_res_{}.txt".format(f1),
#                       'a+', encoding='utf-8') as target:
#                 for r in res:
#                     target.write(r + '\n')
#
#         return precision, recall, f1

def mysplit(device, model, w2idx, source_file, running_id):
    testSet = MSRDataSet(data_path=source_file, w2idx=w2idx)
    model.eval()
    s_res = []
    with torch.no_grad():
        loader_dict = dict(shuffle=False, batch_size=args.batch_size, pin_memory=True, num_workers=4,
                           collate_fn=collate_fn)
        test_loader = DataLoader(testSet, **loader_dict)
        for pack in tqdm(test_loader):
            pad_seq = pack[0].to(device).long()
            lens = pack[2]
            best_labels = model(pad_seq, lens)
            for c, l in zip(best_labels, lens):
                cs, _ = label2split(c, l)
                s_res.append(cs[:])
        res = []
        with open(source_file, "r", encoding="utf-8") as source:
            i = 0
            for line in source.readlines():
                line_split = line.split()
                line = ""
                for ls in line_split:  # 去掉分词的空格
                    line += ls
                if len(line) == 0:
                    continue
                res_i = ""
                for s in s_res[i]:  # 根据模型的分词产生空格
                    res_i += line[s[0]: s[1]+1] + "  "
                res.append(res_i)
                i += 1
            with open('./save/' + running_id + "_split_res.txt",
                      'a+', encoding='utf-8') as target:
                for r in res:
                    target.write(r + '\n')

if __name__ == "__main__":
    from utils_model import BiLSTM_CRF
    from utils_dataset import MSRDataSet
    device = torch.device('cuda') if torch.cuda.is_available() else torch.device('cpu')
    args.char_vector_size=300
    args.pretrain=''
    w2idx, pretrain_embedding = train_embedding(train_w2v())
    model = BiLSTM_CRF(device, pretrain_embedding).to(device).double()
    model.load_state_dict(torch.load("./save/model_debug"))
    mysplit(device, model, w2idx, './dataset/msr_training.txt', 'next')
    #cmp_set = MSRDataSet(data_path="./pretrain/cmp_res2", w2idx=w2idx)
    #evaluate(device, model, testingSet, 'debug', cmp_set)