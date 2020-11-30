使用BiLSTM-CRF进行分词

```powershell
C:.
│  main.py  # 主进程 使用python main.py和其他参数命令来执行程序
│  utils_args.py  # 使用argparser对命令行参数进行处理
│  utils_dataset.py  # 对数据集进行处理的MSRDataSet类
│  utils_draw.py  # 对结果进行记录的Recorder类和绘制结果的Draw类
│  utils_evaluate.py  # 对模型进行评估的evaluate方法
│  utils_model.py  # 对预训练词向量进行处理的一些方法和进行中文分词的BiLSTM-CRF类
│  utils_other.py  # 其他代码文件中需要用到的一些常用方法
│  utils_train.py  # 对模型进行训练的train方法
├─dataset/
│      msr_test.txt  # 测试集语料 仅用于提取词向量
│      msr_test_gold.txt  # 测试集 用于分词和评估
│      msr_training.txt  # 训练集
│
├─pretrain/  # 使用的预训练词向量放这里
├─save/  # 运行结果会保存在这里
```

Requirement：`gensim`、`pytorch`、`numpy`、`tqdm`、`argparse`、`matplotlib`、`logging`等等

运行主进程：

- FIX: use pretrain embeddings，but freeze it while training the model 

  `python main.py --pretrain='sgns' --fix_embedding=True`

- UNFIX: use pretrain embeddings，and train it while training the model

  `python main.py --pretrain='sgns' --fix_embedding=False`

- NOPRE: no pretrain embeddings，and train it while training the model

  `python main.py --pretrain='' --fix_embedding=False`

即可开始训练、同时每一个epoch测试一次（使用GPU，大概10分钟一次）

其他参数见`utils_args.py`



调试用：

- 运行`utils_evaluate.py`，使用保存的最佳模型（如果有，请保存为./save/model_debug）对测试集进行分词，产生分词文件；通过调整注释掉的代码也可以对分词文件直接进行测试，详见代码。

- 运行`utils_draw.py`，对记录的结果进行绘制（如果有）

- 运行`utils_other.py`，使用简单例子对相关算法（batch化）进行debug

