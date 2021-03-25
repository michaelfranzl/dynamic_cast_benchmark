CC=clang
CFLAGS=-O3 -std=c++20 -Wall -I./Priori/include -I./kcl/Source
TARGET=dynamic_cast_benchmark
LDFLAGS=-lstdc++ -L./Priori/src

COMPILE = $(CC) $(CFLAGS) -c

all: $(TARGET)

priori.o: Priori/src/priori.cpp
	$(COMPILE) $<

$(TARGET): priori.o $(TARGET).cpp
	$(CC) $(CFLAGS) -o $@ priori.o $(TARGET).cpp $(LDFLAGS)

clean:
	rm -f $(TARGET) Priori/src/priori.o
