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

// Recursively gather category pointers in post-order so parents print after children
static void _lcms_collectCategoriesPostOrder(Node* node, MyVector<Node*>& out) {
    // Guard against null nodes (should not happen but keeps function safe)
    if (!node) return;
    // Iterate through children first to guarantee they appear before the parent
    MyVector<Node*>& kids = node->getChildren();
    for (int i = 0; i < kids.size(); ++i) {
        _lcms_collectCategoriesPostOrder(kids[i], out);
    }
    // Append the current node once descendants have been processed
    out.push_back(node);
}

// Traverse the entire tree and record category and book matches for a keyword search
static void _lcms_collectMatches(Tree* tree, const string& keyword, MyVector<Node*>& categoryOut, MyVector<Book*>& bookOut) {
    // Protect against an empty tree
    if (!tree || !tree->getRoot()) return;
    // Manual DFS stack to avoid recursion depth issues
    MyVector<Node*> stack;
    stack.push_back(tree->getRoot());
    // Iterate until every node has been inspected
    while (!stack.empty()) {
        int last = stack.size() - 1;
        Node* cur = stack[last];
        stack.removeAt(last);
        // Check category name match (skip pushing the root into results)
        if (cur != tree->getRoot()) {
            if (cur->getName().find(keyword) != string::npos) {
                categoryOut.push_back(cur);
            }
        }
        // Check each book stored directly in this node
        MyVector<Book*>& books = cur->getBooks();
        for (int i = 0; i < books.size(); ++i) {
            Book* candidate = books[i];
            bool match =
                (candidate->getTitle().find(keyword)  != string::npos) ||
                (candidate->getAuthor().find(keyword) != string::npos) ||
                (candidate->getISBN().find(keyword)   != string::npos) ||
                (to_string(candidate->getYear()).find(keyword) != string::npos);
            if (match) {
                bookOut.push_back(candidate);
            }
        }
        // Push children for further processing
        MyVector<Node*>& kids = cur->getChildren();
        for (int i = 0; i < kids.size(); ++i) {
            stack.push_back(kids[i]);
        }
    }
}

// Helper to format plural-sensitive lines (e.g., "1 Book found." vs "2 Books found.")
static void _lcms_printCountLine(int count, const string& singular, const string& plural) {
    // Choose the correct noun based on the count
    const string& noun = (count == 1) ? singular : plural;
    // Emit the line with trailing period to match screenshot style
    cout << count << " " << noun << " found." << endl;
}

// Render the detailed book block bordered by dashed separators
static void _lcms_printBookDetails(const Book* book) {
    // Defensive check to avoid dereferencing null pointers
    if (!book) return;
    cout << "------------------------------------------------------------" << endl;
    cout << "Title:  " << book->getTitle() << endl;
    cout << "Author(s):  " << book->getAuthor() << endl;
    cout << "ISBN:  " << book->getISBN() << endl;
    cout << "Year:  " << book->getYear() << endl;
    cout << "------------------------------------------------------------" << endl;
}

// Print an entire list of books, inserting a blank line between successive entries
static void _lcms_printBookCollection(const MyVector<Book*>& books) {
    for (int i = 0; i < books.size(); ++i) {
        _lcms_printBookDetails(books[i]);
        if (i + 1 < books.size()) {
            cout << endl;
        }
    }
}

