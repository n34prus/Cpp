#pragma once
#include <map>
#include <set>
#include <ranges>

#include "NodeMatrix.h"

class Dijkstra
{
public:
	// todo: usual fields
	Dijkstra() = default;
	// todo: init with matrix only, source and target nodes will be setted by run()
	Dijkstra(NodeMatrix& matrix, Node* sourceNode, Node* targetNode) : matrix{ matrix }, sourceNode{ sourceNode }, targetNode{ targetNode } {};

	
	int iterations = 0;
	bool bIncludeDiagonal = true; //todo: read from source? idk
	bool bExpensiveDiagonal = true;
	std::set<Node*> resultPath;

	template<typename F = std::nullptr_t>
	void Calculate(F callback = nullptr)
	{
		long long expectedLen = matrix.sizex + matrix.sizey;
		int progress = 0;
		iterations = 0;
		// iteration container
		std::set <Node*> lastStepContainer = std::set <Node*>();

		// first step: only neighbours of source node
		for (auto& nbNode : sourceNode->Map | std::views::keys)
		{
			lastStepContainer.insert(nbNode);
		}

		// every next iteration: wide search to neighbours of last iteration container
		while (true)
		{
			std::set<Node*> nextStepContainer = std::set <Node*>();
			for (auto baseNode : lastStepContainer)	// for every node from container
			{
				for (auto& nbNode : baseNode->Map | std::views::keys) // to their neighbours
				{
					if (nbNode != sourceNode && !sourceNode->Map.contains(nbNode))
					{
						// check if __node is reachable by shorter path from any other neighbour
						int basePathLen = sourceNode->Map.find(baseNode)->second.size();
						Node* closestNode = baseNode;
						for (auto& nbnbNode : nbNode->Map | std::views::keys)
						{
							// mark diagonal connections less valuable
							// todo: settable weight conditions
							int weightedPathLen = basePathLen;
							if (bIncludeDiagonal && bExpensiveDiagonal && nbNode->PositionX != nbnbNode->PositionX && nbNode->PositionY != nbnbNode->PositionY) weightedPathLen--;
							if (nbnbNode != sourceNode && sourceNode->Map.contains(nbnbNode) && sourceNode->Map.find(nbnbNode)->second.size() <= weightedPathLen)
							{
								closestNode = nbnbNode;
								basePathLen = sourceNode->Map.find(nbnbNode)->second.size();
							}
						}
						std::vector<Node*> shortestPath = sourceNode->Map.find(closestNode)->second;
						shortestPath.push_back(nbNode);
						sourceNode->Map.insert_or_assign(nbNode, shortestPath);
						nextStepContainer.insert(nbNode);
						iterations++;

						if constexpr (!std::is_same_v<F, std::nullptr_t>)
						{
							uint8_t percent = iterations / expectedLen;
							if (progress != percent)
							{
								progress = percent;
								callback(progress);
							}
						}
					}
				}
			}
			// update iteration container
			lastStepContainer = nextStepContainer;
			if (sourceNode->Map.contains(targetNode) || nextStepContainer.empty()) break;
		}

		if (sourceNode->Map.contains(targetNode))
		{
			for (auto& i : sourceNode->Map.find(targetNode)->second)
			{
				resultPath.insert(i);
			}
		}
	}

	void Draw(HANDLE hwnd, short offset = 0)
	{
		std::vector<CHAR_INFO> buffer;
		auto gen_ci = [](char c, WORD atr = 0x7) -> CHAR_INFO
			{
				return CHAR_INFO
				{
					.Char = {.AsciiChar = c },
					.Attributes = atr
				};
			};

		for (auto i = 0; i < matrix.sizex; i++)
		{
			for (auto j = 0; j < matrix.sizey; j++)
			{
				char symbol = ' ';
				WORD color = 0x7;

				if (matrix.data[i][j]->bActive)
					if (sourceNode->Map.contains(matrix.data[i][j])) { color = 0x7; symbol = '+'; }
					else { color = 0x8; symbol = '.'; }
				if (resultPath.contains(matrix.data[i][j])) { color = 0xD; symbol = 'X'; }
				if (matrix.data[i][j] == sourceNode) { color = 0x2; symbol = 'S'; }
				if (matrix.data[i][j] == targetNode) { color = 0x2; symbol = 'T'; }

				buffer.push_back(gen_ci(symbol, color));
				buffer.push_back(gen_ci(' '));
			}
		}

		// todo: runtime offset calculating OR draw in another place?

		auto bufferSize = COORD(matrix.sizey * 2, matrix.sizex);
		auto bufferCoord = COORD(0, 0);
		auto writeRegion = SMALL_RECT(0, offset, matrix.sizey * 2 - 1, static_cast<short>(offset + matrix.sizex - 1));
		WriteConsoleOutputA(hwnd, buffer.data(), bufferSize, bufferCoord, &writeRegion);
	}

private:
	NodeMatrix& matrix;
	Node* sourceNode;
	Node* targetNode;
};