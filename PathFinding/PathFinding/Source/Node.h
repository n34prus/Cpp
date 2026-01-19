#pragma once

class Node
{
public:
	std::pair<size_t, size_t> position;
	bool bActive;
	/*
	 * 0 1 2
	 * 3 x 4
	 * 5 6 7
	*/
	Node* neighbours[8];

	bool isNeighbourOf(Node* other) const
	{
		if (!other) return false;
		for (auto nb : neighbours)
		{
			if (nb == other) return true;
		}
		return false;
	}

	bool isDiagonalNeighbourOf(Node* other) const
	{
		if (!other) return false;
		for (uint8_t idx = 0; idx < 7; idx+= 2)
		{
			if (neighbours[idx] == other) return true;
		}
		return false;
	}

	Node(std::pair<size_t, size_t> position = {0,0}, bool bActive = 0) : position(position), bActive(bActive)
	{
		for (auto& n : neighbours) n = nullptr;
	};
};