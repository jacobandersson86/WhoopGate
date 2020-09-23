#!/bin/bash
topic=whoopgate/all

VAL=20

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



mosquitto_pub -t $topic -m "$msq2"

for i in {1..10}
do

mosquitto_pub -t $topic -m "$msg1"
sleep 1

mosquitto_pub -t $topic -m "$msg2"
sleep 1

mosquitto_pub -t $topic -m "$msg3"
sleep 1

mosquitto_pub -t $topic -m "$msg4"
sleep 1

done

mosquitto_pub -t $topic -m "$msg5"
