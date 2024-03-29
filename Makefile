test:
	g++ -std=c++20 ./src/*.cpp ./src/AsyncIO/*.cpp test.cpp -g -luring -o test.out
server_test:
	g++ -std=c++20 echo_server.cpp ./src/*.cpp ./src/AsyncIO/*.cpp -luring -g -o server_test.out
pack:
	g++ -std=c++20 ./src/*.cpp ./src/AsyncIO/*.cpp -c -luring
	ar rcs libgoroubine.a *.o
	rm *.o
apply:
	g++ -std=c++20 main.cpp -L. -lgoroubine -luring -o apply.out
server:
	g++ -std=c++20 echo_server.cpp -L. -lgoroubine -luring -o server.out
client:
	g++ echo_client.cpp -o client.out
clean:
	rm *.o *.a *.out