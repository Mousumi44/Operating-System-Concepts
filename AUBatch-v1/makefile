COMPILER = gcc

aubatch: aubatch.o menu.o
	${COMPILER} aubatch.o menu.o -o aubatch -lpthread

aubatch.o: aubatch.c aubatch.h
	${COMPILER} -c -w aubatch.c

menu.o: menu.c menu.h
	${COMPILER} -c menu.c

clean:
	rm *.o aubatch



