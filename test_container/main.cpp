#include <iostream>
#include "container.h" 

using namespace std;
List<int> list;

void print() {
    for (int i = 0; i < list.get_size(); i++) cout << list[i] << " ";
    cout << endl;
};

int main()
{
    list.push_front(2);
    print();

    list.push_front(1);
    print();

    list.push_back(3);
    print();

    list.push_back(4);
    print();

    list.push_back(5);
    print();

    //list.pop_back();
    //print();

    list.insert(556, 1);
    print();

    list.remove(4);
    print();

    //List<int>::Iterator it;
    //it = list.begin();

    //list.pop_front();
}