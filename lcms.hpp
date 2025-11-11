#ifndef _LCMS_H
#define _LCMS_H

// -----------------------------------------------------------------------------
// Library Catalog Project — LCMS (Library Catalog Management System)
// This header ties the user-facing commands (import, list, find, etc.) to the
// underlying Tree/Book types.
// -----------------------------------------------------------------------------

#include <iostream>   // for user I/O (cout/cin)
#include <fstream>    // for file import/export (ifstream/ofstream)

#include "tree.hpp"   // category tree + book storage
#include "book.hpp"   // Book model (fields, printing, CSV)

// -----------------------------------------------------------------------------
// LCMS = thin facade over the Tree with CLI-ish routines for the assignment.
// -----------------------------------------------------------------------------
class LCMS 
{
	private:
		// The single Tree that holds categories and books
	    Tree* libTree;

	public:
	    // Spin up an LCMS with a named root category (e.g., "Library")
	    LCMS(string name);

	    // Free the entire tree hierarchy (nodes + books)
	    ~LCMS();

	    // Read CSV: Title,Author,ISBN,Publication Year,Category
	    int  import(string path);

	    // Write CSV with a header row
	    void exportData(string path);

	    // Keyword search across categories and books
	    void find(string keyword);

	    // List all books under a category (empty path => whole tree)
	    void findAll(string category);

	    // Pretty-print the category outline
	    void list();

	  	// Locate a specific title and print its full block
	    void findBook(string bookTitle);
 		
 		// Gather fields from stdin and append a new Book*
	    void addBook();
  
	    // Menu-driven editor for a located book (with duplicate guard)
	    void editBook(string bookTitle);

	    // Confirm + remove a book (first match anywhere)
	    void removeBook(string bookTitle);

	    // Check if a category path exists and acknowledge it
	    void findCategory(string category);

	    // Ensure a category path exists (mkdir -p style)
	    void addCategory(string category);

	    // Rename a category segment (validated for sibling duplicates)
	    void editCategory(string category);

	    // Delete a category subtree (and announce what got removed)
	    void removeCategory(string category);

	    // NOTE: Add more private helpers if needed, but do not change
	    // the signatures above (per assignment rules).
};
//==========================================================
// Define methods for LCMS class below
//==========================================================

/* ===============================
   Local helpers (file-scope only)
   =============================== */

// ------------------------------------------------------------------
// _lcms_trim: strip leading/trailing spaces/tabs without <algorithm>
// ------------------------------------------------------------------
static string _lcms_trim(const string& s) {
    int start = 0;
    while (start < (int)s.size() && (s[start] == ' ' || s[start] == '\t')) start++;
    int end = (int)s.size() - 1;
    while (end >= start && (s[end] == ' ' || s[end] == '\t')) end--;
    if (end < start) return "";
    return s.substr(start, end - start + 1);
}

// ---------------------------------------------------------------
// _lcms_normalizePath: collapse extra slashes and trim segments.
// Example: "  CS//  Algo  / " -> "CS/Algo"
// ---------------------------------------------------------------
static string _lcms_normalizePath(const string& path) {
    string out = "", seg = "";
    bool lastWasSlash = false;

    for (int i = 0; i < (int)path.size(); ++i) {
        char c = path[i];
        if (c == '/') {
            if (!lastWasSlash) {
                string t = _lcms_trim(seg);
                if (t.size() > 0) {
                    if (out.size() > 0) out += "/";
                    out += t;
                }
                seg = "";
                lastWasSlash = true;
            }
        } else {
            seg += c;
            lastWasSlash = false;
        }
    }
    string t = _lcms_trim(seg);
    if (t.size() > 0) {
        if (out.size() > 0) out += "/";
        out += t;
    }
    return out;
}

// --------------------------------------------------------------------
// _lcms_parseYear: accept optional leading '-' and only digits after.
// Returns true on success, false on malformed input.
// --------------------------------------------------------------------
static bool _lcms_parseYear(const string& s, int& outYear) {
    string t = _lcms_trim(s);
    if (t.size() == 0) return false;

    int i = 0, sign = 1;
    if (t[0] == '-') { sign = -1; i = 1; }
    if (i >= (int)t.size()) return false;

    long val = 0;
    for (; i < (int)t.size(); ++i) {
        char c = t[i];
        if (c < '0' || c > '9') return false;
        val = val * 10 + (c - '0');
    }
    outYear = (int)(sign * val);
    return true;
}

