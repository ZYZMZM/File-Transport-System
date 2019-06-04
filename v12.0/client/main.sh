#!/bin/bash

`openssl md5 -out md5.txt $1`
res=`awk -F "=" '{print $2}' md5.txt`
echo $res  > md5.txt
