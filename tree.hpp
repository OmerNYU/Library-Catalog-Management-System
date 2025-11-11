#ifndef _TREE_H
#define _TREE_H

// NOTE: This header defines the hierarchical category tree used by LCMS.
// Each Node is a category; it owns (1) a vector of subcategories (children)
// and (2) a vector of Book* stored directly in that category. The Tree owns
// the root. Memory ownership: Node deletes its Books and its child Nodes
// recursively in ~Node(), so deleting Tree::root frees the entire structure.

#include <string>
// Bring in stream facilities locally so every translation unit compiles cleanly
#include <iostream>
#include "myvector.hpp"
#include "book.hpp"

using namespace std;

// Class representing a Node in the Tree
class Node 
{
	private:
	    // Category/subcategory display name (e.g., "Computer Science")
	    string name;                // Name of the Node (represents a category or subcategories)
	    // Children categories owned by this node (subtree roots)
	    MyVector<Node*> children;   // List of child Nodes (subcategories)
	    // Books owned by this category only (not including descendants)
	    MyVector<Book*> books;      // List of books stored in this Node
	    // Aggregate count: books in this node + all descendants (kept consistent)
	    unsigned int bookCount;     // Count of books in this Node (Category) and its all subcategories
	    // Parent pointer (nullptr only for root)
	    Node* parent;               // Pointer to the parent Node (nullptr for the root)
	public:
	 	// Construct a node with a given name and parent; bookCount starts at 0
	 	Node(const string& name, Node* parent);
		// Read-only accessor for category name
		string getName() const;
		// Read-only accessor for parent pointer
		Node* getParent() const;
		// Read-only accessor for aggregate book count (this + descendants)
		unsigned int getBookCount() const;
		// Mutable access to children vector (used by Tree internals)
		MyVector<Node*>& getChildren();
		// Const-qualified access for read-only traversals inside const contexts
		const MyVector<Node*>& getChildren() const;
		// Mutable access to books vector (used by Node internals)
		MyVector<Book*>& getBooks();
		// Const-qualified access for read-only traversals inside const contexts
		const MyVector<Book*>& getBooks() const;
		// Allow external callers (LCMS) to rename a category node when validated
		void setName(const string& newName);

		// Search immediate children by name; nullptr if not found
		Node* findChildByName(const string& childName) const;
		// Ensure child exists; return existing or newly created Node*
		Node* addChild(const string& childName);
		// Remove direct child (deleting its subtree) and update bookCount up the chain
		bool removeChildByName(const string& childName);

		// Insert a book into this node (reject duplicates); bubble bookCount +1 up
		bool addBook(Book* book);
		// Remove first book by title from this node only; bubble bookCount -1 up
		bool removeBookByTitle(const string& title);
		// Find a book by title in this node only (no recursion)
		Book* findBookHereByTitle(const string& title) const;

		// Pretty-print subtree with indentation by depth; shows bookCount
		void print(int depth) const;
		// Append all books in this subtree into 'out' (preorder)
		void collectBooksInSubtree(MyVector<Book*>& out) const;
		
		// Destructor: deletes owned Books and child Nodes recursively
		~Node();


};

//==========================================================
// Class representing a Tree structure
class Tree 
{
	private:
	    // Root category node owned by the Tree
	    Node* root;  // Pointer to the root Node of the Tree
	    // Helper used by print() to render the tree with ASCII connectors
	    void printNode(const Node* node, const string& prefix, bool isLast) const;

	public:
		// Construct a tree with a named root category
		Tree(const string& rootName);
		// Destructor deletes the root, which deletes the entire subtree
		~Tree();
		// Read-only access to root pointer
		Node* getRoot() const;

		// Split "A/B/C" by '/', ignoring empty segments; result in parts
		void splitPath(const string& path, MyVector<string>& parts) const;
		// Traverse from root by parts; nullptr if any segment missing
		Node* getNode(const string& path) const;
		// Like getNode, but creates missing nodes along the way (idempotent)
		Node* createNode(const string& path);

		// Remove a category node by path (cannot remove root)
		bool removeNode(const string& path);
		// Print entire tree (root->print(0))
		void print() const;

		// DFS find first Book by title across entire tree
		Book* findBook(const string& title) const;
		// Add a book under a category path (creates path if needed)
		bool addBookAt(const string& categoryPath, Book* book);
		// DFS remove first Book by title across entire tree
		bool removeBookByTitle(const string& title);
		