// ---------------------------------------------------------------------------------
// _lcms_parseCSVLine: manual CSV split with quotes (supports "" escaped quote).
// Expected 5 fields: Title, Author, ISBN, Publication Year, Category
// ---------------------------------------------------------------------------------
static bool _lcms_parseCSVLine(const string& line, MyVector<string>& fieldsOut) {
    fieldsOut.clear();
    string cur = "";
    bool inQuotes = false;

    for (int i = 0; i < (int)line.size(); ++i) {
        char c = line[i];
        if (inQuotes) {
            if (c == '"') {
                if (i + 1 < (int)line.size() && line[i + 1] == '"') {
                    cur += '"'; i++; // escaped quote
                } else {
                    inQuotes = false;
                }
            } else {
                cur += c;
            }
        } else {
            if (c == ',') {
                fieldsOut.push_back(_lcms_trim(cur));
                cur = "";
            } else if (c == '"') {
                inQuotes = true;
            } else {
                cur += c;
            }
        }
    }
    fieldsOut.push_back(_lcms_trim(cur));
    return fieldsOut.size() == 5;
}

// -----------------------------------------------------------------------------
// _lcms_nodePath: build a "A/B/C" style path from a Node* (excluding the root).
// -----------------------------------------------------------------------------
static string _lcms_nodePath(const Node* n) {
    MyVector<const Node*> chain;
    const Node* cur = n;
    while (cur != nullptr) { chain.push_back(cur); cur = cur->getParent(); }
    if (chain.size() <= 1) return "";

    string out = "";
    for (int i = chain.size() - 2; i >= 0; --i) {
        if (out.size() > 0) out += "/";
        out += chain[i]->getName();
    }
    return out;
}

// -----------------------------------------------------------------------------
// _lcms_collectCategoriesPostOrder: children first, then parent (for print order)
// -----------------------------------------------------------------------------
static void _lcms_collectCategoriesPostOrder(Node* node, MyVector<Node*>& out) {
    if (!node) return;
    MyVector<Node*>& kids = node->getChildren();
    for (int i = 0; i < kids.size(); ++i) _lcms_collectCategoriesPostOrder(kids[i], out);
    out.push_back(node);
}

// -----------------------------------------------------------------------------
// _lcms_collectMatches: one DFS that gathers both category and book matches.
// -----------------------------------------------------------------------------
static void _lcms_collectMatches(Tree* tree, const string& keyword, MyVector<Node*>& categoryOut, MyVector<Book*>& bookOut) {
    if (!tree || !tree->getRoot()) return;

    MyVector<Node*> stack;
    stack.push_back(tree->getRoot());

    while (!stack.empty()) {
        int last = stack.size() - 1;
        Node* cur = stack[last];
        stack.removeAt(last);

        if (cur != tree->getRoot()) {
            if (cur->getName().find(keyword) != string::npos) {
                categoryOut.push_back(cur);
            }
        }
        MyVector<Book*>& books = cur->getBooks();
        for (int i = 0; i < books.size(); ++i) {
            Book* b = books[i];
            bool match =
                (b->getTitle().find(keyword)  != string::npos) ||
                (b->getAuthor().find(keyword) != string::npos) ||
                (b->getISBN().find(keyword)   != string::npos) ||
                (to_string(b->getYear()).find(keyword) != string::npos);
            if (match) bookOut.push_back(b);
        }
        MyVector<Node*>& kids = cur->getChildren();
        for (int i = 0; i < kids.size(); ++i) stack.push_back(kids[i]);
    }
}

// -----------------------------------------------------------------------------
// _lcms_printCountLine: small helper to print "1 Book found." / "2 Books found."
// -----------------------------------------------------------------------------
static void _lcms_printCountLine(int count, const string& singular, const string& plural) {
    const string& noun = (count == 1) ? singular : plural;
    cout << count << " " << noun << " found." << endl;
}

// -----------------------------------------------------------------------------
// _lcms_printBookDetails: consistent bordered block for one Book*
// -----------------------------------------------------------------------------
static void _lcms_printBookDetails(const Book* book) {
    if (!book) return;
    cout << "------------------------------------------------------------" << endl;
    cout << "Title:  "      << book->getTitle()  << endl;
    cout << "Author(s):  "  << book->getAuthor() << endl;
    cout << "ISBN:  "       << book->getISBN()   << endl;
    cout << "Year:  "       << book->getYear()   << endl;
    cout << "------------------------------------------------------------" << endl;
}

