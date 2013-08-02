cc=g++
OBJS=Config.o\
	 main.o\
	 Object.o\
	 Process.o\
	 UnknownCmd.o\
	 Log.o\
	 String.o\
	 Dispatch.o\
	 CppMysql.o
	 

ifeq ($(shell getconf LONG_BIT), 32)
    LIB= -L/usr/lib/mysql
else
    LIB= -L/usr/lib64/mysql
endif 

CFLAGS = -lpthread $(LIB) -lmysqlclient -g 


clc_dispatch:$(OBJS)
	${cc} $(OBJS)  $(CFLAGS) -o $@ 

main.o:main.cpp
	${cc} -c -g $< -o $@

Config.o:../../common/Config.cpp
	${cc} -c -g $< -o $@

Object.o:Object.cpp
	${cc} -c -g $< -o $@


Process.o:Process.cpp
	${cc} -c -g $< -o $@

UnknownCmd.o:UnknownCmd.cpp
	${cc} -c -g $< -o $@

Log.o:../../common/Log.cpp
	${cc} -c -g $< -o $@


String.o:../../common/String.cpp
	${cc} -c -g $< -o $@

Dispatch.o:Dispatch.cpp
	${cc} -c -g $< -o $@

CppMysql.o:../../common/CppMysql.cpp
	${cc} -c -g $< -o $@

clean:
	rm -f *.o
	rm -f clc_dispatch

#g++ mysql.c++ -L/usr/lib/mysql  -lmysqlclient
