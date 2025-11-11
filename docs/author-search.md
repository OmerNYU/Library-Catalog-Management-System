# Author Search Feature

## Description
Adds an `LCMS::findByAuthor(string author) const` command that searches the catalog for books whose author field contains the provided text and prints the matching entries.

## Purpose and Usefulness
- Allows users to quickly locate all titles by a specific author or by partial author name.
- Keeps the existing `find()` keyword search generic while offering a focused author-centric lookup.

## Implementation Details
- **Location:** `lcms.hpp`
- **Method:** `LCMS::findByAuthor(string author) const`
- Uses a depth-first traversal over the category tree to gather matching `Book*` instances and reuses existing helpers for consistent output formatting.