		// Print categories and books whose fields contain keyword (substring)
		void findKeyword(const string& keyword) const;
		// List all books under a category path (or entire tree if empty path)
		void listAllBooksIn(const string& categoryPath) const;
		// Enable external callers to remove a specific child from a parent node
		bool removeChild(Node* parentNode, const string& childName);

		

};
//==========================================================
//Define the methods of Node and Tree Class

// Initialize a node with a name and parent; start with zero aggregate books
Node::Node(const string& name, Node* parent) {
	this->name = name;
	this->parent = parent;
	bookCount = 0;
}

// Return category name
string Node::getName() const{
	return name;
}

// Return parent pointer (nullptr for root)
Node* Node::getParent() const {
	return parent;
}

// Return aggregate books in this node and all descendants
unsigned int Node::getBookCount() const {
	return bookCount;
}

// Provide mutable access to children for internal tree ops
MyVector<Node*>& Node::getChildren() {
    return children;
}

// Provide const access to child list so traversal helpers remain const-correct
const MyVector<Node*>& Node::getChildren() const {
    return children;
}

// Provide mutable access to books for node-level ops
MyVector<Book*>& Node::getBooks() {
    return books;
}

// Provide const access to book list so read-only operations preserve immutability
const MyVector<Book*>& Node::getBooks() const {
    return books;
}

// Update the stored category name after validation in higher-level logic
void Node::setName(const string& newName) {
	name = newName;
}

// Linear search over immediate children for a matching name
Node* Node::findChildByName(const string& childName) const {
	for (int i = 0; i < children.size(); i++){
		if (children[i]->getName() == childName) return children[i];
	}
	return nullptr;	
}

// Ensure the named child exists; idempotent creation (returns existing if present)
Node* Node::addChild(const string& childName) {
	Node* exists = findChildByName(childName);
	if (exists != nullptr){
		return exists;
	}
	else {
		Node* child = new Node(childName, this);
		children.push_back(child);
		return child;
	}
}

// 5) Remove a direct child (and its subtree)
bool Node::removeChildByName(const string& childName) {
    // Find child index
    int idx = -1;
    for (int i = 0; i < children.size(); ++i) {
        if (children[i]->getName() == childName) {
            idx = i;
            break;
        }
    }
    if (idx == -1) return false;

    // Subtract that subtree's bookCount from this node and all ancestors
    unsigned int delta = children[idx]->getBookCount();

    // Delete the subtree (child destructor cleans its descendants/books)
    delete children[idx];

    // Remove pointer slot
    children.removeAt(idx);

    // Bubble bookCount decrement up the chain
    Node* p = this;
    while (p != nullptr) {
        p->bookCount -= delta;
        p = p->parent;
    }
    return true;
}

// 6) Add a book to this node (reject duplicates)
bool Node::addBook(Book* book) {
    // Check for duplicates in this node only
    for (int i = 0; i < books.size(); ++i) {
        if (*(books[i]) == *book) {
            return false; // duplicate
        }
    }

    // Insert and bubble +1 up ancestors
    books.push_back(book);

    Node* p = this;
    while (p != nullptr) {
        p->bookCount += 1;
        p = p->parent;
    }
    return true;
}

// 7) Remove a book by title from this node only
bool Node::removeBookByTitle(const string& title) {
    int idx = -1;
    for (int i = 0; i < books.size(); ++i) {
        if (books[i]->getTitle() == title) {
            idx = i;
            break;
        }
    }
    if (idx == -1) return false;

    // Delete the owned Book*
    delete books[idx];

    // Remove from this node's vector
    books.removeAt(idx);

    // Bubble -1 up the ancestors
    Node* p = this;
    while (p != nullptr) {
        p->bookCount -= 1;
        p = p->parent;
    }
    return true;
}

// 8) Find a book by title in this node only
Book* Node::findBookHereByTitle(const string& title) const {
    for (int i = 0; i < books.size(); ++i) {
        if (books[i]->getTitle() == title) {
            return books[i];
        }
    }
    return nullptr;
}

// 9) Pretty-print this subtree
void Node::print(int depth) const {
    // Indentation for this level (2 spaces per depth)
    for (int i = 0; i < depth; ++i) cout << "  ";
    cout << "- " << name << " (books=" << bookCount << ")\n";

    // Print this node's book titles
    for (int i = 0; i < books.size(); ++i) {
        for (int j = 0; j < depth + 1; ++j) cout << "  ";
        cout << "* " << books[i]->getTitle() << "\n";
    }

    // Recurse into children
    for (int i = 0; i < children.size(); ++i) {
        children[i]->print(depth + 1);
    }
}

