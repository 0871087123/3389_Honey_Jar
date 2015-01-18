TARGET=3389breaker
SRC=breaker.c
MAX=3000
PORT=3389

all : $(TARGET)
.PHONY : $(TARGET) test clean
$(TARGET) : $(SRC)
	$(CC) -g -Wall -DMAX=$(MAX) -DPORT=$(PORT) -o $@ $^

test : $(SRC)
	gcc -g -Wall -DMAX=$(MAX) -DPORT=$(PORT) -o $@ $^

clean :
	rm -rf $(TARGET) test
