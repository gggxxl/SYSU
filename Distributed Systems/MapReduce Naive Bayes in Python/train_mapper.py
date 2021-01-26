import sys
import json

reload(sys)

sys.setdefaultencoding('utf8')
    
DOMAIN = 'type'
TRAIN_SPLIT = 0.7
for line in sys.stdin:
	olddata = json.loads(line)
	data = []
	for d in olddata:
	    d = json.loads(d)
	    if DOMAIN in d:
	        data.append(json.dumps(d))
	train_size = int(TRAIN_SPLIT * len(data)) 
	train_data = data[:train_size]
	test_data = data[train_size:]
	for d in train_data:
		d = json.loads(d)
		for w in d['title'].split():
			print(d[DOMAIN] + '\t' + w + '\t' + str(1))
		print('_MYTYPE_' + '\t' + d[DOMAIN] + '\t' + str(1))
	print('_MYTEST_' + '\t' + json.dumps(test_data) + '\t' + str(1))
	break
