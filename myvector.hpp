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
#include <algorithm>

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
// Define the methods for Vector class
//==========================================================

//----------------------------------------------------------
// Default constructor: initialize an empty vector with a
// small starting capacity and allocate its backing array.
// Invariants after construction:
//  - v_size == 0
//  - v_capacity == 2 (room for first two pushes)
//  - array points to a valid T[v_capacity]
//----------------------------------------------------------
template <typename T>
MyVector<T>::MyVector(){
	// Start with zero logical elements
	v_size = 0;
	// Start with a small initial capacity (grow on demand)
	v_capacity = 2;
	// Allocate contiguous storage for elements
	array = new T[v_capacity];
}

//----------------------------------------------------------
// Copy constructor: build a new vector that logically mirrors
// 'other' (same size) and allocates its own buffer (no sharing).
// Note: uses max(other.v_capacity, 2) to ensure a usable buffer.
//----------------------------------------------------------
template <typename T>
MyVector<T>::MyVector(const MyVector<T>& other) {
	// Copy logical size from source
	v_size = other.v_size;
	// Choose a usable capacity (at least 2)
	v_capacity = max(other.v_capacity, 2);

	// Allocate a fresh buffer for this instance
	T* new_array = new T[v_capacity];

	// Copy each element from source into the new buffer
	for (int i = 0; i < v_size; i++) {
		new_array[i] = other.array[i];
	}
	// Transfer ownership of the freshly allocated buffer into this object
	array = new_array;
}

//----------------------------------------------------------
// Copy assignment (copy-and-swap idiom):
// 1) Guard self-assignment.
// 2) Make a copy (reuses copy ctor).
// 3) Swap internals with the temp copy.
// 4) Temp's destructor frees old buffer safely.
// Strong exception safety: either succeeds fully or leaves *this unchanged.
//----------------------------------------------------------
template <typename T>
MyVector<T>& MyVector<T>::operator=(const MyVector<T>& other) {
	// Self-assignment guard to avoid redundant work
	if (this == &other) return *this;

	// Create a temporary copy (may throw; if so, *this is unchanged)
	MyVector<T> tmp(other);

	// Swap the backing array pointers
	T* tmpArr = array;
	array = tmp.array;
	tmp.array = tmpArr;

	// Swap the logical sizes
	int tmpSize = v_size;
	v_size = tmp.v_size;
	tmp.v_size = tmpSize;

	// Swap the capacities
	int tmpCapacity = v_capacity;
	v_capacity = tmp.v_capacity;
	tmp.v_capacity = tmpCapacity;

	// Return the updated object; tmp's dtor will clean old resources
	return *this;
}

//----------------------------------------------------------
// Destructor: release the backing array and reset members.
// Note: Uses delete[] to match new[] allocation.
//----------------------------------------------------------
template <typename T>
MyVector<T>::~MyVector(){
	// Free allocated storage if it exists
	if (array != nullptr){
		delete [] array;
		array = nullptr;
	}
	// Clear metadata (defensive reset)
	v_size = 0;
	v_capacity = 0;
}

//----------------------------------------------------------
// size(): report the number of valid (initialized) elements.
// O(1) accessor; does not modify state.
//----------------------------------------------------------
template <typename T>
int MyVector<T>::size() const{
	return v_size;
}

//----------------------------------------------------------
// capacity(): report the current allocated slots in storage.
// O(1) accessor; does not modify state.
//----------------------------------------------------------
template <typename T>
int MyVector<T>::capacity() const{
	return v_capacity;
}

//----------------------------------------------------------
// empty(): convenience check for v_size == 0.
// O(1) accessor; does not modify state.
//----------------------------------------------------------
template <typename T>
bool MyVector<T>::empty() const{
	return v_size == 0;
}

//----------------------------------------------------------
// clear(): logically remove all elements while retaining the
// allocated buffer for reuse (does not shrink capacity).
// O(1).
//----------------------------------------------------------
template <typename T>
void MyVector<T>::clear() {
	// Reset logical size; contents will be overwritten on next writes
	v_size = 0;
}

//----------------------------------------------------------
// reserve(newCapacity): ensure backing storage can hold at
// least newCapacity elements. If newCapacity <= current
// capacity, no-op. Otherwise, allocate a larger buffer,
// copy existing elements, free old buffer, and update state.
//----------------------------------------------------------
template <typename T>
void MyVector<T>::reserve(int newCapacity) {
	// No action if current capacity already sufficient
	if (newCapacity <= v_capacity) return;

	// Only grow when strictly needed
	if (newCapacity > v_capacity){
		// Allocate a larger buffer
		T* new_array = new T[newCapacity];

		// Copy existing elements into the new buffer
		for (int i = 0; i < v_size; i++){
			new_array[i] = array[i];
		}

		// Release old storage and redirect the pointer
		delete [] array;
		array = new_array;

		// Update the capacity to the new value
		v_capacity = newCapacity;
	}
}

