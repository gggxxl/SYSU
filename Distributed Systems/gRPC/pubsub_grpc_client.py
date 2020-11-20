import grpc
import time
import threading as trd
import pubsub_pb2
import pubsub_pb2_grpc

def run():
    clientId = input("clientId: ")
    channel = grpc.insecure_channel('localhost:50051')
    stub = pubsub_pb2_grpc.PubsubStub(channel)
    def publish(topic, context):
        print("在主题{}发布消息：{}".format(topic, context))
        response = stub.publish(pubsub_pb2.publishRequest(topic=topic, context=context))
        print(response.message)
        
    def browse(topic):
        """浏览主题"""
        print("浏览主题{}".format(topic))
        response = stub.browse(pubsub_pb2.browseRequest(topic=topic))
        for msg in response:
            print(msg.message)
        
    def _subcribe(topic, TTL):
        """订阅主题 由一个新线程执行 因为发起的grpc请求是阻塞式的"""
        for msg in stub.subcribe(pubsub_pb2.subRequest(topic=topic, clientId=clientId, TTL=TTL)):
            print("来自订阅主题{}的新消息：{}".format(topic, msg.message))
    
    def subcribe(topic, TTL=20):
        """订阅主题"""
        print("订阅了主题{}".format(topic))
        thrd = trd.Thread(target=_subcribe, args=(topic,TTL))
        thrd.start()
            
    # 下面是本客户端的请求 可以根据所需修改
    publish('python', '第0秒发的消息')  
    browse('python') # 浏览python主题
    time.sleep(5)
    publish('python', '第5秒发的消息（订阅前）')
    subcribe('python', 20) # 订阅python主题（有效期20秒） 此后会收到python主题的新消息
    publish('python', '第5秒发的消息（订阅后）')
    time.sleep(6)
    browse('python') # 浏览python主题 由于服务器消息存储时间为10秒 应看不到第0秒发的消息
    

if __name__ == '__main__':
    run()