import mongoose

passwd = {"admin": "istrator"}

def fauth(user, pwd):
    if user in passwd and passwd[user] == pwd:
        return True
    return None


def f1(a):
    print("f1", a)
    return "Hi\n"


mongoose.init(f1, fauth)
while True:
    mongoose.poll(1000)
