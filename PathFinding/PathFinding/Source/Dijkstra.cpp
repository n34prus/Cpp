#include <ranges>
#include "Dijkstra.h"

void Dijkstra::Calculate(const std::function<void(uint8_t)>& callback)
{
	if (!sourceNode || !targetNode) return;

	unsigned long long expectedLen = sizex + sizey;
	int progress = 0;
	iterations = 0;
	// iteration container
	std::set <Node*> lastStepContainer;

	// first step: only neighbours of source node
	for (auto nbNode : sourceNode->neighbours)
	{
		if (!nbNode || (!bIncludeDiagonal && sourceNode->isDiagonalNeighbourOf(nbNode))) continue;
		lastStepContainer.insert(nbNode);
		reachableNodes.insert_or_assign(nbNode, std::make_pair(sourceNode, 1));
	}

	// every next iteration: wide search to neighbours of last iteration container
	while (true)
	{
		iterations++;

		std::set<Node*> nextStepContainer;
		for (auto baseNode : lastStepContainer)	// for every node from container
		{
			if (!baseNode) continue;
			for (auto nbNode : baseNode->neighbours) // to their neighbours
			{
				if (!nbNode || (!bIncludeDiagonal && baseNode->isDiagonalNeighbourOf(nbNode))) continue;
				if (nbNode != sourceNode && !reachableNodes.contains(nbNode))
				{
					size_t pathLen = 0;
					if (auto it = reachableNodes.find(baseNode); it != reachableNodes.end()) pathLen = it->second.second;
					if (baseNode->isDiagonalNeighbourOf(nbNode) && bExpensiveDiagonal) pathLen += 2;
					else pathLen += 1;
					reachableNodes.insert_or_assign(nbNode, std::make_pair(baseNode, pathLen));
					nextStepContainer.insert(nbNode);
				}
			}
		}
		// update iteration container
		lastStepContainer = nextStepContainer;

		//if (reachableNodes.contains(targetNode) || nextStepContainer.empty()) break;
		if (reachableNodes.contains(targetNode) || nextStepContainer.empty()) break;

		// progress callback
		if (callback)
		{
			uint8_t percent = iterations * 100 / expectedLen;
			if (percent > 99) percent = 99;
			if (progress != percent)
			{
				progress = percent;
				callback(progress);
			}
		}
	}

	if (reachableNodes.contains(targetNode))
	{
		Node* n = targetNode;
		while (n) {
			resultPath.push_back(n);
			if (n == sourceNode) break;
			if (auto it = reachableNodes.find(n); it != reachableNodes.end()) n = it->second.first;
			else break;
		}
		std::reverse(resultPath.begin(), resultPath.end());
	}
}

void Dijkstra::Draw(HANDLE hwnd, short offset) const
{
	std::set<Node*> path{ this->resultPath.begin(), this->resultPath.end() };
	std::vector<CHAR_INFO> buffer;
	auto gen_ci = [](char c, WORD atr = 0x7) -> CHAR_INFO
	{
		return CHAR_INFO
		{
			.Char = {.AsciiChar = c },
			.Attributes = atr
		};
	};

	for (auto i = 0; i < sizex; i++)
	{
		for (auto j = 0; j < sizey; j++)
		{
			char symbol = ' ';
			WORD color = 0x7;

			if (matrix[i][j]->bActive)
				if (reachableNodes.contains(matrix[i][j]))	{ color = 0x7; symbol = '+'; }
				else										{ color = 0x8; symbol = '.'; }
			if (path.contains(matrix[i][j]))			{ color = 0xD; symbol = 'X'; }
			if (matrix[i][j] == sourceNode)					{ color = 0x2; symbol = 'S'; }
			if (matrix[i][j] == targetNode)					{ color = 0x2; symbol = 'T'; }

			buffer.push_back(gen_ci(symbol, color));
			buffer.push_back(gen_ci(' '));
		}
	}

	// todo: runtime offset calculating OR draw in another place?

	auto bufferSize = COORD(sizey * 2, sizex);
	auto bufferCoord = COORD(0, 0);
	auto writeRegion = SMALL_RECT(0, offset, sizey * 2 - 1, static_cast<short>(offset + sizex - 1));
	WriteConsoleOutputA(hwnd, buffer.data(), bufferSize, bufferCoord, &writeRegion);
}
