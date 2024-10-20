#
#
#
#
#
import math


def findMaxVar(p_list):
    maxValue = 0
    for i in range(len(p_list)):
        if p_list[i] > maxValue:
            maxValue=p_list[i]


    return maxValue


def toPercent(p_list, MaxValue):
    return_list = []

    for i in range(len(p_list)):
        return_list.append(p_list[i]/MaxValue)

    return return_list


def multMatrixbyNum(p_list, Number):
    for i in range(len(p_list)):
        p_list[i] = p_list[i]*Number

    return p_list

def decode_hexstring(hexstring) -> int:
    hexodecNumbers = {"0": 0, "1": 1,
                      "2": 2, "3": 3,
                      "4": 4, "5": 5,
                      "6": 6, "7": 7,
                      "8": 8, "9": 9,
                      "a": 10, "b": 11,
                      "c": 12, "d": 13,
                      "e": 14, "f": 15}

    decoded = int()
    hexchars = (list(hexstring))
    hexchars.reverse()

    for i in range(0, len(hexchars)):
        pot = int(math.pow(16, i))
        decoded += hexodecNumbers[hexchars[i]]*pot
        
    return decoded

def encode_hexodec(decstrings):
    hexodecNumbers = ["0",   "1",
                      "2",   "3",
                      "4",   "5",
                      "6",   "7",
                      "8",   "9",
                      "a",   "b",
                      "c",   "d",
                      "e",   "f",]

    encoded = ""

    decnum = int(decstrings)

    ThirdDigit = (decnum - decnum%256)/256
    encoded += str(hexodecNumbers[int(ThirdDigit)])

    SecondDigit = (decnum%256 - decnum%16) / 16
    encoded += str(hexodecNumbers[int(SecondDigit)])

    FirstDigit = decnum%16

    encoded += str(hexodecNumbers[int(FirstDigit)])

    return encoded

if __name__ == "__main__":
    print(decode_hexstring("22b"))
    print(encode_hexodec(555))


