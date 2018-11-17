/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_DOCUMENTATION_HPP
#define ZILCH_DOCUMENTATION_HPP

namespace Zilch
{
  // Represents a parameter in a function
  class ZeroShared DocumentationParameter
  {
  public:

    // The name of the parameter (or an empty string if it's nothing)
    String Name;

    // The type of the parameter
    String Type;
  };

  // Documentation that describes an individual function
  class ZeroShared DocumentationFunction
  {
  public:

    // Destructor
    ~DocumentationFunction();

    // The name of the function
    String Name;

    // The parameter names and types
    Array<DocumentationParameter*> Parameters;

    // The return type (or empty if it's void / returns nothing)
    String ReturnType;

    // The entire signature (no name, but includes parameters and return type)
    String Signature;

    // A basic description for the function (typically parsed from comments)
    String Description;
    
    // Any samples or remarks about the usage of a particular object or member
    StringArray Remarks;

    // The exceptions that this function can throw
    StringArray ExceptionsThrown;
  };

  // Documentation that describes an individual property
  class ZeroShared DocumentationProperty
  {
  public:

    // Constructor
    DocumentationProperty();

    // Destructor
    ~DocumentationProperty();

    // The name of the property
    String Name;

    // The type of the property
    String Type;

    // A basic description for the property (typically parsed from comments)
    String Description;
    
    // Any samples or remarks about the usage of a particular object or member
    StringArray Remarks;

    // Tells us if this property can be set
    bool IsSettable;

    // Tells us if this property can be retrieved
    bool IsGettable;

    // Tells us if this property is a data member
    // Generally data members should be displayed along with properties
    bool IsField;
  };

  // Documentation that describes an individual type
  class ZeroShared DocumentationType
  {
  public:

    // Destructor
    ~DocumentationType();

    // Constructor
    DocumentationType();

    // The name of the type
    String Name;

    // The name of the base type, or empty if there is none
    String BaseName;

    // Is this a value type or a reference type?
    bool IsValueType;

    // A basic description for the type (typically parsed from comments)
    String Description;
    
    // Any samples or remarks about the usage of a particular object or member
    StringArray Remarks;
    
    // The constructors that belong to this type
    Array<DocumentationFunction*> Constructors;
    
    // The instance methods that belong to this type (sorted by name, owned)
    Array<DocumentationFunction*> InstanceMethods;
    
    // The static methods that belong to this type (sorted by name, owned)
    Array<DocumentationFunction*> StaticMethods;
    
    // The instance methods that belong to this type (sorted by name, includes data members, owned)
    Array<DocumentationProperty*> InstanceProperties;
    
    // The static methods that belong to this type (sorted by name, includes data members, owned)
    Array<DocumentationProperty*> StaticProperties;
  };

  // Represents an entire library built by our documentation engine
  class ZeroShared DocumentationLibrary
  {
  public:

    // Destructor
    ~DocumentationLibrary();
    
    // The name of the library
    String Name;

    // All the types defined within the library
    HashMap<String, DocumentationType*> TypesByName;

    // The same array of types, sorted by name
    Array<DocumentationType*> TypesSorted;
  };

  // Represents an entire library built by our documentation engine
  class ZeroShared DocumentationModule
  {
  public:

    // Destructor
    ~DocumentationModule();

    // All the libraries we're dependent upon
    Array<DocumentationLibrary*> Libraries;
  };

  class ZeroShared RstTable
  {
  public:
    // Constructor
    RstTable();

    // Resize the table to have the number of columns and rows
    void Resize(size_t columns, size_t rows);

    // Set a cell by column and row
    void SetCell(StringParam value, size_t column, size_t row);

    // Get a cell by column and row
    String GetCell(size_t column, size_t row);

    // All the cells in the table (indexed as a 2d array)
    Array<String> Cells;

    // The number of columns and rows of this table
    // Cells must be Rows * Columns in size
    size_t Columns;
    size_t Rows;

    // How many of the first rows are header rows (does not add to the total number)
    // Must be less than or equal to Rows
    size_t HeaderRows;
  };

  namespace RstHeadingType
  {
    enum Enum
    {
      Part,
      Chapter,
      Section,
      SubSection,
      SubSubSection,
      Paragraph
    };
  }

  class ZeroShared RstBuilder : public StringBuilderExtended
  {
  public:
    // We don't want to hide the base class members, but rather add overloads
    using StringBuilderExtended::WriteLine;

    // Write out a table with columns and rows
    void WriteLine(RstTable& table);

    // Write out a heading (typically bold and larger)
    void WriteLineHeading(StringRange heading, RstHeadingType::Enum type);
  };
}

#endif
