#ifndef _UNIT_H_
#define _UNIT_H_

#include <list>

using namespace std;
struct Messages;
class Network;
class Unit;
enum Info { SHORT, FULL };

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
	list <Unit*> subs;				//������ ��������
	list <Unit*> subers;			//������ ����������� (� ����� �� ��?)
	list<Messages> messages;		//��� �������� �����-���� ���������
public:
	Unit(Network* network,list<Unit*>subs = list<Unit*>())
	{
		this->network = network;
		this->subs = subs;
		addToNetwork();
	};
	~Unit() {};

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

	//	������� �������� �� ��������� �� ��
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

	//	���������� ����� ����������� ������ Network
	//	��������� ���� � ������ ����� ����
	void addToNetwork();
	//	���������� ������� ������ ���� (���������)
	void makeEvent();
	//	���������� ������ �� �������� ������ ����
	void makeNewUnit();
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
	Unit* addUnit(){ return new Unit(this); }
	//���������� ������������� ���� (��� ������)
	Unit* addUnit(Unit* unit){
		units.push_back(unit);
		return unit;
	}

	int getSize() { return units.size(); }

	//���������� ������ ����������� ���� unit
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