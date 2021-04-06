test : json.o linkedlist.o genericlist.o
	gcc -o test -g json.o linkedlist.o genericlist.o

json.o : json.c
	gcc -g -c json.c

linkedlist.o : linkedlist.c
	gcc -g -c linkedlist.c

genericlist.o : genericlist.c
	gcc -g -c genericlist.c

clean :
	rm *.o
