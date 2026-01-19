#pragma once
#include <map>
#include <memory>
#include <set>
#include <windows.h>

#include "Cluster.h"

class Dijkstra
{
public:
	// init searching on original matrix
	Dijkstra(Cluster& matrix, Node* sourceNode, Node* targetNode) :
		matrix{ matrix },
		sourceNode{ sourceNode },
		targetNode{ targetNode },
		sizex( matrix.getSize().first ),
		sizey(matrix.getSize().second) {};

	// init searching on its own matrix copy, the only way to multiple parallel calcs
	Dijkstra(const Cluster& matrix, std::pair<size_t,size_t> sourceNode, std::pair<size_t, size_t> targetNode) :
		matrixCopy{ std::make_unique<Cluster>(matrix) },
		matrix{ *matrixCopy},
		bCopyMatrix{ true },
		sourceNode{ matrixCopy->getNode(sourceNode) },
		targetNode{ matrixCopy->getNode(targetNode) },
		sizex(matrix.getSize().first),
		sizey(matrix.getSize().second) {
	};

	// runs searching path from sourceNode to targetNode
	void Calculate(const std::function<void(uint8_t)>& callback = nullptr);

	// draw matrix in console, fine results on sizes like 256*256 or smaller
	void Draw(HANDLE hwnd, short offset = 0) const;

	// if we works with own copy of source matrix
	bool worksWithCopy() const
	{
		return bCopyMatrix;
	}

public:
	// settings
	bool bIncludeDiagonal = true;
	bool bExpensiveDiagonal = true;
	// temp and result
	int iterations = 0;
	std::map< Node*, std::vector<Node*> > reachableNodes;
	std::set<Node*> resultPath;

private:
	std::unique_ptr<Cluster> matrixCopy;	// valid if works with own copy
	const Cluster& matrix;					// ref to original/copied matrix
	bool bCopyMatrix = false;				// marker
	Node* sourceNode;						// start of the path
	Node* targetNode;						// end of the path
	const size_t sizex;
	const size_t sizey;						// cached values of source matrix
};