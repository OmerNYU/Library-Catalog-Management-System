# Library Catalog Management System (LCMS)

A comprehensive command-line application for managing library catalogs with hierarchical category organization. Built in C++ with a tree-based data structure, LCMS provides efficient book storage, retrieval, and management capabilities.

## Overview

LCMS is a robust library management system that organizes books in a hierarchical category structure, similar to a file system. The system supports importing books from CSV files, organizing them into categories and subcategories, and provides powerful search and management features through an interactive command-line interface.

## Features

### Book Management
- **Import/Export**: Import books from CSV files and export the entire catalog back to CSV format
- **Add Books**: Interactively add new books with validation and duplicate detection
- **Edit Books**: Modify book details (title, author, ISBN, publication year) with an intuitive menu system
- **Remove Books**: Delete books with confirmation prompts
- **Search Capabilities**:
  - Keyword search across all book fields and categories
  - Author-specific search (extra feature)
  - Title-based book lookup
  - Category-based book listing

### Category Management
- **Hierarchical Structure**: Organize books in nested categories (e.g., `Science/Physics/Quantum Mechanics`)
- **Category Operations**: Create, edit, rename, and remove categories and subcategories
- **Automatic Path Creation**: Categories are automatically created when importing books or adding books to non-existent paths
- **Category Search**: Find and verify category existence in the catalog

### User Interface
- **Interactive CLI**: Command-line interface with helpful prompts and formatted output
- **Pretty Printing**: Formatted book details with bordered output blocks
- **Tree Visualization**: Display the entire category hierarchy with UTF-8 tree connectors
- **Command Help**: Built-in help system listing all available commands

## Project Structure

```
starter-code/
├── lcms.hpp          # Main LCMS class - facade layer for CLI operations
├── tree.hpp          # Tree and Node classes - hierarchical data structure
├── book.hpp          # Book model with fields and I/O helpers
├── myvector.hpp      # Custom vector implementation
├── main.cpp          # Entry point and command parser
├── booklist.csv      # Sample CSV file with book data
└── docs/
    └── author-search.md  # Documentation for author search feature
```

### Architecture

The project follows a layered architecture:

1. **Presentation Layer** (`main.cpp`): Command parsing and user interaction
2. **Facade Layer** (`lcms.hpp`): Thin wrapper that translates user commands to tree operations
3. **Data Layer** (`tree.hpp`, `book.hpp`): Core data structures and business logic
4. **Utility Layer** (`myvector.hpp`): Custom container implementation

## Building the Project

### Prerequisites
- C++ compiler with C++11 support (g++, clang++, etc.)
- Standard C++ library

### Compilation

Compile the project using your preferred C++ compiler:

```bash
g++ -std=c++11 -o lcms main.cpp
```

Or with additional optimization flags:

```bash
g++ -std=c++11 -O2 -Wall -o lcms main.cpp
```

### Running the Application

```bash
./lcms
```

## Usage

### Starting the Application

Upon launching, LCMS displays a welcome message and lists all available commands. The system is ready to accept commands immediately.

### Available Commands

#### Book Operations

| Command | Description | Example |
|---------|-------------|---------|
| `import <file>` | Import books from a CSV file | `import booklist.csv` |
| `export <file>` | Export all books to a CSV file | `export output.csv` |
| `find <keyword>` | Search for books and categories containing keyword | `find Darwin` |
| `findAuthor <author>` | Find all books by a specific author | `findAuthor Dawkins` |
| `findBook <title>` | Search for a specific book by title | `findBook "The Origin of Species"` |
| `findAll <category>` | List all books in a category/subcategory | `findAll Biology/Evolution` |
| `addBook` | Interactively add a new book | `addBook` |
| `editBook <title>` | Edit an existing book's details | `editBook "The Selfish Gene"` |
| `removeBook <title>` | Remove a book from the catalog | `removeBook "The Origin of Species"` |

#### Category Operations

| Command | Description | Example |
|---------|-------------|---------|
| `findCategory <path>` | Check if a category exists | `findCategory Science/Physics` |
| `addCategory <path>` | Create a new category/subcategory | `addCategory Science/Astronomy` |
| `editCategory <path>` | Rename a category | `editCategory Science/Physics` |
| `removeCategory <path>` | Remove a category and all its contents | `removeCategory Science/Physics` |

#### Utility Commands

| Command | Description |
|---------|-------------|
| `list` | Display the entire category tree structure |
| `help` | Show the list of available commands |
| `exit` | Exit the application |

### Command Aliases

For convenience, many commands support case-insensitive variants and short aliases:

