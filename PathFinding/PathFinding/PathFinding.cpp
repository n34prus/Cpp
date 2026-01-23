#include <chrono>
#include <set>
#include <iostream>
#include <windows.h>
#include <conio.h>

#include "Cluster.h"
#include "Dijkstra.h"
#include "UI.h"

#pragma comment(linker, "/ENTRY:mainCRTStartup")

// changable settings
//#define ASTAR 1
#define ENABLE_CONSOLE true
#define INCLUDE_DIAGONAL true
#define EXPENSIVE_DIAGONAL true
#define ENABLE_GRAPHIC true
#define SIZEX 2560
#define SIZEY 256

int main()
{
	// warning: about O(N) total cost
	// games usual uses hierarchical grids with clusters, so
	// todo: clusterization

	// init

	UI ui{ UI::Mode::Console };
	HANDLE hwnd = ui.InitConsole(SIZEY, SIZEX);

	// counters for average cost
	int calls = 0;
	long long genTime = 0;
	long long calcTime = 0;
	long long totalPathNodes = 0;
	
	while (true)
	{

		// boolean matrix with random noise inside
		ui.print("Call #", ++calls, "\n", "Generate random BitMatrix...\n");
		BitMatrix source{ SIZEX, SIZEY, true };
		//
		ui.print("Random BitMatrix generating done\n");

		// create matrix of Nodes
		ui.print("Prepare graph from source BitMatrix...\n");
		auto startTime = std::chrono::system_clock::now();

		auto printProgress = [](int progress)
			{
				std::cout << "\r" << progress << "%";
			};
		Cluster grid(source, printProgress);

		auto elapsedTime = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime).count();
		genTime += elapsedTime;
		ui.print("\rGraph matrix generating done in ", elapsedTime, " ms\n");

		// single-instanced variant works on original matrix
		/*
		Node* sourceNode = grid.getRandomNode();
		Node* targetNode = grid.getRandomNode();
		std::cout << "Start finding path from " << sourceNode << " to " << targetNode << std::endl;
		Dijkstra dijkstra{ grid, sourceNode, targetNode };
		*/

		// multiple-instanced variant works on matrix copy
		Dijkstra dijkstra{ grid, grid.getRandomNodeIdx(), grid.getRandomNodeIdx() };
		dijkstra.bIncludeDiagonal = INCLUDE_DIAGONAL;
		dijkstra.bExpensiveDiagonal = EXPENSIVE_DIAGONAL;
		dijkstra.Calculate(printProgress);

		elapsedTime = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime).count() - elapsedTime;
		calcTime += elapsedTime;
		totalPathNodes += dijkstra.resultPath.size();

		// prints statistics
		ui.print("\r", dijkstra.iterations, " iterations done in ", elapsedTime, " ms\n");
		if (!dijkstra.resultPath.empty()) ui.print("Path found and contains ", dijkstra.resultPath.size(), " nodes :)\n");
		else ui.print("Path not found :(\n");
		ui.print("Generating time ", genTime / calls, " ms per call | ",
			static_cast<float>(genTime * 1000) / (calls * SIZEX * SIZEY), " us per node\n",
			"Calculating time ", calcTime / calls, " ms per call | ");
		if (totalPathNodes) ui.print(static_cast<float>(calcTime) / (totalPathNodes), " ms per path node\n");

		// dijkstra: console result visualization
		if (ENABLE_GRAPHIC) dijkstra.Draw(hwnd, 10);

		ui.print("Press any key to start again");
		while (!_kbhit()) { Sleep(50); }
		_getch();
		ui.clear();
	}

    return 0;
}