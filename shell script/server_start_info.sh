#!/bin/sh

HOSTNAME="127.0.0.1"
PORT="3306"
USERNAME="root"
PASSWORD="123456"

DBNAME="serverLog"  #数据库名称
TABLENAME="serverStartInfo" #数据库中表的名称

#服务器启动，清空备份文件信息
rm *~

#每次服务器启动，将记录信息存储到日志数据库中

Sdate=`date`
Sdate="\"$Sdate\""
#echo $Sdate
#$mysql -u $USERNAME -p$PASSWORD $DBNAME
mysql -u ${USERNAME} -p$PASSWORD ${DBNAME} << EOF
INSERT INTO ${TABLENAME} (date) VALUES($Sdate)
EOF
