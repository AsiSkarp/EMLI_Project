#!/bin/bash
# This script monitors CPU and memory usage, CPU temperature and the date.

while :
do 
  # Get the current values of temp, cpu, memory.
  cpuUsage=$(top -bn1 | awk '/Cpu/ { print $2}')
  memUsage=$(free -m | awk '/Mem/{print $3}')
  number="$(cat /sys/class/thermal/thermal_zone0/temp)"
  number2="1000"
  temperature=$(($number / $number2))


  # Print the usage.
  echo $(date)
  echo "CPU Usage: $cpuUsage%"
  echo "Memory Usage: $memUsage MB"
  echo "CPU Temperature: $temperature Celcius"

  # Checking for internet connection by pinging google.
  echo -e "GET http://google.com HTTP/1.0\n\n" | nc google.com 80 > /dev/null 2>&1

  if [ $? -eq 0 ]; then
    echo "Internet connection working!" 
  else
    echo "Problems with internet connection!"
  fi

  # Save the values to logs.
  echo $(date) >> system_monitoring_logs.txt
  echo "CPU Usage: $cpuUsage%" >> system_monitoring_logs.txt
  echo "Memory usage: $memUsage MB" >> system_monitoring_logs.txt
  echo "CPU Temperature: $temperature Celsius" >> system_monitoring_logs.txt

  if [ $? -eq 0 ]; then
    echo "Internet connection working!" >> system_monitoring_logs.txt
  else
    echo "Problems with internet connection" >> system_monitoring_logs.txt
  fi

  # Sleep for 1 second.
  sleep 1
done
