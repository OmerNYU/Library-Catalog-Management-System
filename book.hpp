#ifndef _BOOK_H
#define _BOOK_H

#include <string>
using std::string;
// Class representing a Book in the library system
class Book 
{
	private:
	   string title;              // Title of the book
	   string author;             // Author of the book
	   string isbn;               // ISBN (International Standard Book Number) of the book
	   int publication_year;           // Year the book was published

	public:
	   //Constructor
	   Book();
	   //Parameterized Constructor
	   Book(string t, string a, string i, int y);

	   //Getter Methods
	   string getTitle() const;
	   string getAuthor() const;
	   string getISBN() const;
	   int getYear() const;

	   //Setter Methods
	   void setTitle(string t);
	   void setAuthor(string a);
	   void setISBN(string i);
	   void setYear(int y);
};

 //Define the methods for Book class
//=============================================

Book::Book() {
	title = "";
	author = "";
	isbn = "";
	publication_year = 0;
}


Book::Book(string t, string a, string i, int y) {
	title = t;
	author = a;
	isbn = i;
	publication_year = y;	
}



string Book::getTitle() const{
	return title;
}

string Book::getAuthor() const{
	return author;
}

string Book::getISBN() const{
	return isbn;
}

int Book::getYear() const{
	return publication_year;
}


void Book::setTitle(string t) {
	title = t;
}

void Book::setAuthor(string a) {
	author = a;
}

void Book::setISBN(string i) {
	isbn = i;
}

void Book::setYear(int y) {
	publication_year = y;
}


//=============================================
// Do not write any code below this line
#endif