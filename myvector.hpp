#ifndef MYVECTOR_H
#define MYVECTOR_H

//============================================================================
// Name         : myvector.h
// Author       : Omer Hayat
// Version      : 1.3
// Date         : 11-11-2025
// Date Modified: 11-11-2025 
// Description  : Vector implementation in C++
//============================================================================


// -----------------------------------------------------------------------------
// Library Catalog Project — MyVector (lightweight std::vector clone).
// I manage a raw heap array, grow it on demand, and expose a small API
// that’s enough for this assignment (push_back, insert/remove, bounds checks).
// -----------------------------------------------------------------------------

#include <iostream>    // not strictly required here, but handy for quick tests
#include <cstdlib>     // general purpose (kept from starter)
#include <iomanip>     // kept from starter template
#include <stdexcept>   // for std::out_of_range in at() and pop_back()
#include <sstream>     // kept from starter template
#include <algorithm>   // for std::max used in copy-ctor

using namespace std;

// -----------------------------------------------------------------------------
// MyVector: behaves like a simplified vector<T>.
// Owns a contiguous heap buffer and tracks logical size vs. capacity.
// -----------------------------------------------------------------------------
template <typename T>
class MyVector 
{
	private:
		// Pointer to the contiguous storage I allocate with new[] / delete[].
	    T *array;

		// Number of valid elements the user has pushed/inserted.
	    int v_size;

		// Allocated slots available in 'array' (can be >= v_size).
	    int v_capacity;

	public:
		// Default constructor: start with a tiny buffer and size 0.
		MyVector();

		// Copy constructor: deep copy the contents (no sharing of the buffer).
		MyVector(const MyVector<T>& other);

		// Copy assignment: copy-and-swap pattern for safety.
		MyVector<T>& operator=(const MyVector<T>& other);

		// Destructor: free the heap buffer and reset counters.
		~MyVector();

		// -----------------------------------------------------------------
		// Size / capacity helpers — all O(1) and const (metadata only).
		// -----------------------------------------------------------------
		int size()     const;
		int capacity() const;
		bool empty()   const;

		// clear() keeps the buffer but forgets the elements (v_size = 0).
		void clear();

		// reserve() makes sure capacity is at least newCapacity (no shrink).
		void reserve(int newCapacity);

		// -----------------------------------------------------------------
		// Element access
		// operator[] is unchecked (fast); at() is checked (throws).
		// -----------------------------------------------------------------
		T&       operator[](int index);
		const T& operator[](int index) const;

		T&       at(int index);
		const T& at(int index) const;

		// -----------------------------------------------------------------
		// Modifiers
		// push_back appends; insertAt shifts right; removeAt shifts left; 
		// pop_back just reduces v_size by 1 (with underflow guard).
		// -----------------------------------------------------------------
		void push_back(const T& value);
		void insertAt(int index, const T& value);
		void removeAt(int index);
		void pop_back();

		// -----------------------------------------------------------------
		// Search helper: linear scan for the first equal element.
		// -----------------------------------------------------------------
		int indexOf(const T& value) const;
};

// ============================================================================
// Implementation
// ============================================================================

// -----------------------------------------------------------------------------
// Default constructor:
// - Start empty (v_size = 0)
// - Give a tiny starting capacity (2) so the first couple pushes are cheap
// - Allocate the backing array
// -----------------------------------------------------------------------------
template <typename T>
MyVector<T>::MyVector(){
	v_size = 0;
	v_capacity = 2;
	array = new T[v_capacity];
}

// -----------------------------------------------------------------------------
// Copy constructor:
// - Take over other's logical size
// - Allocate our own buffer (at least 2)
// - Copy elements one-by-one into our storage
// -----------------------------------------------------------------------------
template <typename T>
MyVector<T>::MyVector(const MyVector<T>& other) {
	v_size = other.v_size;
	v_capacity = max(other.v_capacity, 2);

	T* new_array = new T[v_capacity];
	for (int i = 0; i < v_size; i++) {
		new_array[i] = other.array[i];
	}
	array = new_array;
}

