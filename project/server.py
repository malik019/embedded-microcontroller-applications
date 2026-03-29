import re
import sys
from typing import NamedTuple
import paho.mqtt.client as mqtt
from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS

# --- InfluxDB 2.x Configuration ---
INFLUXDB_URL = "http://192.168.0.9:8086"
INFLUXDB_TOKEN = "tRkLfjW2m0SqZMgJitIEE24N_VtDCyF9Tq2Emg7JKjvhmsOEs9EK2xOGAPZoQsklhf2X6qX4k-UuvDDFolgIxg=="
INFLUXDB_ORG = "Thomas More"
INFLUXDB_BUCKET = "HomeAutomation"

# --- MQTT Configuration ---
MQTT_ADDRESS = '192.168.0.9'
MQTT_USER = 'cherry'
MQTT_PASSWORD = 'mqtt-test'
MQTT_TOPIC = 'fabriek/+/+' # Changed to match your Regex pattern
MQTT_REGEX = r'fabriek/([^/]+)/([^/]+)'
#MQTT_CLIENT_ID = 'clientId-QQw1Ox8zHk'
MQTT_CLIENT_ID = 'MQTTInfluxDBBridge'

# Create InfluxDB 2.x client
influx_client = InfluxDBClient(url=INFLUXDB_URL, token=INFLUXDB_TOKEN, org=INFLUXDB_ORG)
write_api = influx_client.write_api(write_options=SYNCHRONOUS)

class SensorData(NamedTuple):
    location: str
    measurement: str
    value: float

def on_connect(client, userdata, flags, rc, properties=None):
    print(f"Connected to MQTT with result code: {rc}")
    client.subscribe(MQTT_TOPIC)

def on_message(client, userdata, msg):
    payload = msg.payload.decode('utf-8')
    print(f"MQTT Received: {msg.topic} -> {payload}")
    
    match = re.match(MQTT_REGEX, msg.topic)
    if match:
        location = match.group(1)
        measurement = match.group(2)
        try:
            val = float(payload)
            # Create InfluxDB Point
            point = Point(measurement) \
                .tag("location", location) \
                .field("value", val)
            
            write_api.write(bucket=INFLUXDB_BUCKET, org=INFLUXDB_ORG, record=point)
            print(f"✅ Data written to InfluxDB: {measurement} from {location}")
        except ValueError:
            print("Payload is not a number")
    else:
        print("Topic did not match regex")

def main():
    print("Starting MQTT to InfluxDB bridge...")
    
    mqtt_client = mqtt.Client(client_id=MQTT_CLIENT_ID)
    mqtt_client.username_pw_set(MQTT_USER, MQTT_PASSWORD)
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message

    try:
        mqtt_client.connect(MQTT_ADDRESS, 1883, 60)
        mqtt_client.loop_forever()
    except KeyboardInterrupt:
        print("Stopping...")
    finally:
        influx_client.close()

if __name__ == "__main__":
    main()

