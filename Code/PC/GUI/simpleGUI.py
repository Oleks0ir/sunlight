import self
from kivy.graphics import *
from kivy.app import App
from kivy.metrics import dp
from kivy.uix.anchorlayout import AnchorLayout
from kivy.uix.widget import Widget
from kivy.uix.button import Button
import server_api as server
import mathEngine as mthen



esp = server.server(IP="http://192.168.178.73", host_id="0", host="admin", password="admin", LtV = "2024-10-29 8:14:00")

class SunshineLabsApp(App):
    pass

class Widget_Monocristaline_Power(AnchorLayout):
    pass
"""
class VoltageGraph_Monocristaline(Widget):
    def __init__(self, **kwargs):
        super(VoltageGraph_Monocristaline, self).__init__(**kwargs)

        self.size_hint = (1, 0.8)

        self.points = server.returnPoints()

        self.columns = len(self.points)
        self.spacing = dp(2)
        self.minWidth = dp(5)

        self.usedLen = self.width - self.spacing * (self.columns - 1)
        self.colLen = (self.usedLen) / self.columns

        if (self.colLen+self.spacing)*self.columns>self.width:
            self.size_hint = (None, 0.8)

            self.width = dp(self.minWidth*(self.columns) + self.spacing*(self.columns))

            self.usedLen = self.width - self.spacing*(self.columns-1)
            self.colLen = (self.usedLen) / self.columns

        # Set the width of the widget based on the number of columns

        self.maxHigh = self.height*2

        self.maxPointValue = mthen.findMaxVar(self.points)
        self.percentHigh = mthen.toPercent(self.points, self.maxPointValue)
        self.adaptedHigh = mthen.multMatrixbyNum(self.percentHigh, self.maxHigh)

        baseY = -self.height/2
        for i in range(self.columns):
            self.add_widget(Button(size=(self.colLen, self.adaptedHigh[i]), pos=((self.colLen + self.spacing) * i, baseY), font_size='10dp', text = f"{i}"))
"""

"""class VoltageGraph_Monocristaline(Widget):
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
                Rectangle(size=(15, self.adaptedHigh[i]), pos=(dp(50)+(self.colLen + self.spacing)*i, baseY))"""
class ConfiguratorApp(App):
    pass

class Widget_VoltAmpers(AnchorLayout):

    def sendDate(self, time):
        esp.Ltv = time.text
        esp.calltime()
        pass

    def testopen(self):
        ConfiguratorApp().run()


class VoltageGraph_Monocristaline(Widget):
    def __init__(self, **kwargs):
        super(VoltageGraph_Monocristaline, self).__init__(**kwargs)

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
        #self.width = dp(30) + (self.colLen + self.spacing) * self.columns
        self.width = dp(1000)
        self.height = dp(200)

        baseX = self.width * 0.5
        baseY = self.height*-0.4

        for i in range(self.columns):
            self.add_widget(Button(size=(15, self.adaptedHigh[i]), pos=(dp(30) + (self.colLen + self.spacing) * i, baseY)))


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
        baseY = self.height*-0.4
        for i in range(self.columns):
            with self.canvas:
                Color(1, 1, 1)
                Rectangle(size=(15, self.adaptedHigh[i]), pos=(dp(30) + (self.colLen + self.spacing) * i, baseY))



SunshineLabsApp().run()