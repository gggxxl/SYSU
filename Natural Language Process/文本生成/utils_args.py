import argparse

parser = argparse.ArgumentParser()
parser.add_argument('--batch_size', type=int, help='set the size of learning batch', default=16)
parser.add_argument('--epoch', type=int, default=500)
# optimizer
parser.add_argument('--lr', type=float, default=16e-4)
parser.add_argument('--lr_decay', type=float, default=0.1)
parser.add_argument('--decay_epoch', type=float, default=200)

args = parser.parse_args()



