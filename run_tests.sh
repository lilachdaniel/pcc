#!/bin/sh
gcc -O3 -D_DEFAULT_SOURCE -Wall -std=c11 pcc_client.c -o client

echo ------------------------------test1 -------------------------------------
./client 127.0.0.1 1235 /home/student/pcc/ofirs_repo/os_ex5_sockets/test/test1

echo ------------------------------test2 -------------------------------------
./client 127.0.0.1 1235 /home/student/pcc/ofirs_repo/os_ex5_sockets/test/test2

echo ------------------------------test3 -------------------------------------
./client 127.0.0.1 1235 /home/student/pcc/ofirs_repo/os_ex5_sockets/test/test3

echo ------------------------------test4 -------------------------------------
./client 127.0.0.1 1235 /home/student/pcc/ofirs_repo/os_ex5_sockets/test/test4

echo ------------------------------test5 -------------------------------------
./client 127.0.0.1 1235 /home/student/pcc/ofirs_repo/os_ex5_sockets/test/test5

echo ------------------------------test6 -------------------------------------
./client 127.0.0.1 1235 /home/student/pcc/ofirs_repo/os_ex5_sockets/test/test6

echo ------------------------------test7 -------------------------------------
./client 127.0.0.1 1235 /home/student/pcc/ofirs_repo/os_ex5_sockets/test/test7

echo ------------------------------test8 -------------------------------------
./client 127.0.0.1 1235 /home/student/pcc/ofirs_repo/os_ex5_sockets/test/test8

echo ------------------------------test9 -------------------------------------
./client 127.0.0.1 1235 /home/student/pcc/ofirs_repo/os_ex5_sockets/test/test9
