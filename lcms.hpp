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



//========================================================================
#endif