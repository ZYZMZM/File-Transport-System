#!/bin/bash

climd5=$2

funciton(){
if [ -f $1 ];then
    `openssl md5 -out md5.txt $1`
    res=`awk -F "=" '{print $2}' md5.txt`
else
    return 0;
fi


#echo $res
#echo $climd5

if [ $res = $climd5  ]; then
    return 1;
else
    return 0;
fi
}

funciton $1
echo $?
