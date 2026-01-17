#include <chrono>
#include <map>
#include <vector>
#include <set>
//#include <iomanip>
#include <bitset>
#include <iostream>
#include <ctime>
#include <windows.h>
#include <conio.h>

#include "Node.h"
#include "NodeMatrix.h"
#include "Dijkstra.h"

// changeble settings
//#define ASTAR 1
//#define INCLUDE_DIAGONAL 1
//#define EXPENSIVE_DIAGONAL 1
//#define ENABLE_GRAPHIC 1

// other defines
//#define SCLR(value) SetConsoleTextAttribute(hwnd, value | FOREGROUND_INTENSITY)


void InitConsole(const HANDLE& hwnd, short width, short height)
{
	constexpr int fontSize = 10;

	CONSOLE_FONT_INFOEX fontInfo;
	fontInfo.cbSize = sizeof(fontInfo);
	GetCurrentConsoleFontEx(hwnd, TRUE, &fontInfo);
	//wcsncpy(wchar_t*("Arial Cyr"), fontInfo.FaceName, LF_FACESIZE);
	fontInfo.dwFontSize.X = fontSize;
	fontInfo.dwFontSize.Y = fontSize;
	SetCurrentConsoleFontEx(hwnd, TRUE, &fontInfo);

	/*
	COORD bufferSize = { width, height };
	SetConsoleScreenBufferSize(hwnd, bufferSize);
	SMALL_RECT windowSize = {0,0, width, height };
	SetConsoleWindowInfo(hwnd, TRUE, &windowSize);
	*/

	RECT r;
	GetWindowRect(GetConsoleWindow(), &r);
	MoveWindow(GetConsoleWindow(), r.left, r.top, (fontSize+1)*(width), (fontSize+1)*height, TRUE);
}

int main()
{
	// warning: about O(N) total cost
	size_t SIZEX = 600;
	size_t SIZEY = 300;

	// init
	HANDLE hwnd = GetStdHandle(STD_OUTPUT_HANDLE);
	InitConsole(hwnd, SIZEY, SIZEX);

	// counters for average cost
	int calls = 0;
	long long genTime = 0;
	long long calcTime = 0;
	long long totalPathNodes = 0;
	
	// restart point FOR DEBUG ONLY
restart:

	std::cout << "\033[H" << "Call #" << ++calls << std::endl;;
	std::cout << "Generate random BitMatrix..." << std::endl;
	BitMatrix source{SIZEX, SIZEY};
	source.Randomize();
	std::cout << "Random BitMatrix generating done." << std::endl;
	
	// create matrix of Nodes
	std::cout << "Prepare graph from source BitMatrix..." << std::endl;
	auto startTime = std::chrono::system_clock::now();

	auto printProgress = [](int progress)
		{
			std::cout << "\r" << progress << "%";
		};
	NodeMatrix Space(SIZEX, SIZEY, source, printProgress);

	auto elapsedTime = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime).count();
	genTime += elapsedTime;
	std::cout << "\rGraph matrix generating done in " << elapsedTime << " ms" << std::endl;


	// select two random nodes from graph to find path between
	Node* sourceNode = Space.GetRandomNode();
	Node* targetNode = Space.GetRandomNode();
	std::cout << "Start finding path from " << sourceNode << " to " << targetNode << std::endl;

	// init and run dijkstra
	Dijkstra dijkstra{ Space, sourceNode, targetNode };
	dijkstra.Calculate(printProgress);

	elapsedTime = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime).count() - elapsedTime;
	calcTime += elapsedTime;
	totalPathNodes += dijkstra.resultPath.size();
	std::cout << "\r" << dijkstra.iterations << " iterations done in " << elapsedTime << " ms" << std::endl;
	std::cout << "Path found and contains " << dijkstra.resultPath.size() << " nodes :)" << std::endl;

	// dijkstra: console result visualization
	dijkstra.Draw(hwnd, 9);

	std::cout << "Generating time " << genTime / calls << " ms per call | ";
	std::cout << static_cast<float>(genTime) / (calls * SIZEX * SIZEY) << " ms per path node" << std::endl;
	std::cout << "Calculating time " << calcTime / calls << " ms per call | ";
	std:: cout << calcTime / totalPathNodes << " ms per path node" << std::endl;
	std::cout << "Press any key to start again";
	while (!_kbhit()) { Sleep(50); }
	_getch();
	system("cls");
	// DEADLY bad way, only for debug, sorry for that

	goto restart;


    return 0;
}