//  демонстрация работы методов контейнера
//  Каргин М.С.

#include <iostream>
#include "container.h" 

using namespace std;
List<int> list;

void print() {
    for (int i = 0; i < list.get_size(); i++) cout << list[i] << " ";
    cout << endl << endl;
};
void print(string s) {
    cout << s << endl;
    for (int i = 0; i < list.get_size(); i++) cout << list[i] << " ";
    cout << endl << endl;
};

int main()
{
    srand(time(NULL));

    //  работа по индексу
    //  добавление элементов
    for (int i = 0; i < 5; i++) {
        list.push_front(rand()%10);
        list.push_back(rand() % 10);
    }
    print("push_front & push_back:");

    //  удаление первого и последнего
    list.pop_back();
    list.pop_front();
    print("pop_front & pop_back:");

    //  вставка и удаление по индексу
    list.insert(777, 1);
    list.remove(list.get_size()-2);
    print("insert second & remove penult:");

    //  работа по указателю
    List<int>::Iterator it;
    cout << "iterators work:" << endl;

    //  сдвигаем вперед от нулевого элемента на 3
    it = list.begin();
    it+=2;
    it++;
    cout << *it << " | ";

    //  сдвигаем назад от последнего элемента на 3 (list.end() указывает на пустую область!)
    it = list.last();
    it -= 2;
    it--;
    cout << *it << endl;

    //  меняем текущий элемент и выводим все данные
    *it = 777;
    for (List<int>::Iterator i = list.begin(); i != list.end(); i++) {
        cout << *i << " ";
    }        


}