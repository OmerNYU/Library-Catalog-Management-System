#ifndef _TREE_H
#define _TREE_H

// -----------------------------------------------------------------------------
// Library Catalog Project — General Tree for categories + per-category book lists.
// -----------------------------------------------------------------------------

#include <string>     // for std::string names and paths
#include <iostream>   // for printing in print() and printNode()
#include "myvector.hpp" // custom vector used across nodes (children, books)
#include "book.hpp"     // Book model stored at each category

using namespace std;

// -----------------------------------------------------------------------------
// Node = one category (or sub-category) in the tree.
// Owns:
//   - children (sub-categories)
//   - books placed directly in this category
// Also tracks bookCount = (#books here + #books in all descendants).
// parent == nullptr only for the root.
// -----------------------------------------------------------------------------
class Node 
{
	private:
		// Display name used in the CLI (e.g., "Computer Science")
	    string name;

		// Sub-categories owned by this node
	    MyVector<Node*> children;

		// Books directly attached to this category (not recursive)
	    MyVector<Book*> books;

		// Aggregate count of books in this subtree (kept in sync as we edit)
	    unsigned int bookCount;

		// Parent pointer (nullptr only for the root node)
	    Node* parent;

	public:
		// Build a category node and wire its parent (bookCount starts at 0)
	 	Node(const string& name, Node* parent);

		// Read-only accessors to keep other code tidy
		string getName() const;
		Node* getParent() const;
		unsigned int getBookCount() const;

		// Expose the vectors by reference so Tree can manage them
		MyVector<Node*>& getChildren();
		const MyVector<Node*>& getChildren() const;
		MyVector<Book*>& getBooks();
		const MyVector<Book*>& getBooks() const;

		// Used by LCMS when renaming a validated category
		void setName(const string& newName);

		// ----- Child/category helpers (local scope only) -----

		// Find an immediate child by name (nullptr if it doesn't exist)
		Node* findChildByName(const string& childName) const;

		// Ensure a child exists (returns existing if found)
		Node* addChild(const string& childName);

		// Remove a direct child (deletes its whole subtree and fixes counts)
		bool removeChildByName(const string& childName);

		// ----- Book helpers (operate on the current node only) -----

		// Add a book here if not a duplicate (also bubbles bookCount up)
		bool addBook(Book* book);

		// Remove first book with a matching title (bubbles count down by 1)
		bool removeBookByTitle(const string& title);

		// Local-only lookup by title (does not search children)
		Book* findBookHereByTitle(const string& title) const;

		// ----- Printing / collection helpers -----

		// Pretty-print this subtree with indentation
		void print(int depth) const;

		// Append all books in this subtree into 'out'
		void collectBooksInSubtree(MyVector<Book*>& out) const;

		// Destructor cleans up books here and recursively deletes children
		~Node();
};

// ============================================================================
// Tree: wraps the root Node and provides path-based navigation.
// ----------------------------------------------------------------------------
class Tree 
{
	private:
		// Root category node (owned by the Tree)
	    Node* root;

		// Helper for print(): draws nice branch connectors recursively
	    void printNode(const Node* node, const string& prefix, bool isLast) const;

	public:
		// Spin up a Tree with a named root category
		Tree(const string& rootName);

		// Deleting the root frees the entire hierarchy
		~Tree();

		// Let LCMS access the root when necessary
		Node* getRoot() const;

		// Split "A/B/C" into parts, skipping empty segments
		void splitPath(const string& path, MyVector<string>& parts) const;

		// Walk the hierarchy by path and return the node (nullptr if missing)
		Node* getNode(const string& path) const;

		// mkdir -p behavior: create missing segments, return final node
		Node* createNode(const string& path);

		// Remove a category by path (never the root)
		bool removeNode(const string& path);

		// Render the whole tree in a compact outline form
		void print() const;

		// DFS for first Book* whose title matches
		Book* findBook(const string& title) const;

