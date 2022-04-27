#ifndef _UNIT_H_
#define _UNIT_H_

#include <list>

using namespace std;
struct Messages;
class Network;
class Unit;
enum Info { SHORT, FULL };

struct Messages					//структура сообщения или сообщений от одного источника
{
	Unit* unit;					//указатель источника
	list <int> data;			//список полученных сообщений

	Messages(Unit* unit = nullptr, list<int>data = list<int>())
	{
		this->unit = unit;
		this->data = data;
	};
	Messages(Unit* unit = nullptr, int data = int())
	{
		this->unit = unit;
		this->data.push_back(data);
	};
};

class Unit {
private:
	Network* network;
	list <Unit*> subs;				//список подписок
	list <Unit*> subers;			//список подписчиков (а нужен ли он?)
	list<Messages> messages;		//все принятые когда-либо сообщения
public:
	Unit(Network* network,list<Unit*>subs = list<Unit*>())
	{
		this->network = network;
		this->subs = subs;
		addToNetwork();
	};
	~Unit() {};

	//	возвращает список подписок
	list <Unit*> getSubs(){return subs;};
	//	возвращает число подписк
	int sizeSubs(){return subs.size();};

	//	возвращает список подписчиков
	list <Unit*> getSubers() { return subers; };
	//	возвращает число подписчиков
	int sizeSubers() { return subers.size(); };

	//	добавляет новую уникальную подписку по указателю на неё
	void addSub(Unit* unit)
	{
		if (unit != this)
		{
			subs.push_back(unit);
			subs.unique();
		}
	};

	//	удаляет подписку по указателю на неё
	void removeSub(Unit* unit)
	{
		subs.remove(unit);
	};

	void printInfo(Info info)
	{
		updateSubers();
		cout << "Unit " << this << " info:" << endl;
		cout << "network: " << network << endl;
		if (info == SHORT)
		{
			cout << "subscribes: " << sizeSubs() << endl;
			cout << "subscribers: " << sizeSubers() << endl;
		}
		else if (info == FULL)
		{
			cout << "subscribes: " << endl;
			int i = 1;
			for (list <Unit*>::iterator it = subs.begin(); it != subs.end(); it++)
				cout << i++ << " " << *it << endl;
			cout << endl;
			cout << "subscribers: " << endl;
			i = 1;
			for (list <Unit*>::iterator it = subers.begin(); it != subers.end(); it++)
				cout << i++ << " " << *it << endl;
			cout << endl;
		}
	}

	//	реализация после определения класса Network
	//	добавляет себя в список узлов сети
	void addToNetwork();
	//	генерирует событие внутри сети (сообщение)
	void makeEvent();
	//	генерирует запрос на создание нового узла
	void makeNewUnit();
	//	обновляет список своих подписчиков
	void updateSubers();
};

class Network
{
private:
	list <Unit*> units;		//узлы сети
	list <Messages> pool;	//события текущей итерации
public:
	Network() {};
	~Network() {};

	//добавление нового узла
	Unit* addUnit(){ return new Unit(this); }
	//добавление существующего узла (для дебага)
	Unit* addUnit(Unit* unit){
		units.push_back(unit);
		return unit;
	}

	int getSize() { return units.size(); }

	//возвращает список подписчиков узла unit
	list <Unit*> findSubers(Unit* unit)
	{
		list<Unit*> subers;
		for (list <Unit*>::iterator itc = units.begin(); itc != units.end(); itc++)
		{
			Unit* candidat = *itc;
			list<Unit*> candidatSubs = candidat->getSubs();
			list <Unit*>::iterator its = candidatSubs.begin();
			while (its != candidatSubs.end() && *its != unit)	its++;
			if (its != candidatSubs.end()) subers.push_back(candidat);
		}
		return subers;
	}

	//фиксация вызванных событий в пул событий
	void fixEvent(Messages message) { pool.push_back(message); }
	//обработка событий в пуле
	void doEvents()
	{
		list<int>::iterator itl;
		for (list <Messages>::iterator itm = pool.begin(); itm != pool.end(); itm++)
		{
			Messages buffer = *itm;
			itl = buffer.data.begin();
			cout << &(*itm->unit) << " " << *itl << endl;
		}
	}

	void printInfo(Info info)
	{
		cout << endl << "Network " << this << " size: " << getSize() << endl;
		for (list <Unit*>::iterator it = units.begin(); it != units.end(); it++)
			(*it)->printInfo(info);
	}
};

void Unit::addToNetwork()
{
	network->addUnit(this);
}

void Unit::makeEvent()
{
	Messages message = Messages(this, (rand()%100));
	network->fixEvent(message);
}

void Unit::makeNewUnit()
{
	addSub(network->addUnit());
}

void Unit::updateSubers()
{
	subers = network->findSubers(this);
}

#endif