// Extract the final segment of a category path for concise messages
static string _lcms_lastSegment(const string& path) {
    // Split path manually to avoid using additional libraries
    string result = "";
    string segment = "";
    for (int i = 0; i < (int)path.size(); ++i) {
        char c = path[i];
        if (c == '/') {
            if (segment.size() > 0) {
                result = segment;
                segment = "";
            }
        } else {
            segment += c;
        }
    }
    if (segment.size() > 0) {
        result = segment;
    }
    return result;
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
static int _lcms_dfsExport(Node* node, const string& pathPrefix, ofstream& out) {
    // Compute this node's path for book rows (pathPrefix already excludes root)
    string myPath = pathPrefix;
    if (node->getParent() != nullptr) {
        // Skip adding root name; only add slash when not first segment
        string segment = node->getName();
        if (myPath.size() > 0) myPath += "/";
        myPath += segment;
    }

    // Track the number of rows written from this subtree
    int written = 0;
    // Emit all local books with their category path
    MyVector<Book*>& books = node->getBooks();
    for (int i = 0; i < books.size(); ++i) {
        // Book::toCSV prints Title,Author,ISBN,Year with proper quoting
        out << books[i]->toCSV() << "," << quoteCSV(myPath) << "\n";
        written++;
    }

    // Recurse into children
    MyVector<Node*>& kids = node->getChildren();
    for (int i = 0; i < kids.size(); ++i) {
        written += _lcms_dfsExport(kids[i], myPath, out);
    }
    // Return total rows emitted below this node
    return written;
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

    // Report how many were imported (format aligned with professor sample)
    cout << importedCount << " records have been imported." << endl;

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

    // Emit the exact header string required by the assignment spec for grading
    fout << "Title,Author,ISBN,Year,Category\n";

    // Preorder traversal (root excluded from category path) and count rows
    int exported = _lcms_dfsExport(libTree->getRoot(), "", fout);

    // Announce success using the phrasing from the professor's output
    cout << exported << " records have been successfully exported to " << path << endl;
}

// Search categories and books for a keyword; delegate to Tree helper
void LCMS::find(string keyword) {
    // Normalize keyword to avoid issues with leading/trailing spaces
    string trimmed = _lcms_trim(keyword);
    // Prepare match containers for categories and books
    MyVector<Node*> categoryMatches;
    MyVector<Book*> bookMatches;
    // Populate the containers via a single tree traversal
    _lcms_collectMatches(libTree, trimmed, categoryMatches, bookMatches);

    // Mirror the professor's summary lines for the two match counts
    cout << categoryMatches.size() << (categoryMatches.size() == 1 ? " Category/sub-category found." : " Categories/sub-categories found.") << endl;
    cout << bookMatches.size() << (bookMatches.size() == 1 ? " Book found." : " Books found.") << endl;

    // Display the matching categories with numbering
    // Print a separator to match the screenshot styling
    cout << "============================================================" << endl;
    // Label the category section with the search keyword
    cout << "List of Categories containing <" << trimmed << ">:" << endl;
    if (categoryMatches.size() == 0) {
        // Show explicit placeholder when no categories were found
        cout << "None" << endl;
    } else {
        for (int i = 0; i < categoryMatches.size(); ++i) {
            // Enumerate each matching category using its path string
            cout << (i + 1) << ": " << _lcms_nodePath(categoryMatches[i]) << endl;
        }
    }

    // Display the matching books using the detailed format
    // Drop another separator before listing book matches
    cout << "============================================================" << endl;
    // Headline the book list, again referencing the keyword
    cout << "List of Books containing <" << trimmed << ">:" << endl;
    if (bookMatches.size() == 0) {
        // Provide an explicit "None" message when there are no book matches
        cout << "None" << endl;
    } else {
        // Print every matched book using the shared detailed block helper
        _lcms_printBookCollection(bookMatches);
    }
    // Close out the section with one final separator line
    cout << "============================================================" << endl;
}

// List all books under a category (or entire library if category empty)
void LCMS::findAll(string category) {
    // Normalize the path; an empty result means "whole library"
    string norm = _lcms_normalizePath(category);
    // Determine the starting node (root when path empty)
    Node* start = (norm.size() == 0) ? libTree->getRoot() : libTree->getNode(norm);
    if (!start) {
        // Mirror the professor's phrasing when the category cannot be located
        cout << "No such category/sub-category found in the Catalog." << endl;
        return;
    }
    // Gather every book under the chosen node
    MyVector<Book*> collected;
    start->collectBooksInSubtree(collected);
    if (collected.size() == 0) {
        // Inform the user when the category exists but contains no titles
        cout << "No books found." << endl;
    } else {
        // Otherwise display the entire collection using the standard block output
        _lcms_printBookCollection(collected);
    }
    // Summarize the total number of records printed
    cout << collected.size() << (collected.size() == 1 ? " record found." : " records found.") << endl;
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
        // Keep the messaging consistent with the screenshots on failure
        cout << "Book not found in the library." << endl;
        return;
    }
    // Display full details
    // Introduce the block exactly as the professor's example does
    cout << "Book found in the library:" << endl;
    _lcms_printBookDetails(b);
}

