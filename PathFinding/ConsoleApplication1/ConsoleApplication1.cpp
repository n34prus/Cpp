#include <chrono>
#include <map>
#include <vector>
#include <set>
#include <iomanip>
#include <iostream>
#include <ctime>
#include <windows.h>
#include <conio.h>
#include <chrono>

// changeble settings
#define SIZEX 100
#define SIZEY 100
#define ASTAR 1
#define INCLUDE_DIAGONAL 1
#define EXPENSIVE_DIAGONAL 1
#define ENABLE_GRAPHIC 1

// other defines
#define SCLR(value) SetConsoleTextAttribute(hwnd, value | FOREGROUND_INTENSITY)

class Node
{
public:
    int PositionX;
	int PositionY;
	bool Value;
    std::map< Node*, std::vector<Node*> > Map;  // target node : path

	Node(int _PositionX = 0, int _PositionY = 0, bool _Value = 0)
	{
		PositionX = _PositionX;
		PositionY = _PositionY;
		Value = _Value;
	}
};


bool Source[SIZEX][SIZEY];

int main()
{
	HANDLE hwnd = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_FONT_INFOEX* fontInfo = new CONSOLE_FONT_INFOEX();
	GetCurrentConsoleFontEx(hwnd, TRUE, fontInfo);
	//wcsncpy(wchar_t*("Arial Cyr"), fontInfo.FaceName, LF_FACESIZE);
	fontInfo->dwFontSize.X = 5;
	fontInfo->dwFontSize.Y = 5;
	SetCurrentConsoleFontEx(hwnd, TRUE, fontInfo); // how could i use this shit dog?

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
	auto startTime = std::chrono::system_clock::now();
	uint64_t _percent = 0;
	std::vector< std::vector<Node*> > Matrix;
	for (int i = 0; i < SIZEX; i++)
	{
		std::vector<Node*> _tmp = std::vector<Node*>();
		for (int j = 0; j < SIZEY; j++)
		{
			bool _value = Source[i][j]; // cache
			Node* _node = new Node(i,j, _value);
			_tmp.push_back(_node);

			// fill closest neubourghs
			if (!_value) continue;
			if (j > 0 && _tmp[j - 1]->Value)
			{
				_tmp[j - 1]->Map.insert_or_assign(_tmp[j], std::vector<Node*>({ _tmp[j] }));
				_tmp[j]->Map.insert_or_assign(_tmp[j - 1], std::vector<Node*>({ _tmp[j - 1] }));
			}
			if (i > 0 && Matrix[i - 1][j]->Value)
			{
				Matrix[i - 1][j]->Map.insert_or_assign(_tmp[j], std::vector<Node*>({ _tmp[j] }));
				_tmp[j]->Map.insert_or_assign(Matrix[i - 1][j], std::vector<Node*>({ Matrix[i - 1][j] }));
			}
			// diagonal neubourghs
			if (INCLUDE_DIAGONAL)
			{
				if (i > 0 && j > 0 && Matrix[i - 1][j - 1]->Value)
				{
					Matrix[i - 1][j - 1]->Map.insert_or_assign(_tmp[j], std::vector<Node*>({ _tmp[j] }));
					_tmp[j]->Map.insert_or_assign(Matrix[i - 1][j - 1], std::vector<Node*>({ Matrix[i - 1][j - 1] }));
				}
				if (i > 0 && j < SIZEY - 1 && Matrix[i - 1][j + 1]->Value)
				{
					Matrix[i - 1][j + 1]->Map.insert_or_assign(_tmp[j], std::vector<Node*>({ _tmp[j] }));
					_tmp[j]->Map.insert_or_assign(Matrix[i - 1][j + 1], std::vector<Node*>({ Matrix[i - 1][j + 1] }));
				}
			}
		}
		Matrix.push_back(_tmp);
		std::cout << "\b\b\b";
		uint64_t __percent = (i * 100 / SIZEX);
		if (_percent == __percent) continue;
		else _percent = __percent;
		if (_percent < 10) std::cout << "0";
		std::cout << _percent << "%";
	}
	auto elapsedTime = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime).count();
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
		if (Matrix[_randnodes[0][0]][_randnodes[0][1]]->Value && Matrix[_randnodes[1][0]][_randnodes[1][1]]->Value)
			break;
	}
	Node* SourceNode = Matrix[_randnodes[0][0]][_randnodes[0][1]];
	Node* TargetNode = Matrix[_randnodes[1][0]][_randnodes[1][1]];

	std::cout << "Start finding path from " << SourceNode << " to " << TargetNode << std::endl;

	// dijkstra: init
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
	if (ENABLE_GRAPHIC) for (auto i = 0; i < SIZEX; i++)
	{
		for (auto j = 0; j < SIZEY; j++)
		{
			char symbol = ' ';
			if (Matrix[i][j]->Value)
				if (SourceNode->Map.contains(Matrix[i][j]))		{ SCLR(0x7); symbol = '*'; } 
				else											{ SCLR(0x8); symbol = 'o'; }
			if (_nodesInPath.contains(Matrix[i][j]))			{ SCLR(0xD); symbol = 'X'; }
			if (Matrix[i][j] == SourceNode)						{ SCLR(0x2); symbol = 'S'; }
			if (Matrix[i][j] == TargetNode)						{ SCLR(0x2); symbol = 'T'; }
			std::cout << symbol << " ";
		}
		std::cout << std::endl;
	}
	SCLR(0x7);

    return 0;
}