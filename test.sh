#!/usr/bin/bash
curl -v http://localhost:55243/
echo
curl -v http://localhost:55243/index.html
echo
curl -v http://localhost:55243/nonexsitent.html
echo
curl -v -d 'POST WORKS' http://localhost:55243/test
echo
curl -v -d 'msg=HELLO' http://localhost:55243/jesus
