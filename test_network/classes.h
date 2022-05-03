#ifndef _UNIT_H_
#define _UNIT_H_

#include <list>
#include <windows.h>

using namespace std;
struct Messages;
class Network;
class Unit;
enum class Info { MIN, BASE, FULL };

void textColour(string s)
{
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (s == "lgreen")	SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_GREEN);
	else if (s == "lred")	SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_RED);
	else if (s == "lblue")	SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_BLUE);
	else SetConsoleTextAttribute(handle, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

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
	list <Unit*> subs;								//список подписок
	list <Unit*> subers;							//список подписчиков (а нужен ли он?)
	list<Messages> messages;						//все принятые когда-либо сообщения

public:
	Unit(Network* network, Unit* suber = nullptr)
	{
		srand(time(NULL));
		this->network = network;
		subers = list <Unit*>();
		if (suber != nullptr)	subers.push_back(suber);
		addToNetwork();
	};
	~Unit() {};

	//возвращает итератор на узел по его указателю
	static list <Unit*>::iterator searchUnit(list<Unit*>& container, Unit* unit)
	{
		list <Unit*>::iterator it = container.begin();
		while (it != container.end() && *it != unit)	it++;
		return it;
	}



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

	//	подписка на рандомного соседа
	void addRandSub()
	{
		//составляем список доступных для подписки соседей
		list<Unit*>::iterator it = subers.begin();
		list<Unit*> avalibleUnits;
		//проходим по списку всех подписчиков
		while (it != subers.end())
		{
			//если есть подписчик, но его нет в подписках, добавляем его в список доступных
			if (searchUnit(subs, *it) == subs.end()) avalibleUnits.push_back(*it);
			it++;
		}
		//если получившийся список не пуст, то берем оттуда произвольный элемент и ососеживаемся с ним
		if (avalibleUnits.size() > 0)
		{
			list<Unit*> :: iterator itn = avalibleUnits.begin();
			for (int i = 0; i < rand() % avalibleUnits.size(); i++) itn++;
			addSub(*itn);
		}
	}

	//	удаляет подписку по указателю на неё
	void removeSub(Unit* unit)
	{
		subs.remove(unit);
	};
	//	удаляет подписку на производльного соседа
	void removeRandSub()
	{
		if (subs.size() > 0)
		{
			list<Unit*> ::iterator it = subs.begin();
			for (int i = 0; i < rand() % subs.size(); i++) it++;
			removeSub(*it);
		}
	}

	void printInfo(Info info)
	{
		//updateSubers();
		if (sizeSubs() == 0 && sizeSubers() == 0) textColour("lred");
		else textColour("lgreen");
		cout << "Unit " << this << " info:";
		textColour("");
		if (info == Info::BASE)
		{
			cout << " " << sizeSubs() << " subs |";
			cout << " " << sizeSubers() << " subers" << endl;
		}
		else if (info == Info::FULL)
		{
			cout << endl << "network: " << network << endl;
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

	void handler(Messages message)
	{
		//в полученной структуре смотрим есть ли .unit в наших подписках
		//если да, то:
			//в нашей messages ищем it на Messages с таким .unit
			//если нет то добавляем новый Mesages к нашему списку
			//добавляем в (*it)->data число message.data.front()
			//выводим в консоль для начала все data от этого unit
		for (Unit* sub : subs)	if (message.unit == sub)
		{
			//for (Messages mes : messages)
			list <Messages>::iterator it = messages.begin();
			while (it != messages.end() && it->unit != message.unit) it++;
			if (it != messages.end()) it->data.push_back(message.data.front());
			else
			{
				messages.push_back(message);
				it = messages.end();
				it--;
			}
			cout << message.unit << " -> " << this << " : ";
			if (rand() % 2)
				cout << "N = " << (it)->data.size() << endl;
			else
			{
				int sum = 0;
				for (int n : (it)->data) sum += n;
				cout << "S = " << sum << endl;
			}
		}
			
	}

	//	реализация после определения класса Network
	//	добавляет себя в список узлов сети
	void addToNetwork();
	//	генерирует событие внутри сети (сообщение)
	void makeEvent();
	//	генерирует запрос на создание нового узла
	void makeNewUnit();
	//	очистка подписок от удаленных элементов
	void updateSubs();
	//	обновляет список своих подписчиков
	void updateSubers();
	//	выбор действия и его осуществление
	void doAction();

};

class Network
{
private:
	int iteration = 0;
	list <Unit*> units;		//узлы сети
	list <Messages> pool;	//события текущей итерации
	int chances[4] = { 0,0,0,0 };					//вероятности по умолчанию % (ничего не делать = 100-sum(chances[i]))
	Info info = Info::BASE;
public:
	Network() {};
	~Network() {};

	//создание нового узла (без подписчика)
	Unit* newUnit(){ return new Unit(this); }
	//создание нового узла (с подписчиком)
	Unit* newUnit(Unit* unit) { return new Unit(this, unit); }
	//добавление существующего узла
	Unit* addUnit(Unit* unit)
	{
		units.push_front(unit);
		return unit;
	}

	//удаление существующего узла
	list <Unit*>::iterator removeUnit(Unit* unit)
	{
		list <Unit*>::iterator it = Unit::searchUnit(units, unit);
		if (it != units.end()) it = units.erase(it);
		delete unit;
		return it;
	}

	//операция обновления сети
	void updateNetwork()
	{
		//очистка узлов без соседей
		list <Unit*>::iterator it = units.begin();
		while (1)
		{
			if (it == units.end()) break;
			if ((*it)->sizeSubs() == 0 && (*it)->sizeSubers() == 0) it = removeUnit(*it);
			else if (it != units.end()) it++;
		}
		//передача событий в узлы (перебор узлов)
		doEvents();
		//обновление subs и subers у узлов
		for (list <Unit*>::iterator it = units.begin(); it != units.end(); it++)
			(* it)->updateSubs();
		for (list <Unit*>::iterator it = units.begin(); it != units.end(); it++)
			(*it)->updateSubers();
			//действия узлов (перебор)
		for (list <Unit*>::iterator it = units.begin(); it != units.end(); it++)
			(*it)->doAction();

		iteration++;
	}

	int getSize() { return units.size(); }

	//возвращает список подписчиков узла unit
	list <Unit*> findSubers(Unit* unit)
	{
		list<Unit*> subers;
		for (list <Unit*>::iterator itc = units.begin(); itc != units.end(); itc++)
		{
			list<Unit*> candidatSubs = (*itc)->getSubs();
			if (Unit::searchUnit(candidatSubs, unit) != candidatSubs.end()) subers.push_back(*itc);
		}
		subers.unique();
		return subers;
	}

	//возвращает список активных подписок узла unit
	list <Unit*> findSubs(Unit* unit)
	{
		list<Unit*> subs = unit->getSubs();
		for (list <Unit*>::iterator it = subs.begin(); it != subs.end(); it++)
			if (Unit::searchUnit(units, unit)==units.end())it = subs.erase(it);
		subs.unique();
		return subs;
	}

	//фиксация вызванных событий в пул событий
	void fixEvent(Messages message) { pool.push_back(message); }
	//обработка событий в пуле
	void doEvents()
	{
		// перебрать все юниты
		// перебрать все события
		// попробовать вызвать обработчик у узла
		// очистить пул сообщений
		for (list <Unit*>::iterator itu = units.begin(); itu != units.end(); itu++)
			for (list <Messages>::iterator itm = pool.begin(); itm != pool.end(); itm++)
				(*itu)->handler(*itm);
		pool.clear();
	}

	void setChance(int action, int chance, bool slient = 0)
	{
		int lastChance = chances[action];
		if (chance >= 0 && chance <= 100) chances[action] = chance;
		int sum = 0;
		for (int i = 0; i < size(chances); i++) sum += chances[i];
		if (sum > 100)
		{
			chances[action] = chances[action] + 100 - sum;
			if (!slient)
				cout << "Sum of chances can not be higher than 100%. Value was decreased." << endl;
		}
	}

	int getChance(int action)
	{
		return chances[action];
	}

	void printChances()
	{
		cout << "- create event: " << chances[0] << "%" << endl;
		cout << "- add subscription: " << chances[1] << "%" << endl;
		cout << "- delete subscription: " << chances[2] << "%" << endl;
		cout << "- create new unit: " << chances[3] << "%" << endl;
		int sum = 0;
		for (int i = 0; i < size(chances); i++) sum += chances[i];
		cout << "- do nothing: " << (100 - sum) << "%" << endl;
	}

	void initFewUnits(int count)
	{
		srand(time(NULL));
		for (int i = 0; i < count; i++) newUnit();
		for (auto unit1 : units)
			for (auto unit2 : units)
				if (rand() % 2) unit1->addSub(unit2);
	}

	void setInfoType(Info type)
	{
		info = type;
	}

	void printInfo()
	{
		textColour("lblue");
		cout << endl << "Network " << this << " | iteration " << iteration << " | size " << getSize() << endl;
		textColour(" ");
		if (info != Info::MIN)
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
	addSub(network->newUnit(this));
}

void Unit::updateSubs()
{
	subs = network->findSubs(this);
}

void Unit::updateSubers()
{
	subers = network->findSubers(this);
}

void Unit::doAction()
{
	uint8_t k = rand() % 100;
	int i;
	for (i = 0; i < 4; i++)
	{
		if (k < network->getChance(i)) break;
		else k -= network->getChance(i);
	}
	if (!i) makeEvent();
	else if (i == 1) addRandSub();
	else if (i == 2) removeRandSub();
	else if (i == 3) makeNewUnit();
}

#endif