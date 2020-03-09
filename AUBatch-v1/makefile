COMPILER = gcc

aubatch: aubatch.o
	${COMPILER} aubatch.o -o aubatch -lpthread

aubatch.o: aubatch.c aubatch.h
	${COMPILER} -c aubatch.c

clean:
	rm *.o aubatch



