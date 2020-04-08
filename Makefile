OPT=-g -I./ -I/usr/include/c++/9.3.0 -fPIC -std=c++17 -Werror
OPT_TEST=$(OPT) -I./third_party/googletest/googletest/include
LINK_TEST=./third_party/googletest/googletest/build/lib/libgtest.a -lpthread
CXX=g++
OBJECTS=node.o page.o page_ele.o db_impl.o crc32c.o status.o
TEST_OBJECTS=node_test.o main_test.o db_test.o
ALL_OBJECTS=$(OBJECTS) $(TEST_OBJECTS)
MAIN_TEST=main_test

all: $(ALL_OBJECTS)

node.o: db/node.h db/node.cc
	$(CXX) $(OPT) -c -o node.o db/node.cc

page.o: db/page.h db/page.cc
	$(CXX) $(OPT) -c -o page.o db/page.cc

page_ele.o: db/page_ele.h db/page_ele.cc
	$(CXX) $(OPT) -c -o page_ele.o db/page_ele.cc

crc32c.o: db/crc32c.h db/crc32c.cc
	$(CXX) $(OPT) -c -o crc32c.o db/crc32c.cc

status.o: db/status.cc
	$(CXX) $(OPT) -c -o status.o db/status.cc

db_impl.o: db/db_impl.h db/db_impl.cc db/bucket_impl.h
	$(CXX) $(OPT) -c -o db_impl.o db/db_impl.cc

node_test.o: db/node.h db/node_test.cc
	$(CXX) $(OPT_TEST) -c -o node_test.o db/node_test.cc

test_node: db/node.h db/node_test.cc main_test.o
	$(CXX) $(OPT_TEST) -c -o node_test.o db/node_test.cc
	$(CXX) -o $(MAIN_TEST) $(OBJECTS) main_test.o node_test.o $(LINK_TEST)
	./$(MAIN_TEST)

db_test.o: db/db_impl_test.cc
	$(CXX) $(OPT_TEST) -c -o db_test.o db/db_impl_test.cc

test_db: db_test.o main_test.o $(OBJECTS)
	$(CXX) -o $(MAIN_TEST) $(OBJECTS) main_test.o db_test.o $(LINK_TEST)
	./$(MAIN_TEST)

main_test.o: db/main_test.cc
	$(CXX) $(OPT_TEST) -c -o main_test.o db/main_test.cc

run_all_test: $(OBJECTS) $(TEST_OBJECTS)
	$(CXX) -o $(MAIN_TEST) $(ALL_OBJECTS) $(LINK_TEST)
	./$(MAIN_TEST)

PHONY: clean
clean:
	-rm -rf $(ALL_OBJECTS) $(MAIN_TEST)
