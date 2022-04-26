#pragma once
template <typename T>
class List {
private:

	template <typename T>
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

	int size;			//	размер контейнера
	Node<T> *head;		//	указатель на нулевой элемент
	Node<T>* tail;		//	указатель на последний элемент

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
	
	//	добавление элемента в конец
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
	
	//	добавление элемента в начало
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

	//	удаление первого элемента
	void pop_front() {
		Node<T> *temp = head;
		head = head->pNext;
		delete temp;
		size--;
	}

	//	удаление последнего элемента (однсявязный список)
	void pop_back() {
		Node<T>* temp = tail;
		tail = tail->pPrev;
		delete temp;
		size--;
	}

	// достаем указатель на элемент по индексу
	Node<T>* search(int index) {
		Node<T>* ptr = nullptr;
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

	//	добавление элемента по индексу
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

	//	удаление элемента по индексу
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

	//	очистка контейнера
	void clear() {
		while (size) pop_front();
	}

	//	перегрузка [] для обращения по абсолютному индексу
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
	//	ранние версии методов
	//	добавление элемента в конец (односвязный список)
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

	//	добавление элемента в начало (односвязный список)
	void push_front(T data) {
		head = new Node<T>(data, head);
		if (!size) tail = head;
		size++;
	};

	//	удаление последнего элемента (однсявязный список)
	void pop_back() {
		remove(size - 1);
	}

	//	добавление элемента по индексу
	void insert(T data, int index) {
		if (!index) push_front(data);
		else if (index == size) push_back(data);
		//	проход с головы
		else if (index <= (int)(size/2)) {
			Node<T> *ptr = this->head;
			for (int i = 0; i < index - 1; i++) {
				ptr = ptr->pNext;
			}
			//записываем в новый элемент указатели на соседние элементы
			Node<T> *newNode = new Node<T>(data, ptr->pNext, ptr);
			//записываем в предыдущий элемент указатель на новый элемент
			ptr->pNext = newNode;
			//записываем в следующий элемент указатель на новый элемент
			ptr = newNode->pNext;
			ptr->pPrev = newNode;
			size++;
		}
		//	проход с хвоста
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

	//	удаление элемента по индексу (односвязный список)
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

	//	перегрузка [] для обращения по абсолютному индексу
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