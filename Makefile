CC=g++
CFLAGS=-Wall -std=c++2a
LIBS=-lpthread
TARGET=server

all: $(patsubst %.cpp, %.o, $(wildcard *.cpp))
	$(CC) $(CFLAGS) *.o -o $(TARGET) $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

sudorun: $(TARGET)
	sudo ./$(TARGET)

clean:
	rm -f *.o
	rm -f $(TARGET)