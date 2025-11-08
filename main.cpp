#include<iostream>
#include "lcms.hpp"
//=====================================
void listCommands()
{
	cout<<" ===================================================================================="<<endl
        <<" Welcome to the Library Catalog Management System!\n"<<endl
        <<" List of available Commands:"<<endl
		<<" import <file_name>                          : Read a Book file from a file"<<endl
		<<" export <file_name>                          : Export Books to a file"<<endl
		<<" find <keyword>                              : List all books and categories containing the <keyword>"<<endl
		<<" findBook <title of the book>                : Search a book in the catalog"<<endl
		<<" findAll <category/sub-category/..>          : List all books in a category/sub-category"<<endl
		<<" addBook <book-title>                        : Add a book to the catalog"<<endl
		<<" editBook <book-title>                       : Edit a book detail in the catalog"<<endl
		<<" removeBook <book-title>                     : Remove a book from the catalog"<<endl
		<<" findCategory  <category-name>               : Find a category in the catalog"<<endl
		<<" addCategory <category/sub-category/...>     : Add a category/sub-category to the catalog"<<endl
		<<" editCategory <category/sub-category/...>    : Edit a category/sub-category"<<endl
		<<" removeCategory <category/sub-category/...>  : Remove a category/sub-category from the catalog"<<endl
		<<" list                                        : Display all categories from the catalog"<<endl
		<<" help                                        : Display the list of available commands"<<endl
		<<" exit                                        : Exit the Program"<<endl
		<<" ====================================================================================\n"<<endl;
		
}
//=======================================
// main function
int main()
{

	LCMS lcms("Library");

	listCommands();


	do
	{
		string user_input="";
		string command="";
		string parameter1="";
		try
		{
			cout<<"> ";
			getline(cin,user_input);
			
			// parse user-input into command and parameter(s)
			stringstream sstr(user_input);
			getline(sstr,command,' ');
			getline(sstr,parameter1);
	
			
			if(command=="import") 										
			    lcms.import(parameter1); 
			else if(command=="export")    	    							
				lcms.exportData(parameter1);
			else if(command=="list")										
				lcms.list();
			else if(command=="find") 						     			
				lcms.find(parameter1);
			else if(command=="findBook" or command=="findbook" or command == "fb")				
				lcms.findBook(parameter1);
			else if(command=="findAll" or command=="findall" or command == "fa")     			
				lcms.findAll(parameter1);
			else if(command=="addBook" or command=="addbook" or command == "ab") 				
				lcms.addBook();
			else if(command=="editBook" or command=="editbook" or command == "eb")				
				lcms.editBook(parameter1);
			else if(command=="removeBook" or command=="removebook" or command == "rb") 		
				lcms.removeBook(parameter1);
			else if(command=="findCategory" or command=="findcategory"  or command == "fc")    	
				lcms.findCategory(parameter1);
			else if(command=="addCategory" or command=="addcategory" or command =="ac")    	
				lcms.addCategory(parameter1);
			else if(command=="editCategory" or command=="editcategory" or command =="ec")    
				lcms.editCategory(parameter1);
			else if(command=="removeCategory" or command=="removecategory" or command =="rc") 
				lcms.removeCategory(parameter1);
			else if(command == "help" or command =="h")										
				listCommands();
			else if(command == "exit" or command =="quit")										
				break;
			else cout<<"Invalid Command!"<<endl;
			
			fflush(stdin);
			cin.clear();
		}
		catch(exception &ex)
		{
			cout<<ex.what()<<endl;
		}
	}while(true);

	return EXIT_SUCCESS;
}
