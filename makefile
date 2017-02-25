GRAPH_DIR = graphs

all:
	g++ main.cpp -std=c++11
clean:
	rm -rf *.out $(GRAPH_DIR)/*.dot $(GRAPH_DIR)/*.ps

