.PHONY:all 
all:math_cgi
math_cgi:math_cgi.cpp
	g++ -o math_cgi math_cgi.cpp
	 

.PHONY:output
output: all
	cp -f math_cgi ../../output/htdoc/cgi_bin

.PHONY:clean
clean:
	 rm -f math_cgi

