CC=clang++
FLAGS=-Wall -Wextra --std=c++17
DIR=build
PARFLAGS=-lpthread -ltbb
CPPFILES=document.cpp process_queries.cpp read_input_functions.cpp remove_duplicates.cpp \
		 request_queue.cpp search_server.cpp string_processing.cpp
MAIN=main.cpp 
TEST=./unit-testing/search-server-unit-tests.cpp

all: search_server

search_server:
	$(CC) $(FLAGS) $(PARFLAGS) -g -O0 $(CPPFILES) $(MAIN) -o search_server.out

release:
	$(CC) $(FLAGS) $(PARFLAGS) -O3 $(CPPFILES) $(MAIN) -o search_server.out

test:
	$(CC) $(FLAGS) $(PARFLAGS) -g -O0 $(CPPFILES) $(TEST) -o ./unit-testing/test.out

clean:
	rm -rf build/* *.out

rebuild: clean all
