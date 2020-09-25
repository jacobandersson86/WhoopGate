#!/bin/bash
topic_gate1=whoopgate/gatebfc0
topic_gate2=whoopgate/gate1871
topic_gate3=whoopgate/gateae20
topic_all=whoopgate/all


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

msg_off=$msg4
msg_on=$msg3

host=$(jq -r ".MQTT_URI" "$config_json_path" | sed "s/mqtt:\/\///" | cut -d: -f1)
printf "$host\n"
port=$(jq -r ".MQTT_URI" "$config_json_path" | sed "s/mqtt:\/\///" | cut -d: -f2)
printf "$port\n"
pass=$(jq -r ".MQTT_PASSWORD" "$config_json_path")
printf "$pass\n"
user=$(jq -r ".MQTT_USER" "$config_json_path")
printf "$user\n"


mosquitto_pub -t $topic_all -m "$msg_off" -h "$host" -p "$port" -u "$user" -P "$pass"
sleep 2

while true
do

mosquitto_pub -t $topic_gate1 -m "$msg_on" -h "$host" -p "$port" -u "$user" -P "$pass"
sleep 0.25

mosquitto_pub -t $topic_gate1 -m "$msg_off" -h "$host" -p "$port" -u "$user" -P "$pass"
mosquitto_pub -t $topic_gate2 -m "$msg_on" -h "$host" -p "$port" -u "$user" -P "$pass"
sleep 0.25

mosquitto_pub -t $topic_gate2 -m "$msg_off" -h "$host" -p "$port" -u "$user" -P "$pass"
mosquitto_pub -t $topic_gate3 -m "$msg_on" -h "$host" -p "$port" -u "$user" -P "$pass"
sleep 0.25
mosquitto_pub -t $topic_gate3 -m "$msg_off" -h "$host" -p "$port" -u "$user" -P "$pass"

sleep 1


done

mosquitto_pub -t $topic_all -m "$msg5" -h "$host" -p "$port" -u "$user" -P "$pass"