// -----------------------------------------------------------------------------
// _lcms_printBookCollection: print multiple books with spacing between entries
// -----------------------------------------------------------------------------
static void _lcms_printBookCollection(const MyVector<Book*>& books) {
    for (int i = 0; i < books.size(); ++i) {
        _lcms_printBookDetails(books[i]);
        if (i + 1 < books.size()) cout << endl;
    }
}

// -----------------------------------------------------------------------------
// _lcms_lastSegment: return last path component (for friendlier messages)
// -----------------------------------------------------------------------------
static string _lcms_lastSegment(const string& path) {
    string result = "", segment = "";
    for (int i = 0; i < (int)path.size(); ++i) {
        char c = path[i];
        if (c == '/') {
            if (segment.size() > 0) { result = segment; segment = ""; }
        } else {
            segment += c;
        }
    }
    if (segment.size() > 0) result = segment;
    return result;
}

// -----------------------------------------------------------------------------
// _lcms_libraryContains: DFS check for a duplicate Book (uses operator==)
// -----------------------------------------------------------------------------
static bool _lcms_libraryContains(Tree* tree, const Book& b) {
    MyVector<Node*> stack;
    stack.push_back(tree->getRoot());

    while (!stack.empty()) {
        int last = stack.size() - 1;
        Node* cur = stack[last];
        stack.removeAt(last);

        MyVector<Book*>& local = cur->getBooks();
        for (int i = 0; i < local.size(); ++i) {
            if (*(local[i]) == b) return true;
        }
        MyVector<Node*>& kids = cur->getChildren();
        for (int i = 0; i < kids.size(); ++i) stack.push_back(kids[i]);
    }
    return false;
}

// ---------------------------------------------------------------------------------
// _lcms_libraryContainsExcept: same as above but skip a specific pointer (for edits)
// ---------------------------------------------------------------------------------
static bool _lcms_libraryContainsExcept(Tree* tree, const Book& b, const Book* skip) {
    MyVector<Node*> stack;
    stack.push_back(tree->getRoot());

    while (!stack.empty()) {
        int last = stack.size() - 1;
        Node* cur = stack[last];
        stack.removeAt(last);

        MyVector<Book*>& local = cur->getBooks();
        for (int i = 0; i < local.size(); ++i) {
            if (local[i] == skip) continue;
            if (*(local[i]) == b) return true;
        }
        MyVector<Node*>& kids = cur->getChildren();
        for (int i = 0; i < kids.size(); ++i) stack.push_back(kids[i]);
    }
    return false;
}

// -----------------------------------------------------------------------------------
// _lcms_dfsExport: preorder over nodes; write each book row with full category path.
// Returns number of rows written (handy for a friendly success message).
// -----------------------------------------------------------------------------------
static int _lcms_dfsExport(Node* node, const string& pathPrefix, ofstream& out) {
    string myPath = pathPrefix;
    if (node->getParent() != nullptr) {
        string segment = node->getName();
        if (myPath.size() > 0) myPath += "/";
        myPath += segment;
    }

    int written = 0;
    MyVector<Book*>& books = node->getBooks();
    for (int i = 0; i < books.size(); ++i) {
        out << books[i]->toCSV() << "," << quoteCSV(myPath) << "\n";
        written++;
    }

    MyVector<Node*>& kids = node->getChildren();
    for (int i = 0; i < kids.size(); ++i) {
        written += _lcms_dfsExport(kids[i], myPath, out);
    }
    return written;
}

/* ===============================
   LCMS methods (public interface)
   =============================== */

// --------------------------------------------------------
// ctor: allocate the backing Tree with the given root name
// --------------------------------------------------------
LCMS::LCMS(string name) {
    libTree = new Tree(name);
}

// --------------------------------------------------------
// dtor: delete the Tree; Node dtor recursively frees all
// --------------------------------------------------------
LCMS::~LCMS() {
    delete libTree;
    libTree = nullptr;
}

