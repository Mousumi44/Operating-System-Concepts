COMPILER = gcc

aubatch: aubatch.o menu.o module.o
	${COMPILER} aubatch.o menu.o module.o -o aubatch -lpthread

aubatch.o: aubatch.c module.h
	${COMPILER} -c -w aubatch.c

module.o: module.c module.h
	${COMPILER} -c -w module.c

menu.o: menu.c menu.h
	${COMPILER} -c menu.c

clean:
	rm *.o aubatch



