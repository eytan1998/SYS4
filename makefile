
.PHONY: clean all
all: react_server st_reactor.so

react_server: react_server.c st_reactor.so
	gcc react_server.c -o $@ -L. -lst_reactor

reactor.o: reactor.c
	gcc -c reactor.c

st_reactor.so : reactor.o
	gcc -shared reactor.o -o libst_reactor.so
clean:
	rm - f *.o
