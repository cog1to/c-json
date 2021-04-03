test : json.o
	gcc -o test json.o

json.o : json.c
	gcc -c json.c

clean :
	rm *.o
