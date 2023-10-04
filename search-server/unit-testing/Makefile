
CC=clang++
FLAGS=-Wall -Wextra --std=c++20
DIR=build
PARFLAGS=-lpthread -ltbb

all: test

test: ./search-server-unit-tests.cpp ../document.cpp ../process_queries.cpp ../read_input_functions.cpp \
	  ../remove_duplicates.cpp ../request_queue.cpp ../search_server.cpp ../string_processing.cpp
	$(CC) $(FLAGS) $(PARFLAGS) -g -O0 $^ -o test.out

clean:
	rm -rf build/* *.out

rebuild: clean all
