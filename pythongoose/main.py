import mongoose

passwd = {"admin": "istrator"}

def fauth(user, pwd):
    if user in passwd and passwd[user] == pwd:
        return True
    return None


def f1(a):
    print("f1", a)
    return "Hi\n"


def on_connect_fn():
    print("on_connect: subscribing and publishing")
    mqttc.sub("embrel_src", 1)  # request QoS1
    mqttc.pub("embrel_src", "Hello, myself!", 1, False) # send with QoS1, don't retain

def on_publish_fn(topic, msg):
    print("on_publish: got this message in", topic, "-->", msg)


mongoose.init(f1, fauth)

mqttc = mongoose.MQTTclient(on_connect_fn, on_publish_fn)
mqttc.connect("mqtt://broker.hivemq.com:1883", None, None) # no user, no pwd, no last-will msg

while True:
    mongoose.poll(1000)