// ---------------------------------------------------------------------
// import: read CSV lines, validate, normalize paths, avoid duplicates.
// On success returns 0; prints how many records got imported.
// ---------------------------------------------------------------------
int LCMS::import(string path) {
    ifstream fin(path.c_str());
    if (!fin.is_open()) return -1; // couldn't open file

    int importedCount = 0;
    string line;
    bool firstLine = true;

    while (std::getline(fin, line)) {
        if (firstLine) {
            firstLine = false;
            if (line.size() >= 6 && line.substr(0, 6) == "Title,") continue; // skip header
        }

        MyVector<string> fields;
        if (!_lcms_parseCSVLine(line, fields)) continue;

        string title  = fields[0];
        string author = fields[1];
        string isbn   = fields[2];
        string yearS  = fields[3];
        string cat    = fields[4];

        int year = 0;
        if (!_lcms_parseYear(yearS, year)) continue;

        string pathNorm = _lcms_normalizePath(cat);
        if (pathNorm.size() == 0) continue;

        Book candidate(title, author, isbn, year);
        if (_lcms_libraryContains(libTree, candidate)) continue;

        Node* node = libTree->createNode(pathNorm);
        if (!node) continue;

        Book* added = new Book(title, author, isbn, year);
        if (node->addBook(added)) {
            importedCount++;
        } else {
            delete added;
        }
    }

    cout << importedCount << " records have been imported." << endl;
    return 0;
}

// ---------------------------------------------------------------------
// exportData: write header + all rows via preorder traversal.
// Also prints a nice confirmation line with the count.
// ---------------------------------------------------------------------
void LCMS::exportData(string path) {
    ofstream fout(path.c_str());
    if (!fout.is_open()) return;

    // NOTE: header must match grading script expectations
    fout << "Title,Author,ISBN,Year,Category\n";
    int exported = _lcms_dfsExport(libTree->getRoot(), "", fout);

    cout << exported << " records have been successfully exported to " << path << endl;
}

// ---------------------------------------------------------------------
// find: unified keyword search with a tidy summary and two sections:
//       matching categories + matching books (detailed blocks).
// ---------------------------------------------------------------------
void LCMS::find(string keyword) {
    string trimmed = _lcms_trim(keyword);
    MyVector<Node*> categoryMatches;
    MyVector<Book*> bookMatches;

    _lcms_collectMatches(libTree, trimmed, categoryMatches, bookMatches);

    _lcms_printCountLine(categoryMatches.size(), "Category/sub-category", "Categories/sub-categories");
    _lcms_printCountLine(bookMatches.size(),     "Book",                 "Books");

    cout << "============================================================" << endl;
    cout << "List of Categories containing <" << trimmed << ">:" << endl;
    if (categoryMatches.size() == 0) {
        cout << "None" << endl;
    } else {
        for (int i = 0; i < categoryMatches.size(); ++i) {
            cout << (i + 1) << ": " << _lcms_nodePath(categoryMatches[i]) << endl;
        }
    }

    cout << "============================================================" << endl;
    cout << "List of Books containing <" << trimmed << ">:" << endl;
    if (bookMatches.size() == 0) {
        cout << "None" << endl;
    } else {
        _lcms_printBookCollection(bookMatches);
    }
    cout << "============================================================" << endl;
}

// ---------------------------------------------------------------------
// findAll: print all books under a category (or entire tree when empty).
// ---------------------------------------------------------------------
void LCMS::findAll(string category) {
    string norm = _lcms_normalizePath(category);
    Node* start = (norm.size() == 0) ? libTree->getRoot() : libTree->getNode(norm);
    if (!start) {
        cout << "No such category/sub-category found in the Catalog." << endl;
        return;
    }

    MyVector<Book*> collected;
    start->collectBooksInSubtree(collected);

    if (collected.size() == 0) {
        cout << "No books found." << endl;
    } else {
        _lcms_printBookCollection(collected);
    }
    cout << collected.size() << (collected.size() == 1 ? " record found." : " records found.") << endl;
}

// ---------------------------------------------------------------------
// list: rely on Tree::print() to render the nice outline
// ---------------------------------------------------------------------
void LCMS::list() {
    libTree->print();
}

// ---------------------------------------------------------------------
// findBook: locate a single title (DFS) and print its details
// ---------------------------------------------------------------------
void LCMS::findBook(string bookTitle) {
    Book* b = libTree->findBook(bookTitle);
    if (!b) {
        cout << "Book not found in the library." << endl;
        return;
    }
    cout << "Book found in the library:" << endl;
    _lcms_printBookDetails(b);
}

