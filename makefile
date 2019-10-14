make: ex52.o ex51.o

ex52.o: ex52.c
	gcc -o Draw.out ex52.c

ex51.o: ex51.c
	gcc ex51.c

run: a.out
	./a.out
