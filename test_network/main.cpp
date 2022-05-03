//  демонстрация работы методов контейнера
//  Каргин М.С.
//  INFO указывает насколько подробно в консоли описывается состояние сети
//  доступны режимы MIN BASE FULL

#include <iostream>
#include <string>
#include "classes.h"

using namespace std;

Network* network = new Network();
string cmd = "";

int main()
{
    //параметры, доступные для ввода: стартовое кол-во узлов в сети, вероятности событий
    //рандомные параметры: подписки узлов (вероятность подписаться на другой узел при создании сети жестко 1/2)

    cout << "Hello! Welcome to test_network project." << endl << "Please configure network. Enter empty string to set random value." << endl << endl;
    srand(time(NULL));

    cout << "Enter chances in % of actions in one line (example '5 10 20 30'): " << endl;
    cout << "EVENT SUBSCR UNSUB CREATE" << endl;
    while (1)
    {
        cout << "> ";
        getline(cin, cmd);
        if (cmd == "")
        {
            //генератор рандомных вероятностей с нормальным распределением
            int values[5];
            int sum = 0;
            for (int i = 0; i < 5; i++)
            {
                values[i] = rand() % 1000;
                sum += values[i];
            }
            for (int i = 0; i < 4; i++)
            {
                values[i] = (int)(values[i] * 100 / sum);
                network->setChance(i, values[i], true);
            }
            network->printChances();
            break;
        }
        else try
        {
            string s;
            int pos = 0;
            for (int i = 0; i < 4; i++)
            {
                s = "";
                s.append(cmd, pos, cmd.find_first_of(" ", pos)-pos);
                pos = cmd.find_first_of(" ", pos) + 1;
                //while ()
                network->setChance(i, stoi(s));
            }
            network->printChances();
            break;
        }
        //std::out_of_range&
        //std::invalid_argument&
        catch (...)
        {
            cout << "ERROR: invalid value. Try Again:" << endl;
        }
    }
    
    int startSize = 0;
    cout << "Enter start network size:" << endl;
    while (1)
    {
        cout << "> ";
        getline(cin, cmd);
        if (cmd == "")
        {
            while (startSize<2) startSize = rand() % 10;
            break;
        }
        else try
        {
            startSize = stoi(cmd);
            break;
        }
        catch (const std::invalid_argument& e)
        {
            cout << "ERROR: invalid value. Try Again:" << endl;
        }
    }

    //создание сети
    network->initFewUnits(startSize);
    network->printInfo();

    while (1)
    {
        cout << "Press ENTER for next step. Write 'help' to view avalible commands." << endl;
        while (1)
        {
            cout << "> ";
            getline(cin, cmd);
            if (cmd == "") goto NEXT;
            if (cmd == "stop") goto STOP;
            if (cmd == "min") network->setInfoType(Info::MIN);
            if (cmd == "base") network->setInfoType(Info::BASE);
            if (cmd == "full") network->setInfoType(Info::FULL);
            if (cmd == "show") network->printInfo();
            if (cmd == "help")
            {
                cout << "Avalible commands | 'command' : description" << endl;
                cout << "'help' : show hint" << endl;
                cout << "'stop' : terminate simulating" << endl;
                cout << "'min'  : minumum info in console (only network state)" << endl;
                cout << "'base' : base info in console (list of units)" << endl;
                cout << "'full' : maximum info in console" << endl;
                cout << "'show' : print (update) information in console" << endl;
            }
        }
        NEXT:
        network->updateNetwork();
        network->printInfo();
        if (!(network->getSize()))
        {
        STOP:
            break;
        }
    }
}
