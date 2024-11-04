import requests as req
import time
import json

IP = "http://192.168.178.73/"

Ltv = "2024-10-29 8:14:00"

name = "Testlog.log"

debug = False #True

lastLog = -1

callbackData = {
    "ID": "0",
    "HOST": "admin",
    "PASSWORD": "admin"
}

rights = []


def display(type, response)->None:
    if debug:
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

def calltime():
    print("===TIME TEST STARTED===")
    print(f"Device time reference: {Ltv}")
    resp = req.post(url=IP + "current_time", data={"currentTime": Ltv})
    display("Current time setup, POST", resp)
    display("Fetch dataTime, GET", req.get(url=IP+"fetchTime"))
    print("======TIME TEST FINISHED ======")

def sendBigFile():
    f = open(name, "r")
    listed_f = f.readlines()

    print(f'\n ===Gate opened: {req.post(url=IP + "openGate", data={"data": 1}).status_code}')

    i=0
    while i< len(listed_f):
        print(f'Writing of {listed_f[i]} \n -> {req.post(url=IP + "dunkGate", data={"filename": "logDay.log", "line": str(listed_f[i])}).status_code}')
        i+=1

    print(f'\n ===Gate closed: {req.get(url=IP + "closeGate").status_code}')

def forceReadVoltage():
    resp = req.get(url=IP + "forceReadVoltage")
    display("Force Read Voltage, GET", resp)

def forceReadTemp():
    resp = req.get(url=IP + "forceReadTemp")
    display("Force Read Temp, GET", resp)

def forceUpdateLog():
    resp = req.get(url=IP + "forceUpdateLog")
    display("Force Read Voltage, GET", resp)

def updateConfig(jsonInput):

    resp = req.post(url =IP + "updateJson", json = jsonInput)
    display("updateConfig POST", resp)

def uploadJson():
    f = open("configESP.json", "r")
    listed_f = f.readlines()

    print(f'\n ===Gate opened: {req.post(url=IP + "openGate", data={"dataIndex": -1}).status_code}')

    i=0
    while i< len(listed_f):
        print(f'Writing of {listed_f[i]} \n -> {req.post(url=IP + "dunkJsonGate", data={"filename": "config.json", "line": str(listed_f[i])}).status_code}')
        i+=1

    print(f'\n ===Gate closed: {req.get(url=IP + "closeGate").status_code}')




if __name__ == "__main__":
    main()
    mainTime = time.perf_counter()
    newTime = time.perf_counter()

    calltime()
    calltimeTime = time.perf_counter() - newTime
    newTime = time.perf_counter()

    #sendBigFile()
    sendBigFileTime = time.perf_counter() - newTime
    newTime = time.perf_counter()


    forceReadVoltage()
    forceReadTemp()
    forceUpdateLog()
    forceTime = time.perf_counter() - newTime
    newTime = time.perf_counter()

    uploadJson()
    uploadJsonTime = time.perf_counter() - newTime

    completeTime= time.perf_counter()

    print(f"\n ==Execution time: {completeTime :.4f}s \n  >main(): {mainTime:.4f}s  ({mainTime * 100 / completeTime :.2f}%)\n  >calltime(): {calltimeTime :.4f}s  ({calltimeTime * 100 / completeTime :.2f}%)\n  >sendBigFile(): {sendBigFileTime :.4f}s ({sendBigFileTime * 100 / completeTime :.2f}%)\n  >forceOperations(): {forceTime :.4f}s ({forceTime * 100 / completeTime :.2f}%)\n  >uploadJson(): {uploadJsonTime :.4f}s ({uploadJsonTime * 100 / completeTime :.2f}%)")


