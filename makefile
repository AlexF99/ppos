run:
	gcc -Wall -c queue.c -o queue.o
	gcc -Wall -c ppos_core.c -o ppos_core.o
	gcc -Wall queue.o ppos_core.o tests_suspend.c -o teste

turnin:
	tar -zcvf aopf20.tar.gz *.h ppos_core.c queue.c

clean:
	rm *.o
	rm teste