// Prompt for a new book and insert it into a category if not duplicate
void LCMS::addBook() {
    // Collect fields from user
    string title, author, isbn, yearS, category;

    // Prompt: title
    cout << "Enter Title: ";
    std::getline(cin, title);

    // Prompt: author
    cout << "Enter Author(s): ";
    std::getline(cin, author);

    // Prompt: ISBN
    cout << "Enter ISBN: ";
    std::getline(cin, isbn);

    // Prompt: publication year
    cout << "Enter Publication Year: ";
    std::getline(cin, yearS);

    // Prompt: category path
    cout << "Enter Category: ";
    std::getline(cin, category);

    // Validate year
    int year = 0;
    if (!_lcms_parseYear(yearS, year)) {
        cout << "Invalid publication year. Aborting add." << endl;
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
        cout << "Book already exists in the catalog." << endl;
        return;
    }

    // Create node path if needed
    Node* node = libTree->createNode(norm);
    if (!node) {
        cout << "Could not locate or create category. Aborting add." << endl;
        return;
    }

    // Allocate and insert
    Book* added = new Book(title, author, isbn, year);
    if (node->addBook(added)) {
        // Report success using the professor's celebratory phrasing
        cout << title << " has been successfully added into the Catalog." << endl;
    } else {
        delete added;
        // Inform the user when the specific category already holds an equivalent entry
        cout << "Book already exists in the selected category." << endl;
    }
}

// Edit an existing book found by title; keep fields if left blank
void LCMS::editBook(string bookTitle) {
    // Look up the book across entire tree
    Book* b = libTree->findBook(bookTitle);
    if (!b) {
        cout << "Book not found in the library." << endl;
        return;
    }

    // Display the current details just like the professor's example
    cout << "Book found in the library:" << endl;
    _lcms_printBookDetails(b);

    // Preserve originals so we can revert if necessary
    string originalTitle = b->getTitle();
    string originalAuthor = b->getAuthor();
    string originalISBN = b->getISBN();
    int originalYear = b->getYear();

    // Present the editing menu repeatedly until the user chooses to exit
    while (true) {
        cout << "1: Title" << endl;
        cout << "2: Author" << endl;
        cout << "3: ISBN" << endl;
        cout << "4: Publication_year" << endl;
        cout << "5: exit" << endl;
        cout << "choose the field that you want to edit: ";
        string choice;
        std::getline(cin, choice);

        if (choice == "5") {
            break;
        } else if (choice == "1") {
            cout << "Enter Title: ";
            string newValue;
            std::getline(cin, newValue);
            if (_lcms_trim(newValue).size() > 0) {
                b->setTitle(newValue);
            }
        } else if (choice == "2") {
            cout << "Enter Author(s): ";
            string newValue;
            std::getline(cin, newValue);
            if (_lcms_trim(newValue).size() > 0) {
                b->setAuthor(newValue);
            }
        } else if (choice == "3") {
            cout << "Enter ISBN: ";
            string newValue;
            std::getline(cin, newValue);
            if (_lcms_trim(newValue).size() > 0) {
                b->setISBN(newValue);
            }
        } else if (choice == "4") {
            cout << "Enter Publication Year: ";
            string newYear;
            std::getline(cin, newYear);
            if (_lcms_trim(newYear).size() > 0) {
                int parsed = 0;
                if (_lcms_parseYear(newYear, parsed)) {
                    b->setYear(parsed);
                } else {
                    cout << "Invalid publication year." << endl;
                }
            }
        } else {
            cout << "Invalid option." << endl;
        }
    }

    // Prevent duplicate records by rolling back if a clash is detected
    if (_lcms_libraryContainsExcept(libTree, *b, b)) {
        b->setTitle(originalTitle);
        b->setAuthor(originalAuthor);
        b->setISBN(originalISBN);
        b->setYear(originalYear);
        cout << "Edit would create a duplicate; changes reverted." << endl;
    }
}

// Remove a book by title (first match anywhere)
void LCMS::removeBook(string bookTitle) {
    // Locate the book first so we can show its details to the user
    Book* b = libTree->findBook(bookTitle);
    if (!b) {
        // Match the screenshot when the requested title is missing
        cout << "Book not found in the library." << endl;
        return;
    }

    // Announce the located title and show details before confirming deletion
    cout << "Book found in the library:" << endl;
    _lcms_printBookDetails(b);
    cout << "Are you sure you want to delete the book " << b->getTitle() << " (yes/no): ";
    string ans; std::getline(cin, ans);
    if (ans != "yes" && ans != "YES" && ans != "Yes") {
        // Provide explicit confirmation that the book was preserved
        cout << "Book \"" << b->getTitle() << "\" was not deleted." << endl;
        return;
    }

    if (libTree->removeBookByTitle(bookTitle)) {
        // Mirror the professor's phrasing when deletion succeeds
        cout << "Book \"" << bookTitle << "\" has been deleted from the library" << endl;
    } else {
        // Edge case: the tree failed to remove the title after confirmation
        cout << "Book \"" << bookTitle << "\" could not be deleted." << endl;
    }
}

