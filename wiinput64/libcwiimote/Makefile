# Makefile for libcwiimote + examples

TEST_DIR=test
SRC_DIR=src

all: libcwiimote.a test/test1 test/test2 test/test3 test/test4

libcwiimote.a:
	@(cd $(SRC_DIR) && $(MAKE))

test/test1:
	@(cd $(TEST_DIR) && $(MAKE) test1)

test/test2:
	@(cd $(TEST_DIR) && $(MAKE) test2)

test/test3:
	@(cd $(TEST_DIR) && $(MAKE) test3)

test/test4:
	@(cd $(TEST_DIR) && $(MAKE) test4)

clean:
	@(cd $(SRC_DIR) && $(MAKE) clean)
	@(cd $(TEST_DIR) && $(MAKE) clean)
