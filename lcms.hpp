#ifndef _LCMS_H
#define _LCMS_H

#include <iostream>
#include <fstream>

#include "tree.hpp"
#include "book.hpp"

// Class representing a Library Content Management System (LCMS)
class LCMS 
{
	private:
	    Tree* libTree;  // Pointer to the Tree structure that represents the library's hierarchical organization

	public:
	    // Constructor to initialize the LCMS with a root category name (Library)
	    LCMS(string name);

	    // Destructor to clean up resources (e.g., delete the Tree)
	    ~LCMS();

	    // Method to import data from a file located at the given path
	    int import(string path);

	    // Method to export the library data to a file located at the given path
	    void exportData(string path);

	    // Method to find all books and categories containing the keyworkd
	    void find(string keyword);

	    // Method to find and display all books under a specific category/subcategory
	    void findAll(string category);

	    // Method to list all categories and books in the library
	    void list();

	  	// Method to find and display details of a book by its title
	    void findBook(string bookTitle);
 		
 		// Method to add a new book to the library
	    void addBook();
  
	    // Method to edit the details of an existing book by its title
	    void editBook(string bookTitle);

	    // Method to remove a book from the library by its title
	    void removeBook(string bookTitle);

	    // Method to find and display details of a specific category
	    void findCategory(string category);

	    // Method to add a new category to the library
	    void addCategory(string category);

	    // Method to edit the name of an existing category
	    void editCategory(string category);

	    // Method to remove a category (and its subcategories/books) from the library
	    void removeCategory(string category);

	    //Add more helper methods if requried, DO NOT change the signature/format of the the methods defined above
};
//==========================================================
// Define methods for LCMS class below
//==========================================================

/* ===============================
   Local helpers (file-scope only)
   =============================== */

// Trim leading and trailing spaces/tabs from a string (no STL algorithms)
static string _lcms_trim(const string& s) {
    // Left trim
    int start = 0;
    while (start < (int)s.size() && (s[start] == ' ' || s[start] == '\t')) start++;
    // Right trim
    int end = (int)s.size() - 1;
    while (end >= start && (s[end] == ' ' || s[end] == '\t')) end--;
    // Substring safely
    if (end < start) return "";
    return s.substr(start, end - start + 1);
}

// Normalize a category path: trim segments and collapse repeated slashes
static string _lcms_normalizePath(const string& path) {
    // Prepare a builder for normalized output
    string out = "";
    string seg = "";
    bool lastWasSlash = false;

    // Iterate characters and rebuild without duplicate slashes
    for (int i = 0; i < (int)path.size(); ++i) {
        char c = path[i];
        if (c == '/') {
            if (!lastWasSlash) {
                // Close current segment (trim it) if any
                string t = _lcms_trim(seg);
                if (t.size() > 0) {
                    if (out.size() > 0) out += "/";
                    out += t;
                }
                seg = "";
                lastWasSlash = true;
            } else {
                // Skip extra slashes
            }
        } else {
            seg += c;
            lastWasSlash = false;
        }
    }
    // Flush final segment
    string t = _lcms_trim(seg);
    if (t.size() > 0) {
        if (out.size() > 0) out += "/";
        out += t;
    }
    return out;
}

// Parse an integer year manually (allows optional leading '-'; returns false if malformed)
static bool _lcms_parseYear(const string& s, int& outYear) {
    // Trim input first
    string t = _lcms_trim(s);
    if (t.size() == 0) return false;

    // Handle optional sign
    int i = 0;
    int sign = 1;
    if (t[0] == '-') { sign = -1; i = 1; }
    if (i >= (int)t.size()) return false;

    // Accumulate digits
    long val = 0;
    for (; i < (int)t.size(); ++i) {
        char c = t[i];
        if (c < '0' || c > '9') return false;
        val = val * 10 + (c - '0');
        // (No overflow checks needed for typical year ranges)
    }
    outYear = (int)(sign * val);
    return true;
}

