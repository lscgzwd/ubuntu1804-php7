CC=g++ -std=c++11 -O2
MAGIC_CONFIG=`Magick++-config --cppflags --cxxflags --ldflags --libs`
JSON_CONFIG=-ljsoncpp -I/usr/include/jsoncpp

PHP_INCLUDES=`php-config --includes`
PHP_EXTENSION_DIR=`php-config --extension-dir`

CCFLAG=$(MAGIC_CONFIG) $(JSON_CONFIG)
LDFLAG=$(MAGIC_CONFIG) $(JSON_CONFIG)

all: test extract-lines show-lines deskew-image \
	   	calc-weight skew-offset yjf-deskew image_tool.so

image.o: image.cpp
	$(CC) -o image.o -c image.cpp $(CCFLAG)

skew-offset.o: skew-offset.cpp
	$(CC) -o skew-offset.o -c skew-offset.cpp $(CCFLAG)

skew-offset: skew-offset.o image.o utils.o
	$(CC) -o skew-offset skew-offset.o image.o utils.o $(LDFLAG)

calc-weight.o: calc-weight.cpp
	$(CC) -o calc-weight.o -c calc-weight.cpp $(CCFLAG)

calc-weight: calc-weight.o
	$(CC) -o calc-weight calc-weight.o $(LDFLAG)

deskew.o: deskew.cpp
	$(CC) -o deskew.o -c deskew.cpp $(CCFLAG)

yjf-deskew: deskew.o image.o utils.o
	$(CC) -o yjf-deskew deskew.o image.o utils.o $(LDFLAG)

deskew-image.o: deskew-image.cpp
	$(CC) -o deskew-image.o -c deskew-image.cpp $(CCFLAG)

deskew-image: deskew-image.o image.o utils.o
	$(CC) -o deskew-image deskew-image.o image.o utils.o $(LDFLAG)

show-lines.o: show-lines.cpp
	$(CC) -o show-lines.o -c show-lines.cpp $(CCFLAG)

show-lines: show-lines.o
	$(CC) -o show-lines show-lines.o $(LDFLAG)

extract-lines.o: extract-lines.cpp
	$(CC) -o extract-lines.o -c extract-lines.cpp $(CCFLAG)

extract-lines: extract-lines.o image.o utils.o
	$(CC) -o extract-lines extract-lines.o image.o utils.o $(LDFLAG)

test.o: test.cpp image.cpp
	$(CC) -o test.o -c test.cpp $(CCFLAG)

utils.o: utils.cpp
	$(CC) -o utils.o -c utils.cpp $(CCFLAG)

test: test.o image.o utils.o
	$(CC) -o test test.o utils.o image.o $(LDFLAG)

image_tool_wrap.cxx:
	swig -c++ -php7 image_tool.i

image_tool_wrap.o: image_tool_wrap.cxx
	$(CC) -fpic $(PHP_INCLUDES) -fpermissive -c image_tool_wrap.cxx -o image_tool_wrap.o

image_tool.so.o: image.cpp
	$(CC) -fpic -O2 -o image_tool.so.o -c image.cpp $(CCFLAG) $(DEBUG_FLAG)

utils.so.o: utils.cpp
	$(CC) -fpic -O2 -o utils.so.o -c utils.cpp $(CCFLAG) $(DEBUG_FLAG)

image_tool.so: image_tool_wrap.o image_tool.so.o utils.so.o
	$(CC) -shared -o image_tool.so image_tool_wrap.o image_tool.so.o utils.so.o $(CCFLAG)

.PHONY: clean install
clean:
	-rm -rf test *.o *.so image_tool_wrap.cxx

install:
	install yjf-deskew /usr/local/bin
	sudo cp image_tool.so $(PHP_EXTENSION_DIR)
