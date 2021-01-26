# -*- coding: utf-8 -*-
import sys
from collections import defaultdict    
import json

test_data_jsonstr = None  # 记录测试集数据 
stat = defaultdict(lambda : defaultdict(lambda :1e-1))  # stat[t][w] 记录 P(w|t)
stat_type = defaultdict(float)  # stat_type[t] 记录 p(t)
all_words = set()

cur_tp = None
tp_w = defaultdict(int)


def get_dict():
    """从当前cur_tp的所有键值对中合并信息"""
    global test_data_jsonstr
    if cur_tp == '_MYTYPE_':
        for k in tp_w:
            stat_type[k] = tp_w[k]
    elif cur_tp == '_MYTEST_':
        for k in tp_w:
            test_data_jsonstr = k
            break
    else:
        for k in tp_w:
            all_words.add(k)
            stat[cur_tp][k] = tp_w[k]


for line in sys.stdin:
    tp, w, cnt = line.split('\t')
    # 因为键值对都排好序 而且相同键的键值对都在同一个reducer中 所以当tp变化时 就说明已经收集到cur_tp的全部键值对 可以合并了
    if cur_tp == None:
        cur_tp = tp
    elif cur_tp != tp:
        get_dict()
        cur_tp = tp
        tp_w = defaultdict(int)
    tp_w[w] += int(cnt)
# 最后的cur_tp
get_dict()

# 统计信息并保存到hdfs中（通过标准输出流）
all_types = list(stat.keys())

for tp in all_types:
    stat_sum = float(sum([stat[tp][w] for w in all_words]))
    for w in stat[tp]:
        stat[tp][w] /= stat_sum
        
all = float(sum(stat_type.values()))  
if all:
    for tp in all_types:
        stat_type[tp] /= all
json_dict = {}
json_dict['test_data'] = test_data_jsonstr
json_dict['tp_prob'] = json.dumps(stat_type)
json_dict['w_tp_prob'] = json.dumps(stat)

print(json.dumps(json_dict))