// -----------------------------------------------------------------------------
// Copy assignment (copy-and-swap):
// - Guard against self-assign
// - Make a copy first (may throw; if it does, *this is unchanged)
// - Swap internals with the temp
// - Temp's destructor frees the old buffer safely
// -----------------------------------------------------------------------------
template <typename T>
MyVector<T>& MyVector<T>::operator=(const MyVector<T>& other) {
	if (this == &other) return *this;

	MyVector<T> tmp(other);

	T* tmpArr = array;
	array = tmp.array;
	tmp.array = tmpArr;

	int tmpSize = v_size;
	v_size = tmp.v_size;
	tmp.v_size = tmpSize;

	int tmpCapacity = v_capacity;
	v_capacity = tmp.v_capacity;
	tmp.v_capacity = tmpCapacity;

	return *this;
}

// -----------------------------------------------------------------------------
// Destructor:
// - Match new[] with delete[]
// - Null the pointer and zero the counters (defensive cleanup)
// -----------------------------------------------------------------------------
template <typename T>
MyVector<T>::~MyVector(){
	if (array != nullptr){
		delete [] array;
		array = nullptr;
	}
	v_size = 0;
	v_capacity = 0;
}

// -----------------------------------------------------------------------------
// size/capacity/empty: quick metadata accessors (const and O(1)).
// -----------------------------------------------------------------------------
template <typename T>
int MyVector<T>::size() const { return v_size; }

template <typename T>
int MyVector<T>::capacity() const { return v_capacity; }

template <typename T>
bool MyVector<T>::empty() const { return v_size == 0; }

// -----------------------------------------------------------------------------
// clear:
// - Do not free memory; just forget the elements
// - Next pushes will overwrite previous contents
// -----------------------------------------------------------------------------
template <typename T>
void MyVector<T>::clear() {
	v_size = 0;
}

// -----------------------------------------------------------------------------
// reserve(newCapacity):
// - Only grows (never shrinks here)
// - Allocate new buffer, copy current elements, free old buffer, update state
// -----------------------------------------------------------------------------
template <typename T>
void MyVector<T>::reserve(int newCapacity) {
	if (newCapacity <= v_capacity) return;

	T* new_array = new T[newCapacity];
	for (int i = 0; i < v_size; i++){
		new_array[i] = array[i];
	}

	delete [] array;
	array = new_array;
	v_capacity = newCapacity;
}

// -----------------------------------------------------------------------------
// operator[] (unchecked):
// - Caller must ensure 0 <= index < v_size
// - Useful in tight loops when you already know bounds are valid
// -----------------------------------------------------------------------------
template <typename T>
T& MyVector<T>::operator[](int index){
	return array[index];
}

template <typename T>
const T& MyVector<T>::operator[](int index) const {
    return array[index];
}

// -----------------------------------------------------------------------------
// at(index) (checked):
// - Throws std::out_of_range if index is invalid
// - Safer when handling user-provided indices
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// push_back(value):
// - Grow when full (double capacity)
// - Write at the end and bump v_size
// -----------------------------------------------------------------------------
template <typename T>
void MyVector<T>::push_back(const T& value){
	if (v_size == v_capacity){
		reserve(v_capacity * 2);
	}
	array[v_size] = value;
	v_size++;
}

// -----------------------------------------------------------------------------
// insertAt(index, value):
// - Valid indices are [0..v_size] (inserting at v_size == append)
// - Make room by shifting elements to the right
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// removeAt(index):
// - Valid indices are [0..v_size-1]
// - Close the hole by shifting elements left
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// pop_back():
// - Underflow-guarded decrement of v_size (no return value)
// -----------------------------------------------------------------------------
template <typename T>
void MyVector<T>::pop_back(){
	if (v_size == 0){
		throw out_of_range("Vector is empty");
	}
	v_size--;
}

// -----------------------------------------------------------------------------
// indexOf(value):
// - Linear scan using operator==
// - Returns index or -1 if not found
// -----------------------------------------------------------------------------
template <typename T>
int MyVector<T>::indexOf(const T& value) const {
	for (int i = 0; i < v_size; i++) {
		if (array[i] == value) return i;
	}
	return -1;
}

// -----------------------------------------------------------------------------
// Guard line from the starter: don’t append code below this point.
// -----------------------------------------------------------------------------
#endif
