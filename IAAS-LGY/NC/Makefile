cc=g++
OBJS=Config.o\
	 main.o\
	 VM.o\
	 StartVM.o\
	 Process.o\
	 UnknownCmd.o\
	 Log.o\
	 CppMysql.o\
	 CloseVM.o\
	 String.o\
	 PrepareDir.o\
	 MigrateVM.o ExpandVMMem.o\
	 XML.o\
	 GetVNCConsole.o\
	 libvirtEventHandle.o\
	 HardwareInfo.o\
	 NCHeartBeat.o
		 
ifeq ($(shell getconf LONG_BIT), 32)
    LIB= -L/usr/lib/mysql
else
    LIB= -L/usr/lib64/mysql
endif 

CFLAGS = -lpthread $(LIB)  -lxml2  -lmysqlclient -lvirt -g



nc_vm:$(OBJS)
	@v=`(cat version)`;v=`expr $$v + 1`;echo $$v>version
	${cc} $(OBJS)  $(CFLAGS)   -o $@

main.o:main.cpp
	${cc} -c -g $< -o $@

Config.o:../common/Config.cpp
	${cc} -c -g $< -o $@

VM.o:VM.cpp
	${cc} -c -g $< -o $@

StartVM.o:StartVM.cpp
	${cc} -c -g $< -o $@

Process.o:Process.cpp
	${cc} -c -g $< -o $@

UnknownCmd.o:UnknownCmd.cpp
	${cc} -c -g $< -o $@

Log.o:../common/Log.cpp
	${cc} -c -g $< -o $@

CloseVM.o:CloseVM.cpp
	${cc} -c -g $< -o $@

String.o:../common/String.cpp
	${cc} -c -g $< -o $@
PrepareDir.o:PrepareDir.cpp
	${cc} -c -g $< -o $@

MigrateVM.o:MigrateVM.cpp
	${cc} -c -g $< -o $@

StartCopyVM.o:StartCopyVM.cpp
	${cc} -c -g $< -o $@

ExpandVMMem.o:ExpandVMMem.cpp
	${cc} -c -g $< -o $@

XML.o:XML.cpp
	${cc} -c -g $< -o $@
CppMysql.o:../common/CppMysql.cpp
	${cc} -c -g $< -o $@
	

GetVNCConsole.o:GetVNCConsole.cpp
	${cc} -c -g -I /usr/include/libxml2/ $< -o $@

libvirtEventHandle.o:libvirtEventHandle.cpp
	${cc} -c -g $< -o $@

HardwareInfo.o:HardwareInfo.cpp
	${cc} -c -g $< -o $@

NCHeartBeat.o:NCHeartBeat.cpp
	${cc} -c -g $< -o $@
	

clean:
	rm -f *.o
	rm -f nc_vm

#g++ mysql.c++ -L/usr/lib/mysql  -lmysqlclient
