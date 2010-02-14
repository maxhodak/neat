INCLUDE = src/config.cpp src/config.h
CFLAGS = ${INCLUDE} -O3
LIBS = -I/usr/local/include/boost-1_38/

build:
	$(CC) -o neat $(CFLAGS) $(LIBS) src/main.cpp

clean:
	rm -rf neat build
