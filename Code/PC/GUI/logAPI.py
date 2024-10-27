import datetime
from mathEngine import decode_hexstring, encode_hexodec

class logFile():
    def __init__(self, name, type, path):
        self.name = name
        self.type = type
        self.path = path
        self.lines = []
        if type == "r":
            self.get_all_lines(self.path)

    def __str__(self):
        lines = ""
        for i in range(len(self.lines)):
            lines += f"\n{self.lines[i]}\n"
        return f"====== {self.name} of type <{self.type} - logFile> ======\n Path: /{self.path}\n{lines} \n============"


    def get_all_lines(self, path) -> list:
        maxLine = 0
        notLimit = True

        file = open(path, "r")
        while notLimit:
            testLine = file.readline()
            maxLine +=1
            if testLine == "":
                notLimit = False
                maxLine -= 1



        for i in range(maxLine):
            #print("===========")
            currentLine = line(path, i)
            currentLine.readLine()
            self.lines.append(currentLine)
            #print(currentLine.temperature)
            #for i in range(4):
                #print("----")
                #print(currentLine.voltage[i])
                #print(currentLine.current[i])

        #print(self.lines[maxLine-1])

    def rewrite_log_with_given(self, voltage = {}, current = {}, temperature = []):
        Maxlines = len(temperature)
        for i in range(Maxlines):
            curLine = line(path = self.path, line = i, voltage = voltage["v"+str(i)],current = current["c"+str(i)], temperature = temperature)
            curLine.writeline(i)
            print(i)

    def self_to_rewrite_log_to(self):
        for i in range(len(self.lines)):
            curLine = line(path = self.path, line= i, datetime = self.lines[i].date, voltage= self.lines[i].voltage, current = self.lines[i].current, temperature = self.lines[i].temperature)
            curLine.writeline(i)



class line():
    def __init__(self, path, line=0, voltage = [], current = [], temperature = float(), datetime = datetime.datetime.now()):
        self.path = str(path)
        self.line = int(line)
        self.date = datetime
        self.voltage = voltage
        self.current = current
        self.temperature = temperature

    def __str__(self):
        return f"{self.date} : \nVoltage: {self.voltage}\nCurrent: {self.current}\nTemperature: {self.temperature}"


    def readLine(self):
        file = open(self.path, "r")

        if self.line>0:
            for i in range(self.line):
                protoLine = file.readline()     #ist genutzt, um an die gewünste Linie zu kommen

        protoLine = file.readline()

        #decode_hexstring(protoLine[20+i*17:23+i*17].lower()

        self.date = datetime.datetime(year=int(protoLine[0:4]), month=int(protoLine[6:7]), day=int(protoLine[9:10]), hour=int(protoLine[11:13]), minute=int(protoLine[14:16]), second=int(protoLine[17:19]))
        for i in range(int((len(protoLine)-25)/16)): # division into Vx1:Ax2|Vx1:Ax2| blocks
            leftBorder = 20 + i*17
            rightBorder = leftBorder +16

            self.voltage.append(decode_hexstring(protoLine[leftBorder:leftBorder+3].lower()))
            self.current.append(decode_hexstring(protoLine[leftBorder+4:leftBorder + 7].lower()))
            self.voltage.append(decode_hexstring(protoLine[leftBorder + 8:leftBorder + 11].lower()))
            self.current.append(decode_hexstring(protoLine[leftBorder + 12:leftBorder + 15].lower()))

        self.temperature = decode_hexstring(protoLine[len(protoLine)-4:len(protoLine)-1].lower())

        file.close()

    def writeline(self, i):
        if i==0:
            file = open(self.path, "w")
        else:
            file = open(self.path, "a")

        file.write(f"{self.date} {encode_hexodec(self.voltage[0])}:{encode_hexodec(self.current[0])}|{encode_hexodec(self.voltage[1])}:{encode_hexodec(self.current[1])}||{encode_hexodec(self.voltage[2])}:{encode_hexodec(self.current[2])}|{encode_hexodec(self.voltage[3])}:{encode_hexodec(self.current[3])} {encode_hexodec(self.temperature)}\n")
        file.close()

if __name__ == "__main__":
    log = logFile("logDay", "r", "TestlogR.log")
    print(log)

    rewriteLog = logFile("logDay", "w", "TestlogW.log")
    rewriteLog.lines = log.lines

    rewriteLog.self_to_rewrite_log_to()
