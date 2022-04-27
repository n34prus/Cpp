#ifndef _UNIT_H_
#define _UNIT_H_

#include <list>

using namespace std;

template< typename T >
class Unit {
private:
	T* network;
	list <Unit*> subs;		//������ �����������
	struct Messages					//�������� ���������
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

	//	���������� ������ ��������
	list <Unit*> getSubs(){return subs;};

	//	���������� ����� �������
	int sizeSubs(){return subs.size();};

	//	��������� ����� ���������� �������� �� ��������� �� ��
	void addSub(Unit* unit)
	{
		subs.push_back(unit);
		subs.unique();
	};

	//	������� �������� �� ��������� �� ��
	void removeSub(Unit* unit)
	{
		subs.remove(unit);
	};

	

};

#endif