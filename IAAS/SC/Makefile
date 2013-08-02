cc=g++
OBJS=CMDreceiver.o\
	CMDsender.o\
	CMDType.o\
	Config.o\
	CppMysql.o\
	Log.o \
	main.o \
	Object.o \
	SCnode.o \
	String.o\
	SC.o

ifeq ($(shell getconf LONG_BIT), 32)
    LIB= -L/usr/lib/mysql
else
    LIB= -L/usr/lib64/mysql
endif

CFLAGS = $(LIB) $(LIB)  -lmysqlclient -o -g 

SC:$(OBJS)
	${cc} $(OBJS)  $(CFLAGS)   -o $@
SC.o:SC.cpp
	g++ -c $< -o $@
CMDreceiver.o:CMDreceiver.cpp
	g++ -c $< -o $@
CMDsender.o:CMDsender.cpp
	g++ -c $< -o $@
CMDType.o:CMDType.cpp
	g++ -c $< -o $@
Config.o:../common/Config.cpp 
	g++ -c $< -o $@
CppMysql.o:../common/CppMysql.cpp 
	g++ -c $< -o $@
Log.o:../common/Log.cpp
	g++ -c $< -o $@
main.o:main.cpp 
	g++ -c $< -o $@
Object.o:Object.cpp 
	g++ -c $< -o $@
SC.o:SC.cpp
	g++ -c $< -o $@
SCnode.o:SCnode.cpp
	g++ -c $< -o $@
String.o:../common/String.cpp
	g++ -c $< -o $@

clean:
	rm -f *.o
	rm -f SC


