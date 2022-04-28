//  демонстрация работы методов контейнера
//  Каргин М.С.

#include <iostream>
#include "classes.h"

using namespace std;

Network* network = new Network();
int main()
{
    Unit* unit1 = new Unit(network);
    Unit* unit2 = new Unit(network);
    network->newUnit();
    network->newUnit();
    network->newUnit();
    network->newUnit();
    network->newUnit();
    network->newUnit();
    network->newUnit();
    network->newUnit();

    /*
    cout << "*** generating events: ***" << endl;
    unit1->makeEvent();
    network->doEvents();
    */

    unit1->addSub(unit2);
    unit2->addSub(unit1);
    unit1->makeNewUnit();
    network->printInfo(FULL);

    network->updateNetwork();
    network->printInfo(FULL);

    //network->removeUnit(unit1);
    //network->printInfo(FULL);

    network->updateNetwork();
    network->printInfo(FULL);

    network->updateNetwork();
    network->printInfo(FULL);
}
