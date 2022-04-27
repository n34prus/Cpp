#ifndef _UNIT_H_
#define _UNIT_H_

#include <list>

using namespace std;

template< typename T >
class Unit {
private:
	T* network;
	list <Unit*> subs;		//список подписчиков
	struct Messages					//принятые сообщения
	{
		Unit* unit;
		list <int> data;
		Messages(Unit* unit = nullptr, list<int>data = list<int>())
		{
			this->unit = unit;
			this->data = data;
		};
	};
	Messages messages;
public:
	template< typename T >
	Unit(T* network,list<Unit*>subs = list<Unit*>(), Messages messages = Messages())
	{
		this->network = network;
		this->subs = subs;
		this->messages = messages;
	};
	~Unit() {};

	//	возвращает список подписок
	list <Unit*> getSubs(){return subs;};

	//	возвращает число подписк
	int sizeSubs(){return subs.size();};

	//	добавляет новую уникальную подписку по указателю на неё
	void addSub(Unit* unit)
	{
		subs.push_back(unit);
		subs.unique();
	};

	//	удаляет подписку по указателю на неё
	void removeSub(Unit* unit)
	{
		subs.remove(unit);
	};

	

};

#endif