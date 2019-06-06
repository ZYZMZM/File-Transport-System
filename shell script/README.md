# shell script

脚本文件主要用来进行数据库的一些操作，与编写的数据库API接口辅助完成相应功能。

| Filename                                                     | Function                                                     |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [clear.sh](https://github.com/ZYZMZM/File-Transport-System/blob/master/shell%20script/clear.sh) | 清空文件信息数据库和登录日志数据库                           |
| [com.sh](https://github.com/ZYZMZM/File-Transport-System/blob/master/shell%20script/com.sh) | 将客端收到的命令存入文件中                                   |
| [fileMd5.sh](https://github.com/ZYZMZM/File-Transport-System/blob/master/shell%20script/fileMd5.sh) | 服务器在首次启动时初始化文件信息数据库（遍历当前目录并计算Md5值存入） |
| [main.sh](https://github.com/ZYZMZM/File-Transport-System/blob/master/shell%20script/main.sh) | 计算客户端将待上传文件的MD5值                                |
| [sermain.sh](https://github.com/ZYZMZM/File-Transport-System/blob/master/shell%20script/sermain.sh) | v9.0版本使用，匹配Md5值（12.0版本使用数据库API匹配）         |
| [server_start_info.sh](https://github.com/ZYZMZM/File-Transport-System/blob/master/shell%20script/server_start_info.sh) | 服务器每次启动都启动次数和启动时间信息存入登录日志数据库     |

