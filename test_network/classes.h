#ifndef _UNIT_H_
#define _UNIT_H_

#include <list>
#include <windows.h>

using namespace std;
struct Messages;
class Network;
class Unit;
enum Info { BASE, FULL };

void textColour(string s)
{
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (s == "lgreen")	SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_GREEN);
	else if (s == "lred")	SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_RED);
	else if (s == "lblue")	SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_BLUE);
	else SetConsoleTextAttribute(handle, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

struct Messages					//��������� ��������� ��� ��������� �� ������ ���������
{
	Unit* unit;					//��������� ���������
	list <int> data;			//������ ���������� ���������

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
	list <Unit*> subs;								//������ ��������
	list <Unit*> subers;							//������ ����������� (� ����� �� ��?)
	list<Messages> messages;						//��� �������� �����-���� ���������
	uint8_t chances[5] = {20,20,20,20,20};			//����������� �� ��������� %

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

	//���������� �������� �� ���� �� ��� ���������
	static list <Unit*>::iterator searchUnit(list<Unit*>& container, Unit* unit)
	{
		list <Unit*>::iterator it = container.begin();
		while (it != container.end() && *it != unit)	it++;
		return it;
	}

	//	���������� ������ ��������
	list <Unit*> getSubs(){return subs;};
	//	���������� ����� �������
	int sizeSubs(){return subs.size();};

	//	���������� ������ �����������
	list <Unit*> getSubers() { return subers; };
	//	���������� ����� �����������
	int sizeSubers() { return subers.size(); };

	//	��������� ����� ���������� �������� �� ��������� �� ��
	void addSub(Unit* unit)
	{
		if (unit != this)
		{
			subs.push_back(unit);
			subs.unique();
		}
	};

	//	�������� �� ���������� ������
	void addRandSub()
	{
		//���������� ������ ��������� ��� �������� �������
		list<Unit*>::iterator it = subers.begin();
		list<Unit*> avalibleUnits;
		while (it != subers.end())
		{
			if (searchUnit(subs, *it) != subs.end()) avalibleUnits.push_back(*it);
			it++;
		}
		//���� ������������ ������ �� ����, �� ����� ������ ������������ ������� � ������������� � ���
		if (avalibleUnits.size() > 0)
		{
			list<Unit*> :: iterator itn = avalibleUnits.begin();
			for (int i = 0; i < rand() % avalibleUnits.size(); i++) itn++;
			addSub(*itn);
		}
	}

	//	������� �������� �� ��������� �� ��
	void removeSub(Unit* unit)
	{
		subs.remove(unit);
	};
	void printInfo(Info info)
	{
		//updateSubers();
		if (sizeSubs() == 0 && sizeSubers() == 0) textColour("lred");
		else textColour("lgreen");
		cout << "Unit " << this << " info:";
		textColour("");
		if (info == BASE)
		{
			cout << " " << sizeSubs() << " subs |";
			cout << " " << sizeSubers() << " subers" << endl;
		}
		else if (info == FULL)
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

	//	����� �������� � ��� �������������
	void doAction()
	{
		uint8_t k = rand() % 100;
		int i;
		for (i = 0; i < size(chances); i++)
		{
			if (k < chances[i]) break;
			else k -= chances[i];
		}
		if (!i) makeEvent();
		else if (i == 1) addRandSub();
	}

	//	���������� ����� ����������� ������ Network
	//	��������� ���� � ������ ����� ����
	void addToNetwork();
	//	���������� ������� ������ ���� (���������)
	void makeEvent();
	//	���������� ������ �� �������� ������ ����
	void makeNewUnit();
	//	������� �������� �� ��������� ���������
	void updateSubs();
	//	��������� ������ ����� �����������
	void updateSubers();

};

class Network
{
private:
	list <Unit*> units;		//���� ����
	list <Messages> pool;	//������� ������� ��������
public:
	Network() {};
	~Network() {};

	//���������� ������ ����
	Unit* newUnit(){ return new Unit(this); }
	Unit* newUnit(Unit* unit) { return new Unit(this, unit); }
	//���������� ������������� ����
	Unit* addUnit(Unit* unit)
	{
		units.push_back(unit);
		return unit;
	}

	//�������� ������������� ����
	list <Unit*>::iterator removeUnit(Unit* unit)
	{
		list <Unit*>::iterator it = Unit::searchUnit(units, unit);
		if (it != units.end()) it = units.erase(it);
		return it;
	}

	//�������� ���������� ����
	void updateNetwork()
	{
		//������� ����� ��� �������
		list <Unit*>::iterator it = units.begin();
		while (1)
		{
			if (it == units.end()) break;
			if ((*it)->sizeSubs() == 0 && (*it)->sizeSubers() == 0) it = removeUnit(*it);
			else if (it != units.end()) it++;
		}

		//for (list <Unit*>::iterator it = units.begin(); it != units.end(); it++)
		//	if ((*it)->sizeSubs() == 0 && (*it)->sizeSubers() == 0) it=removeUnit(*it);
		//�������� ������� � ���� (������� �����)
		doEvents();

		//���������� subs � subers � �����
		for (list <Unit*>::iterator it = units.begin(); it != units.end(); it++)
			(* it)->updateSubs();
		for (list <Unit*>::iterator it = units.begin(); it != units.end(); it++)
			(*it)->updateSubers();
			//�������� ����� (�������)
		for (list <Unit*>::iterator it = units.begin(); it != units.end(); it++)
			(*it)->doAction();
	}

	int getSize() { return units.size(); }

	//���������� ������ ����������� ���� unit
	list <Unit*> findSubers(Unit* unit)
	{
		list<Unit*> subers;
		for (list <Unit*>::iterator itc = units.begin(); itc != units.end(); itc++)
		{
			list<Unit*> candidatSubs = (*itc)->getSubs();
			if (Unit::searchUnit(candidatSubs, unit) != candidatSubs.end()) subers.push_back(*itc);
		}
		return subers;
	}

	//���������� ������ �������� �������� ���� unit
	list <Unit*> findSubs(Unit* unit)
	{
		list<Unit*> subs = unit->getSubs();
		for (list <Unit*>::iterator it = subs.begin(); it != subs.end(); it++)
			if (Unit::searchUnit(units, unit)==units.end())it = subs.erase(it);
		return subs;
	}

	//�������� ��������� ������� � ��� �������
	void fixEvent(Messages message) { pool.push_back(message); }
	//��������� ������� � ����
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
		textColour("lblue");
		cout << endl << "Network " << this << " size: " << getSize() << endl;
		textColour(" ");
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

#endif