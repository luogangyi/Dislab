cc=g++
OBJS=Config.o\
	main.o\
	Object.o\
	Log.o\
	String.o\
	CppMysql.o\
	AutoScaling.o\
	VMMigrate.o
	 
TARGET=clc_AutoScaling
	 
ifeq ($(shell getconf LONG_BIT), 32)
    LIB= -L/usr/lib/mysql
else
    LIB= -L/usr/lib64/mysql
endif  

CFLAGS =$(LIB) -lpthread -lmysqlclient -lmcheck -g 

$(TARGET):$(OBJS)
	${cc} $(OBJS)  $(CFLAGS)   -o $@

main.o:main.cpp
	${cc} -c -g $< -o $@

Config.o:../../common/Config.cpp
	${cc} -c -g $< -o $@

Object.o:Object.cpp
	${cc} -c -g $< -o $@

AutoScaling.o:AutoScaling.cpp
	${cc} -c -g $< -o $@


Log.o:../../common/Log.cpp
	${cc} -c -g $< -o $@


String.o:../../common/String.cpp
	${cc} -c -g $< -o $@


CppMysql.o:../../common/CppMysql.cpp
	${cc} -c -g $< -o $@

VMMigrate.o:VMMigrate.cpp
	${cc} -c -g $< -o $@



clean:
	rm -f *.o
	rm -f $(TARGET)


