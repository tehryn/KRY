CCFLAGS = -std=c++11 -g
all: breaker

breaker: breaker.o base64.o
	g++ $(CCFLAGS) -o $@ $^

breaker.o: breaker.cpp base64.h
	g++ $(CCFLAGS) -c $< -o $@

base64.o: base64.cpp base64.h
	g++ $(CCFLAGS) -c $< -o $@

.PHONY: clean
	
clean:
	rm -f breaker.o base64.o breaker