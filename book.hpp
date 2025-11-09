#ifndef _BOOK_H
#define _BOOK_H

// NOTE: This header defines the Book entity used throughout the LCMS.
// It provides a lightweight data model with accessors, comparison,
// formatted printing, and CSV conversion for import/export flows.

// Includes string utilities for core fields (Title, Author, ISBN)
#include <string>
using std::string;
// NOTE: printBook() below uses cout/endl; ensure <iostream> is included
// somewhere before compilation and that std::cout is in scope.

// Class representing a Book in the library system
class Book 
{
	private:
	   // Stores the title of the book (may contain commas/quotes)
	   string title;              // Title of the book
	   // Stores author(s); free-form text; may include commas
	   string author;             // Author of the book
	   // Primary key for equality if present; fall back to Title+Author+Year
	   string isbn;               // ISBN (International Standard Book Number) of the book
	   // Publishing year; can be negative for BCE (as in sample CSV)
	   int publication_year;           // Year the book was published

	public:
	   // Default constructor: creates an "empty" book placeholder
	   Book();
	   // Fully-initializing constructor: set Title, Author, ISBN, Year
	   Book(string t, string a, string i, int y);

	   // Read-only accessors: do not modify object state (const-correct)
	   string getTitle() const;
	   string getAuthor() const;
	   string getISBN() const;
	   int getYear() const;

	   // Mutators: allow LCMS to edit fields during "editBook" flow
	   void setTitle(string t);
	   void setAuthor(string a);
	   void setISBN(string i);
	   void setYear(int y);

	   // Equality comparator:
	   // - If either ISBN is missing, compare Title+Author+Year
	   // - Otherwise, compare by ISBN (primary key)
	   bool operator==(const Book& other) const;

	   // Pretty print for console commands (find, list, findBook)
	   void printBook() const;

	   // Convert to CSV fields (without category).
	   // LCMS will append the category path when exporting.
	   string toCSV() const;
};

 //Define the methods for Book class
//=============================================

// Default constructor: initialize all fields to a neutral/empty state
Book::Book() {
	title = "";
	author = "";
	isbn = "";
	publication_year = 0;
}

// Parameterized constructor: set all attributes at creation time
Book::Book(string t, string a, string i, int y) {
	title = t;
	author = a;
	isbn = i;
	publication_year = y;	
}

// Getter: return current title (no side effects)
string Book::getTitle() const{
	return title;
}

// Getter: return current author(s)
string Book::getAuthor() const{
	return author;
}

// Getter: return current ISBN
string Book::getISBN() const{
	return isbn;
}

// Getter: return publishing year
int Book::getYear() const{
	return publication_year;
}

// Setter: update the title field
void Book::setTitle(string t) {
	title = t;
}

// Setter: update the author field
void Book::setAuthor(string a) {
	author = a;
}

// Setter: update the ISBN field
void Book::setISBN(string i) {
	isbn = i;
}

// Setter: update the publishing year
void Book::setYear(int y) {
	publication_year = y;
}

// Equality: prefer ISBN match; if either ISBN is blank, fall back to
// Title+Author+Year (useful for imperfect data in imports)
bool Book::operator==(const Book& other) const {
	if (isbn == "" || other.isbn == ""){
		return (title == other.title && author == other.author && publication_year == other.publication_year);
	}
	return isbn == other.isbn;
}

// Formatted console output for user-facing commands
// (ensure std::cout and std::endl are available in translation unit)
void Book::printBook() const {
	cout << "Title: " << title << endl;
	cout << "Author: " << author << endl;
	cout << "ISBN: " << isbn << endl;
	cout << "Publication Year: " << publication_year << endl;
}

// Utility to safely quote a field for CSV
// - Adds surrounding quotes
// - Doubles any internal quotes to preserve CSV correctness
// - Makes fields with commas/quotes safe for re-import
string quoteCSV(const string& field) {
    string safe = field;
    // Walk string and escape embedded quotes by doubling them
    size_t pos = 0;
    while ((pos = safe.find('"', pos)) != string::npos) {
        safe.insert(pos, "\"");
        pos += 2;  // Skip over the doubled quotes
    }
    // Return the quoted field
    return "\"" + safe + "\"";
}

// CSV serialization for the four book attributes.
// Category will be appended by LCMS during export.
string Book::toCSV() const {
    return quoteCSV(title) + "," +
           quoteCSV(author) + "," +
           quoteCSV(isbn) + "," +
           to_string(publication_year);
}

 //=============================================
// Do not write any code below this line
#endif
