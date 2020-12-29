from utils_args import args
from utils_dataset import collate_fn
from utils_draw import Drawer
from utils_evaluate import evaluate

import torch
import torch.optim as optim
from torch.utils.data import DataLoader
import torch.optim.lr_scheduler as lr_scheduler

import logging


def get_logger(filename, verbosity=1, name=None):
    level_dict = {0: logging.DEBUG, 1: logging.INFO, 2: logging.WARNING}
    formatter = logging.Formatter(
        "[%(asctime)s][%(filename)s][line:%(lineno)d] %(message)s"
    )
    logger = logging.getLogger(name)
    logger.setLevel(level_dict[verbosity])

    fh = logging.FileHandler(filename, "w")
    fh.setFormatter(formatter)
    logger.addHandler(fh)

    sh = logging.StreamHandler()
    sh.setFormatter(formatter)
    logger.addHandler(sh)

    return logger


def train(device, model, training_set, recoder, running_id):
    epochs = args.epoch
    logger = get_logger('./save/' + running_id + '_logs')
    f = model.to(device).double()
    loader_dict = dict(shuffle=False, batch_size=args.batch_size, pin_memory=True, num_workers=4, collate_fn=collate_fn)
    train_loader = DataLoader(training_set, **loader_dict)
    optimer = optim.Adam(f.parameters(), lr=args.lr)
    scheduler = lr_scheduler.StepLR(optimer, step_size=args.decay_epoch, gamma=args.lr_decay)
    logger.info('start training!')
    best_loss = 0x3f3f3f3f
    for epoch in range(epochs):
        f.train()
        avg_loss = 0
        for batch_idx, pack in enumerate(train_loader):
            optimer.zero_grad()
            pad_seq = pack[0].to(device).long()
            lens = pack[1]
            loss = f.criterion(pad_seq, lens)
            avg_loss += loss.item()
            loss.backward()
            optimer.step()
        scheduler.step()
        avg_loss /= len(train_loader)
        logger.info('Epoch:[{}/{}] loss={:.5f}'.format(epoch, epochs, avg_loss))
        recoder.update('loss', avg_loss, epoch)
        if epoch % 5 == 0:
            evaluate(f, training_set, 2, limit=epoch * 10)
            if avg_loss < best_loss:
                best_loss = avg_loss
                torch.save(model.state_dict(), "./save/model_best")
            recoder.save()
            Drawer('./save/' + running_id + '.pkl', running_id).draw()
    logger.info('finish training!')
    recoder.save()