		// Ensure categoryPath exists and add the book there
		bool addBookAt(const string& categoryPath, Book* book);

		// DFS remove first matching title anywhere
		bool removeBookByTitle(const string& title);

		// Print categories and books that contain a keyword (substring match)
		void findKeyword(const string& keyword) const;

		// List books under a category path (or all if path is empty)
		void listAllBooksIn(const string& categoryPath) const;

		// Small wrapper so LCMS can request child removal through Tree
		bool removeChild(Node* parentNode, const string& childName);
};

// ============================================================================
// Node methods
// ============================================================================

// Constructor wires up the label, parent pointer, and resets the running count.
inline Node::Node(const string& name, Node* parent) {
	this->name = name;
	this->parent = parent;
	bookCount = 0;
}

// Simple metadata getters (const so they can be used on const Nodes)
inline string Node::getName() const { return name; }
inline Node* Node::getParent() const { return parent; }
inline unsigned int Node::getBookCount() const { return bookCount; }

// Mutable children access (Tree/print helpers use this)
inline MyVector<Node*>& Node::getChildren() { return children; }

// Const children access (lets print/search use a const Node)
inline const MyVector<Node*>& Node::getChildren() const { return children; }

// Mutable books access (used internally for inserts/removals)
inline MyVector<Book*>& Node::getBooks() { return books; }

// Const books view (safe for read-only traversals)
inline const MyVector<Book*>& Node::getBooks() const { return books; }

// Only called after LCMS validates the new name
inline void Node::setName(const string& newName) { name = newName; }

// Linear search across immediate children (small n, fine for this project)
inline Node* Node::findChildByName(const string& childName) const {
	for (int i = 0; i < children.size(); i++){
		if (children[i]->getName() == childName) return children[i];
	}
	return nullptr;	
}

// Create-or-return child; keeps the tree tidy without duplicates
inline Node* Node::addChild(const string& childName) {
	Node* exists = findChildByName(childName);
	if (exists != nullptr) return exists;

	Node* child = new Node(childName, this);
	children.push_back(child);
	return child;
}

// Remove a direct child and decrement bookCount along the parent chain
inline bool Node::removeChildByName(const string& childName) {
	// Find which slot to remove
	int idx = -1;
	for (int i = 0; i < children.size(); ++i) {
		if (children[i]->getName() == childName) {
			idx = i;
			break;
		}
	}
	if (idx == -1) return false;

	// Remember how many books lived in that subtree
	unsigned int delta = children[idx]->getBookCount();

	// Drop the entire subtree (child dtor frees its own children & books)
	delete children[idx];

	// Close the hole in the children vector
	children.removeAt(idx);

	// Bubble the aggregate count change up to the root
	Node* p = this;
	while (p != nullptr) {
		p->bookCount -= delta;
		p = p->parent;
	}
	return true;
}

// Add a book here if local-duplicate check passes (also updates counts upward)
inline bool Node::addBook(Book* book) {
	for (int i = 0; i < books.size(); ++i) {
		if (*(books[i]) == *book) return false; // duplicate in this category
	}
	books.push_back(book);

	// Increment counts up the chain
	Node* p = this;
	while (p != nullptr) {
		p->bookCount += 1;
		p = p->parent;
	}
	return true;
}

// Remove the first title match from this category only (and update counts)
inline bool Node::removeBookByTitle(const string& title) {
	int idx = -1;
	for (int i = 0; i < books.size(); ++i) {
		if (books[i]->getTitle() == title) { idx = i; break; }
	}
	if (idx == -1) return false;

	// We own the Book*, so delete it here
	delete books[idx];
	books.removeAt(idx);

	// Decrement counts up the chain
	Node* p = this;
	while (p != nullptr) {
		p->bookCount -= 1;
		p = p->parent;
	}
	return true;
}

// Local-only lookup by title (does not recurse into children)
inline Book* Node::findBookHereByTitle(const string& title) const {
	for (int i = 0; i < books.size(); ++i) {
		if (books[i]->getTitle() == title) return books[i];
	}
	return nullptr;
}

