all : httpd
flags = -std=c++11 -Wall

httpd : httpd.cpp
	g++ -std=c++11 -Wall -Werror -o httpd httpd.cpp
client : client.o
	g++ $(flags) -o client client.o
%.o : %.cpp
	g++ $(flags) -c $< -o $@

.PHONY : clean
clean :
	rm httpd client *.o
