server: server.o digraph.o dijkstra.o
	g++ server.o digraph.o dijkstra.o -o server

dijkstra.o: dijkstra.h dijkstra.cpp
	g++ -std=c++11 -Wall -g -c dijkstra.cpp

digraph.o: digraph.h digraph.cpp
	g++ -std=c++11 -Wall -g -c digraph.cpp

server.o: server.cpp
	g++ -std=c++11 -Wall -g -c server.cpp

clean:
	-rm -f server.o digraph.o dijkstra.o server
