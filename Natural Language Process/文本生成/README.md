使用LSTM进行文本生成

Text Generation by LSTM

```powershell
C:.
│  main.py  # 主进程 使用python main.py和其他参数命令来执行程序
│  utils_args.py  # 使用argparser对命令行参数进行处理
│  utils_dataset.py  # 对数据集进行处理的MSRDataSet类
│  utils_draw.py  # 对结果进行记录的Recorder类和绘制结果的Draw类
│  utils_evaluate.py  # 对模型进行评估的evaluate方法
│  utils_model.py  # LSTM文本生成模型
│  utils_train.py  # 对模型进行训练的train方法
├─dataset
│      split1.txt  # 训练集1 由中文分词训练集分词得到
│      split2.txt  # 训练集2 由中文分词测试集分词得到
├─save/  # 保存模型参数和训练日志
```

Requirement：`pytorch`、`numpy`、`tqdm`、`argparse`、`matplotlib`、`logging`

training：`python main.py`

application(Text Generation): `python utils_evaluate.py` 

- then input model path like `./save/adam_16e-4_200/model_best`
- then input start words, it should be in vocabulary.

more arguments shown in `utils_args.py`

pretrain model: [预训练](./save/adam_16e-4_200/model_best)

