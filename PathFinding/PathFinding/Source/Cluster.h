#pragma once

#include <functional>
#include <stdexcept>
#include <vector>

#include "Node.h"

// bit matrix class as interlayer between user data and graph. light weighted, fast access
class BitMatrix
{
public:
	BitMatrix(){};
	BitMatrix(size_t x, size_t y, bool randomValues = false) : sizex(x), sizey(y)
	{
		matrix = std::vector<uint8_t>((sizex* sizey + 7) / 8, 0);
		if (randomValues) randomize();
	}
	BitMatrix(size_t x, size_t y, const bool* source) : BitMatrix(x,y)
	{
		for (size_t i = 0; i < sizex; ++i)
			for (size_t j = 0; j < sizey; ++j)
			{
				if (source[i * sizey + j])
				{
					size_t idx = i * sizey + j;
					matrix[idx >> 3] |= static_cast<uint8_t>(1u << (idx & 7));
				}
			}
	}

	// operator [] proxy classes
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
			return BitProxy(mat.matrix[idx >> 3], 1 << (idx & 7));
		}
		
	private:
		BitMatrix& mat;
		size_t row;
		size_t rowOffset;
	};

	// operator []
	RowProxy operator[](size_t row)
	{
		if (row >= sizex) throw std::out_of_range("Row out of range");
		return RowProxy(*this, row);
	}

	// fill matrix by random values
	void randomize();

	// const get bit by its idx
	bool getBit(size_t i, size_t j) const;

	// set bit by its idx
	void setBit(size_t i, size_t j, bool val);

	// get <sizex, sizey> of the matrix
	std::pair<size_t,size_t> getSize() const;
	
private:
	size_t sizex = 0;
	size_t sizey = 0;
	std::vector<uint8_t> matrix;
};

// node matrix class considers node's neighbours. shiuld be generated from bitmatrix
class Cluster : public Node
{
public:
	Cluster() = delete;
	Cluster(size_t x, size_t y) : Node(), sizex(x), sizey(y), data(x, std::vector<Node*>(y, nullptr)) {};

	Cluster(const BitMatrix& source, const std::function<void(uint8_t)>& callback = nullptr) : Cluster(source.getSize().first, source.getSize().second)
	{
		loadFromBitmatrix(source, callback);
	}
	Cluster(BitMatrix&& source, const std::function<void(uint8_t)>& callback = nullptr) : Cluster(source.getSize().first, source.getSize().second)
	{
		loadFromBitmatrix(source, callback);
	}

	~Cluster()
	{
		for (auto& row : data)
		{
			for (auto node : row)
			{
				delete node;
			}
		}
	}

	// fill space by nodes from static boolean matrix
	void loadFromBitmatrix(const BitMatrix& source, const std::function<void(uint8_t)>& callback = nullptr);

	// get ref to random active node
	Node* getRandomNode() const;

	// get idx of random active node
	std::pair<int, int> getRandomNodeIdx() const;

	// get node by its idx
	Node* getNode(std::pair<int, int> position);

	// get <sizex, sizey> of the matrix
	std::pair<size_t, size_t> getSize() const;

	// get active nodes
	const std::vector<Node*>& getActiveNodes() const;

	// operator [] promotes to data
	std::vector<Node*>& operator[] (size_t row)
	{
		if (row >= sizex) throw std::out_of_range("Row out of range");
		return data[row];
	}
	const std::vector<Node*>& operator[](size_t row) const
	{
		if (row >= sizex) throw std::out_of_range("Row out of range");
		return data[row];
	}

private:
	size_t sizex = 0;
	size_t sizey = 0;
	std::vector< std::vector<Node*>> data;
	std::vector<Node*> activeNodes;
};

