run:
	gcc -Wall -c queue.c -o queue.o
	gcc -Wall -c ppos_core.c -o ppos_core.o
	gcc -Wall queue.o ppos_core.o tests_ppos_tasks1.c -o teste