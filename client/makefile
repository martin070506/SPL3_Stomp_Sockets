CFLAGS:=-c -Wall -Weffc++ -g -std=c++11 -Iinclude
LDFLAGS:=-lboost_system -lpthread

all: StompWCIClient

# Link Step: Now includes StompProtocol, ProcessorInput, and ProcessorOutput
StompWCIClient: bin/ConnectionHandler.o bin/StompClient.o bin/event.o bin/StompProtocol.o bin/ProcessorInput.o bin/ProcessorOutput.o
	g++ -o bin/StompWCIClient bin/ConnectionHandler.o bin/StompClient.o bin/event.o bin/StompProtocol.o bin/ProcessorInput.o bin/ProcessorOutput.o $(LDFLAGS)

# Compile ConnectionHandler
bin/ConnectionHandler.o: src/ConnectionHandler.cpp
	g++ $(CFLAGS) -o bin/ConnectionHandler.o src/ConnectionHandler.cpp

# Compile Main (StompClient)
bin/StompClient.o: src/StompClient.cpp
	g++ $(CFLAGS) -o bin/StompClient.o src/StompClient.cpp

# Compile Event
bin/event.o: src/event.cpp
	g++ $(CFLAGS) -o bin/event.o src/event.cpp

# Compile StompProtocol (You need this since you moved logic here!)
bin/StompProtocol.o: src/StompProtocol.cpp
	g++ $(CFLAGS) -o bin/StompProtocol.o src/StompProtocol.cpp

# Compile ProcessorInput
bin/ProcessorInput.o: src/ProcessorInput.cpp
	g++ $(CFLAGS) -o bin/ProcessorInput.o src/ProcessorInput.cpp

# Compile ProcessorOutput
bin/ProcessorOutput.o: src/ProcessorOutput.cpp
	g++ $(CFLAGS) -o bin/ProcessorOutput.o src/ProcessorOutput.cpp

.PHONY: clean
clean:
	rm -f bin/*