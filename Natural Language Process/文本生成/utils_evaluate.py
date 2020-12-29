def evaluate(model, dataset, in_idx, limit):
    # 使用模型生成文本
    model.eval()
    res = model(in_idx, limit)
    print(dataset.idx2seq(res))

if __name__ == "__main__":
    import torch
    from utils_model import LSTM
    from utils_dataset import MSRDataSet
    device = torch.device('cpu')
    trainingSet = MSRDataSet()
    model = LSTM(device, len(trainingSet.word2idx))
    model.load_state_dict(torch.load(input("model 路径：")))
    model = model.to(device).double()
    model.eval()
    while True:
        word = input("输入一个开始词汇：")
        if word not in trainingSet.word2idx:
            print("这个词超纲了，程序即将退出")
            break
        evaluate(model, trainingSet, trainingSet.word2idx[word], limit=1000)