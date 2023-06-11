run:
	gcc -Wall -c queue.c -o queue.o
	gcc -Wall -c ppos_core.c -o ppos_core.o
	gcc -Wall -c ppos_ipc.c -o ppos_ipc.o
	gcc -Wall queue.o ppos_core.o ppos_ipc.o pingpong-prodcons.c -o teste

semaphore:
	gcc -Wall -c queue.c -o queue.o
	gcc -Wall -c ppos_core.c -o ppos_core.o
	gcc -Wall -c ppos_ipc.c -o ppos_ipc.o
	gcc -Wall queue.o ppos_core.o ppos_ipc.o tests_sem.c -o teste

turnin:
	tar -zcvf aopf20.tar.gz *.h ppos_core.c queue.c

clean:
	rm *.o
	rm teste