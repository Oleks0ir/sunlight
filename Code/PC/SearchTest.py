import requests

hostname = "MyESP8266Device.local"  # or just "MyESP8266Device" depending on your network
url = f"http://{hostname}/"  # Adjust this to match the endpoint on the ESP8266

try:
    response = requests.get(url)
    print("Device found:", response.text)
except requests.exceptions.RequestException as e:
    print("Could not find device:", e)
