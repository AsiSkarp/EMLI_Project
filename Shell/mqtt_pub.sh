#!/bin/bash

while :
do

read -r line < /dev/ttyACM0

#mosquitto_pub -h localhost -p 1883 -u emli19 -P 123 -t emli19/temparture -m $(bash temperature.sh)

alarm1=$(echo "$line" | cut -d ',' -f1)
alarm2=$(echo "$line" | cut -d ',' -f2)
moist=$(echo "$line" | cut -d ',' -f3)
light=$(echo "$line" | cut -d ',' -f4)

echo $(mosquitto_pub -h localhost -p 1883 -u emli19 -P 123 -t emli19/plant_water_alarm -m $alarm1)
echo $(mosquitto_pub -h localhost -p 1883 -u emli19 -P 123 -t emli19/pump_water_alarm -m $alarm2)
echo $(mosquitto_pub -h localhost -p 1883 -u emli19 -P 123 -t emli19/moisture -m $moist)
echo $(mosquitto_pub -h localhost -p 1883 -u emli19 -P 123 -t emli19/light -m $light)

#echo "Water alarm: $alarm1"
#echo "Pump water alarm: $alarm2"
#echo "Moisture: $moist"
#echo "Light sensor: $light"

sleep 1
done