// Pretty-print this subtree with indentation (simple ASCII version)
inline void Node::print(int depth) const {
	// 2 spaces per depth level
	for (int i = 0; i < depth; ++i) cout << "  ";
	cout << "- " << name << " (books=" << bookCount << ")\n";

	// Show titles directly under this category
	for (int i = 0; i < books.size(); ++i) {
		for (int j = 0; j < depth + 1; ++j) cout << "  ";
		cout << "* " << books[i]->getTitle() << "\n";
	}

	// Recurse into sub-categories
	for (int i = 0; i < children.size(); ++i) {
		children[i]->print(depth + 1);
	}
}

// Append all books in this subtree to 'out' (preorder)
inline void Node::collectBooksInSubtree(MyVector<Book*>& out) const {
	for (int i = 0; i < books.size(); ++i) out.push_back(books[i]);
	for (int i = 0; i < children.size(); ++i) children[i]->collectBooksInSubtree(out);
}

// Destructor: delete local books, then recursively delete each child subtree
inline Node::~Node() {
	for (int i = 0; i < books.size(); ++i) delete books[i];
	for (int i = 0; i < children.size(); ++i) delete children[i];
}

// ============================================================================
// Tree methods
// Marked as 'inline' to allow definition in header file without violating the one-definition rule.
// This avoids linker errors when the header is included in multiple translation units.
// Also suggests to the compiler that this small function may be inlined for performance.
// ============================================================================

// Build a tree with a named root category
inline Tree::Tree(const string& rootName) {
	root = new Node(rootName, nullptr);
}

// Delete the root; Node::~Node handles full recursive cleanup
inline Tree::~Tree() {
	delete root;
	root = nullptr;
}

// Expose the root to LCMS (read-only pointer)
inline Node* Tree::getRoot() const { return root; }

// Split a path like "A/B/C" into {"A","B","C"}, skipping empty segments
inline void Tree::splitPath(const string& path, MyVector<string>& parts) const {
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
	if (current.size() > 0) parts.push_back(current);
}

// Follow a path from root; return nullptr as soon as a segment is missing
inline Node* Tree::getNode(const string& path) const {
	if (!root) return nullptr;
	if (path.size() == 0 || path == "/") return root;

	MyVector<string> parts;
	splitPath(path, parts);

	Node* cur = root;
	for (int i = 0; i < parts.size(); ++i) {
		Node* next = cur->findChildByName(parts[i]);
		if (!next) return nullptr;
		cur = next;
	}
	return cur;
}

// mkdir -p style creation: create any missing nodes along the path
inline Node* Tree::createNode(const string& path) {
	if (!root) return nullptr;
	if (path.size() == 0 || path == "/") return root;

	MyVector<string> parts;
	splitPath(path, parts);

	Node* cur = root;
	for (int i = 0; i < parts.size(); ++i) {
		cur = cur->addChild(parts[i]); // addChild is idempotent
	}
	return cur;
}

// Remove a category by path (refuses to remove the root)
inline bool Tree::removeNode(const string& path) {
	if (!root) return false;
	if (path.size() == 0 || path == "/") return false;

	MyVector<string> parts;
	splitPath(path, parts);
	if (parts.size() == 0) return false;

	// Build parent path (everything except the last segment)
	string last = parts[parts.size() - 1];
	string parentPath = "";
	for (int i = 0; i < parts.size() - 1; ++i) {
		if (i > 0) parentPath += "/";
		parentPath += parts[i];
	}

	// Parent is either root (when removing an immediate child) or a deeper node
	Node* parentNode = (parentPath.size() == 0) ? root : getNode(parentPath);
	if (!parentNode) return false;

	return parentNode->removeChildByName(last);
}

