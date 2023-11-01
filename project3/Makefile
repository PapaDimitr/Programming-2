project3f: project3.o objdb.o
	gcc -Wall -g project3.o objdb.o -o project3f -fsanitize=address
objdb.o: objdb.c objdb.h
	gcc -Wall -g -c objdb.c -o objdb.o
project3.o: project3.c objdb.h
	gcc -Wall -g -c project3.c -o project3.o
