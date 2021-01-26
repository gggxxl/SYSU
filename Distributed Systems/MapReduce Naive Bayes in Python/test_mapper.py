import sys
import json

reload(sys)

sys.setdefaultencoding('utf8')
    
DOMAIN = 'type'
for line in sys.stdin:
	json_dict = json.loads(line)
	test_data = json.loads(json_dict['test_data'])
	stat = json.loads(json_dict['w_tp_prob'])
	stat_type = json.loads(json_dict['tp_prob'])
	for d in test_data:
		d = json.loads(d)
		prob = {}
		for tp in stat:
			prob[tp] = stat_type[tp]
			for w in d['title'].split():
				prob[tp] *= stat[tp][w] if w in stat[tp] else 1e-10
		pred = max(prob, key=prob.get)
		if pred == d[DOMAIN]:
			print('correct' + '\t' + str(1))
		print('total' + '\t' + str(1))	
	break
