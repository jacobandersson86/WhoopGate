#!/bin/bash
topic=whoopgate/all
working_dir='../../build/config'
config_json_path="$working_dir/sdkconfig.json"

msg1='{
    "color": 
    {
        "hue" : 0,
        "saturation" : 255,
        "value" : 30
    }
}'

msg2='{
    "color": 
    {
        "hue" : 63,
        "saturation" : 255,
        "value" : 30
    }
}'

msg3='{
    "color": 
    {
        "hue" : 127,
        "saturation" : 255,
        "value" : 30
    }
}'

msg4='{
    "color": 
    {
        "hue" : 191,
        "saturation" : 255,
        "value" : 30
    }
}'

msg5='{
    "color": 
    {
        "hue" : 191,
        "saturation" : 255,
        "value" : 0
    }
}'
host=$(jq -r ".MQTT_URI" "$config_json_path" | sed "s/mqtt:\/\///" | cut -d: -f1)
printf "$host\n"
port=$(jq -r ".MQTT_URI" "$config_json_path" | sed "s/mqtt:\/\///" | cut -d: -f2)
printf "$port\n"
pass=$(jq -r ".MQTT_PASSWORD" "$config_json_path")
printf "$pass\n"
user=$(jq -r ".MQTT_USER" "$config_json_path")
printf "$user\n"


mosquitto_pub -t $topic -m "$msq2" -h "$host" -p "$port" -u "$user" -P "$pass"

for i in {1..10}
do

mosquitto_pub -t $topic -m "$msg1" -h "$host" -p "$port" -u "$user" -P "$pass"
sleep 1

mosquitto_pub -t $topic -m "$msg2" -h "$host" -p "$port" -u "$user" -P "$pass"
sleep 1

mosquitto_pub -t $topic -m "$msg3" -h "$host" -p "$port" -u "$user" -P "$pass"
sleep 1

mosquitto_pub -t $topic -m "$msg4" -h "$host" -p "$port" -u "$user" -P "$pass"
sleep 1

done

mosquitto_pub -t $topic -m "$msg5" -h "$host" -p "$port" -u "$user" -P "$pass"
