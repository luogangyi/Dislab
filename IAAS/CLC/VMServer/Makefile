cc=g++
OBJS=Config.o\
	 main.o\
	 Object.o\
	 Process.o\
	 UnknownCmd.o\
	 Log.o\
	 String.o\
	 CppMysql.o\
	 Login.o\
	 CreateVM.o\
	 StartVM.o\
	 CloseVM.o\
	 DeleteVM.o\
	 QueryVM.o\
	 GetVNCConsole.o
	
ifeq ($(shell getconf LONG_BIT), 32)
    LIB= -L/usr/lib/mysql
else
    LIB= -L/usr/lib64/mysql
endif 

CFLAGS = -lpthread $(LIB)  -lmysqlclient -lvirt  -lmcheck -g 

clc_VMServer:$(OBJS)
	${cc} $(OBJS)  $(CFLAGS)   -o $@

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

Login.o:Login.cpp
	${cc} -c -g $< -o $@

CreateVM.o:CreateVM.cpp
	${cc} -c -g $< -o $@

StartVM.o:StartVM.cpp
	${cc} -c -g $< -o $@

CloseVM.o:CloseVM.cpp
	${cc} -c -g $< -o $@

DeleteVM.o:DeleteVM.cpp
	${cc} -c -g $< -o $@

QueryVM.o:QueryVM.cpp
	${cc} -c -g $< -o $@

GetVNCConsole.o:GetVNCConsole.cpp
	${cc} -c -g $< -o $@
	

clean:
	rm -f *.o
	rm -f clc_VMServer


