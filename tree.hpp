#ifndef _TREE_H
#define _TREE_H

#include <string>
#include "myvector.hpp"
#include "book.hpp"

using namespace std;

// Class representing a Node in the Tree
class Node 
{
	private:
	    string name;                // Name of the Node (represents a category or subcategories)
	    MyVector<Node*> children;   // List of child Nodes (subcategories)
	    MyVector<Book*> books;      // List of books stored in this Node
	    unsigned int bookCount;     // Count of books in this Node (Category) and its all subcategories
	    Node* parent;               // Pointer to the parent Node (nullptr for the root)
	Public:
	 //Decleare required methods for Book class
};

//==========================================================
// Class representing a Tree structure
class Tree 
{
	private:
	    Node* root;  // Pointer to the root Node of the Tree

	public:
 	//Declare the required methods for Tree class

};
//==========================================================
//Define the methods of Node and Tree Class





//==========================================================
// Do not write any code below this line
#endif
