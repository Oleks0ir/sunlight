import json
from kivy.app import App
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.label import Label
from kivy.uix.textinput import TextInput
from kivy.uix.switch import Switch
from kivy.uix.button import Button

import server_api as server
import json


# Load initial configuration from JSON
def load_config():
    with open("configESP.json", "r") as f:
        return json.load(f)


# Save updated configuration to JSON
def save_config(config):
    with open("configESP.json", "w") as f:
        json.dump(config, f, separators=(',', ':'), ensure_ascii=False)


class ConfiguratorApp(App):
    def build(self):
        self.config_data = load_config()

        root = BoxLayout(orientation='vertical', spacing=10, padding=10)

        # Network Section
        root.add_widget(Label(text="Networks", font_size=24))
        self.network_inputs = []
        for net in self.config_data["network"]:
            box = BoxLayout(orientation='horizontal')
            ssid_input = TextInput(text=net["SSID"])
            pass_input = TextInput(text=net["PASSWORD"])
            self.network_inputs.append((ssid_input, pass_input))
            box.add_widget(Label(text=f"ID {net['ID']}:"))
            box.add_widget(ssid_input)
            box.add_widget(pass_input)
            root.add_widget(box)

        # Clients Section
        root.add_widget(Label(text="Clients", font_size=24))
        self.client_inputs = []
        for client in self.config_data["clients"]:
            box = BoxLayout(orientation='horizontal')
            host_input = TextInput(text=client["HOST"])
            pass_input = TextInput(text=client["PASSWORD"])
            rights_switches = [Switch(active=right) for right in client["RIGHTS"]]
            self.client_inputs.append((host_input, pass_input, rights_switches))
            box.add_widget(host_input)
            box.add_widget(pass_input)
            for switch in rights_switches:
                box.add_widget(switch)
            root.add_widget(box)

        # Statics Section
        root.add_widget(Label(text="Statics", font_size=24))
        self.timer_inputs = {}
        for timer in self.config_data["statics"][0]["timers"]:
            key = list(timer.keys())[0]
            value = timer[key]
            box = BoxLayout(orientation='horizontal')
            box.add_widget(Label(text=key))
            timer_input = TextInput(text=str(value))
            self.timer_inputs[key] = timer_input
            box.add_widget(timer_input)
            root.add_widget(box)

        # Allow Section
        root.add_widget(Label(text="Allow", font_size=24))
        self.allow_inputs = []
        for allow in self.config_data["statics"][0]["allow"]:
            box = BoxLayout(orientation='horizontal')
            box.add_widget(Label(text=allow))
            allow_input = TextInput(text=allow)
            self.allow_inputs.append(allow_input)
            box.add_widget(allow_input)
            root.add_widget(box)

        # Save Button line
        ButtonBox = BoxLayout(orientation='horizontal')
        save_button = Button(text="Save Configuration", size_hint_y=None, height=50, size_hint_x=0.5)
        upload_button = Button(text="Upload to ESP", size_hint_y=None, height=50, size_hint_x=0.5)
        save_button.bind(on_press=self.save_configuration)
        upload_button.bind(on_press=self.UploadConfig)

        ButtonBox.add_widget(upload_button)
        ButtonBox.add_widget(save_button)

        root.add_widget(ButtonBox)

        return root

    def save_configuration(self, instance):
        # Update networks
        for i, net in enumerate(self.config_data["network"]):
            ssid, password = self.network_inputs[i]
            net["SSID"] = ssid.text
            net["PASSWORD"] = password.text

        # Update clients
        for i, client in enumerate(self.config_data["clients"]):
            host, password, rights_switches = self.client_inputs[i]
            client["HOST"] = host.text
            client["PASSWORD"] = password.text
            client["RIGHTS"] = [switch.active for switch in rights_switches]

        # Update timers
        for key, timer_input in self.timer_inputs.items():
            self.config_data["statics"][0]["timers"] = [
                {key: int(timer_input.text)} for key, timer_input in self.timer_inputs.items()
            ]

        # Update allow list
        self.config_data["statics"][0]["allow"] = [allow_input.text for allow_input in self.allow_inputs]

        # Save updated configuration
        save_config(self.config_data)
        print("Configuration saved.")

    def UploadConfig(self, instance):
        esp.uploadJson()



if __name__ == "__main__":
    userConfig = json.loads(open("UserConfig.json", "r").read())

    esp = server.server(IP=userConfig["stations"][0]["IP"], host_id=userConfig["user"]["id"],
                        host=userConfig["user"]["hostname"], password=userConfig["user"]["password"],
                        LtV=userConfig["stations"][0]["LtV"])

    ConfiguratorApp().run()
