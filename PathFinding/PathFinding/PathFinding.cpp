#include <chrono>
#include <map>
#include <vector>
#include <set>
//#include <iomanip>
#include <iostream>
#include <ctime>
#include <windows.h>
#include <conio.h>

#include "Node.h"
#include "NodeMatrix.h"

// changeble settings
#define SIZEX 100
#define SIZEY 300
#define ASTAR 1
#define INCLUDE_DIAGONAL 1
#define EXPENSIVE_DIAGONAL 1
#define ENABLE_GRAPHIC 1

// other defines
#define SCLR(value) SetConsoleTextAttribute(hwnd, value | FOREGROUND_INTENSITY)


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

	//COORD bufferSize = { width, height };
	//SetConsoleScreenBufferSize(hwnd, bufferSize);

	//SMALL_RECT windowSize = {0,0, width, height };
	//SetConsoleWindowInfo(hwnd, TRUE, &windowSize);

	RECT r;
	GetWindowRect(GetConsoleWindow(), &r);
	MoveWindow(GetConsoleWindow(), r.left, r.top, (fontSize+1)*(width), (fontSize+1)*height, TRUE);
}

int main()
{
	// init
	HANDLE hwnd = GetStdHandle(STD_OUTPUT_HANDLE);
	InitConsole(hwnd, SIZEY, SIZEX);

	bool Source[SIZEX][SIZEY];
	std::chrono::time_point<std::chrono::system_clock> startTime;
	long long elapsedTime;

	auto printProgress = [](int percent)
		{
			std::cout << "\b\b\b";
			if (percent < 10) std::cout << "0";
			std::cout << percent << "%";
		};

	// restart point FOR DEBUG ONLY
	restart:

	// randomize source
	std::cout << "Generate random matrix..." << std::endl;
	srand(time(0));
	for (int i = 0; i < SIZEX; i++)
	{
		for (int j = 0; j < SIZEY; j++)
		{
			Source[i][j] = rand()%2;
		}
	}
	std::cout << "Random matrix generating done." << std::endl;
	
	// create matrix of Nodes
	std::cout << "Generate graph matrix from source matrix..." << std::endl;
	startTime = std::chrono::system_clock::now();

	NodeMatrix Space(SIZEX, SIZEY, reinterpret_cast<const bool*>(Source), printProgress);
	elapsedTime = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime).count();
	std::cout << "\b\b\bGraph matrix generating done in " << elapsedTime << " ms" << std::endl;

	// print existing paths
	/*
	for (auto& i : Matrix)
	{
		for (auto& j : i)
		{
			std::cout << j->Position << " * * *" << std::endl;
			// for eack key in j.Map
			for (auto& [key, value] : j->Map)
			{
				std::cout << key->Position << " : ";
				for (auto& k : value)
				{
					std::cout << k->Position << " -> ";
				}
				std::cout << std::endl;
			}
		}
		std::cout << std::endl;
	}
	*/

	// dijkstra: pick random start and end nodes
	int _randnodes[2][2];
	while (true)
	{
		for (int i = 0; i < 2; i++)
		{
			_randnodes[i][0] = rand() % SIZEX;
			_randnodes[i][1] = rand() % SIZEY;
		}
		if (Space.data[_randnodes[0][0]][_randnodes[0][1]]->bActive && Space.data[_randnodes[1][0]][_randnodes[1][1]]->bActive)
			break;
	}
	Node* SourceNode = Space.data[_randnodes[0][0]][_randnodes[0][1]];
	Node* TargetNode = Space.data[_randnodes[1][0]][_randnodes[1][1]];

	std::cout << "Start finding path from " << SourceNode << " to " << TargetNode << std::endl;

	// dijkstra: init
	// todo: separate as class
	uint64_t iterations = 0;
	std::set <Node*> _lastAddedNodes = std::set <Node*>();
	for (auto& [key, value] : SourceNode->Map)
	{
		_lastAddedNodes.insert(key);
	}
	
	// dijkstra: main loop. for each node from lastAddedNodes get its neutbours and check if they are reachable by shorter path
	startTime = std::chrono::system_clock::now();
	while (true)
	{
		std::set <Node*> _newAddedNodes = std::set <Node*>();
		for (auto _node : _lastAddedNodes)	// from
		{
			for (auto& [__node, __path] : _node->Map)	// to
			{
				if (__node != SourceNode && !SourceNode->Map.contains(__node))
				{
					// check if __node is reachable by shorter path from any other neighbour
					int _pathlength = SourceNode->Map.find(_node)->second.size();
					Node* _shortest = _node;
					for (auto& [___node, ___path] : __node->Map)
					{
						// mark diagonal connections less valuable
						int __pathlength = _pathlength;
						if (INCLUDE_DIAGONAL && EXPENSIVE_DIAGONAL && __node->PositionX != ___node->PositionX && __node->PositionY != ___node->PositionY) __pathlength--;
						if (___node != SourceNode && SourceNode->Map.contains(___node) && SourceNode->Map.find(___node)->second.size() <= __pathlength)
						{
							_shortest = ___node;
							_pathlength = SourceNode->Map.find(___node)->second.size();
						}
					}
					std::vector<Node*> ___path = SourceNode->Map.find(_shortest)->second;
					___path.push_back(__node);
					SourceNode->Map.insert_or_assign(__node, ___path);
					_newAddedNodes.insert(__node);
					iterations++;
					//std::cout << "found path to " << __node << std::endl;	// prints all nodes connected to SourceNode
				}
			}
		}
		_lastAddedNodes = _newAddedNodes;
		if (SourceNode->Map.contains(TargetNode) || _newAddedNodes.size() == 0) break;
	}
	elapsedTime = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime).count();
	std::cout << iterations << " iterations done in " << elapsedTime << " ms" << std::endl;

	/*
	// debug. prints path to all nodes connected to SourceNode
	for (auto& [key, value] : SourceNode->Map)
	{
		for (auto& k : value)
		{
			std::cout << k << " -> ";
		}
		std::cout << std::endl;
	}
	*/

	std::set<Node*> _nodesInPath;
	if (!SourceNode->Map.contains(TargetNode))
	{
		std::cout << "Path not found :(" << std::endl;
	}
	else
	{
		for (auto& i : SourceNode->Map.find(TargetNode)->second)
		{
			_nodesInPath.insert(i);
		}
		std::cout << "Path found and contains " << _nodesInPath.size() << " nodes :)" << std::endl;
	}

	// dijkstra: console result visualization
	// todo: generate buffer before draw
	//std::string buffer;
	if (ENABLE_GRAPHIC) for (auto i = 0; i < SIZEX; i++)
	{
		for (auto j = 0; j < SIZEY; j++)
		{
			char symbol = ' ';
			if (Space.data[i][j]->bActive)
				if (SourceNode->Map.contains(Space.data[i][j]))		{ SCLR(0x7); symbol = '+'; } 
				else												{ SCLR(0x8); symbol = '.'; }
			if (_nodesInPath.contains(Space.data[i][j]))			{ SCLR(0xD); symbol = 'X'; }
			if (Space.data[i][j] == SourceNode)						{ SCLR(0x2); symbol = 'S'; }
			if (Space.data[i][j] == TargetNode)						{ SCLR(0x2); symbol = 'T'; }
			//buffer += symbol;
			//buffer += " ";
			std::cout << symbol << " ";
		}
		//buffer += "\n";
		std::cout << std::endl;
	}
	//std::cout << buffer;
	SCLR(0x7);

	std::cout << "Press any key to start again";

	while (!_kbhit()) { Sleep(50); }
	_getch();
	system("cls");
	// DEADLY bad way, only for debug, sorry for that
	goto restart;


    return 0;
}