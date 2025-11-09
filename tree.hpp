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
	public:
	 	Node(string name, Node* parent);
		string getName() const;
		Node* getParent() const;
		unsigned int getBookCount() const;
		MyVector<Node*>& getChildren();
		MyVector<Book*>& getBooks();

		Node* findChildByName(const string& childName);
		Node* addChild(const string& childName);
		bool removeChildByName(const string& childName);

		bool addBook(Book* book);
		bool removeBookByTitle(const string& title);
		Book* findBookHereByTitle(const string& title) const;

		void print(int depth) const;
		void collectBooksInSubtree(MyVector<Book*>& out) const;
		
		~Node();


};

//==========================================================
// Class representing a Tree structure
class Tree 
{
	private:
	    Node* root;  // Pointer to the root Node of the Tree

	public:
		Tree(const string& rootName);
		~Tree();
		Node* getRoot() const;

		void splitPath(const string& path, MyVector<string>& parts) const;
		Node* getNode(const string& path) const;
		Node* createNode(const string& path);

		bool removeNode(const string& path);
		void print() const;

		Book* findBook(const string& title) const;
		bool addBookAt(const string& categoryPath, Book* book);
		bool removeBookByTitle(const string& title);
		
		void findKeyword(const string& keyword) const;
		void listAllBooksIn(const string& categoryPath) const;

		


};
//==========================================================
//Define the methods of Node and Tree Class





//==========================================================
// Do not write any code below this line
#endif
