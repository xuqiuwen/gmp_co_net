main:
	g++ -std=c++20 ./impl2/*.cpp -g
test:
	g++ test.cpp ./impl4/AsyncIO/IOUringAsyncIO.cpp -g -luring
clean:
	rm a.out