#pragma once
template <typename T>
class List {
private:

	template <typename T>
	//	�������� ����������
	class Node {
	public:
		Node* pNext;	//	��������� �� ��������� �������
		Node* pPrev;	//	��������� �� ���������� �������
		T data;			//	���� ����
		Node(T data=T(), Node* pNext = nullptr, Node* pPrev = nullptr) {
			this->data = data;
			this->pNext = pNext;
			this->pPrev = pPrev;
		}
	};

	int size;			//	������ ����������
	Node<T> *head;		//	��������� �� ������� �������
	Node<T>* tail;		//	��������� �� ��������� �������

public:
	
	List() {
		size = 0;
		head = nullptr;
		tail = nullptr;
	}
	~List() {
		clear();
	}

	int get_size() { return size; };
	
	//	���������� �������� � �����
	void push_back(T data) {
		if (size) {
			Node<T>* temp = tail;
			tail = new Node<T>(data, nullptr, tail);
			temp->pNext = tail;
		}
		else {
			tail = new Node<T>(data);
			head = tail;
		}
		size++;
	}
	
	//	���������� �������� � ������
	void push_front(T data) {
		if (size) {
			Node<T>* temp = head;
			head = new Node<T>(data, head, nullptr);
			temp->pPrev = head;
		}
		else {
			head = new Node<T>(data);
			tail = head;
		}
		size++;
	}

	//	�������� ������� ��������
	void pop_front() {
		Node<T> *temp = head;
		head = head->pNext;
		delete temp;
		size--;
	}

	//	�������� ���������� �������� (����������� ������)
	void pop_back() {
		Node<T>* temp = tail;
		tail = tail->pPrev;
		delete temp;
		size--;
	}

	// ������� ��������� �� ������� �� �������
	Node<T>* search(int index) {
		Node<T>* ptr = nullptr;
		//	������ � ������
		if (index < (int)(size / 2)) {
			ptr = this->head;
			for (int i = 0; i < index; i++) {
				ptr = ptr->pNext;
			}
		}
		//	������ � ������
		else {
			ptr = this->tail;
			for (int i = size - 1; i > index; i--) {
				ptr = ptr->pPrev;
			}
		}
		return ptr;
	}

	//	���������� �������� �� �������
	void insert(T data, int index) {
		if (!index) push_front(data);
		else if (index == size) push_back(data);
		else{
			Node<T>* ptr = search(index);
			Node<T>* newNode = new Node<T>(data, ptr, ptr->pPrev);
			ptr->pPrev = newNode;
			ptr = newNode->pPrev;
			ptr->pNext = newNode;
			size++;
		}
	}

	//	�������� �������� �� �������
	void remove(int index) {
		if (!index) pop_front();
		else if (index == size - 1) pop_back();
		else {
			Node<T>* ptr = search(index);
			Node<T>* temp = ptr->pPrev;
			temp->pNext = ptr->pNext;
			temp = ptr->pNext;
			temp->pPrev = ptr->pPrev;
			delete ptr;
			size--;
		}
	}

	//	������� ����������
	void clear() {
		while (size) pop_front();
	}

	//	���������� [] ��� ��������� �� ����������� �������
	T& operator[](const int index) {
		Node<T>* ptr = search(index);
		return ptr->data;
	}
	/*
	class Iterator {
	protected:
		Node<T> *nodePtr;
	public:
		explicit Iterator(Node<T>* nodePtr = nullptr);
		Iterator(const Iterator& Iterator<T>);
		~Iterator() {

		}
		/*
		iterator *begin() {
			nodePtr = head;
			return this;
		};
		*/
		//void end();
		/*
		T& get() {
			return *nodePtr->data;
		}
		*//*
	};

	Iterator& operator=(const List<T>::Iterator &*a) {
		this->nodePtr = a.nodePtr;
	}

	Iterator<T> begin() { return Iterator<T>(head); }
	Iterator<T> end() { return Iterator<T>(tail); }	//->pNext?

	Iterator<T> begin() const { return Iterator<T>(head); }
	Iterator<T> end() const { return Iterator<T>(tail); }

	Iterator<const T> cbegin() const { return head; }
	Iterator<const T> cend() const { return tail; }*/

	/*
	//	������ ������ �������
	//	���������� �������� � ����� (����������� ������)
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

	//	���������� �������� � ������ (����������� ������)
	void push_front(T data) {
		head = new Node<T>(data, head);
		if (!size) tail = head;
		size++;
	};

	//	�������� ���������� �������� (����������� ������)
	void pop_back() {
		remove(size - 1);
	}

	//	���������� �������� �� �������
	void insert(T data, int index) {
		if (!index) push_front(data);
		else if (index == size) push_back(data);
		//	������ � ������
		else if (index <= (int)(size/2)) {
			Node<T> *ptr = this->head;
			for (int i = 0; i < index - 1; i++) {
				ptr = ptr->pNext;
			}
			//���������� � ����� ������� ��������� �� �������� ��������
			Node<T> *newNode = new Node<T>(data, ptr->pNext, ptr);
			//���������� � ���������� ������� ��������� �� ����� �������
			ptr->pNext = newNode;
			//���������� � ��������� ������� ��������� �� ����� �������
			ptr = newNode->pNext;
			ptr->pPrev = newNode;
			size++;
		}
		//	������ � ������
		else {
			Node<T>* ptr = this->tail;
			for (int i = size-1; i > index; i--) {
				ptr = ptr->pPrev;
			}
			Node<T>* newNode = new Node<T>(data, ptr, ptr->pPrev);
			ptr->pPrev = newNode;
			ptr = newNode->pPrev;
			ptr->pNext = newNode;
			size++;
		}
	}

	//	�������� �������� �� ������� (����������� ������)
	void remove(int index) {
		if (!index) pop_front;
		else if (index == size - 1) pop_back();
		else {
			Node<T> *ptr = this->head;
			for (int i = 0; i < index - 1; i++) {
				ptr = ptr->pNext;
			}
			Node<T>* temp = ptr->pNext;
			ptr->pNext = temp->pNext;
			delete temp;
			size--;
		}
	}

	//	���������� [] ��� ��������� �� ����������� �������
	T& operator[](const int index) {
		int counter;
		if (index <= (int)(size / 2)) {
			counter = 0;
			Node<T>* ptr = this->head;
			while (ptr != nullptr)
			{
				if (counter == index) return ptr->data;
				ptr = ptr->pNext;
				counter++;
			}
		}
		else {
			counter = size-1;
			Node<T>* ptr = this->tail;
			while (ptr != nullptr)
			{
				if (counter == index) return ptr->data;
				ptr = ptr->pPrev;
				counter--;
			}
		}
	}

	*/

};