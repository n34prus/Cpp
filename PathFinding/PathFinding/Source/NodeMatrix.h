#pragma once

#include <vector>
#include "Node.h"

class NodeMatrix
{
public:
	NodeMatrix();
	NodeMatrix(size_t x, size_t y) : sizex(x), sizey(y), data(x, std::vector<Node*>(y, nullptr)) {};

	template<typename F = std::nullptr_t>
	NodeMatrix(size_t x, size_t y, const bool* source, F callback = nullptr) : sizex(x), sizey(y)
	{
		FillFromMatrix(source, callback);
	}

	// fill space by nodes from source boolean matrix, calling F for each row done
	template<typename F = std::nullptr_t>
	void FillFromMatrix(const bool* source, F callback = nullptr)
	{
		uint64_t _percent = 0;
		for (size_t i = 0; i < sizex; i++)
		{
			std::vector<Node*> row = std::vector<Node*>();
			for (size_t j = 0; j < sizey; j++)
			{
				bool _value = source[i * sizey + j]; // cache
				Node* _node = new Node(i, j, _value);
				row.push_back(_node);

				// fill closest neubourghs
				if (!_value) continue;
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
			data.push_back(row);

			if constexpr (!std::is_same_v<F, std::nullptr_t>) {
				//callback(i);
				callback(i * 100 / sizex);
			}
		}
	}
private:
	size_t sizex;
	size_t sizey;
	// todo: dynamic set
	bool bIncludeDiagonal = true;
public:
	std::vector< std::vector<Node*>> data;

};