// Find a category node by path and display it
void LCMS::findCategory(string category) {
    // Normalize and resolve
    string norm = _lcms_normalizePath(category);
    Node* n = libTree->getNode(norm);

    // Print result or a not-found message
    if (!n) {
        cout << "No such category/sub-category found in the Catalog." << endl;
        return;
    }
    string label = (norm.size() == 0) ? libTree->getRoot()->getName() : _lcms_lastSegment(norm);
    // Confirm discovery using the professor's wording
    cout << "Category " << label << " was found in the Catalog" << endl;
}

// Add (or ensure) a category path exists
void LCMS::addCategory(string category) {
    // Normalize input path
    string norm = _lcms_normalizePath(category);
    if (norm.size() == 0) {
        cout << "Invalid category path.\n";
        return;
    }

    bool existed = (libTree->getNode(norm) != nullptr);
    Node* created = libTree->createNode(norm);
    string label = _lcms_lastSegment(norm);
    if (existed) {
        // Inform that the requested category was already present
        cout << label << " already exists in the Catalog." << endl;
    } else if (created) {
        // Announce the freshly created category using the professor's wording
        cout << label << " has been successfully created." << endl;
    } else {
        // Handle unexpected failure during category creation
        cout << "Could not create the category." << endl;
    }
}

// Rename a category by path (requires Node::setName to exist)
void LCMS::editCategory(string category) {
    // Normalize the incoming path so comparisons behave consistently
    string norm = _lcms_normalizePath(category);
    // Resolve the target category using the tree navigation helper
    Node* n = libTree->getNode(norm);
    if (!n) {
        cout << "Category not found.\n";
        return;
    }

    // Prompt the user for the replacement name of the category segment
    cout << "Enter new category name: ";
    string replacement;
    std::getline(cin, replacement);
    // Trim whitespace so empty responses are detected correctly
    string trimmed = _lcms_trim(replacement);
    if (trimmed.size() == 0) {
        cout << "Invalid category name.\n";
        return;
    }

    // Grab the parent so we can watch for sibling-name collisions
    Node* parent = n->getParent();
    if (parent != nullptr) {
        // Walk the sibling list and refuse a duplicate label
        MyVector<Node*>& siblings = parent->getChildren();
        for (int i = 0; i < siblings.size(); ++i) {
            if (siblings[i] != n && siblings[i]->getName() == trimmed) {
                cout << "Duplicate category name under the same parent.\n";
                return;
            }
        }
    }

    // Commit the rename once validation passes
    n->setName(trimmed);
    cout << "Category renamed to: " << trimmed << "\n";
}

// Remove a category (and descendants) identified by its normalized path
void LCMS::removeCategory(string category) {
    // Normalize the requested path prior to lookup
    string norm = _lcms_normalizePath(category);
    if (norm.size() == 0) {
        cout << "Invalid category path.\n";
        return;
    }

    // Locate the node so we know whether the category exists
    Node* target = libTree->getNode(norm);
    if (!target) {
        cout << "Category not found.\n";
        return;
    }

    // Prevent accidental removal of the root since it anchors the catalog
    if (target == libTree->getRoot()) {
        cout << "Cannot remove the root category.\n";
        return;
    }

    // Acquire the parent so we can issue the removal request
    Node* parent = target->getParent();
    if (!parent) {
        cout << "Category removal failed.\n";
        return;
    }

    // List every book that will be removed to mirror professor's sample
    MyVector<Book*> doomedBooks;
    target->collectBooksInSubtree(doomedBooks);
    for (int i = 0; i < doomedBooks.size(); ++i) {
        // Print the removal notice for each book before the nodes are deleted
        cout << "Book \"" << doomedBooks[i]->getTitle() << "\" has been deleted from the library" << endl;
    }

    // Gather categories in post-order so children appear before the target in output
    MyVector<Node*> doomedCategories;
    _lcms_collectCategoriesPostOrder(target, doomedCategories);
    for (int i = 0; i < doomedCategories.size(); ++i) {
        if (doomedCategories[i] == target) continue;
        // Announce each sub-category removal in the required format
        cout << "Category \"" << doomedCategories[i]->getName() << "\" has been deleted from the Library." << endl;
    }

    // Delegate actual deletion to the tree-level helper
    if (libTree->removeChild(parent, target->getName())) {
        // Final confirmation for the primary category
        cout << "Category \"" << target->getName() << "\" has been deleted from the Library." << endl;
    } else {
        // Inform the user if removal failed unexpectedly
        cout << "Category removal failed.\n";
    }
}

//========================================================================
#endif