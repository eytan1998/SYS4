
.PHONY: clean all
all: react_server

HashMap.o: HashMap.c HashMap.h
	gcc -g -c -fPIC HashMap.c

reactor.o: reactor.c reactor.h HashMap.h
	gcc -g -c -fPIC reactor.c

st_reactor.so : reactor.o HashMap.o
	gcc -shared -o libst_reactor.so reactor.o HashMap.o

react_server: react_server.c st_reactor.so
	gcc -g react_server.c -o react_server -L. -lst_reactor -Wl,-rpath,.

clean:
	rm -f *.o *.so react_server
