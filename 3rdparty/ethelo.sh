#!/bin/bash

#based on http://stackoverflow.com/questions/356100/how-to-wait-in-bash-for-several-subprocesses-to-finish-and-return-exit-code-0

FAIL=0

cd ./plog && ./ethelo.sh && cd ..
if [ $? -ne 0 ];
then
  echo "PLOG build failed"
  exit 1
fi

cd ./rapidjson && ./ethelo.sh && cd ..
if [ $? -ne 0 ];
then
  echo "Rapidjson build failed"
  exit 1
fi

echo "starting parallel build"

cd ./Bonmin
./ethelo.sh &
cd ../CppAD
./ethelo.sh &
cd ../libantlr3c
./ethelo.sh &

for job in `jobs -p`
do
  wait $job || let "FAIL+=1"
done

if [ "$FAIL" == "0" ];
then
  echo "All 3rdparty builds successful."
else
  echo "FAIL! ($FAIL)"
  exit 1 
fi
