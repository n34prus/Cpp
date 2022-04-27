//  демонстрация работы методов контейнера
//  Каргин М.С.

#include <iostream>
#include "unit.h"
#include "network.h"

using namespace std;

Network* network1 = new Network();
Unit<Network>*unit1 = new Unit<Network>(network1);
Unit<Network>* unit2 = new Unit<Network>(network1);

int main()
{
    srand(time(NULL));

    network1->addUnit();
    cout << network1->getSize() << endl;
    network1->addUnit();
    cout << network1->getSize() << endl;

    /*
    cout << unit1->sizeSubs() << endl;

    unit1->addSub(unit2);
    cout << unit1->sizeSubs() << endl;

    unit1->addSub(unit2);
    cout << unit1->sizeSubs() << endl;

    unit1->removeSub(unit2);
    cout << unit1->sizeSubs() << endl;
    */

}
