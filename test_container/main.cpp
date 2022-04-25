#include <iostream>
#include "container.h" 

using namespace std;

int main()
{
    List<int> list;
    list.push_back(5);
    list.push_back(40);
    list.push_back(300);

    cout << list.get_size() << endl;
    cout << list[1] << endl;

    list.pop_front();

    cout << list[1] << endl;
}