//  демонстрация работы методов контейнера
//  Каргин М.С.

#include <iostream>
#include <string>
#include "classes.h"

using namespace std;

Network* network = new Network();
string cmd = "";

int main()
{
    Unit* unit1 = new Unit(network);
    Unit* unit2 = new Unit(network);
    unit1->addSub(unit2);
    unit2->addSub(unit1);

    /*
    cout << "*** generating events: ***" << endl;
    unit1->makeEvent();
    network->doEvents();
    */


    //network->printInfo(FULL);

    //network->updateNetwork();
    //network->printInfo(FULL);

    while (1)
    {
        cout << "Press ENTER for next step" << endl;
        while (1)
        {
            getline(cin, cmd);
            if (cmd == "") goto NEXT;
            if (cmd == "stop") goto STOP;
        }
        NEXT:
        network->updateNetwork();
        network->printInfo(BASE);
        if (0)
        {
        STOP:
            break;
        }
    }

    //int netSize = network->getSize();
    //while (network->getSize() == netSize) network->updateNetwork();
    //network->printInfo(FULL);

    //network->removeUnit(unit1);
    //network->printInfo(FULL);
}
