// linklist.h
// single linked lists
// Copyright 1996 by Richard E. Haskell -- All rights reserved.
// Recoded based on std::list, Frank Palazzolo 2021

#include <list>

// generic linked list

template<class T>
class Linklist : public std::list<T>
{
public:
	Linklist() {};                   	// constructor
	~Linklist() {};                 		// destructor
	void push(T val);       				// add val to list
	T pop();      // pop val from list
};

template<class T>
void Linklist<T>::push(T val)
{
	this->push_back(val);
}

template<class T>
T Linklist<T>::pop()
{
	T rv = this->back(); 
	this->pop_back();
	return rv;
}
