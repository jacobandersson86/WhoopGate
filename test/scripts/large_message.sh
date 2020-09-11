#!/bin/bash
topic=/whoopgate/all
msg1='{
    "color": 
    {
        "hue" : 10,
        "saturation" : 255,
        "value" : 20
    }
}'

msq2='I am a long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long v long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long  long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long v long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long v v long long long long long long long long long long long long long long long long message'


mosquitto_pub -t $topic -m "$msq2"

for i in {1..3}
do

mosquitto_pub -t $topic -m "$msg1"
sleep 2

done