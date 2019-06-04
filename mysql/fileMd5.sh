#!/bin/bash
# get all filename in specified path

cat /dev/null > filename.txt

HOSTNAME="127.0.0.1"
PORT="3306"
USERNAME="root"
PASSWORD="123456"

DBNAME="md5"  #数据库名称
TABLENAME="md5table" #数据库中表的名称

#遍历服务器根目录，得到所有的文件
files=`ls`
for filename in $files
do
    `md5sum $filename > new.txt`
    res=`awk -F " " '{print $1}' new.txt`
    echo $filename $res>> filename.txt
done

#逐一计算每个文件的md5值
while read line;
do
    res=`awk -F " " '{print $1}' filename.txt`
    userName="root"
    query=`echo $line | awk -F " " '{ printf("\"%s\",\"%s\"", $1, $2)}'`
    query=$query",""\"$userName\"","1"
    statement=`echo "INSERT INTO $TABLENAME VALUES($query);"`

    mysql -u $USERNAME -p123456 $DBNAME << EOF
    INSERT INTO $TABLENAME (filename,md5Value,username,intact) VALUES($query);
EOF
done < filename.txt


