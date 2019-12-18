#!/bin/bash                                          
# 批量执行命令
# 此脚本的作用是在多个机器上执行同一命令，而不需要分别登录每台机器执行命令

# server_list_file中每一行格式 username@ip_address e.g. lg@222.29.39.26
# server_list_file: each line format: username@ip_address e.g. lg@222.29.39.26
# 如果无需认证，则可以直接返回结果，否则需根据提示输入登录密码

if [ "$#" -ne 2 ] ; then
    echo "USAGE: $0 server_list_file command"
    exit -1
fi

file_name=$1
cmd_str=$2

cwd=$(pwd)
cd $cwd
serverlist_file="$cwd/$file_name"

if [ ! -e $serverlist_file ] ; then
    echo 'server_list_file not exist';
    exit 0
fi

echo -e "\nCommand: " $cmd_str "\n"

while read line
do
    #echo $line
    if [ -n "$line" ] ; then
        echo -e "\nHost --->>>>>" $line "<<<<<<<"
        #echo -e "Command: " $cmd_str "\n"

        #ssh $line $cmd_str < /dev/null > /dev/null
        ssh $line $cmd_str < /dev/null
        if [ $? -eq 0 ] ; then
            echo -e "--->>>> command: $cmd_str done!\n"
        else
            echo -e "error: " $? "\n"
        fi
    fi
done < $serverlist_file
