

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