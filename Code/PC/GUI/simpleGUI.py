import self
from kivy.graphics import *
from kivy.app import App
from kivy.metrics import dp
from kivy.uix.anchorlayout import AnchorLayout
from kivy.uix.widget import Widget
from kivy.uix.button import Button
from kivy.uix.togglebutton import ToggleButton
from kivy.uix.popup import Popup
from kivy.uix.label import Label

import server_api as server
import mathEngine as mthen
import logAPI
import json


class SunshineLabsApp(App):
    pass


class Widget_Monocristaline_Power(AnchorLayout):
    pass


class Widget_VoltAmpers(AnchorLayout):

    def sendDate(self, time):
        esp.Ltv = time.text
        esp.calltime()
        pass

    def updateLogs(self):
        esp.requestLogs()
        log = logAPI.logFile("logDay", "r", "logDay.log")

        popup = Popup(title='Program warning',
                      content=Label(text='Open and close the program'),
                      size_hint=(None, None), size=(400, 400))
        popup.open()


    def connect(self, toggle):
        if toggle.state != "normal":
            esp.callback()
            esp.setcallback()
            toggle.text = "Disconnect ESP"
        else:
            toggle.text = "Connect ESP"
            esp.forceRestart()


class VoltageGraph_Voc(Widget):
    def __init__(self, **kwargs):
        super(VoltageGraph_Voc, self).__init__(**kwargs)

        self.points = log.voltage_OC
        self.columns = len(self.points)
        self.spacing = dp(5)

        self.size_hint = (None, None)
        self.height = dp(self.columns*25)
        self.width = dp(self.columns*25)
        self.maxLen = self.height
        self.colLen = (self.maxLen - (self.columns - 1) * self.spacing) / (self.columns+1)
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
            self.add_widget(Button(text = str(self.points[i]), font_size = 10,size=(15, self.adaptedHigh[i]), pos=(dp(30) + (self.colLen + self.spacing) * i, baseY)))


class VoltageGraph_Res(Widget):
    def __init__(self, **kwargs):
        super(VoltageGraph_Res, self).__init__(**kwargs)

        self.points = log.voltage_Res

        self.columns = len(self.points)
        self.spacing = dp(5)

        self.size_hint = (None, None)
        self.height = dp(self.columns*25)
        self.width = dp(self.columns*25)
        self.maxLen = self.height
        self.colLen = (self.maxLen - (self.columns - 1) * self.spacing) / (self.columns+1)
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
            self.add_widget(Button(text=str(self.points[i]), font_size=10, size=(15, self.adaptedHigh[i]),
                                   pos=(dp(30) + (self.colLen + self.spacing) * i, baseY)))


class Temperature_Graph(Widget):
    def __init__(self, **kwargs):
        super(Temperature_Graph, self).__init__(**kwargs)

        self.points = log.temperature
        self.columns = len(self.points)
        self.spacing = dp(5)

        self.size_hint = (None, None)
        self.height = dp(self.columns * 25)
        self.width = dp(self.columns * 25)
        self.maxLen = self.height
        self.colLen = (self.maxLen - (self.columns - 1) * self.spacing) / (self.columns+1)
        self.maxHigh = dp(200)

        self.maxPointValue = mthen.findMaxVar(self.points)
        self.percentHigh = mthen.toPercent(self.points, self.maxPointValue)
        self.adaptedHigh = mthen.multMatrixbyNum(self.percentHigh, self.maxHigh)

        # Set the width of the widget based on the number of columns
        self.width = dp(30) + (self.colLen + self.spacing) * self.columns

        self.height = dp(200)

        baseX = self.width * 0.5
        baseY = self.height * -0.4

        for i in range(self.columns):
            self.add_widget(Button(text = f"{self.points[i]/100}", font_size = 9,size=(15, self.adaptedHigh[i]), pos=(dp(30) + (self.colLen + self.spacing) * i, baseY)))



if __name__ == "__main__":
    userConfig = json.loads(open("UserConfig.json", "r").read())

    esp = server.server(IP=userConfig["stations"][0]["IP"], host_id=userConfig["user"]["id"], host=userConfig["user"]["hostname"], password=userConfig["user"]["password"],
                        LtV=userConfig["stations"][0]["LtV"])

    log = logAPI.logFile("logDay", "r", "logDay.log")

    SunshineLabsApp().run()