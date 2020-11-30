from collections import defaultdict
import numpy as np
import matplotlib.pyplot as plt


class Recorder():
    def __init__(self, name):
        self.record = defaultdict(list)
        self.name = name

    def update(self, key, data, epoch):
        self.record[key].append([epoch, data])

    def save(self):
        np.save('./save/' + self.name + '.pkl', self.record)


class Drawer():
    def __init__(self, path, image_path):
        self.record = np.load(path + ".npy", allow_pickle=True).tolist()
        self.image_path = "./save/" + image_path

    def plot(self, key, type, label):
        support = np.array(self.record[key])
        support_x = support[:, 0]
        support_y = support[:, 1]
        plt.plot(support_x, support_y, type, label=label)

    def draw(self):
        keys = list(self.record.keys())
        for key in ['loss', 'precision', 'recall', 'f1score']:
            plt.figure()
            self.plot(key, '-', key)
            plt.title(key + ' vs. epoches')
            plt.ylabel(key)
            plt.legend(loc='best')
            plt.savefig(self.image_path + key + '.png')


if __name__ == "__main__":
    record = {}
    for setting in ['fix', 'unfix', 'nopre']:
        record[setting] = np.load('./save/' + setting + '.npy', allow_pickle=True).tolist()
    for key in ['loss', 'precision', 'recall', 'f1score']:
        plt.figure()
        for setting in ['fix', 'unfix', 'nopre']:
            support = np.array(record[setting][key])
            plt.plot(support[:, 0], support[:, 1], '-', label=setting)
        if key == 'loss':
            plt.title(key + ' vs. batchs')
        else:
            plt.title(key + ' vs. epoches')
        plt.ylabel(key)
        plt.legend(loc='best')
        plt.savefig('./save/' + key + '.png')


