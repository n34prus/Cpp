#pragma once
template <typename T>
class List {
private:

	template <typename T>
	//	�������� ����������
	class Node {
	public:
		Node* pNext;	//	��������� �� ��������� �������
		T data;			//	���� ����
		Node(T data=T(), Node* pNext = nullptr) {
			this->data = data;
			this->pNext = pNext;
		}
	};

	int size;			//	������ ����������
	Node<T> *head;		//	��������� �� ������� �������

public:
	
	List() {
		size = 0;
		head = nullptr;
	}
	~List() {
		clear();
	}

	int get_size() { return size; };

	//	���������� �������� � �����
	void push_back(T data) {
		if (head == nullptr) head = new Node<T>(data);
		else {
			Node<T>* current = this->head;
			while (current->pNext != nullptr)
			{
				current = current->pNext;
			}
			current->pNext = new Node<T>(data);
		}
		size++;
	};

	//	�������� ������� ��������
	void pop_front() {
		Node<T> *temp = head;
		head = head->pNext;
		delete temp;
		size--;
	}

	//	���������� �������� � ������
	void push_front(T data) {
		head = new Node<T>(data, head);
		size++;
	};

	//	���������� �������� �� �������
	void insert(T data, int index) {
		if (!index) push_front(data);
		else {
			Node<T> *prev = this->head;
			for (int i = 0; i < index - 1; i++) {
				prev = prev->pNext;
			}
			Node<T> *newNode = new Node<T>(data, prev->pNext);
			prev->pNext = newNode;
		}
		
	}

	//	�������� �������� �� �������
	void remove(int index) {
		if (!index) pop_front;
		else {
			Node<T> *prev = this->head;
			for (int i = 0; i < index - 1; i++) {
				prev = prev->pNext;
			}
			Node<T>* temp = prev->pNext;
			prev->pNext = temp->pNext;
			delete temp;
			size--;
		}
	}

	//	�������� ���������� ��������
	vod pop_back() {
		remove(size - 1);
	}

	//	������� ����������
	void clear() {
		while (size) pop_front();
	}

	//	���������� [] ��� ��������� �� ����������� �������
	T& operator[](const int index) {
		int counter = 0;
		Node<T>* current = this->head;
		while (current != nullptr)
		{
			if (counter == index) return current->data;
			current = current->pNext;
			counter++;
		}
	}
};