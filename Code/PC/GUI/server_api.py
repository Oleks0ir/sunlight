import requests as req
import time

lastLog = -1
name = "TestlogR.log"


def formattime(time):
    pass

def returnPoints():
    return [1,2,3,4,4,100,30,2,5,67,89,571,2,3,4,4,100,30,2,5,67,89,571,2,3,4,4,100,30,2,5,67,89,571,2,3,4,4,100,30,2,5,67,89,57]


class server():
    def __init__(self, IP, host_id, host, password, LtV):
        self.IP = IP + "/"
        self.id = host_id
        self.host = host
        self.password = password
        self.Ltv = LtV
        self.callbackData = {
                "ID": host_id,
                "HOST": host,
                "PASSWORD": password
            }
        self.isOnline = self.ping()
        self.files_recieved = False
        self.logsrecieved= []

    def ping(self)->bool:
        try:
            resp = req.get(url=self.IP+"ping")
            if resp.status_code == 200:
                return True
            else:
                return False
        except:
            return False

    def calltime(self):
        print("===TIME TEST STARTED===")
        print(f"Device time reference: {self.Ltv}")
        resp = req.post(url=self.IP + "current_time", data={"currentTime": self.Ltv})
        self.display("Current time setup, POST", resp)
        self.display("Fetch dataTime, GET", req.get(url=self.IP + "fetchTime"))
        print("======TIME TEST FINISHED ======")

    def sendBigFile(self, path):
        f = open(path, "r")
        listed_f = f.readlines()

        print(f'\n ===Gate opened: {req.post(url=self.IP + "openGate", data={"data": 1}).status_code}')

        i = 0
        while i < len(listed_f):
            print(
                f'Writing of {listed_f[i]} \n -> {req.post(url=self.IP + "dunkGate", data={"filename": "logDay.log", "line": str(listed_f[i])}).status_code}')
            i += 1

        print(f'\n ===Gate closed: {req.get(url=self.IP + "closeGate").status_code}')

    def display(self, type, response) -> None:
        print("\n==================================\n")
        print(f"===>REQUEST of -{type}")
        print(f" -Status code>{response.status_code}")
        print(f" -Content:  \n {response.content}")
        print("\n==================================\n")

    def callback(self) -> bool:
        resp = req.post(url=self.IP + "callback", data=self.callbackData)
        self.display("callback, POST", resp)

        return resp.status_code == 200

    def setcallback(self) -> bool:
        resp = req.post(url=self.IP + "setcallback", data={"lastTimeVisit": self.Ltv})
        self.display("setcallback, POST}", resp)

        return resp.status_code == 200

    def pullconfig(self) -> bool:
        resp = req.get(url=self.IP + "pullconfig", data={"ID": "0"})
        self.display("pullconfig, GET", resp)

        return resp.status_code == 200

    def requestLogs(self):
        print("\n==================================\n")
        self.writeLogs(req.get(url=self.IP + "getDay"), "logDay.log")
        self.writeLogs(req.get(url=self.IP + "getWeek"), "logWeek.log")
        self.writeLogs(req.get(url=self.IP + "getMonth"), "logMonth.log")
        self.writeLogs(req.get(url=self.IP + "getYear"), "logYear.log")

    def writeLogs(self, request, log):
        if req.status_codes == 200:
            self.logsrecieved.append(log)
            self.files_recieved = True

        f = open(log, "w")
        f.write(request.content)
        f.close()