// ---------------------------------------------------------------------
// addBook: gather fields interactively, validate, dedupe, insert
// ---------------------------------------------------------------------
void LCMS::addBook() {
    string title, author, isbn, yearS, category;

    cout << "Enter Title: ";           std::getline(cin, title);
    cout << "Enter Author(s): ";       std::getline(cin, author);
    cout << "Enter ISBN: ";            std::getline(cin, isbn);
    cout << "Enter Publication Year: ";std::getline(cin, yearS);
    cout << "Enter Category: ";        std::getline(cin, category);

    int year = 0;
    if (!_lcms_parseYear(yearS, year)) {
        cout << "Invalid publication year. Aborting add." << endl;
        return;
    }

    string norm = _lcms_normalizePath(category);
    if (norm.size() == 0) {
        cout << "Invalid category path. Aborting add." << endl;
        return;
    }

    Book candidate(title, author, isbn, year);
    if (_lcms_libraryContains(libTree, candidate)) {
        cout << "Book already exists in the catalog." << endl;
        return;
    }

    Node* node = libTree->createNode(norm);
    if (!node) {
        cout << "Could not locate or create category. Aborting add." << endl;
        return;
    }

    Book* added = new Book(title, author, isbn, year);
    if (node->addBook(added)) {
        cout << title << " has been successfully added into the Catalog." << endl;
    } else {
        delete added;
        cout << "Book already exists in the selected category." << endl;
    }
}

// ---------------------------------------------------------------------
// editBook: small menu; blank keeps field; revert if duplicate would occur
// ---------------------------------------------------------------------
void LCMS::editBook(string bookTitle) {
    Book* b = libTree->findBook(bookTitle);
    if (!b) {
        cout << "Book not found in the library." << endl;
        return;
    }

    cout << "Book found in the library:" << endl;
    _lcms_printBookDetails(b);

    string originalTitle  = b->getTitle();
    string originalAuthor = b->getAuthor();
    string originalISBN   = b->getISBN();
    int    originalYear   = b->getYear();

    while (true) {
        cout << "1: Title" << endl;
        cout << "2: Author" << endl;
        cout << "3: ISBN" << endl;
        cout << "4: Publication_year" << endl;
        cout << "5: exit" << endl;
        cout << "choose the field that you want to edit: ";
        string choice; std::getline(cin, choice);

        if (choice == "5") break;

        if (choice == "1") {
            cout << "Enter Title: ";
            string v; std::getline(cin, v);
            if (_lcms_trim(v).size() > 0) b->setTitle(v);
        } else if (choice == "2") {
            cout << "Enter Author(s): ";
            string v; std::getline(cin, v);
            if (_lcms_trim(v).size() > 0) b->setAuthor(v);
        } else if (choice == "3") {
            cout << "Enter ISBN: ";
            string v; std::getline(cin, v);
            if (_lcms_trim(v).size() > 0) b->setISBN(v);
        } else if (choice == "4") {
            cout << "Enter Publication Year: ";
            string v; std::getline(cin, v);
            if (_lcms_trim(v).size() > 0) {
                int parsed = 0;
                if (_lcms_parseYear(v, parsed)) b->setYear(parsed);
                else cout << "Invalid publication year." << endl;
            }
        } else {
            cout << "Invalid option." << endl;
        }
    }

    if (_lcms_libraryContainsExcept(libTree, *b, b)) {
        b->setTitle(originalTitle);
        b->setAuthor(originalAuthor);
        b->setISBN(originalISBN);
        b->setYear(originalYear);
        cout << "Edit would create a duplicate; changes reverted." << endl;
    }
}

// ---------------------------------------------------------------------
// removeBook: confirm, then delete first match; mirror sample wording
// ---------------------------------------------------------------------
void LCMS::removeBook(string bookTitle) {
    Book* b = libTree->findBook(bookTitle);
    if (!b) {
        cout << "Book not found in the library." << endl;
        return;
    }

    cout << "Book found in the library:" << endl;
    _lcms_printBookDetails(b);

    cout << "Are you sure you want to delete the book " << b->getTitle() << " (yes/no): ";
    string ans; std::getline(cin, ans);
    if (ans != "yes" && ans != "YES" && ans != "Yes") {
        cout << "Book \"" << b->getTitle() << "\" was not deleted." << endl;
        return;
    }

    if (libTree->removeBookByTitle(bookTitle)) {
        cout << "Book \"" << bookTitle << "\" has been deleted from the library" << endl;
    } else {
        cout << "Book \"" << bookTitle << "\" could not be deleted." << endl;
    }
}

