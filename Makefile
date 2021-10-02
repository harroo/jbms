
output: bin/main.o bin/server.o
	g++ bin/main.o bin/server.o -o output

bin/main.o: src/main.cpp
	g++ -c src/main.cpp -o bin/main.o

bin/server.o: src/server.cpp src/server.hpp
	g++ -c src/server.cpp -o bin/server.o

run:
	./output

clean:
	rm bin/*.o

ready:
	mkdir bin
