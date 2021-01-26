# 在虚拟机hadoop集群中使用python实现MapReduce版的朴素贝叶斯分类器

```powershell
D:.
    alldata.json  # 数据集 
    get_data.ipynb  # 爬取数据
    Naive Bayes.ipynb  # 单机版朴素贝叶斯
    README.md  
    test.sh  # mapreduce版朴素贝叶斯测试脚本
    test_mapper.py  
    test_reducer.py
    train.sh  # mapreduce版朴素贝叶斯训练脚本
    train_mapper.py
    train_reducer.py
```

**分类问题：**

> 在Github代码仓库中，存在大量已分类（即加上标签）的软件bug。但是，现在的分类标签大都是基于人工添加的，效率比较低。本项目通过爬取大量具有分类标签的Bug，利用MapReduce分布式编程模型，实现分类算法，自动给Bug加上标签

定义一个在issues数据上的分类问题：给定issues的标题，分类器应给出issues在comp主题或type等主题下的类别。该问题是一个多标签（主题）的多分类问题，在具体的实现中，我们分别对每个标签训练一个分类器。

**爬取数据：**

- 运行环境为物理机windows 10上的`jupyter notebook` `python3`

- 运行`get_data.ipynb`即可，可修改数据源、保存数据名

**单机版：**

- 运行环境为物理机windows 10上的`jupyter notebook` `python3`

- 运行`Naive Bayes.ipynb`即可

**在Hadoop集群上运行MapReduce版朴素贝叶斯的过程**

- 搭建环境：
  - 两个可以互相通过host name进行ssh免密登录的Ubuntu 18.04虚拟机，配置相同的下列环境
  - Java version "1.8.0_201"
  - hadoop 3.2.2
  - 自带python环境

- 开启Hadoop集群：`start-all.sh`（或：`start-dfs.sh, start-yarn.sh`）
- 创建文件夹：`hadoop fs -mkdir train_input``hadoop fs -mkdir train_output``hadoop fs -mkdir test_output`
- 将数据集上传到hdfs：`hdfs dfs -copyFromLocal alldata.json train_input`
- 训练：`bash train.sh`
- （查看参数：`hadoop fs -cat train_output/part-00000`）
- 测试：`bash test.sh`
- 查看结果：`hadoop fs -cat test_output/part-00000`
- 关闭Hadoop集群：`stop-all.sh`

参考结果：

单机版：

> 训练集比例为0.7，基本次数为1e-1，基本频率为1e-10的情况下，在测试集上的实验结果如下：
>
> | 主题    | accuracy           | train time | test time |
> | ------- | ------------------ | ---------- | --------- |
> | type    | 0.5516299399016572 | 0.3861s    | 0.2522s   |
> | comp    | 0.5177506390230048 | -          | -         |
> | subtype | 0.627906976744186  | -          | -         |

MapReduce版：

> 在单个datanode的hadoop集群中，MapReduce版朴素贝叶斯在type主题分类任务中的结果如下：
>
> - 训练用时：4.632s
>
> - 测试用时：3.755s
>
> - 测试集accuracy：0.5545

