/usr/bin/pkill main

sleep 5

lsof -i:10000
lsof -i:10001
lsof -i:10002
lsof -i:10086
lsof -i:12345

sleep 3

/home/ITC100/main &

