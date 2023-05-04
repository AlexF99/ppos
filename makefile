run:
	gcc -Wall -c queue.c -o queue.o
	gcc -Wall -c ppos_core.c -o ppos_core.o
	gcc -Wall queue.o ppos_core.o tests_preempt.c -o teste

scheduler:
	gcc -Wall -c queue.c -o queue.o
	gcc -Wall -c ppos_core.c -o ppos_core.o
	gcc -Wall queue.o ppos_core.o tests_scheduler.c -o teste

dispatch:
	gcc -Wall -c queue.c -o queue.o
	gcc -Wall -c ppos_core.c -o ppos_core.o
	gcc -Wall queue.o ppos_core.o tests_dispatcher.c -o teste

tasks:
	gcc -Wall -c queue.c -o queue.o
	gcc -Wall -c ppos_core.c -o ppos_core.o
	gcc -Wall queue.o ppos_core.o tests_ppos_tasks2.c -o teste

queue:
	gcc -Wall -c queue.c -o queue.o
	gcc -Wall queue.o tests_testafila.c -o teste


clean: 
	rm a.out
	rm *.o
	rm teste