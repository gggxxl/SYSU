import sys

now = None
sumnow = 0
score = {}

for line in sys.stdin:
	what, cnt_str = line.split('\t')
	if now == None:
		now = what
	elif now != what:
		score[now] = sumnow
		sumnow = 0
		now = what
	sumnow += int(cnt_str)

score[now] = float(sumnow)
print("accuracy = {:.4f}".format(score['correct'] / score['total']))