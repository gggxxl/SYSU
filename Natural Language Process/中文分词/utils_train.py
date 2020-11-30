from utils_args import args
from utils_other import collate_fn
from utils_draw import Recorder, Drawer

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


def train(device, model, training_set, testing_set, evaluate_fn, recoder, running_id):
    epochs = args.epoch
    logger = get_logger('./save/' + running_id + '_logs')
    f = model.to(device).double()
    # for name, param in f.named_parameters():
    #     if param.requires_grad:
    #         print(name, ':', param.size())
    loader_dict = dict(shuffle=True, batch_size=args.batch_size, pin_memory=True, num_workers=4, collate_fn=collate_fn)
    train_loader = DataLoader(training_set, **loader_dict)
    optimer = optim.SGD(f.parameters(), lr=args.lr, momentum=args.momentum, weight_decay=args.weight_decay)
    scheduler = lr_scheduler.StepLR(optimer, step_size=args.decay_epoch, gamma=args.lr_decay)
    logger.info('start training!')
    best_score = f1_score = 0.0
    for epoch in range(epochs):
        f.train()
        for batch_idx, pack in enumerate(train_loader):
            optimer.zero_grad()
            pad_seq = pack[0].to(device).long()
            pad_label = pack[1].to(device).long()
            lens = pack[2]
            loss = f.criterion(pad_seq, pad_label, lens)
            loss.backward()
            optimer.step()
            logger.info('Epoch:[{:.3f}/{}/{}] loss={:.5f}'.format(batch_idx / len(train_loader), epoch, epochs, loss))
            recoder.update('loss', loss.data, epoch*len(train_loader)+batch_idx)
        scheduler.step()
        precision, recall, f1_score = evaluate_fn(device, model, testing_set, running_id)
        if f1_score > best_score:
            best_score = f1_score
            torch.save(model.state_dict(), "./save/" + running_id + "_model_param_best")
        logger.info('Epoch:[{}/{}  precision={:.3f} recall={:.3f} f1score={:.3f}'
                    ' best_f1={:.3f}'.format(epoch, epochs, precision, recall, f1_score, best_score))
        recoder.update('precision', precision, epoch)
        recoder.update('recall', recall, epoch)
        recoder.update('f1score', f1_score, epoch)
        if epoch % 5 == 0:
            recoder.save()
            Drawer('./save/' + running_id + '.pkl', running_id).draw()
    logger.info('finish training!')
    recoder.save()
