# Tiny httpd
This software is based on J. David Blackstone's httpd, i rewrite a C++ version of this software. 
It's a simple http server that can accept GET and POST request.

# File constructure
html - store static html page file.
cgi-bin - store cgi program, request that is POST or GET with query string will invoke requested cgi program.
client.cpp - a client that use socket to communicate with server, print the response simply, you will need to **modify the port** in source code when run it in your local machine.
test.sh - similary to client.cpp, to test whether the server application runs correctly using curl(**you need to modify port manually as well**).

# Compile
`make`
Then you will generate a program named httpd, this is the server program. You will be noticed with the port that server is listening on, you can using test.sh to test it(or browser).