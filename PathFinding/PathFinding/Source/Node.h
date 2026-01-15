#pragma once
#include <map>

class Node
{
public:
    int PositionX;
	int PositionY;
	bool bActive;
    std::map< Node*, std::vector<Node*> > Map;  // target node : path

	Node(int PositionX = 0, int PositionY = 0, bool bActive = 0) : PositionX(PositionX), PositionY(PositionY), bActive(bActive) {};
};