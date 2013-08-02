ifeq ($(shell getconf LONG_BIT),32)
	LIB= -L/usr/lib/mysql
else
	LIB= -L/usr/lib64/mysql
endif

NCPerformance:main.o InfoManager.o RealTimeInfo.o Log.o String.o Config.o CppMysql.o
	g++ InfoManager.o main.o RealTimeInfo.o Log.o String.o Config.o  CppMysql.o $(LIB) -L/usr/include/mysql -lmysqlclient -lvirt -o NCPerformance
main.o:main.cpp
	g++ -c $< -o $@
InfoManager.o:InfoManager.cpp
	g++ -c $< -o $@
RealTimeInfo.o:RealTimeInfo.cpp
	g++ -c $< -o $@
Log.o:Log.cpp
	g++ -c $< -o $@
String.o:String.cpp
	g++ -c $< -o $@
Config.o:Config.cpp
	g++ -c $< -o $@
CppMysql.o:CppMysql.cpp
	g++ -c $< -o $@

clean:
	rm -f *.o
	rm -f NCPerformance