// Print the whole tree using a compact outline (root header + recursive branches)
inline void Tree::print() const {
	if (!root) return;

	// Print root summary first
	cout << root->getName() << "(" << root->getBookCount() << ")\n";

	// Then pretty-print each child with connectors
	const MyVector<Node*>& kids = root->getChildren();
	for (int i = 0; i < kids.size(); ++i) {
		bool isLast = (i == kids.size() - 1);
		printNode(kids[i], "", isLast);
	}
}

// Helper for print(): draw branch connectors and recurse
inline void Tree::printNode(const Node* node, const string& prefix, bool isLast) const {
	// Use UTF-8 box drawing for nicer CLI output (as suggested in class)
	const string connector = isLast ? "└── " : "├── ";
	const string spacer    = isLast ? "    " : "│   ";

	cout << prefix;
	cout << connector;
	cout << node->getName() << "(" << node->getBookCount() << ")\n";

	string nextPrefix = prefix + spacer;

	const MyVector<Node*>& kids = node->getChildren();
	for (int i = 0; i < kids.size(); ++i) {
		bool childIsLast = (i == kids.size() - 1);
		printNode(kids[i], nextPrefix, childIsLast);
	}
}

// DFS for first book whose title matches
inline Book* Tree::findBook(const string& title) const {
	if (!root) return nullptr;

	MyVector<Node*> stack;
	stack.push_back(root);

	while (!stack.empty()) {
		int last = stack.size() - 1;
		Node* cur = stack[last];
		stack.removeAt(last);

		Book* here = cur->findBookHereByTitle(title);
		if (here) return here;

		const MyVector<Node*>& kids = cur->getChildren();
		for (int i = 0; i < kids.size(); ++i) stack.push_back(kids[i]);
	}
	return nullptr;
}

// Ensure category exists and add the book there
inline bool Tree::addBookAt(const string& categoryPath, Book* book) {
	if (!root || !book) return false;
	Node* node = createNode(categoryPath);
	if (!node) return false;
	return node->addBook(book);
}

// DFS remove first matching title anywhere
inline bool Tree::removeBookByTitle(const string& title) {
	if (!root) return false;

	MyVector<Node*> stack;
	stack.push_back(root);

	while (!stack.empty()) {
		int last = stack.size() - 1;
		Node* cur = stack[last];
		stack.removeAt(last);

		if (cur->removeBookByTitle(title)) return true;

		MyVector<Node*>& kids = cur->getChildren(); // non-const in case we later mutate
		for (int i = 0; i < kids.size(); ++i) stack.push_back(kids[i]);
	}
	return false;
}

// Print categories + books containing the keyword (simple substring match)
inline void Tree::findKeyword(const string& keyword) const {
	if (!root) return;

	MyVector<Node*> stack;
	stack.push_back(root);

	while (!stack.empty()) {
		int last = stack.size() - 1;
		Node* cur = stack[last];
		stack.removeAt(last);

		// Category name match
		if (cur->getName().find(keyword) != string::npos) {
			cout << "[Category] " << cur->getName() << "\n";
		}

		// Book field match (title/author/isbn/year)
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
				b->printBook();  // multi-line block
			}
		}

		// Continue DFS
		const MyVector<Node*>& kids = cur->getChildren();
		for (int i = 0; i < kids.size(); ++i) stack.push_back(kids[i]);
	}
}

// List every book under a category path (or whole tree when path empty)
inline void Tree::listAllBooksIn(const string& categoryPath) const {
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

	MyVector<Book*> collected;
	start->collectBooksInSubtree(collected);

	for (int i = 0; i < collected.size(); ++i) {
		collected[i]->printBook();
	}
}

// Small wrapper so LCMS can remove a child via Tree without touching Node directly
inline bool Tree::removeChild(Node* parentNode, const string& childName) {
	if (!parentNode) return false;
	return parentNode->removeChildByName(childName);
}

// -----------------------------------------------------------------------------
// End guard: keep headers clean and avoid accidental extra code below.
// -----------------------------------------------------------------------------
#endif
