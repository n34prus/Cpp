//  демонстрация работы методов контейнера
//  Каргин М.С.

#include <iostream>
#include "classes.h"

using namespace std;

Network* network = new Network();
Unit * unit1 = new Unit(network);
Unit * unit2 = new Unit(network);

int main()
{
    srand(time(NULL));

    cout << "*** adding units. network sizes: ***" << endl;
    cout << network->getSize() << endl;
    network->addUnit();
    cout << network->getSize() << endl;
    network->addUnit();
    cout << network->getSize() << endl;

    cout << "*** generating events: ***" << endl;
    unit1->makeEvent();
    unit1->makeEvent();
    unit1->makeEvent();
    unit2->makeEvent();
    network->doEvents();

    unit1->addSub(unit2);
    unit1->makeNewUnit();
    unit2->addSub(unit1);

    network->printInfo(FULL);

}
