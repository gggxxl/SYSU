from utils_dataset import MSRDataSet
from utils_model import LSTM
from utils_train import train
from utils_draw import Recorder, Drawer
from utils_evaluate import evaluate

import time
import torch

if __name__ == "__main__":
    # 使用时间戳作为运行的ID，和其他运行结果进行区分
    runningID = str(int(time.time()) % 10000)
    # 使用的计算设备 如果有GPU就用GPU
    device = torch.device('cuda') if torch.cuda.is_available() else torch.device('cpu')
    # 构建训练集和测试集 将每条句子转换为编号的序列 并提取实际分词的标签序列
    trainingSet = MSRDataSet()
    vocab_size = len(trainingSet.word2idx)
    # 在计算设备上用预训练参数初始化模型
    model = LSTM(device, vocab_size)
    # 开始记录运行中的结果
    recoder = Recorder(runningID)
    # 开始模型训练 在训练过程中每个batch计算loss 每1个epoch进行一次评估 保存f1最高的模型参数
    train(device=device,
          model=model,
          training_set=trainingSet,
          recoder=recoder,
          running_id=runningID)
    evaluate(model, trainingSet, 2, limit=1000)
    # 根据运行记录绘制学习曲线
    Drawer('./save/' + runningID + '.pkl', runningID).draw()
