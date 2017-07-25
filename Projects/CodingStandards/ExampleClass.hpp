///////////////////////////////////////////////////////////////////////////////
///
///  \file ExampleClass.hpp
///  Declares the simple ExampleClass.
///
///  Authors: Chris Peters
///  Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once
#include "Standard.hpp"
#include "Dependency.hpp"
#include "Folder/SubFolder/Dependency.hpp"


//Always to root
#include "Physics/Blah/Blah.hpp"

namespace Zero
{

//Prefixes for vars
//m for members
//c for constants
//i for index variables and iterators

//USE FORWARD DECLARTIONS
class SomeClassName;

//Enums in a class or in a namespace;
//Do not use ambiguous abbreviations.
//Filenames same as class names.
//Namespace names same as class names.

//Local variables camel case localVariableName
//Member variables mMemberVariables
//Global const variables cSomeFixedConstant

//Global objects pointers SubNamespace::Object->
//NO GLOBAL VARIABLES OF CLASS TYPE


///Number of tests used by the example.
const int cNoOfTests;

///No constant of class types.

///Short description. Longer more detailed description. You should
///answer the what the class DOES (not how). 
class ClassName : public BaseClass
{
public: //before private

  ///Short desc. Longer desc. Why and how to use not what it does.
  void DoOperation(int personId, float otherVar);

  ///Getters and setters
  int GetA();
  void SetA(int value);


  //Immutable string class
  void FunctionTakesAString(StringRef name);
  string MyString;


  //GameObjectId;
private:

  int mA;
  int mMemberVariable;

};

///Example function
void SimpleFunction()
{
  int newAge = oldAge + 1;
}

///Class with Abbreviations.
class UrlTableMap
{
};

//inlined functions
inline void SimpleInlinedFunction()
{
  ///Less
  //then
  //10 
  //lines
}


//Enumerations
namespace Alignment
{
  enum AligmentType
  {
    Left,
    Right,
  }
}


template<typename type>
void SomeTemplatedFunction()
{

}


void ExampleData(uint a, uint b)
{
  for(uint iShape = 0; iShape < Shapes.size(); ++iShape)
  {

  }
}


}// namespace Zero
