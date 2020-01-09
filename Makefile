compile: socket

socket: Client.o frontendserver.o server.o
	g++ -pthread Client.o -o client 
	g++ -pthread frontendserver.o -o front
	g++ -pthread server.o -o server        

Berkeley.o: Client.cpp
	g++ -c Client.cpp -pthread

causalordering.o: frontendserver.cpp
	g++ -c frontendserver.cpp -pthread

noncausal.o: server.cpp
	g++ -c server.cpp -pthread

clean:
	rm -rf *o client server
 
