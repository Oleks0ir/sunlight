import requests as req
import time

IP = "http://192.168.178.67/"

Ltv = "2023-9-20 16:54:08"



lastLog = -1

callbackData = {
    "ID": "0",
    "HOST": "admin",
    "PASSWORD": "admin"
}

rights = []


def display(type, response)->None:
    print("\n==================================\n")
    print(f"===>REQUEST of -{type}")
    print(f" -Status code>{response.status_code}")
    print(f" -Content:  \n {response.content}")
    print("\n==================================\n")

def callback()->bool:
    resp = req.post(url=IP + "callback", data=callbackData)
    display("callback, POST", resp)

    return resp.status_code == 200

def setcallback()->bool:
    resp = req.post(url=IP + "setcallback", data={"lastTimeVisit": Ltv})
    display("setcallback, POST}", resp)

    return resp.status_code == 200



def pullconfig()->bool:
    resp = req.get(url=IP + "pullconfig", data = {"ID": "0"})
    display("pullconfig, GET", resp)

    return resp.status_code == 200

def requestLogs():
    print("\n==================================\n")
    print(req.get(url=IP + "getDay").content)
    print(req.get(url=IP + "getWeek").content)
    print(req.get(url=IP + "getMonth").content)
    print(req.get(url=IP + "getYear").content)


def main():
    print("===TEST STARTED===")
    callback()
    pullconfig()
    setcallback()
    requestLogs()
    print("====== TEST FINISHED ======")

if __name__ == "__main__":
    main()
