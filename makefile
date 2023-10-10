all:	
	g++ ./code/client-phase1.cpp -o client-phase1 -std=c++17 -pthread
	g++ ./code/client-phase2.cpp -o client-phase2 -std=c++17 -pthread
	g++ ./code/client-phase3.cpp -o client-phase3 -std=c++17 -pthread
	g++ ./code/client-phase4.cpp -o client-phase4 -std=c++17 -pthread
	g++ ./code/client-phase5.cpp -o client-phase5 -std=c++17 -pthread

clean:
	rm client-phase1 client-phase2 client-phase3 client-phase4 client-phase5 