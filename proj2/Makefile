CCFLAGS = -std=c++11 -g
all: kry

kry: kry.o
	g++ $(CCFLAGS) -o $@ $^ -lgmp

kry.o: kry.cpp
	g++ $(CCFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm kry.o kry