// CSV line parser: extracts exactly 5 fields, handling quotes and commas within quotes.
// Returns true on success; false if malformed.
static bool _lcms_parseCSVLine(const string& line, MyVector<string>& fieldsOut) {
    // Clear the output vector first
    fieldsOut.clear();

    // State for parsing
    string cur = "";
    bool inQuotes = false;

    // Walk character-by-character
    for (int i = 0; i < (int)line.size(); ++i) {
        char c = line[i];

        if (inQuotes) {
            // Inside quotes: accept all chars except treat "" as escaped quote
            if (c == '"') {
                // If next char is also a quote, that is an escaped quote
                if (i + 1 < (int)line.size() && line[i + 1] == '"') {
                    cur += '"';
                    i++; // consume second quote
                } else {
                    // Closing quote
                    inQuotes = false;
                }
            } else {
                cur += c;
            }
        } else {
            // Outside quotes: comma ends field, quote starts quoted region
            if (c == ',') {
                // Push trimmed field
                fieldsOut.push_back(_lcms_trim(cur));
                cur = "";
            } else if (c == '"') {
                inQuotes = true;
            } else {
                cur += c;
            }
        }
    }

    // Push final field
    fieldsOut.push_back(_lcms_trim(cur));

    // Expect exactly 5 fields: Title, Author, ISBN, Publication Year, Category
    if (fieldsOut.size() != 5) return false;
    return true;
}

// Build full path of a node relative to root, excluding the root name itself.
// e.g., Library -> Computer Science -> Algorithms  => "Computer Science/Algorithms"
static string _lcms_nodePath(const Node* n) {
    // Collect segments by walking up; we’ll later reverse into a string
    MyVector<const Node*> chain;

    // Walk parents up to (and including) root
    const Node* cur = n;
    while (cur != nullptr) {
        chain.push_back(cur);
        cur = cur->getParent();
    }

    // If there's only root, return empty path
    if (chain.size() <= 1) return "";

    // Rebuild from child-of-root downwards; skip the last element (root) at the end of chain
    string out = "";
    for (int i = chain.size() - 2; i >= 0; --i) {
        const Node* node = chain[i];
        if (out.size() > 0) out += "/";
        out += node->getName();
    }
    return out;
}

// DFS: does the library already contain an equal book? (uses Book::operator==)
static bool _lcms_libraryContains(Tree* tree, const Book& b) {
    // Stack for manual DFS
    MyVector<Node*> stack;
    stack.push_back(tree->getRoot());

    // Traverse all nodes
    while (!stack.empty()) {
        int last = stack.size() - 1;
        Node* cur = stack[last];
        stack.removeAt(last);

        // Check books at this node
        MyVector<Book*>& local = cur->getBooks();
        for (int i = 0; i < local.size(); ++i) {
            if (*(local[i]) == b) return true;
        }

        // Push children
        MyVector<Node*>& kids = cur->getChildren();
        for (int i = 0; i < kids.size(); ++i) stack.push_back(kids[i]);
    }
    return false;
}

// DFS: duplicate check but ignore a specific Book* (used during edits)
static bool _lcms_libraryContainsExcept(Tree* tree, const Book& b, const Book* skip) {
    MyVector<Node*> stack;
    stack.push_back(tree->getRoot());

    // Traverse all nodes
    while (!stack.empty()) {
        int last = stack.size() - 1;
        Node* cur = stack[last];
        stack.removeAt(last);

        // Check books at this node
        MyVector<Book*>& local = cur->getBooks();
        for (int i = 0; i < local.size(); ++i) {
            if (local[i] == skip) continue;
            if (*(local[i]) == b) return true;
        }

        // Push children
        MyVector<Node*>& kids = cur->getChildren();
        for (int i = 0; i < kids.size(); ++i) stack.push_back(kids[i]);
    }
    return false;
}

// Preorder export: write all books in subtree as CSV rows with full category path
static void _lcms_dfsExport(Node* node, const string& pathPrefix, ofstream& out) {
    // Compute this node's path for book rows (pathPrefix already excludes root)
    string myPath = pathPrefix;
    if (node->getParent() != nullptr) {
        // Skip adding root name; only add slash when not first segment
        string segment = node->getName();
        if (myPath.size() > 0) myPath += "/";
        myPath += segment;
    }

    // Emit all local books with their category path
    MyVector<Book*>& books = node->getBooks();
    for (int i = 0; i < books.size(); ++i) {
        // Book::toCSV prints Title,Author,ISBN,Year with proper quoting
        out << books[i]->toCSV() << "," << quoteCSV(myPath) << "\n";
    }

    // Recurse into children
    MyVector<Node*>& kids = node->getChildren();
    for (int i = 0; i < kids.size(); ++i) {
        _lcms_dfsExport(kids[i], myPath, out);
    }
}


/* ===============================
   LCMS methods (public interface)
   =============================== */

// Constructor to initialize the LCMS with a root category name (e.g., "Library")
LCMS::LCMS(string name) {
    // Create the tree that backs the catalog
    libTree = new Tree(name);
}

