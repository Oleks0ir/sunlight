import datetime
from mathEngine import decode_hexstring, encode_hexodec



class logFile():
    def __init__(self, name, type, path):
        self.name = name
        self.type = type
        self.path = path
        self.lines = []
        self.voltage_OC = []
        self.voltage_Res = []
        self.temperature = []

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
            currentLine = line(path, i)
            currentLine.readLine()
            self.lines.append(currentLine)

            self.voltage_OC.append(currentLine.voltageOC)
            self.voltage_Res.append(currentLine.voltageRes)
            self.temperature.append(currentLine.temperature)


    def rewrite_log_with_given(self, voltageOC = [], current = [], temperature = []):
        Maxlines = len(temperature)
        for i in range(Maxlines):
            curLine = line(path = self.path, line = i, voltageOC = voltageOC[i],voltageRes = current[i], temperature = temperature[i])
            curLine.writeline(i)
            print(i)

    def self_to_rewrite_log_to(self):
        for i in range(len(self.lines)):
            curLine = line(path = self.path, line= i, datetime = self.lines[i].date, voltageOC= self.lines[i].voltageOC, voltageRes = self.lines[i].voltageRes, temperature = self.lines[i].temperature)
            curLine.writeline(i)



class line():
    def __init__(self, path, line=0, voltageOC = 0, voltageRes = 0, temperature = float(), datetime = datetime.datetime.now()):
        self.path = str(path)
        self.line = int(line)
        self.date = datetime
        self.voltageOC = voltageOC
        self.voltageRes = voltageRes
        self.temperature = temperature

    def __str__(self):
        return f"{self.date} : \nVoltage: {self.voltageOC}\nCurrent: {self.voltageRes}\nTemperature: {self.temperature}"


    def readLine(self):
        file = open(self.path, "r")

        if self.line>0:
            for i in range(self.line):
                protoLine = file.readline()     #ist genutzt, um an die gewünste Linie zu kommen

        import datetime

        protoLine = file.readline().strip()  # Remove trailing newline and spaces

        # Locate delimiters in protoLine for parsing
        div1 = protoLine.index("-")
        div2 = div1 + 1 + protoLine[div1 + 1:].index("-")
        space1 = div2 + 1 + protoLine[div2 + 1:].index(" ")

        div3 = space1 + 1 + protoLine[space1 + 1:].index(":")
        div4 = div3 + 1 + protoLine[div3 + 1:].index(":")
        space2 = div4 + 1 + protoLine[div4 + 1:].index(" ")

        # Extract date and time parts
        year = int(protoLine[0:div1])
        month = int(protoLine[div1 + 1:div2])
        day = int(protoLine[div2 + 1:space1])

        hour = int(protoLine[space1 + 1:div3])
        minute = int(protoLine[div3 + 1:div4])
        second = int(protoLine[div4 + 1:space2])

        # Assign date and time to self.date
        self.date = datetime.datetime(year=year, month=month, day=day,
                                      hour=hour, minute=minute, second=second)

        # Parse voltage and temperature data
        pointadress = protoLine.index("|")
        self.voltageOC = decode_hexstring(protoLine[space2 + 1:pointadress])

        # Adjust pointadress for next "|"
        pointadress2 = pointadress + 1 + protoLine[pointadress + 1:].index(" ")
        self.voltageRes = decode_hexstring(protoLine[pointadress + 1:pointadress2])

        # Extract remaining part for temperature
        self.temperature = decode_hexstring(protoLine[pointadress2 + 1:])

        file.close()

    def writeline(self, i):
        if i==0:
            file = open(self.path, "w")
        else:
            file = open(self.path, "a")

        file.write(f"{self.date} {encode_hexodec(self.voltageOC[0])}|{encode_hexodec(self.voltageRes[0])}_{encode_hexodec(self.temperature)}\n")
        file.close()

if __name__ == "__main__":
    log = logFile("logDay", "r", "logDay.log")
    print(log)

    #rewriteLog = logFile("logDay", "w", "TestlogW.log")
    #rewriteLog.lines = log.lines

    #rewriteLog.self_to_rewrite_log_to()
