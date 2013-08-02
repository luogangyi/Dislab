ifeq ($(shell getconf LONG_BIT), 32)
	LIB= -L/usr/lib/mysql
else
	LIB= -L/usr/lib64/mysql
endif

SupervisorServer:main.o String.o CppMysql.o SupervisorVM.o Log.o Config.o
	g++ SupervisorVM.o main.o String.o CppMysql.o Log.o Config.o  $(LIB)  -lmysqlclient -lvirt -o SupervisorVM
main.o:main.cpp
	g++ -c $< -o $@
String.o:../common/String.cpp
	g++ -c $< -o $@
CppMysql.o:../common/CppMysql.cpp
	g++ -c $< -o $@
SupervisorServer.o:SupervisorVM.cpp
	g++ -c $< -o $@
Log.o:../common/Log.cpp
	g++ -c $< -o $@
Config.o:../common/Config.cpp
	g++ -c $< -o $@

clean:
	rm -f *.o
	rm -f SupervisorVM
