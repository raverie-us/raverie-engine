///////////////////////////////////////////////////////////////////////////////
///
/// \file ColorScheme.hpp
/// 
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
DeclareEvent(ColorSchemeChanged);
}

// Global loaded color scheme
class ColorScheme : public ExplicitSingleton<ColorScheme, EventObject>
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ColorScheme();
  ~ColorScheme();

  String GetActiveScheme();
  void SetActiveScheme(StringParam name);

  String GetSaveName() {return mSaveName;}
  void SetSaveName(StringParam newName);

  void LoadSchemes();
  void Load(StringParam name);
  void Save();
  void Serialize(Serializer& stream);

  void Modified();
  void UpdateConfig();
  void Enumerate(StringParam path);

  OrderedHashMap<String, String> mAvailableSchemes;
  String mFilePath;
  String mActiveName;
  String mSaveName;

  // Color Settings
  Vec4 Default;
  Vec4 Background;
  Vec4 Selection;
  Vec4 LineSelection;
  Vec4 Comment;
  Vec4 StringLiteral;
  Vec4 Character;
  Vec4 Number;
  Vec4 Keyword;
  Vec4 Operator;
  Vec4 ClassName;
  Vec4 FunctionName;
  Vec4 SpecialWords;
  Vec4 Error;
  Vec4 Whitespace;
  Vec4 Directive;
  Vec4 Gutter;
  Vec4 GutterText;
  Vec4 Link;
};

//Get the global color scheme object
ColorScheme* GetColorScheme();

}//namespace Zero
