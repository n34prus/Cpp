#pragma once
template <typename T>
class List {
private:

	template <typename T>
	//	элементы контейнера
	class Node {
	public:
		Node* pNext;	//	указатель на следующий элемент
		T data;			//	юзер дата
		Node(T data=T(), Node* pNext = nullptr) {
			this->data = data;
			this->pNext = pNext;
		}
	};

	int size;			//	размер контейнера
	Node<T> *head;		//	указатель на нулевой элемент

public:
	
	List() {
		size = 0;
		head = nullptr;
	}
	~List() {
		clear();
	}

	int get_size() { return size; };

	//	добавление элемента в конец
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

	//	удаление первого элемента
	void pop_front() {
		Node<T> *temp = head;
		head = head->pNext;
		delete temp;
		size--;
	}

	//	добавление элемента в начало
	void push_front(T data) {
		head = new Node<T>(data, head);
		size++;
	};

	//	добавление элемента по индексу
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

	//	удаление элемента по индексу
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

	//	удаление последнего элемента
	vod pop_back() {
		remove(size - 1);
	}

	//	очистка контейнера
	void clear() {
		while (size) pop_front();
	}

	//	перегрузка [] для обращения по абсолютному индексу
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