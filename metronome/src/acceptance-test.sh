 #!/bin/sh

###
### Unit tests for metronome.c
###
### Usage
###   ./acceptance-test.sh 
###
### Author
### Soojin Han
###   han00070@algonquinlive.com
###

echo "\n"
echo "Unit Test ID 1: ./metronome"
echo "Expected: usage message"
./metronome
sleep 5

echo "\n"
echo "Unit Test ID 2: ./metronome 120 2 4"
echo "Expected: 1 measure per second."
echo "Output:"
./metronome 120 2 4 &
sleep 10

echo "\n"
echo "Unit Test ID 3: cat /dev/local/metronome"
echo "Expected: [metronome: 120 beats/min, time signature 2/4, secs-per-interval: 0.25, nanoSecs: 250000000]"
echo "Output:"
cat /dev/local/metronome
sleep 10

echo "\n"
echo "Unit Test ID 4: cat /dev/local/metronome-help"
echo "Expected: information regarding the metronome resmgr’s API."
echo "Output:"
cat /dev/local/metronome-help
sleep 10

echo "\n"
echo "Unit Test ID 5: echo set 100 2 4 > /dev/local/metronome"
echo "Expected: metronome regmgr changes settings to: 100 bpm in 2/4 time; run-time behaviour of
metronome changes to 100 bpm in 2/4 time."
echo "Output:"
echo set 100 2 4 > /dev/local/metronome
cat /dev/local/metronome
sleep 10

echo "\n"
echo "Unit Test ID 6: cat /dev/local/metronome"
echo "Expected: [metronome: 100 beats/min, time signature 2/4, secs-per-interval: 0.30, nanoSecs: 300000000]"
echo "Output:"
cat /dev/local/metronome
sleep 10

echo "\n"
echo "Unit Test ID 7: echo set 200 5 4 > /dev/local/metronome"
echo "Expected: metronome regmgr changes settings to: 200 bpm in 5/4 time; run-time behaviour of
metronome changes to 200 bpm in 5/4 time"
echo "Output:"
echo set 200 5 4 > /dev/local/metronome
cat /dev/local/metronome
sleep 10

echo "\n"
echo "Unit Test ID 8: cat /dev/local/metronome"
echo "Expected: [metronome: 200 beats/min, time signature 5/4, secs-per-interval: 0.15, nanoSecs: 150000000]"
echo "Output:"
cat /dev/local/metronome
sleep 10

echo "\n"
echo "Unit Test ID 9: echo stop > /dev/local/metronome"
echo "Expected: metronome stops running; metronome resmgr is still running as a process: pidin | grep
metronome."
echo stop > /dev/local/metronome && pidin | grep metronome
sleep 10

echo "\n"
echo "Unit Test ID 10: echo start > /dev/local/metronome"
echo "Expected: metronome starts running again at 200 bpm in 5/4 time, which is the last setting; metronome
resmgr is still running as a process: pidin | grep metronome"
echo "Output:"
cat /dev/local/metronome
echo start > /dev/local/metronome
sleep 10

echo "\n"
echo "Unit Test ID 11: cat /dev/local/metronome"
echo "Expected: [metronome: 200 beats/min, time signature 5/4, secs-per-interval: 0.15, nanoSecs: 150000000]"
echo "Output:"
cat /dev/local/metronome
sleep 10

echo "\n"
echo "Unit Test ID 12: echo stop > /dev/local/metronome"
echo "Expected: metronome stops running; metronome resmgr is still running as a process: pidin | grep
metronome."
echo stop > /dev/local/metronome && pidin | grep metronome
sleep 10

echo "\n"
echo "Unit Test ID 13: echo stop > /dev/local/metronome"
echo "Expected: metronome remains stopped; metronome resmgr is still running as a process: pidin | grep
metronome."
echo stop > /dev/local/metronome && pidin | grep metronome
sleep 10

echo "\n"
echo "Unit Test ID 14: echo start > /dev/local/metronome"
echo "Expected: metronome starts running again at 200 bpm in 5/4 time, which is the last setting; metronome
resmgr is still running as a process: pidin | grep metronome"
echo "Output:"
cat /dev/local/metronome
echo start > /dev/local/metronome
sleep 10

echo "\n"
echo "Unit Test ID 15: echo start > /dev/local/metronome"
echo "Expected: metronome is still running again at 200 bpm in 5/4 time, which is the last setting; metronome
resmgr is still running as a process: pidin | grep metronome"
echo "Output:"
cat /dev/local/metronome
echo start > /dev/local/metronome
sleep 10

echo "\n"
echo "Unit Test ID 16: cat /dev/local/metronome"
echo "Expected: [metronome: 200 beats/min, time signature 5/4, secs-per-interval: 0.15, nanoSecs: 150000000]"
echo "Output:"
cat /dev/local/metronome
sleep 10

echo "\n"
echo "Unit Test ID 17: echo pause 3 > /dev/local/metronome"
echo "Expected: metronome continues on next beat (not next measure)."
echo pause 3 > /dev/local/metronome
sleep 10

echo "\n"
echo "Unit Test ID 18: echo pause 10 > /dev/local/metronome"
echo "Expected: properly formatted error message, and metronome continues to run."
echo "Output:"
echo pause 10 > /dev/local/metronome
sleep 10

echo "\n"
echo "Unit Test ID 19: echo bogus > /dev/local/metronome"
echo "Expected: properly formatted error message, and metronome continues to run."
echo "Output:"
echo bogus > /dev/local/metronome
sleep 10

echo "\n"
echo "Unit Test ID 20: echo set 120 2 4 > /dev/local/metronome"
echo "Expected: 1 measure per second. "
echo "Output:"
echo set 120 2 4 > /dev/local/metronome
cat /dev/local/metronome
sleep 10

echo "\n"
echo "Unit Test ID 21: cat /dev/local/metronome"
echo "Expected: [metronome: 120 beats/min, time signature 2/4, secs-per-interval: 0.25, nanoSecs: 250000000]"
echo "Output:"
cat /dev/local/metronome
sleep 10

echo "\n"
echo "Unit Test ID 22: cat /dev/local/metronome-help"
echo "Expected: information regarding the metronome resmgr’s API."
cat /dev/local/metronome-help
sleep 10

echo "\n"
echo "Unit Test ID 23: echo Writes-Not-Allowed > /dev/local/metronome-help"
echo "Expected: properly formatted error message, and metronome continues to run."
echo "Output:"
echo Writes-Not-Allowed > /dev/local/metronome-help
sleep 10

echo "\n"
echo "Unit Test ID 24: echo quit > /dev/local/metronome && pidin | grep metronome"
echo "Expected: metronome gracefully terminates."
echo quit > /dev/local/metronome && pidin | grep metronome