
import torch
import torch.nn as nn



class LSTM(nn.Module):
    def __init__(self, device, one_hot_size, hidden_dim=128):
        super(LSTM, self).__init__()
        self.one_hot_size = one_hot_size
        self.hidden_dim = hidden_dim
        self.device = device  # CPU或GPU
        self.embedding = nn.Embedding(one_hot_size, hidden_dim, padding_idx=1)
        self.lstm = nn.LSTM(self.hidden_dim,
                            self.hidden_dim,
                            num_layers=1,
                            batch_first=True)
        self.fc = nn.Linear(self.hidden_dim, self.one_hot_size)

    def criterion(self, pad_seq, lens):
        """交叉熵 loss"""
        # 用pack-lstm-pad的方式得到lstm输出
        embed_seq = self.embedding(pad_seq)
        packed_seq = torch.nn.utils.rnn.pack_padded_sequence(embed_seq,
                                                             lens,  # 最后一个词EOS不用输入LSTM 因为它表示已经没有下一个词了
                                                             batch_first=True,
                                                             enforce_sorted=False)  # pack
        pack_lstm_out, _ = self.lstm(packed_seq)
        pad_lstm_out, _ = torch.nn.utils.rnn.pad_packed_sequence(pack_lstm_out, batch_first=True)  # (b, l - 1, hot)
        pad_pred = self.fc(pad_lstm_out)
        # 预测的词和实际下一个词的差异 共有b*（len-1）对  (b, C, l) (b, l)
        ce_loss = torch.nn.functional.cross_entropy(pad_pred.permute(0, 2, 1)[:, :, :-1], pad_seq[:, 1:], ignore_index=1)
        return torch.mean(ce_loss)

    def forward(self, start, limit=100):
        """用一个给定的词作为开头 生成文本"""
        out = torch.LongTensor([[start]]).to(self.device)  # torch.nn.functional.one_hot(start, num_classes=self.one_hot_size)
        hidden = None
        res = [out.item()]
        while True:
            out = self.embedding(out)
            out, hidden = self.lstm(out, hidden)
            out = self.fc(out)
            out = torch.argmax(out)
            res.append(out.item())
            out = out.view(1, 1)
            if res[-1] == 0 or len(res) >= limit:
                break
        return res
