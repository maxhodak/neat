INCLUDE = src/config.cc src/config.h
CFLAGS = ${INCLUDE} -O3
LIBS = -I/usr/local/include/boost-1_38/

build:
	$(CXX) -o neatd $(CFLAGS) $(LIBS) src/main.cc

clean:
	rm -rf neatd build