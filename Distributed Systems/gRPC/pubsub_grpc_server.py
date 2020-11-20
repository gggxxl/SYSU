from threading import Event
from concurrent import futures
import time
import grpc


import pubsub_pb2
import pubsub_pb2_grpc

class Pubsub(object):
    """一个本地的Pubsub模型"""
    def __init__(self):
        self.storage = {}
        self.event = {}
        
    def publish(self, topic, message):
        """发布主题消息 返回发布是否成功"""
        msg = ""
        if topic not in self.storage:
            self.storage[topic] = [{'createTime': time.time(), 'message': message}]
            msg += "create topic: {}\n".format(topic)
        else:
            self.storage[topic].append({'createTime': time.time(), 'message': message})
        if topic in self.event:
            for client in self.event[topic]:
                self.event[topic][client].set()
        msg += "publish successful"
        return msg
    
    def gen_msg(self, msg):
        """将存储的消息转化为可读消息"""
        return str(msg['createTime']) + ": " + msg['message']
    
    def refresh(self, TTL=10):
        """每个一段时间刷新存储的消息 可以控制消息在服务器存储的时间"""
        ddl = time.time() - 10
        for topic in self.storage:
            while len(self.storage[topic]) and self.storage[topic][0]['createTime'] <= ddl:
                del self.storage[topic][0]
                
    def browse(self, topic):
        """浏览主题 返回主题所有消息的generator"""
        if topic not in self.storage:
            return ["topic not created"]
        for msg in self.storage[topic]:
            yield self.gen_msg(msg)
    
    def subcribe(self, topic, clientId, TTL=20):
        """订阅主题 返回主题新消息的generator"""
        if topic not in self.event:
            self.event[topic] = {}
        self.event[topic][clientId] = Event()
        createTime = time.time()
        remainTime = TTL
        while True:
            self.event[topic][clientId].wait(remainTime)  # 等待主题的新消息
            remainTime = TTL - (time.time() - createTime)
            if remainTime <= 0:
                break
            yield self.gen_msg(self.storage[topic][-1])  # 推送消息
            self.event[topic][clientId].clear()  # 新消息已推送

class PubsubService(pubsub_pb2_grpc.Pubsub):
    """实现grpc的相关过程"""
    def __init__(self):
        self.pubsub = Pubsub()  # 本地Pubsub模型
        
    def publish(self, request, context):  # context参数的作用？
        msg = self.pubsub.publish(request.topic, request.context)
        return pubsub_pb2.reply(message = msg)
    
    def browse(self, request, context):
        for msg in self.pubsub.browse(request.topic):
            yield pubsub_pb2.reply(message=msg)  # 
    
    def subcribe(self, request, context):  # 写错函数名："Exception calling application: module 'grpc' has no attribute 'experimental'"
        for msg in self.pubsub.subcribe(request.topic, request.clientId, request.TTL):   
            yield pubsub_pb2.reply(message=msg)

            
def serve():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))  # 申请grpc服务
    pubsubServe = PubsubService()  # grpc方法所在的PubsubService 
    pubsub_pb2_grpc.add_PubsubServicer_to_server(pubsubServe, server)  # 将PubsubService加入grpc服务
    server.add_insecure_port('[::]:50051')  # 绑定地址
    server.start()  # 启动服务
    try:
        while True:
            time.sleep(1)
            pubsubServe.pubsub.refresh() # 每隔一秒钟刷新服务器存储的消息
    except KeyboardInterrupt:
        server.stop(0)

if __name__ == '__main__':
    serve()