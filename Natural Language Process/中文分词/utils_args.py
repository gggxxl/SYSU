import argparse

parser = argparse.ArgumentParser()
parser.add_argument('--pretrain', type=str, help='use a pretrain word2vec model', default='sgns')
parser.add_argument('--batch_size', type=int, help='set the size of learning batch', default=128)
parser.add_argument('--char_vector_size', type=int, default=300)
parser.add_argument('--epoch', type=int, default=60)
parser.add_argument('--debug', type=bool, default=False)
parser.add_argument('--fix_embedding', type=bool, default=False)
parser.add_argument('--train_set', type=str, default='./dataset/msr_training.txt')
parser.add_argument('--test_set', type=str, default='./dataset/msr_test_gold.txt')
# optimizer
parser.add_argument('--lr', type=float, default=0.01)
parser.add_argument('--momentum', type=float, default=.9)
parser.add_argument('--weight_decay', type=float, default=5e-5)
parser.add_argument('--lr_decay', type=float, default=0.7)
parser.add_argument('--decay_epoch', type=float, default=20)

args = parser.parse_args()
if args.pretrain:
    args.char_vector_size = 300
if args.debug:
    args.train_set = './dataset/msr_tiny.txt'
    args.test_set = './dataset/msr_tiny.txt'
    args.epoch = 10
    args.lr = 0.01