// 10) Collect all books in this subtree into 'out'
void Node::collectBooksInSubtree(MyVector<Book*>& out) const {
    // Add local books
    for (int i = 0; i < books.size(); ++i) {
        out.push_back(books[i]);
    }
    // Recurse into children
    for (int i = 0; i < children.size(); ++i) {
        children[i]->collectBooksInSubtree(out);
    }
}

// 11) Destructor: free owned Books and child Nodes
Node::~Node() {
    // Delete all books owned by this node
    for (int i = 0; i < books.size(); ++i) {
        delete books[i];
    }
    // Delete all subtrees (recursive cleanup)
    for (int i = 0; i < children.size(); ++i) {
        delete children[i];
    }
}


// =========================
// Tree: construct / access
// =========================

// Build a tree with a named root category
Tree::Tree(const string& rootName) {
    root = new Node(rootName, nullptr);
}

// Delete the root; recursively frees the entire hierarchy
Tree::~Tree() {
    delete root;   // Node dtor recursively deletes subtree
    root = nullptr;
}

// Expose the root pointer for LCMS operations
Node* Tree::getRoot() const {
    return root;
}

// =========================================
// Tree: path utilities (split / get / create)
// =========================================

// Split a path like "A/B/C" by '/', ignoring empty segments
void Tree::splitPath(const string& path, MyVector<string>& parts) const {
    parts.clear();
    int n = (int)path.size();
    string current = "";
    for (int i = 0; i < n; ++i) {
        char c = path[i];
        if (c == '/') {
            if (current.size() > 0) {
                parts.push_back(current);
                current = "";
            }
        } else {
            current += c;
        }
    }
    if (current.size() > 0) {
        parts.push_back(current);
    }
}

// Walk from root following the path; return nullptr if any segment is missing
Node* Tree::getNode(const string& path) const {
    if (!root) return nullptr;

    // Empty or "/" means root
    if (path.size() == 0) return root;
    if (path == "/") return root;

    MyVector<string> parts;
    splitPath(path, parts);

    Node* cur = root;
    for (int i = 0; i < parts.size(); ++i) {
        Node* next = cur->findChildByName(parts[i]); // must be const method
        if (!next) return nullptr;
        cur = next;
    }
    return cur;
}

// Like getNode, but creates missing nodes along the way
Node* Tree::createNode(const string& path) {
    if (!root) return nullptr;

    if (path.size() == 0 || path == "/") return root;

    MyVector<string> parts;
    splitPath(path, parts);

    Node* cur = root;
    for (int i = 0; i < parts.size(); ++i) {
        // addChild is idempotent (returns existing if present)
        cur = cur->addChild(parts[i]);
    }
    return cur;
}

// =============================
// Tree: category-level operations
// =============================

// Remove a category node by path (cannot remove root)
bool Tree::removeNode(const string& path) {
    if (!root) return false;

    // Disallow removing the root
    if (path.size() == 0 || path == "/") return false;

    MyVector<string> parts;
    splitPath(path, parts);
    if (parts.size() == 0) return false;

    // Parent path is everything except last segment
    string last = parts[parts.size() - 1];

    // Build parent path string
    string parentPath = "";
    for (int i = 0; i < parts.size() - 1; ++i) {
        if (i > 0) parentPath += "/";
        parentPath += parts[i];
    }

    Node* parentNode = nullptr;
    if (parentPath.size() == 0) {
        // Direct child of root
        parentNode = root;
    } else {
        parentNode = getNode(parentPath);
    }
    if (!parentNode) return false;

    // Remove the child subtree by name
    return parentNode->removeChildByName(last);
}

// Print the full tree
void Tree::print() const {
    if (!root) return;
    // Print the root line first using the aggregate book count
    cout << root->getName() << "(" << root->getBookCount() << ")\n";
    // Fetch children using the const accessor to preserve immutability guarantees
    const MyVector<Node*>& kids = root->getChildren();
    // Render each child with appropriate branch markers
    for (int i = 0; i < kids.size(); ++i) {
        bool isLast = (i == kids.size() - 1);
        printNode(kids[i], "", isLast);
    }
}