// ---------------------------------------------------------------------
// findCategory: normalize path, check existence, and acknowledge
// ---------------------------------------------------------------------
void LCMS::findCategory(string category) {
    string norm = _lcms_normalizePath(category);
    Node* n = libTree->getNode(norm);
    if (!n) {
        cout << "No such category/sub-category found in the Catalog." << endl;
        return;
    }
    string label = (norm.size() == 0) ? libTree->getRoot()->getName() : _lcms_lastSegment(norm);
    cout << "Category " << label << " was found in the Catalog" << endl;
}

// ---------------------------------------------------------------------
// addCategory: create or acknowledge existing; mirror sample wording
// ---------------------------------------------------------------------
void LCMS::addCategory(string category) {
    string norm = _lcms_normalizePath(category);
    if (norm.size() == 0) {
        cout << "Invalid category path.\n";
        return;
    }

    bool existed = (libTree->getNode(norm) != nullptr);
    Node* created = libTree->createNode(norm);
    string label = _lcms_lastSegment(norm);

    if (existed) {
        cout << label << " already exists in the Catalog." << endl;
    } else if (created) {
        cout << label << " has been successfully created." << endl;
    } else {
        cout << "Could not create the category." << endl;
    }
}

// ---------------------------------------------------------------------
// editCategory: rename a category segment (no duplicates among siblings)
// ---------------------------------------------------------------------
void LCMS::editCategory(string category) {
    string norm = _lcms_normalizePath(category);
    Node* n = libTree->getNode(norm);
    if (!n) {
        cout << "Category not found.\n";
        return;
    }

    cout << "Enter new category name: ";
    string replacement; std::getline(cin, replacement);
    string trimmed = _lcms_trim(replacement);
    if (trimmed.size() == 0) {
        cout << "Invalid category name.\n";
        return;
    }

    Node* parent = n->getParent();
    if (parent != nullptr) {
        MyVector<Node*>& siblings = parent->getChildren();
        for (int i = 0; i < siblings.size(); ++i) {
            if (siblings[i] != n && siblings[i]->getName() == trimmed) {
                cout << "Duplicate category name under the same parent.\n";
                return;
            }
        }
    }

    n->setName(trimmed);
    cout << "Category renamed to: " << trimmed << "\n";
}

// ---------------------------------------------------------------------
// removeCategory: announce sub-removals, then delete the subtree
// ---------------------------------------------------------------------
void LCMS::removeCategory(string category) {
    string norm = _lcms_normalizePath(category);
    if (norm.size() == 0) {
        cout << "Invalid category path.\n";
        return;
    }

    Node* target = libTree->getNode(norm);
    if (!target) {
        cout << "Category not found.\n";
        return;
    }

    if (target == libTree->getRoot()) {
        cout << "Cannot remove the root category.\n";
        return;
    }

    Node* parent = target->getParent();
    if (!parent) {
        cout << "Category removal failed.\n";
        return;
    }

    // Announce every book that will go away (matches sample outputs)
    MyVector<Book*> doomedBooks;
    target->collectBooksInSubtree(doomedBooks);
    for (int i = 0; i < doomedBooks.size(); ++i) {
        cout << "Book \"" << doomedBooks[i]->getTitle() << "\" has been deleted from the library" << endl;
    }

    // Announce child categories (post-order so kids list before the parent)
    MyVector<Node*> doomedCategories;
    _lcms_collectCategoriesPostOrder(target, doomedCategories);
    for (int i = 0; i < doomedCategories.size(); ++i) {
        if (doomedCategories[i] == target) continue;
        cout << "Category \"" << doomedCategories[i]->getName() << "\" has been deleted from the Library." << endl;
    }

    // Issue the actual delete via the Tree wrapper
    if (libTree->removeChild(parent, target->getName())) {
        cout << "Category \"" << target->getName() << "\" has been deleted from the Library." << endl;
    } else {
        cout << "Category removal failed.\n";
    }
}

//========================================================================
#endif
