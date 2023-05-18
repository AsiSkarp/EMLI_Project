#!/bin/bash

echo $(mosquitto_sub -h localhost -p 1883 -u emli19 -P 123 -t emli19/button)


while :
do 

    #msg=$(echo "$payload" | cut -d ' ' -f 2)
    #echo ${msg}
    
done


