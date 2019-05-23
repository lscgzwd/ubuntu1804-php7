CC=g++ -std=c++11
MAGIC_CONFIG=`Magick++-config --cppflags --cxxflags --ldflags --libs`
JSON_CONFIG=-ljsoncpp -I/usr/include/jsoncpp
PHP_INCLUDES=`php-config --includes`
PHP_EXTENSION_DIR=`php-config --extension-dir`

CCFLAG=$(MAGIC_CONFIG) $(JSON_CONFIG)

DEBUG=
DEBUG_FLAG=
ifdef DEBUG
  DEBUG_FLAG= -DYIMAGE_DEBUG
endif

all: test so

yimage.o: 
	$(CC) -O2 -fpermissive -o yimage.o -c yimage.cpp $(CCFLAG)

test.o: 
	$(CC) -O2 -o test.o -c test.cpp $(CCFLAG)

test: yimage.o test.o
	$(CC) -O2 -o test yimage.o test.o $(CCFLAG)

yjf-deskew: yimage.o yjf-deskew.o
	$(CC) -O2 -o yjf-deskew yimage.o yjf-deskew.o $(CCFLAG)

yimage_wrap.cpp:
	swig -c++ -php yimage.i

yimage_wrap.o: yimage_wrap.cpp
	$(CC) -fpic $(PHP_INCLUDES) -fpermissive -c yimage_wrap.cpp -o yimage_wrap.o

yimage.so.o:
	$(CC) -fpic -O2 -o yimage.so.o -c yimage.cpp $(CCFLAG) $(DEBUG_FLAG)

so: yimage_wrap.o yimage.so.o
	$(CC) -shared -o yimage.so yimage_wrap.o yimage.so.o $(CCFLAG)

install:
	cp yimage.so $(PHP_EXTENSION_DIR)

.PHONY: clean check
clean:
	-rm test *.o yimage_wrap.cpp *.so php_yimage.h

check:
	php t.php
