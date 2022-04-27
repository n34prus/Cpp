#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <list>
#include "unit.h"

using namespace std;

class Network
{
private:
	list <Unit<Network>*> units;
public:
	Network() {};
	~Network() {};
	void addUnit()
	{
		Network* ptr = this;
		units.push_back(new Unit<Network>(ptr));
	}
	int getSize() { return units.size(); }
};

#endif