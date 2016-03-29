objs := main_server.o rio.c webServer.o wrap.o err.o
CFLAGS := -g -O2 -Wall



webServer: $(objs)
	gcc $(CFLAGS) -o $@ $^

plus: plus.o
	gcc $(CFLAGS) -o $@ $^
	mv $@ ./cgi-bin/$@

%.o : %.c
	gcc -c $(CFLAGS) -o $@ $^

clean:
	rm -rf *.o webServer plus

