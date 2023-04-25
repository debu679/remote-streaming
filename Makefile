CC = g++
LIBS = `pkg-config --cflags --libs gstreamer-1.0`

all: remotesrc.o remotesrc.so remotemp3.o remotemp3.so remoteWebM.o remoteWebM.so remoteAvi.o remoteAvi.so exe 

remotesrc.o:	remotesrc.cpp
	$(CC) -c remotesrc.cpp $(LIBS) -fPIC
remotesrc.so:	remotesrc.o
	$(CC) -shared -o libremotesrc.so remotesrc.o $(LIBS)
remotemp3.o:	remotemp3.cpp
	$(CC) -c remotemp3.cpp $(LIBS) -fPIC
remotemp3.so:	remotemp3.o
	$(CC) -shared -o libremotemp3.so remotemp3.o $(LIBS)
remoteWebM.o:	remoteWebM.cpp
	$(CC) -c remoteWebM.cpp $(LIBS) -fPIC
remoteWebM.so:	remoteWebM.o
	$(CC) -shared -o libremoteWebM.so remoteWebM.o $(LIBS)
remoteAvi.o:	remoteAvi.cpp
	$(CC) -c remoteAvi.cpp $(LIBS) -fPIC
remoteAvi.so:	remoteAvi.o
	$(CC) -shared -o libremoteAvi.so remoteAvi.o $(LIBS)
exe: main.cpp 
	$(CC) -o exe main.cpp -lremotesrc -lremotemp3 -lremoteWebM -lremoteAvi $(LIBS) -I . -L .
run: exe
	./exe
clean:
	rm -rf *.o *.so exe