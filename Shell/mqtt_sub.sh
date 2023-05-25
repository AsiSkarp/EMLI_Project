#!/bin/bash

mosquitto_sub -h localhost -p 1883 -u emli19 -P 123 -t emli19/button | while read -r payload
do 
    if [[ $payload -eg 1 ]];
    then
        echo 'p' | sudo tee -a /dev/ttyACM0
    fi
done


