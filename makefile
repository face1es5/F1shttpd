all : httpd
test : httpd_t client
flags = -std=c++11 -Wall

httpd : httpd.cpp
	g++ -std=c++11 -Wall -Werror -o httpd httpd.cpp
httpd_t : httpd.o
	g++ $(flags) -o httpd_t httpd.o
client : client.o
	g++ $(flags) -o client client.o
%.o : %.cpp
	g++ $(flags) -c $< -o $@

.PHONY : clean
clean :
	rm httpd httpd_t client *.o
