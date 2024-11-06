import requests as req
import time


def formattime(time):
    pass


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
        self.isOnline = False
        self.files_recieved = False
        self.logsrecieved= []
        self.debug = False

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
        if self.debug:
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
        print("\n===============LOGS REQUESTED===================\n")
        self.writeLogs(req.get(url=self.IP + "getDay"), "logDay.log")
        self.writeLogs(req.get(url=self.IP + "getWeek"), "logWeek.log")
        self.writeLogs(req.get(url=self.IP + "getMonth"), "logMonth.log")
        self.writeLogs(req.get(url=self.IP + "getYear"), "logYear.log")

        self.files_recieved = True

    def writeLogs(self, request, log):
        print(f"{request} << {log} <<< {request.status_code}\n")
        if request.status_code == 200:
            self.logsrecieved.append(log)
            self.files_recieved = True

            f = open(log, "w")
            f.write(request.content.decode("utf-8"))
            f.close()

    def forceReadVoltage(self):
        resp = req.get(url=self.IP + "forceReadVoltage")
        self.display("Force Read Voltage, GET", resp)

    def forceUpdateLog(self):
        resp = req.get(url=self.IP + "forceUpdateLog")
        self.display("Force Read Voltage, GET", resp)

    def uploadJson(self):
        f = open("configESP.json", "r")
        listed_f = f.readlines()

        print(f'\n ===Gate opened: {req.post(url=self.IP + "openGate", data={"dataIndex": -1}).status_code}')

        i = 0
        while i < len(listed_f):
            print(
                f'Writing of {listed_f[i]} \n -> {req.post(url=self.IP + "dunkJsonGate", data={"filename": "configESP.json", "line": str(listed_f[i])}).status_code}')
            i += 1

        print(f'\n ===Gate closed: {req.get(url=self.IP + "closeGate").status_code}')


    def forceRestart(self):
        self.display(type="forceReset(), GET", response= req.get(url=self.IP+"forceRestart"))


    def main(self):
        print("===TEST STARTED===")
        self.callback()
        self.pullconfig()
        self.setcallback()
        self.requestLogs()
        print("====== TEST FINISHED ======")

if __name__ == "__main__":
    esp = server(IP="http://192.168.178.73", host_id="0", host="admin", password="admin", LtV = "2024-11-04 19:14:00")
    esp.debug = True

    esp.main()
    mainTime = time.perf_counter()
    newTime = time.perf_counter()

    #esp.calltime()
    calltimeTime = time.perf_counter() - newTime
    newTime = time.perf_counter()

    #esp.sendBigFile()
    sendBigFileTime = time.perf_counter() - newTime
    newTime = time.perf_counter()


    esp.forceReadVoltage()
    esp.forceUpdateLog()
    forceTime = time.perf_counter() - newTime
    newTime = time.perf_counter()

    esp.uploadJson()
    uploadJsonTime = time.perf_counter() - newTime

    esp.forceRestart()

    completeTime= time.perf_counter()


    if esp.debug:
        print(f"\n ==Execution time: {completeTime :.4f}s \n  >main(): {mainTime:.4f}s  ({mainTime * 100 / completeTime :.2f}%)\n  >calltime(): {calltimeTime :.4f}s  ({calltimeTime * 100 / completeTime :.2f}%)\n  >sendBigFile(): {sendBigFileTime :.4f}s ({sendBigFileTime * 100 / completeTime :.2f}%)\n  >forceOperations(): {forceTime :.4f}s ({forceTime * 100 / completeTime :.2f}%)\n  >uploadJson(): {uploadJsonTime :.4f}s ({uploadJsonTime * 100 / completeTime :.2f}%)")




