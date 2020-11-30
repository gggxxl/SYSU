from utils_dataset import MSRDataSet
from utils_evaluate import evaluate
from utils_model import train_embedding, train_w2v, BiLSTM_CRF
from utils_train import train
from utils_draw import Recorder, Drawer
from utils_args import args

import time
import torch

if __name__ == "__main__":
    # 使用时间戳作为运行的ID，和其他运行结果进行区分
    runningID = str(int(time.time()))+str(args.char_vector_size)+args.pretrain
    # 使用的计算设备 如果有GPU就用GPU
    device = torch.device('cuda') if torch.cuda.is_available() else torch.device('cpu')
    # 从预训练词向量中获得当前语料中每个字的唯一编号、BiLSTM-CRF的embedding初始化参数
    w2idx, pretrain_embedding = train_embedding(train_w2v())
    # 构建训练集和测试集 将每条句子转换为编号的序列 并提取实际分词的标签序列
    trainingSet = MSRDataSet(data_path=args.train_set, w2idx=w2idx)
    testingSet = MSRDataSet(data_path=args.test_set, w2idx=w2idx)
    # 在计算设备上用预训练参数初始化模型
    model = BiLSTM_CRF(device, pretrain_embedding)
    # 开始记录运行中的结果
    recoder = Recorder(runningID)
    # 开始模型训练 在训练过程中每个batch计算loss 每1个epoch进行一次评估 保存f1最高的模型参数
    train(device=device,
          model=model,
          training_set=trainingSet,
          testing_set=testingSet,
          evaluate_fn=evaluate,
          recoder=recoder,
          running_id=runningID)
    # 训练结束后 加载保存的最佳模型 重新测试并生成分词文件
    model.load_state_dict(torch.load("./save/" + runningID + "_model_param_best"))
    evaluate(device, model, testingSet, runningID, save=True)
    # 根据运行记录绘制学习曲线
    Drawer('./save/' + runningID + '.pkl', runningID).draw()
