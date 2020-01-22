OPT=-I./ -I/usr/include/c++/9.2.0 -fPIC -std=c++11
CXX=g++
OBJECTS=node.o page.o

all: $(OBJECTS)

node.o: db/node.h db/node.cc
	$(CXX) $(OPT) -c -o node.o db/node.cc

page.o: db/page.h db/page.cc
	$(CXX) $(OPT) -c -o page.o db/page.cc

PHONY: clean
clean:
	-rm -rf $(OBJECTS)