//----------------------------------------------------------
// operator[] (non-const): unchecked element access by index.
// Precondition: 0 <= index < v_size (caller responsibility).
// Returns a modifiable reference.
//----------------------------------------------------------
template <typename T>
T& MyVector<T>::operator[](int index){
	return array[index];
}

//----------------------------------------------------------
// operator[] (const): unchecked element access for const objects.
// Precondition: 0 <= index < v_size.
// Returns a read-only reference.
//----------------------------------------------------------
template <typename T>
const T& MyVector<T>::operator[](int index) const {
    return array[index];
}

//----------------------------------------------------------
// at(int) (non-const): bounds-checked access. Throws
// std::out_of_range if index is invalid. Returns modifiable ref.
//----------------------------------------------------------
template <typename T>
T& MyVector<T>::at(int index) {
	// Validate index to prevent undefined behavior
	if (index < 0 || index >= v_size){
		throw out_of_range("Index out of range");
	}
	// Safe access after checks
	return array[index];
}

//----------------------------------------------------------
// at(int) (const): bounds-checked read-only access for const
// vectors. Throws std::out_of_range on invalid index.
//----------------------------------------------------------
template <typename T>
const T& MyVector<T>::at(int index) const {
	// Validate index to prevent undefined behavior
	if (index < 0 || index >= v_size){
		throw out_of_range("Index out of range");
	}
	// Safe access after checks
	return array[index];
}

//----------------------------------------------------------
// push_back(value): append an element to the end. Grows the
// buffer when full (doubling strategy via reserve).
// Amortized O(1).
//----------------------------------------------------------
template <typename T>
void MyVector<T>::push_back(const T& value){
	// Grow capacity on demand before writing
	if (v_size == v_capacity){
		reserve(v_capacity * 2);
	}
	// Place the new element at the next free slot
	array[v_size] = value;
	// Advance logical size
	v_size++;
}

//----------------------------------------------------------
// insertAt(index, value): insert at position index, shifting
// existing elements [index..v_size-1] one step to the right.
// Valid indices: 0..v_size (inserting at v_size == append).
// O(n) due to shifting.
//----------------------------------------------------------
template <typename T>
void MyVector<T>::insertAt(int index, const T& value) {
	// Enforce valid insertion points
	if (index < 0 || index > v_size){
		throw out_of_range("Index is out of range");
	}

	// Grow when full before shifting/writing
	if (v_size == v_capacity){
		reserve(v_capacity * 2);
	}

	// Shift elements right, starting from the last valid one
	for (int i = v_size - 1; i >= index; i--){
		array[i+1] = array[i];
	}

	// Write the new value into the vacated slot
	array[index] = value;

	// One more logical element now
	v_size++;
}

//----------------------------------------------------------
// removeAt(index): remove the element at index, shifting
// elements [index+1..v_size-1] one step to the left.
// Valid indices: 0..v_size-1. O(n) due to shifting.
//----------------------------------------------------------
template <typename T>
void MyVector<T>::removeAt(int index){
	// Enforce valid removal points
	if (index < 0 || index >= v_size){
		throw out_of_range("Index is out of range");
	}

	// Shift elements left to fill the gap
	for (int i = index; i < v_size - 1; i++){
		array[i] = array[i+1];
	}

	// Decrease logical size (last slot is now considered free)
	v_size--;	
}

//----------------------------------------------------------
// pop_back(): remove the last element. Throws if already empty.
// O(1).
//----------------------------------------------------------
template <typename T>
void MyVector<T>::pop_back(){
	// Underflow guard: cannot pop from an empty vector
	if (v_size == 0){
		throw out_of_range("Vector is empty");
	}
	// Reduce logical count by one
	v_size--;
}

//----------------------------------------------------------
// indexOf(value): linear search for the first occurrence
// of 'value' using operator==. Returns the index if found,
// or -1 if not found. O(n).
//----------------------------------------------------------
template <typename T>
int MyVector<T>::indexOf(const T& value) const {
	// Scan all valid elements for a match
	for (int i = 0; i < v_size; i++) {
		if (array[i] == value) return i;
	}
	// Sentinel for "not found"
	return -1;
}



//==========================================================
// Do not write any code below this line
#endif