- `findAuthor`, `findauthor`, `fauth`
- `findBook`, `findbook`, `fb`
- `findAll`, `findall`, `fa`
- `addBook`, `addbook`, `ab`
- `editBook`, `editbook`, `eb`
- `removeBook`, `removebook`, `rb`
- `findCategory`, `findcategory`, `fc`
- `addCategory`, `addcategory`, `ac`
- `editCategory`, `editcategory`, `ec`
- `removeCategory`, `removecategory`, `rc`
- `help`, `h`
- `exit`, `quit`

## CSV File Format

LCMS uses a standard CSV format for importing and exporting books. The format is:

```csv
Title,Author,ISBN,Year,Category
"Book Title","Author Name","ISBN-13",2023,"Category/Subcategory"
```

### Format Specifications

- **Header Row**: The first line must contain: `Title,Author,ISBN,Year,Category`
- **Fields**: Each book entry requires exactly 5 fields
- **Quotes**: Fields containing commas or special characters should be quoted
- **Escaped Quotes**: Use double quotes (`""`) to represent a literal quote within a quoted field
- **Category Path**: Use forward slashes (`/`) to separate category levels
- **Year**: Must be a valid integer (supports negative years for historical dates)

### Example CSV Entry

```csv
"The Structure of Scientific Revolutions","Thomas S. Kuhn","978-0226458120",1962,"Philosophy/Philosophy of Science"
```

## Key Design Features

### Memory Management
- Automatic memory management through RAII principles
- Proper cleanup of dynamically allocated objects
- No memory leaks through careful ownership semantics

### Data Integrity
- Duplicate detection prevents adding the same book twice
- Validation of input data (years, paths, etc.)
- Path normalization handles edge cases (extra slashes, whitespace)

### Search Efficiency
- Depth-first search (DFS) for tree traversal
- Efficient keyword matching across all book fields
- Optimized collection of matches in single pass

### User Experience
- Clear error messages and validation feedback
- Confirmation prompts for destructive operations
- Formatted output with consistent styling
- Helpful command suggestions and aliases

## Technical Details

### Data Structures

- **Tree**: General tree structure for hierarchical category organization
- **Node**: Represents a category, containing child nodes and books
- **Book**: Simple data class with title, author, ISBN, and publication year
- **MyVector**: Custom vector implementation used throughout the project

### Algorithm Complexity

- **Search Operations**: O(n) where n is the total number of books and categories
- **Insertion**: O(h) where h is the height of the category path
- **Deletion**: O(n) for subtree deletion (includes all descendants)
- **Export**: O(n) for complete catalog export

## Example Workflow

1. **Import Initial Data**:
   ```
   > import booklist.csv
   22 records have been imported.
   ```

2. **Browse Categories**:
   ```
   > list
   Library
   ├── Biology
   │   ├── Evolution
   │   ├── Evolutionary Biology
   │   ├── Molecular Biology
   │   └── Neuroscience
   ├── Philosophy
   │   └── Philosophy of Science
   ├── Physics
   │   └── Cosmology
   └── Psychology
       ├── Behavioral Economics
       └── Psychoanalysis
   ```

3. **Search for Books**:
   ```
   > find Darwin
   1 Category/sub-category found.
   1 Book found.
   ============================================================
   List of Categories containing <Darwin>:
   1: Biology/Evolutionary Biology
   ============================================================
   List of Books containing <Darwin>:
   ------------------------------------------------------------
   Title:  The Origin of Species
   Author(s):  Charles Darwin
   ISBN:  978-0486450063
   Year:  1859
   ------------------------------------------------------------
   ```

4. **Add a New Book**:
   ```
   > addBook
   Enter Title: The Double Helix
   Enter Author(s): James D. Watson
   Enter ISBN: 978-0684852799
   Enter Publication Year: 1968
   Enter Category: Biology/Molecular Biology
   The Double Helix has been successfully added into the Catalog.
   ```

5. **Export Updated Catalog**:
   ```
   > export updated_catalog.csv
   23 records have been successfully exported to updated_catalog.csv
   ```

## Notes

- The system automatically normalizes category paths (removes extra slashes, trims whitespace)
- Books are compared for duplicates using ISBN when available, otherwise by title, author, and year
- Category paths are case-sensitive
- The root category cannot be removed or renamed
- All operations maintain data consistency and prevent orphaned references

## License

This project is provided as-is for educational and demonstration purposes.

## Author

Developed as part of a library catalog management system project. Some CLI logic is inspired by a previous shell implementation project: [Shell_C](https://github.com/OmerNYU/Shell_C).

---

For questions or issues, please refer to the inline code documentation or contact the project maintainer.

