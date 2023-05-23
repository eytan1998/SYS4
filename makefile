
.PHONY: clean all
all: react_server

HashMap.o: HashMap.c HashMap.h
	gcc -c -fPIC HashMap.c

reactor.o: reactor.c reactor.h HashMap.h
	gcc -c -fPIC reactor.c

react_server.o: react_server.c reactor.h
	gcc -c react_server.c

st_reactor.so : reactor.o HashMap.o
	gcc -shared -o libst_reactor.so reactor.o HashMap.o

react_server: st_reactor.so react_server.o
	gcc react_server.o -o react_server -L. -lst_reactor -Wl,-rpath,. -pthread

clean:
	rm -f *.o *.so react_server