INCLUDE = src/config.cpp src/config.h
CFLAGS = ${INCLUDE} -O3
LIBS = -I/usr/local/include/boost-1_38/

build:
	$(CXX) -o neatd $(CFLAGS) $(LIBS) src/main.cpp

clean:
	rm -rf neatd build