// Destructor to clean up resources and free the entire tree
LCMS::~LCMS() {
    // Delete tree; Node destructors recursively clear books and subtrees
    delete libTree;
    libTree = nullptr;
}

// Import catalog items from a CSV file (Title,Author,ISBN,Publication Year,Category)
int LCMS::import(string path) {
    // Open the input file for reading
    ifstream fin(path.c_str());
    if (!fin.is_open()) {
        // File not accessible per requirements
        return -1;
    }

    // Track number of successfully imported books
    int importedCount = 0;
    string line;
    bool firstLine = true;

    // Read file line-by-line
    while (std::getline(fin, line)) {
        // Skip header if present on first line
        if (firstLine) {
            firstLine = false;
            // Basic heuristic: starts with "Title,"
            if (line.size() >= 6) {
                string head = line.substr(0, 6);
                // Case-sensitive OK for this assignment
                if (head == "Title,") continue;
            }
        }

        // Parse into exactly 5 fields
        MyVector<string> fields;
        if (!_lcms_parseCSVLine(line, fields)) {
            // Ignore malformed records
            continue;
        }

        // Extract fields in order
        string title  = fields[0];
        string author = fields[1];
        string isbn   = fields[2];
        string yearS  = fields[3];
        string cat    = fields[4];

        // Validate year (allow negatives like -500)
        int year = 0;
        if (!_lcms_parseYear(yearS, year)) {
            // Ignore malformed records
            continue;
        }

        // Normalize category path and skip if empty
        string pathNorm = _lcms_normalizePath(cat);
        if (pathNorm.size() == 0) {
            // Ignore malformed records
            continue;
        }

        // Construct a candidate Book on the stack
        Book candidate(title, author, isbn, year);

        // Skip duplicates (library-wide check)
        if (_lcms_libraryContains(libTree, candidate)) {
            continue;
        }

        // Ensure category exists and insert the book
        Node* node = libTree->createNode(pathNorm);
        if (!node) {
            // Should not happen if tree exists, but guard anyway
            continue;
        }

        // Allocate a new Book on the heap; ownership transfers to the node
        Book* added = new Book(title, author, isbn, year);

        // Add to this category node; if rejected (very unlikely now), delete
        if (node->addBook(added)) {
            importedCount++;
        } else {
            delete added;
        }
    }

    // Report how many were imported (format to match your course’s expectation)
    cout << importedCount << " books imported" << endl;

    // Success per spec when file opened
    return 0;
}

// Export the current catalog to CSV with a header row
void LCMS::exportData(string path) {
    // Open output stream
    ofstream fout(path.c_str());
    if (!fout.is_open()) {
        // Could print an error message; spec doesn't require it
        return;
    }

    // Write the header exactly as sample expects
    fout << "Title,Author,ISBN,Publication Year,Category\n";

    // Preorder traversal (root excluded from category path)
    _lcms_dfsExport(libTree->getRoot(), "", fout);
}

// Search categories and books for a keyword; delegate to Tree helper
void LCMS::find(string keyword) {
    // Simple delegation: prints matches internally
    libTree->findKeyword(keyword);
}

// List all books under a category (or entire library if category empty)
void LCMS::findAll(string category) {
    // Normalize the path; an empty result means "whole library"
    string norm = _lcms_normalizePath(category);
    libTree->listAllBooksIn(norm);
}

// Display the whole tree (categories and titles per node)
void LCMS::list() {
    // Delegates to Tree::print()
    libTree->print();
}

// Find a single book by title and display its details
void LCMS::findBook(string bookTitle) {
    // Lookup across entire tree
    Book* b = libTree->findBook(bookTitle);
    if (!b) {
        cout << "Book not found" << endl;
        return;
    }
    // Display full details
    b->printBook();
}

