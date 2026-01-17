#pragma once

#include <random>
#include <vector>
#include <ranges>

#include "Node.h"

class BitMatrix
{
public:
	BitMatrix(){};
	BitMatrix(size_t x, size_t y) : sizex(x), sizey(y), sizexy(x* y)
	{
		matrix = std::vector<uint8_t>((sizexy + 7) / 8, 0);
	}

	// operator [] features
	
	class RowProxy
	{
	public:
		RowProxy(BitMatrix& mat, size_t r) : mat(mat), row(r), rowOffset(r * mat.sizey) {}

		class BitProxy {
		public:
			BitProxy(uint8_t& byte, uint8_t mask) : b(byte), m(mask) {}
			operator bool() const { return (b & m) != 0; }
			BitProxy& operator=(bool val) {
				if (val) b |= m; else b &= ~m;
				return *this;
			}
		private:
			uint8_t& b;
			uint8_t m;
		};

		BitProxy operator[](size_t col) {
			if (col >= mat.sizey) throw std::out_of_range("Column out of range");
			size_t idx = rowOffset + col;
			//return BitProxy(mat.matrix[idx / 8], 1 << (idx % 8)); // too slow
			return BitProxy(mat.matrix[idx >> 3], 1 << (idx & 7));
			
		}
	private:
		BitMatrix& mat;
		size_t row;
		size_t rowOffset;
	};

	RowProxy operator[](size_t row)
	{
		if (row >= sizex) throw std::out_of_range("Row out of range");
		return RowProxy(*this, row);
	}

	// end of operator [] features

	void Randomize()
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
	/*
	bool get(size_t i, size_t j)
	{
		size_t idx = i * sizey + j;
		return (matrix[idx / 8] >> (idx % 8)) & 1;
	}

	void set(size_t i, size_t j, bool val)
	{
		size_t idx = i * sizex + j;
		if (val)
			matrix[idx / 8] |= (1 << (idx % 8));
		else
			matrix[idx / 8] &= ~(1 << (idx % 8));
	}
	*/
private:
	size_t sizex = 0;
	size_t sizey = 0;
	size_t sizexy = 0;
	std::vector<uint8_t> matrix;
};

class NodeMatrix
{
public:
	NodeMatrix() = delete;

	NodeMatrix(size_t x, size_t y) : sizex(x), sizey(y), data(x, std::vector<Node*>(y, nullptr)) {};

	template<typename SourceObj, typename F = std::nullptr_t>
	NodeMatrix(size_t x, size_t y, SourceObj source, F callback = nullptr) : NodeMatrix(x,y)
	{
		FillFromMatrix(source, callback);
	}

	// fill space by nodes from static boolean matrix, calling F for each row done
	template<typename SourceObj, typename F = std::nullptr_t>
	void FillFromMatrix(SourceObj source, F callback = nullptr)
	{
		// low speed, i'd better to use Node*[8] 
		uint8_t progress = 0;
		for (size_t i = 0; i < sizex; i++)
		{
			std::vector<Node*>& row = data[i];
			for (size_t j = 0; j < sizey; j++)
			{
				bool isActive = GetBitFromSource(source, i, j); // expensive call. i'd better to use something inline
				Node* newNode = new Node(i, j, isActive);
				row[j] = newNode;

				if (!isActive) continue;
				activeNodes.push_back(newNode);

				// fill closest neubourghs
				
				if (j > 0 && row[j - 1]->bActive)
				{
					row[j - 1]->Map.insert_or_assign(row[j], std::vector<Node*>({ row[j] }));
					row[j]->Map.insert_or_assign(row[j - 1], std::vector<Node*>({ row[j - 1] }));
				}
				if (i > 0 && data[i - 1][j]->bActive)
				{
					data[i - 1][j]->Map.insert_or_assign(row[j], std::vector<Node*>({ row[j] }));
					row[j]->Map.insert_or_assign(data[i - 1][j], std::vector<Node*>({ data[i - 1][j] }));
				}
				// diagonal neubourghs
				if (bIncludeDiagonal)
				{
					if (i > 0 && j > 0 && data[i - 1][j - 1]->bActive)
					{
						data[i - 1][j - 1]->Map.insert_or_assign(row[j], std::vector<Node*>({ row[j] }));
						row[j]->Map.insert_or_assign(data[i - 1][j - 1], std::vector<Node*>({ data[i - 1][j - 1] }));
					}
					if (i > 0 && j < sizey - 1 && data[i - 1][j + 1]->bActive)
					{
						data[i - 1][j + 1]->Map.insert_or_assign(row[j], std::vector<Node*>({ row[j] }));
						row[j]->Map.insert_or_assign(data[i - 1][j + 1], std::vector<Node*>({ data[i - 1][j + 1] }));
					}
				}
			}
			
			// send percentage
			if constexpr (!std::is_same_v<F, std::nullptr_t>)
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

	Node* GetRandomNode() const
	{
		static std::mt19937 rng(std::random_device{}());
		std::uniform_int_distribution<size_t> dist(0, activeNodes.size() - 1);
		return activeNodes.at(dist(rng));
	}

private:
	
	// todo: dynamic set
	bool bIncludeDiagonal = true;
	// filler helpers
	bool GetBitFromSource(const bool* source, size_t i, size_t j) const
	{
		return source[i * sizey + j];
	}

	bool GetBitFromSource(BitMatrix source, size_t i, size_t j) const
	{
		return source[i][j];
	}
public:
	size_t sizex = 0;
	size_t sizey = 0;
	std::vector< std::vector<Node*>> data;
	std::vector<Node*> activeNodes;
};

