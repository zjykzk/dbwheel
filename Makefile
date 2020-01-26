OPT=-g -I./ -I/usr/include/c++/9.2.0 -fPIC -std=c++11 -Werror
OPT_TEST=$(OPT) -I./third_party/googletest/googletest/include
LINK_TEST=./third_party/googletest/googletest/build/lib/libgtest.a -lpthread
CXX=g++
OBJECTS=node.o page.o page_ele.o\
		node_test.o main_test.o
MAIN_TEST=main_test

all: $(OBJECTS)

node.o: db/node.h db/node.cc
	$(CXX) $(OPT) -c -o node.o db/node.cc

node_test.o: db/node.h db/node_test.cc
	$(CXX) $(OPT_TEST) -c -o node_test.o db/node_test.cc

page.o: db/page.h db/page.cc
	$(CXX) $(OPT) -c -o page.o db/page.cc

page_ele.o: db/page_ele.h db/page_ele.cc
	$(CXX) $(OPT) -c -o page_ele.o db/page_ele.cc

main_test.o: db/main_test.cc
	$(CXX) $(OPT_TEST) -c -o main_test.o db/main_test.cc

run_test: $(OBJECTS)
	$(CXX) -o $(MAIN_TEST) $(OBJECTS) $(LINK_TEST)
	./$(MAIN_TEST)

PHONY: clean
clean:
	-rm -rf $(OBJECTS) $(MAIN_TEST)
