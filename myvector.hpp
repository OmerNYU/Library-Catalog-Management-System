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

template <typename T>
int MyVector<T>::capacity() const{
	return v_capacity;
}


template <typename T>
bool MyVector<T>::empty() const{
	return v_size == 0;
}



template <typename T>
void MyVector<T>::clear() {
	v_size = 0;
}


template <typename T>
void MyVector<T>::reserve(int newCapacity) {
	if (newCapacity <= v_capacity) return;
	if (newCapacity > v_capacity){
		T* new_array = new T[newCapacity];
		for (int i = 0; i < v_size; i++){
			new_array = array[i];
		}
		delete [] array;
		array = new_array;
		v_capacity = newCapacity;
	}
}


template <typename T>
T& MyVector<T>::operator[](int index){
	return array[index];
}

template <typename T>
const T& MyVector<T>::operator[](int index) const {
    return array[index];
}

template <typename T>
T& MyVector<T>::at(int index) {
	if (index < 0 || index >= v_size){
		throw out_of_range("Index out of range");
	}
	return array[index];
}

template <typename T>
const T& MyVector<T>::at(int index) const {
	if (index < 0 || index >= v_size){
		throw out_of_range("Index out of range");
	}
	return array[index];
}




template <typename T>
void MyVector<T>::push_back(const T& value){
	if (v_size == v_capacity){
		reserve(v_capacity * 2);
	}
	array[v_size] = value;
	v_size++;
}

template <typename T>
void MyVector<T>::insertAt(int index, const T& value) {
	if (index < 0 || index > v_size){
		throw out_of_range("Index is out of range");
	}
	if (v_size == v_capacity){
		reserve(v_capacity * 2);
	}
	for (int i = v_size - 1; i >= index; i--){
		array[i+1] = array[i];
	}
	array[index] = value;
	v_size++;
}


template <typename T>
void MyVector<T>::removeAt(int index){
	if (index < 0 || index >= v_size){
		throw out_of_range("Index is out of range");
	}
	for (int i = index; i < v_size - 1; i++){
		array[i] = array[i+1];
	}
	v_size--;	
}


template <typename T>
void MyVector<T>::pop_back(){
	if (v_size == 0){
		throw out_of_range("Vector is empty");
	}
	v_size--;
}


template <typename T>
int MyVector<T>::indexOf(const T& value) const {
	for (int i = 0; i < v_size; i++) {
		if (array[i] == value) return i;
	}
	return -1;
}


//==========================================================
// Do not write any code below this line
#endif