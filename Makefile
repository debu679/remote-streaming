CC = g++
LIBS = `pkg-config --cflags --libs gstreamer-1.0`

all: hostsrc.o hostsrc.so exe

hostsrc.o:	hostsrc.cpp
	$(CC) -c hostsrc.cpp $(LIBS) -fPIC
hostsrc.so:	hostsrc.o
	$(CC) -shared -o libhostsrc.so hostsrc.o $(LIBS)
exe: main.cpp 
	$(CC) -o exe main.cpp -lhostsrc $(LIBS) -I . -L .
run: exe
	./exe
clean:
	rm -rf *.o *.so exe