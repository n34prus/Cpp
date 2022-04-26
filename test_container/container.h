#pragma once
template <class T>
class List {
private:
	//	элементы контейнера
	class Node {
	public:
		Node* pNext;	//	указатель на следующий элемент
		Node* pPrev;	//	указатель на предыдущий элемент
		T data;			//	юзер дата
		Node(T data=T(), Node* pNext = nullptr, Node* pPrev = nullptr) {
			this->data = data;
			this->pNext = pNext;
			this->pPrev = pPrev;
		}
	};
	int size;		//	размер контейнера
	Node* head;		//	указатель на нулевой элемент
	Node* tail;		//	указатель на последний элемент

	// достаем указатель на элемент по индексу
	Node* search(int index) {
		Node* ptr = nullptr;
		//	проход с головы
		if (index < (int)(size / 2)) {
			ptr = this->head;
			for (int i = 0; i < index; i++) {
				ptr = ptr->pNext;
			}
		}
		//	проход с хвоста
		else {
			ptr = this->tail;
			for (int i = size - 1; i > index; i--) {
				ptr = ptr->pPrev;
			}
		}
		return ptr;
	}

public:
	class Iterator
	{
	private:
		Node* nodePtr;

	public:
		Iterator() { this->nodePtr = nullptr; }
		Iterator(Node* nodePtr) { this->nodePtr = nodePtr; }

		Iterator operator++(int) {
			Iterator it = *this;
			nodePtr = nodePtr->pNext;
			return it;
		}
		Iterator operator--(int) {
			Iterator it = *this;
			nodePtr = nodePtr->pPrev;
			return it;
		}
		Iterator operator+=(const int& shift) {
			Iterator it = *this;
			for (int i=0; i<shift; i++)	nodePtr = nodePtr->pNext;
			return it;
		}
		Iterator operator-=(const int& shift) {
			Iterator it = *this;
			for (int i = 0; i < shift; i++)	nodePtr = nodePtr->pPrev;
			return it;
		}
		T& operator*() { return nodePtr->data; }
		bool operator==(const Iterator& it) { return nodePtr->data == it.nodePtr->data; }
		bool operator!=(const Iterator& it) { return nodePtr->data != it.nodePtr->data; }
	};

	List() {
		size = 0;
		head = nullptr;
		tail = nullptr;
	}
	~List() {
		clear();
	}

	Iterator begin() { return Iterator(head); }
	Iterator last() { return Iterator(tail); }
	Iterator end() { return Iterator(tail->pNext); }

	//	возвращает размер контейнера
	int get_size() { return size; };
	
	//	добавление элемента в конец
	void push_back(T data) {
		if (size) {
			Node* temp = tail;
			tail = new Node(data, nullptr, tail);
			temp->pNext = tail;
		}
		else {
			tail = new Node(data);
			head = tail;
		}
		size++;
	}
	
	//	добавление элемента в начало
	void push_front(T data) {
		if (size) {
			Node* temp = head;
			head = new Node(data, head, nullptr);
			temp->pPrev = head;
		}
		else {
			head = new Node(data);
			tail = head;
		}
		size++;
	}

	//	удаление первого элемента
	void pop_front() {
		Node *temp = head;
		head = head->pNext;
		delete temp;
		size--;
	}

	//	удаление последнего элемента
	void pop_back() {
		Node* temp = tail;
		tail = tail->pPrev;
		delete temp;
		size--;
	}

	//	добавление элемента по индексу
	void insert(T data, int index) {
		if (!index) push_front(data);
		else if (index == size) push_back(data);
		else{
			Node* ptr = search(index);
			Node* newNode = new Node(data, ptr, ptr->pPrev);
			ptr->pPrev = newNode;
			ptr = newNode->pPrev;
			ptr->pNext = newNode;
			size++;
		}
	}

	//	удаление элемента по индексу
	void remove(int index) {
		if (!index) pop_front();
		else if (index == size - 1) pop_back();
		else {
			Node* ptr = search(index);
			Node* temp = ptr->pPrev;
			temp->pNext = ptr->pNext;
			temp = ptr->pNext;
			temp->pPrev = ptr->pPrev;
			delete ptr;
			size--;
		}
	}

	//	очистка контейнера
	void clear() {
		while (size) pop_front();
	}

	//	перегрузка [] для обращения по абсолютному индексу
	T& operator[](const int index) {
		Node* ptr = search(index);
		return ptr->data;
	}

};