#include <chrono>
#include <set>
#include <iostream>
#include <ctime>
#include <windows.h>
#include <conio.h>

#include "Node.h"
#include "Cluster.h"
#include "Dijkstra.h"

#pragma comment(linker, "/ENTRY:mainCRTStartup")

// changable settings
//#define ASTAR 1
#define ENABLE_CONSOLE true
#define INCLUDE_DIAGONAL true
#define EXPENSIVE_DIAGONAL true
#define ENABLE_GRAPHIC true
#define SIZEX 256
#define SIZEY 256


HANDLE InitConsole(short width, short height)
{
	AllocConsole(); // call FreeConsole() to disable

	HANDLE hwnd = CreateConsoleScreenBuffer(
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		nullptr,
		CONSOLE_TEXTMODE_BUFFER,
		nullptr
	);
	SetConsoleActiveScreenBuffer(hwnd);

	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);
	freopen_s(&f, "CONOUT$", "w", stderr);
	freopen_s(&f, "CONIN$", "r", stdin);

	constexpr int fontSize = 10;

	CONSOLE_FONT_INFOEX fontInfo;
	fontInfo.cbSize = sizeof(fontInfo);
	GetCurrentConsoleFontEx(hwnd, TRUE, &fontInfo);
	fontInfo.dwFontSize.X = fontSize;
	fontInfo.dwFontSize.Y = fontSize;
	SetCurrentConsoleFontEx(hwnd, TRUE, &fontInfo);
	COORD bufferSize = { static_cast<short>(width * 2), static_cast<short>(height * 2) };
	SetConsoleScreenBufferSize(hwnd, bufferSize);

	if (width > 64 && height > 64)
	{
		RECT r;
		GetWindowRect(GetConsoleWindow(), &r);
		MoveWindow(GetConsoleWindow(), r.left, r.top, (fontSize + 1) * (width), (fontSize + 1) * height, TRUE);
	}

	return hwnd;

}

int main()
{
	// warning: about O(N) total cost
	// games usual uses hierarchical grids with clusters, so
	// todo: clusterization

	// init
	HANDLE hwnd = InitConsole(SIZEY, SIZEX);

	// counters for average cost
	int calls = 0;
	long long genTime = 0;
	long long calcTime = 0;
	long long totalPathNodes = 0;
	
	// restart point FOR DEBUG ONLY
restart:

	std::cout << "\033[H" << "Call #" << ++calls << std::endl;;
	std::cout << "Generate random BitMatrix..." << std::endl;
	// boolean matrix with random noise inside
	BitMatrix source{SIZEX, SIZEY, true};
	std::cout << "Random BitMatrix generating done" << std::endl;
	
	// create matrix of Nodes
	std::cout << "Prepare graph from source BitMatrix..." << std::endl;
	auto startTime = std::chrono::system_clock::now();

	auto printProgress = [](int progress)
		{
			std::cout << "\r" << progress << "%";
		};
	Cluster grid(source, printProgress);

	auto elapsedTime = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime).count();
	genTime += elapsedTime;
	std::cout << "\rGraph matrix generating done in " << elapsedTime << " ms" << std::endl;

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
	std::cout << "\r" << dijkstra.iterations << " iterations done in " << elapsedTime << " ms" << std::endl;
	if (!dijkstra.resultPath.empty())
	{
		std::cout << "Path found and contains " << dijkstra.resultPath.size() << " nodes :)" << std::endl;
	}
	else
	{
		std::cout << "Path not found :(" << std::endl;
	}

	// dijkstra: console result visualization
	if (ENABLE_GRAPHIC) dijkstra.Draw(hwnd, 10);

	std::cout << "Generating time " << genTime / calls << " ms per call | ";
	std::cout << static_cast<float>(genTime) / (calls * SIZEX * SIZEY) << " ms per path node" << std::endl;
	std::cout << "Calculating time " << calcTime / calls << " ms per call | ";
	if (totalPathNodes)
		std:: cout << calcTime / totalPathNodes << " ms per path node" << std::endl;
	std::cout << "Press any key to start again";
	while (!_kbhit()) { Sleep(50); }
	_getch();
	system("cls");
	// DEADLY bad way, only for debug, sorry for that

	goto restart;


    return 0;
}