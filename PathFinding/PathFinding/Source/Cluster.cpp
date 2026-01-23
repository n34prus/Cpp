#include <random>
#include <vector>
#include <ranges>
#include <assert.h>

#include "Cluster.h"
#include "Node.h"

void BitMatrix::randomize()
{
	static std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<size_t> dist(0, 1);
	for (int i = 0; i < sizex; i++)
	{
		for (int j = 0; j < sizey; j++)
		{
			(*this)[i][j] = dist(rng);
		}
	}
}

bool BitMatrix::getBit(size_t i, size_t j) const
{
	size_t idx = i * sizey + j;
	return (matrix[idx / 8] >> (idx % 8)) & 1;
}

void BitMatrix::setBit(size_t i, size_t j, bool val)
{
	assert(i < sizex);
	assert(j < sizey);
	size_t idx = i * sizey + j;
	if (val)
		matrix[idx / 8] |= (1 << (idx % 8));
	else
		matrix[idx / 8] &= ~(1 << (idx % 8));
}

std::pair<size_t, size_t> BitMatrix::getSize() const
{
	return { sizex, sizey };
}

void Cluster::loadFromBitmatrix(const BitMatrix& source, const std::function<void(uint8_t)>& callback)
{
	uint8_t progress = 0;
	for (size_t i = 0; i < sizex; i++)
	{
		std::vector<Node*>& row = data[i];
		for (size_t j = 0; j < sizey; j++)
		{
			bool isActive = source.getBit(i, j);
			Node* newNode = new Node({ i, j }, isActive);
			row[j] = newNode;

			if (!isActive) continue;
			activeNodes.push_back(newNode);

			// fill closest neighbours
			// 0 1 2
			// 3 x 4
			// 5 6 7
			auto Neighbourize = [newNode](Node* nb, uint8_t direction)
				{
					if (!nb) return;
					if (!nb->bActive) return; // todo: check if this is needed, we can want to change the grid in runtime
					if (direction > 7) return;
					newNode->neighbours[direction] = nb;
					nb->neighbours[7 - direction] = newNode;
				};
			if (j > 0)					Neighbourize(row[j - 1], 3);
			if (i > 0)					Neighbourize(data[i - 1][j], 1);
			if (i > 0 && j > 0)			Neighbourize(data[i - 1][j - 1], 0);
			if (i > 0 && j < sizey - 1) Neighbourize(data[i - 1][j + 1], 2);
		}

		// send percentage
		if (callback)
		{
			uint8_t percent = i * 100 / sizex;
			if (progress != percent)
			{
				progress = percent;
				callback(progress);
			}
		}
	}
}

BitMatrix Cluster::generateBitMatrix() const
{
	BitMatrix result{ sizex, sizey, false };
	for (size_t i = 0; i < sizex; i++)
	{
		for (size_t j = 0; j < sizey; j++)
		{
			if (data[i][j])
			{
				result.setBit(i, j, data[i][j]->bActive);
			}
		}
	}
	return result;
}

Node* Cluster::getRandomNode() const
{
	if (activeNodes.empty()) return nullptr;
	static std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<size_t> dist(0, activeNodes.size() - 1);
	return activeNodes.at(dist(rng));
}
std::pair<int,int> Cluster::getRandomNodeIdx() const
{
	if (activeNodes.empty()) return { 0,0 };
	static std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<size_t> dist(0, activeNodes.size() - 1);
	return activeNodes.at(dist(rng))->position;
}
Node* Cluster::getNode(std::pair<int,int> position)
{
	if (data.empty()) return nullptr;
	return data[position.first][position.second];
}

std::pair<size_t, size_t> Cluster::getSize() const
{
	return { sizex, sizey };
}

const std::vector<Node*>& Cluster::getActiveNodes() const
{
	return activeNodes;
}
