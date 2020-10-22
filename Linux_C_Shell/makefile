OFLAGS=  -o
CFLAGS=  -c

myshell: myshell.o
	gcc $(OFLAGS) myshell myshell.o 
myshell.o: myshell.c
	gcc $(CFLAGS) myshell.c 
clean:
	rm *.o myshell

