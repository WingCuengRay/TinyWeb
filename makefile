objs := main_server.o rio.c webServer.o wrap.o err.o
CFLAGS := -g 



webServer: $(objs)
	gcc $(CFLAGS) -o $@ $^

%.o : %.c
	gcc -c $(CFLAGS) -o $@ $^

clean:
	rm -rf *.o webServer

