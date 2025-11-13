#ifndef _BOOK_H
#define _BOOK_H

// -----------------------------------------------------------------------------
// Library Catalog Project — Book model (small and self-contained).
// I keep this header simple: a few fields, basic accessors, and helpers for I/O.
// -----------------------------------------------------------------------------

// I need std::string for the text fields and std::cout for printBook().
#include <string>
#include <iostream>

// Pull only what I actually use into scope
using std::string;
using std::cout;
using std::endl;

// -----------------------------------------------------------------------------
// Book: holds the minimal info I need across the whole program.
// Title/Author/ISBN are strings; Year is an int (can handle negative years in data).
// I use inline functions so that the function definitions inside the header file don’t cause linker errors when the header is included in multiple source files
// Source: https://stackoverflow.com/questions/5057021/why-do-inline-functions-have-to-be-defined-in-a-header-file
// -----------------------------------------------------------------------------
class Book 
{
	private:
		// Store the book title as-is (quotes/commas are handled during CSV output).
		string title;

		// Keep author as one string so multi-author cases stay intact (e.g., "A; B").
		string author;

		// If present, I treat ISBN as the primary identifier for equality.
		string isbn;

		// Year is an int so I can parse simple numeric input directly.
		int publication_year;

	public:
		// Default constructor: build an "empty" book that I can fill later.
		Book();

		// Full constructor: quick way to create a ready-to-use Book in one shot.
		Book(string t, string a, string i, int y);

		// Getters: read-only access to internals (no copies of big objects).
		string getTitle() const;
		string getAuthor() const;
		string getISBN() const;
		int getYear()  const;

		// Setters: used by the edit menu in LCMS (to update fields safely).
		void setTitle(string t);
		void setAuthor(string a);
		void setISBN(string i);
		void setYear(int y);

		// Equality: prefer ISBN if both have it; otherwise fall back to (title, author, year).
		bool operator==(const Book& other) const;

		// Pretty-print: matches the block formatting shown in the assignment screenshots.
		void printBook() const;

		// CSV helper: returns "Title,Author,ISBN,Year" (category is handled by the tree).
		string toCSV() const;
};

// -----------------------------------------------------------------------------
// Default constructor: start with blank strings and 0 for the year.
// This keeps object creation cheap and friendly for parsing code.
// -----------------------------------------------------------------------------
inline Book::Book() {
	title = "";
	author = "";
	isbn = "";
	publication_year = 0;
}

// -----------------------------------------------------------------------------
// Parameterized constructor: initialize all fields right away.
// Handy when importing or creating from user prompts in one go.
// -----------------------------------------------------------------------------
inline Book::Book(string t, string a, string i, int y) {
	title = t;
	author = a;
	isbn = i;
	publication_year = y;
}

// -----------------------------------------------------------------------------
// Getters: simple pass-through access. Marked const so they work on const objects.
// -----------------------------------------------------------------------------
inline string Book::getTitle() const { return title; }
inline string Book::getAuthor() const { return author; }
inline string Book::getISBN()   const { return isbn; }
inline int    Book::getYear()   const { return publication_year; }

// -----------------------------------------------------------------------------
// Setters: straightforward field updates used by the edit flow.
// -----------------------------------------------------------------------------
inline void Book::setTitle(string t) { title = t; }
inline void Book::setAuthor(string a){ author = a; }
inline void Book::setISBN(string i)  { isbn = i; }
inline void Book::setYear(int y)     { publication_year = y; }

// -----------------------------------------------------------------------------
// Equality rule:
// - If either side lacks an ISBN, fall back to (title && author && year).
// - If both have ISBNs, compare just the ISBNs (treat as the main key).
// This covers older/sparse data while still respecting ISBN when present.
// -----------------------------------------------------------------------------
inline bool Book::operator==(const Book& other) const {
	if (isbn == "" || other.isbn == "") {
		return (title == other.title &&
		        author == other.author &&
		        publication_year == other.publication_year);
	}
	return isbn == other.isbn;
}

// -----------------------------------------------------------------------------
// printBook: show the book neatly on the console (one field per line).
// I keep the labels exactly as the screenshots expect.
// -----------------------------------------------------------------------------
inline void Book::printBook() const {
	cout << "Title: " << title << endl;
	cout << "Author: " << author << endl;
	cout << "ISBN: " << isbn << endl;
	cout << "Publication Year: " << publication_year << endl;
}

// -----------------------------------------------------------------------------
// Small CSV utility:
//
// quoteCSV(field) wraps a value in double quotes and doubles any inner quotes.
// Example:  Hello "World"  ->  "Hello ""World"""
// This keeps commas/quotes in titles/authors safe for CSV consumers.
// -----------------------------------------------------------------------------
inline string quoteCSV(const string& field) {
	string safe = field;

	// Walk through the string and double any existing quotes.
	size_t pos = 0;
	while ((pos = safe.find('"', pos)) != string::npos) {
		safe.insert(pos, "\"");
		pos += 2; // Skip past the doubled quotes we just inserted.
	}

	// Wrap the final text in quotes for CSV.
	return "\"" + safe + "\"";
}

// -----------------------------------------------------------------------------
// toCSV: return the 4 plain columns for a book.
// The category gets appended by the tree export routine later.
// -----------------------------------------------------------------------------
inline string Book::toCSV() const {
	return quoteCSV(title) + "," +
	       quoteCSV(author) + "," +
	       quoteCSV(isbn) + "," +
	       std::to_string(publication_year);
}

// I leave the guard to prevent accidental extra code at the end of the header.
#endif