// Recursively render a subtree using ASCII branches similar to the professor's sample
void Tree::printNode(const Node* node, const string& prefix, bool isLast) const {
    // Print the accumulated prefix (vertical bars and spaces)
    cout << prefix;
    // Choose connector: "`-- " for the final child, "|-- " otherwise
    cout << (isLast ? "\\-- " : "|-- ");
    // Display the node name followed by the total book count held in the subtree
    cout << node->getName() << "(" << node->getBookCount() << ")\n";
    // Build the prefix for children: add spaces when last, vertical bar otherwise
    string nextPrefix = prefix + (isLast ? "    " : "|   ");
    // Access children via const overload to avoid accidental mutation
    const MyVector<Node*>& kids = node->getChildren();
    // Iterate through children and render them with updated prefix context
    for (int i = 0; i < kids.size(); ++i) {
        bool childIsLast = (i == kids.size() - 1);
        printNode(kids[i], nextPrefix, childIsLast);
    }
}

// =============================
// Tree: book-level operations
// =============================

// Find first book by title (DFS)
Book* Tree::findBook(const string& title) const {
    if (!root) return nullptr;

    // Simple DFS using a stack of Node*
    MyVector<Node*> stack;
    stack.push_back(root);

    while (!stack.empty()) {
        // Pop last
        int last = stack.size() - 1;
        Node* cur = stack[last];
        stack.removeAt(last);

        // Check this node
        Book* here = cur->findBookHereByTitle(title);
        if (here) return here;

        // Push children
        const MyVector<Node*>& kids = cur->getChildren();
        for (int i = 0; i < kids.size(); ++i) {
            stack.push_back(kids[i]);
        }
    }
    return nullptr;
}

// Add a book at a category path (creates path if missing)
bool Tree::addBookAt(const string& categoryPath, Book* book) {
    if (!root || !book) return false;
    Node* node = createNode(categoryPath);
    if (!node) return false;
    return node->addBook(book);
}

// Remove a book by title anywhere in the tree (first match)
bool Tree::removeBookByTitle(const string& title) {
    if (!root) return false;

    // DFS over nodes; try removal at each
    MyVector<Node*> stack;
    stack.push_back(root);

    while (!stack.empty()) {
        int last = stack.size() - 1;
        Node* cur = stack[last];
        stack.removeAt(last);

        // Try removing here
        if (cur->removeBookByTitle(title)) {
            return true; // success
        }

        // Continue search
        MyVector<Node*>& kids = cur->getChildren(); // non-const since remove could be needed later
        for (int i = 0; i < kids.size(); ++i) {
            stack.push_back(kids[i]);
        }
    }
    return false;
}

// =================================
// Tree: search / listing helpers
// =================================

// Print categories containing keyword and books whose fields contain keyword
void Tree::findKeyword(const string& keyword) const {
    if (!root) return;

    // DFS traversal
    MyVector<Node*> stack;
    stack.push_back(root);

    while (!stack.empty()) {
        int last = stack.size() - 1;
        Node* cur = stack[last];
        stack.removeAt(last);

        // Category match (simple substring match, case-sensitive)
        if (cur->getName().find(keyword) != string::npos) {
            cout << "[Category] " << cur->getName() << "\n";
        }

        // Book matches in this node
        const MyVector<Book*>& bvec = cur->getBooks();
        for (int i = 0; i < bvec.size(); ++i) {
            Book* b = bvec[i];
            bool match =
                (b->getTitle().find(keyword)  != string::npos) ||
                (b->getAuthor().find(keyword) != string::npos) ||
                (b->getISBN().find(keyword)   != string::npos) ||
                (to_string(b->getYear()).find(keyword) != string::npos);
            if (match) {
                cout << "[Book] ";
                b->printBook();  // prints multiple lines
            }
        }

        // Push children
        const MyVector<Node*>& kids = cur->getChildren();
        for (int i = 0; i < kids.size(); ++i) {
            stack.push_back(kids[i]);
        }
    }
}

// List all books under a category path (or all books if path empty)
void Tree::listAllBooksIn(const string& categoryPath) const {
    if (!root) return;

    const Node* start = nullptr;

    if (categoryPath.size() == 0) {
        start = root;
    } else {
        start = getNode(categoryPath);
        if (!start) {
            cout << "Category not found: " << categoryPath << "\n";
            return;
        }
    }

    // Collect and print
    MyVector<Book*> collected;
    start->collectBooksInSubtree(collected);

    for (int i = 0; i < collected.size(); ++i) {
        collected[i]->printBook();
    }
}



// Provide a small wrapper so LCMS can remove a child via the Tree abstraction
bool Tree::removeChild(Node* parentNode, const string& childName) {
    if (!parentNode) {
        return false;
    }
    return parentNode->removeChildByName(childName);
}

//==========================================================
// Do not write any code below this line
#endif