// Prompt for a new book and insert it into a category if not duplicate
void LCMS::addBook() {
    // Collect fields from user
    string title, author, isbn, yearS, category;

    // Prompt: title
    cout << "Enter title: ";
    std::getline(cin, title);

    // Prompt: author
    cout << "Enter author: ";
    std::getline(cin, author);

    // Prompt: ISBN
    cout << "Enter ISBN: ";
    std::getline(cin, isbn);

    // Prompt: publication year
    cout << "Enter publication year: ";
    std::getline(cin, yearS);

    // Prompt: category path
    cout << "Enter category (e.g., Computer Science/Algorithms): ";
    std::getline(cin, category);

    // Validate year
    int year = 0;
    if (!_lcms_parseYear(yearS, year)) {
        cout << "Invalid year. Aborting add." << endl;
        return;
    }

    // Normalize category
    string norm = _lcms_normalizePath(category);
    if (norm.size() == 0) {
        cout << "Invalid category path. Aborting add." << endl;
        return;
    }

    // Build candidate for duplicate check
    Book candidate(title, author, isbn, year);
    if (_lcms_libraryContains(libTree, candidate)) {
        cout << "Duplicate book exists. Not added." << endl;
        return;
    }

    // Create node path if needed
    Node* node = libTree->createNode(norm);
    if (!node) {
        cout << "Internal error: category creation failed." << endl;
        return;
    }

    // Allocate and insert
    Book* added = new Book(title, author, isbn, year);
    if (node->addBook(added)) {
        cout << "Book added." << endl;
    } else {
        delete added;
        cout << "Add failed (duplicate in node)." << endl;
    }
}

// Edit an existing book found by title; keep fields if left blank
void LCMS::editBook(string bookTitle) {
    // Look up the book across entire tree
    Book* b = libTree->findBook(bookTitle);
    if (!b) {
        cout << "Book not found." << endl;
        return;
    }

    // Show current values and accept new values (blank keeps old)
    cout << "Editing book. Leave a field blank to keep current value.\n";

    // Title
    cout << "Title [" << b->getTitle() << "]: ";
    string t; std::getline(cin, t);
    // Author
    cout << "Author [" << b->getAuthor() << "]: ";
    string a; std::getline(cin, a);
    // ISBN
    cout << "ISBN [" << b->getISBN() << "]: ";
    string i; std::getline(cin, i);
    // Year
    cout << "Publication Year [" << b->getYear() << "]: ";
    string y; std::getline(cin, y);

    // Save old values in case we need to revert
    string oldT = b->getTitle();
    string oldA = b->getAuthor();
    string oldI = b->getISBN();
    int    oldY = b->getYear();

    // Tentatively apply new values if provided
    if (_lcms_trim(t).size() > 0) b->setTitle(t);
    if (_lcms_trim(a).size() > 0) b->setAuthor(a);
    if (_lcms_trim(i).size() > 0) b->setISBN(i);
    if (_lcms_trim(y).size() > 0) {
        int yy = 0;
        if (_lcms_parseYear(y, yy)) {
            b->setYear(yy);
        } else {
            cout << "Invalid year; keeping old year.\n";
        }
    }

    // Check duplicates after edit (exclude the same pointer)
    if (_lcms_libraryContainsExcept(libTree, *b, b)) {
        // Revert and notify
        b->setTitle(oldT);
        b->setAuthor(oldA);
        b->setISBN(oldI);
        b->setYear(oldY);
        cout << "Edit would create a duplicate; changes reverted.\n";
        return;
    }

    // Confirm success
    cout << "Book updated.\n";
}

// Remove a book by title (first match anywhere)
void LCMS::removeBook(string bookTitle) {
    // Ask for simple confirmation (optional)
    cout << "Remove \"" << bookTitle << "\"? (y/n): ";
    string ans; std::getline(cin, ans);
    if (ans != "y" && ans != "Y") {
        cout << "Cancelled.\n";
        return;
    }

    // Delegate to Tree-wide removal
    if (libTree->removeBookByTitle(bookTitle)) {
        cout << "Book removed.\n";
    } else {
        cout << "Book not found.\n";
    }
}

// Find a category node by path and display it
void LCMS::findCategory(string category) {
    // Normalize and resolve
    string norm = _lcms_normalizePath(category);
    Node* n = libTree->getNode(norm);

    // Print result or a not-found message
    if (!n) {
        cout << "Category not found.\n";
        return;
    }
    n->print(0);
}

// Add (or ensure) a category path exists
void LCMS::addCategory(string category) {
    // Normalize input path
    string norm = _lcms_normalizePath(category);
    if (norm.size() == 0) {
        cout << "Invalid category path.\n";
        return;
    }

    // Create path (idempotent)
    libTree->createNode(norm);
    cout << "Category ensured: " << norm << "\n";
}

// Rename a category by path (requires Node::setName to exist)
void LCMS::editCategory(string category) {
    // Normalize and locate node
    string norm = _lcms_normalizePath(category);
    Node* n = libTree->getNode(norm);
    if (!n) {
        cout << "Category not found.\n";
        return;
    }

    // Root rename is allowed or not? You can decide; here we allow it.

}

//========================================================================
#endif