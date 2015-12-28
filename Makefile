CC=clang++
CFLAGS=-O2 -Wall -std=c++14
SOURCE_FILES=$(wildcard *.cpp)
HEADER_FILES=$(wildcard *.hpp)

all:
	$(CC) $(CFLAGS) $(SOURCE_FILES) -o zdb

clean:
	rm -f *.o zdb