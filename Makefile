
output: bin/main.o
	g++ bin/main.o -o output

bin/main.o: src/main.cpp
	g++ -c src/main.cpp -o bin/main.o

run:
	./output

clean:
	rm bin/*.o

ready:
	mkdir bin
