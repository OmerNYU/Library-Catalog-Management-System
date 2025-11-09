#ifndef MYVECTOR_H
#define MYVECTOR_H

//============================================================================
// Name         : myvector.h
// Author       : 
// Version      : 1.0
// Date         : 
// Date Modified: 09-02-2025 
// Description  : Vector implementation in C++
//============================================================================

#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <stdexcept>
#include <sstream>

using namespace std;

// Template class for a custom vector implementation
template <typename T>
class MyVector 
{
	private:
	    T *array;                // Pointer to dynamically allocated array to store elements
	    int v_size;              // Current number of elements in the vector
	    int v_capacity;          // Total capacity of the vector (size of the allocated array)

	public:
	   //Constructors and Destructors
	   //Default Constructor
	   MyVector();
	   //Copy Constructor
	   MyVector(const MyVector<T>& other);
	   //Copy Assignment
	   MyVector<T>& operator=(const MyVector<T>& other);
	   //Destructor
	   ~MyVector();


	   // Capacity / size
	   int size() const;
	   int capacity() const;
	   bool empty() const;
	   void clear();
	   void reserve(int newCapacity);

       // Element access
	   // unchecked access
       T& operator[](int index);
	   // const overload
       const T& operator[](int index) const;
       // checked access
       T& at(int index);                           
       const T& at(int index) const;

       // Modifiers
       void push_back(const T& value);
       void insertAt(int index, const T& value);
       void removeAt(int index);
       void pop_back();

       // Search helpers
       int indexOf(const T& value) const;
	   


};
//==========================================================
 //Define the methods for Vector class

//Constructor
template <typename T>
MyVector<T>::MyVector(){
	v_size = 0;
	//Construct with small initial capacity
	v_capacity = 2;
	//Allocate a dynamic array
	array = new T[v_capacity];
}

//Destructor:
template <typename T>
MyVector<T>::~MyVector(){
	if (array != nullptr){
		delete [] array;
		array = nullptr;
	}
	v_size = 0;
	v_capacity = 0;
}


template <typename T>
int MyVector<T>::size() const{
	return v_size;
}



//==========================================================
// Do not write any code below this line
#endif