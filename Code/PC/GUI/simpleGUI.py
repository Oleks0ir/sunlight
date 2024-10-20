import self
from kivy.graphics import *
from kivy.app import App
from kivy.metrics import dp
from kivy.uix.anchorlayout import AnchorLayout
from kivy.uix.widget import Widget
from kivy.uix.button import Button
import server_api as server
import mathEngine as mthen



#esp = server.server(IP="http://192.168.178.67", host_id="0", host="admin", password="admin", LtV = "2020-09-21 17:07:11")

class SunshineLabsApp(App):
    pass

class Widget_Monocristaline_Power(AnchorLayout):
    pass

"""class VoltageGraph_Multiplexer(Widget):
    points=server.returnPoints()

    columns = len(points)
    spacing = dp(5)

    maxLen=dp(1000)
    colLen = (maxLen-(columns-1)*spacing)/columns

    maxHigh = dp(200)
    maxPointValue = mthen.findMaxVar(points)
    percentHigh = mthen.toPercent(points, maxPointValue)
    adaptedHigh = mthen.multMatrixbyNum(percentHigh, maxHigh)

    def __init__(self, **kwargs):
        super(VoltageGraph_Multiplexer, self).__init__()
        baseX = self.width*0.5
        baseY = self.height*0.5
        for i in range(self.columns):
            with self.canvas:
                Color(1, 1, 1)
                Rectangle(size=(15, self.adaptedHigh[i]), pos=(dp(30)+(self.colLen + self.spacing)*i, baseY))"""


class VoltageGraph_Monocristaline(Widget):
    points=server.returnPoints()

    columns = len(points)
    spacing = dp(5)

    maxLen=dp(300)
    colLen = (maxLen-(columns-1)*spacing)/columns

    maxHigh = dp(200)
    maxPointValue = mthen.findMaxVar(points)
    percentHigh = mthen.toPercent(points, maxPointValue)
    adaptedHigh = mthen.multMatrixbyNum(percentHigh, maxHigh)

    def __init__(self, **kwargs):
        super(VoltageGraph_Monocristaline, self).__init__()
        baseX = self.width*0.5
        baseY = self.height*0.5
        for i in range(self.columns):
            with self.canvas:
                Color(1, 1, 1)
                Rectangle(size=(15, self.adaptedHigh[i]), pos=(dp(50)+(self.colLen + self.spacing)*i, baseY))


class VoltageGraph_Polycristaline(Widget):
    def __init__(self, **kwargs):
        super(VoltageGraph_Polycristaline, self).__init__(**kwargs)

        self.points = server.returnPoints()
        self.columns = len(self.points)
        self.spacing = dp(5)
        self.maxLen = dp(1000)
        self.colLen = (self.maxLen - (self.columns - 1) * self.spacing) / self.columns
        self.maxHigh = dp(200)
        self.maxPointValue = mthen.findMaxVar(self.points)
        self.percentHigh = mthen.toPercent(self.points, self.maxPointValue)
        self.adaptedHigh = mthen.multMatrixbyNum(self.percentHigh, self.maxHigh)

        # Set the width of the widget based on the number of columns
        self.width = dp(30) + (self.colLen + self.spacing) * self.columns
        self.height = dp(200)

        baseX = self.width * 0.5
        baseY = 0
        for i in range(self.columns):
            with self.canvas:
                Color(1, 1, 1)
                Rectangle(size=(15, self.adaptedHigh[i]), pos=(dp(30) + (self.colLen + self.spacing) * i, baseY))

SunshineLabsApp().run()