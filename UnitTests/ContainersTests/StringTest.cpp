///////////////////////////////////////////////////////////////////////////////
///
///  \file StringTest.cpp
///  Unit tests for String and StringRange.
///
///  Authors: Nathan Carlson
///  Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "ContainerTestStandard.hpp"
#include "CppUnitLite2/CppUnitLite2.h"

#include "Common/String/StringRange.hpp"
#include "String/StringBuilder.hpp"
#include "Platform/Windows/WString.hpp"
#include "Platform/Thread.hpp"
#include "Platform/ThreadSync.hpp"

#include "WindowsDebugTimer.hpp"

using Zero::String;
using Zero::StringParam;
using Zero::StringRange;
using Zero::StringIterator;
using Zero::Array;
using Zero::StringSplitRange;



TEST(Compare)
{
  WindowsDebugTimer timer("Compare");
  CHECK(StringRange("b").CompareTo("a") == 1);
  CHECK(StringRange("b").CompareTo("b") == 0);
  CHECK(StringRange("b").CompareTo("c") == -1);

  CHECK(StringRange("bat").CompareTo("batman") == -1);
  CHECK(StringRange("batman").CompareTo("batman") == 0);
  CHECK(StringRange("batman").CompareTo("bat") == 1);
}

TEST(FindFirstOf)
{
  WindowsDebugTimer timer("FindFirstOfSingle");
  String text("the quick brown fox jumps over the lazy dog");
  //           0000000000111111111122222222223333333333444
  //           0123456789012345678901234567890123456789012
  text.IsAllUpper();
  // character find
  CHECK(text.FindFirstOf('a') == 'a');
  CHECK(text.FindFirstOf('b') == 'b');
  CHECK(text.FindFirstOf('c') == 'c');
  CHECK(text.FindFirstOf('d') == 'd');
  CHECK(text.FindFirstOf('e') == 'e');
  CHECK(text.FindFirstOf('f') == 'f');
  CHECK(text.FindFirstOf('g') == 'g');
  CHECK(text.FindFirstOf('h') == 'h');
  CHECK(text.FindFirstOf('i') == 'i');
  CHECK(text.FindFirstOf('j') == 'j');
  CHECK(text.FindFirstOf('k') == 'k');
  CHECK(text.FindFirstOf('l') == 'l');
  CHECK(text.FindFirstOf('m') == 'm');
  CHECK(text.FindFirstOf('n') == 'n');
  CHECK(text.FindFirstOf('o') == 'o');
  CHECK(text.FindFirstOf('p') == 'p');
  CHECK(text.FindFirstOf('q') == 'q');
  CHECK(text.FindFirstOf('r') == 'r');
  CHECK(text.FindFirstOf('s') == 's');
  CHECK(text.FindFirstOf('t') == 't');
  CHECK(text.FindFirstOf('u') == 'u');
  CHECK(text.FindFirstOf('v') == 'v');
  CHECK(text.FindFirstOf('w') == 'w');
  CHECK(text.FindFirstOf('x') == 'x');
  CHECK(text.FindFirstOf('y') == 'y');
  CHECK(text.FindFirstOf('z') == 'z');

  // range find
  StringRange foundRange = text.FindFirstOf("the");
  CHECK(foundRange.Begin().ReadCurrentRune() == 't');
  CHECK((foundRange.End() - 1).ReadCurrentRune() == 'e');
  foundRange = text.FindFirstOf("quick");
  CHECK(foundRange.Begin().ReadCurrentRune() == 'q');
  CHECK((foundRange.End() - 1).ReadCurrentRune() == 'k');
  foundRange = text.FindFirstOf("brown");
  CHECK(foundRange.Begin().ReadCurrentRune() == 'b');
  CHECK((foundRange.End() - 1).ReadCurrentRune() == 'n');
  foundRange = text.FindFirstOf("fox");
  CHECK(foundRange.Begin().ReadCurrentRune() == 'f');
  CHECK((foundRange.End() - 1).ReadCurrentRune() == 'x');
  foundRange = text.FindFirstOf("jumps");
  CHECK(foundRange.Begin().ReadCurrentRune() == 'j');
  CHECK((foundRange.End() - 1).ReadCurrentRune() == 's');
  foundRange = text.FindFirstOf("over");
  CHECK(foundRange.Begin().ReadCurrentRune() == 'o');
  CHECK((foundRange.End() - 1).ReadCurrentRune() == 'r');
  foundRange = text.FindFirstOf("the");
  CHECK(foundRange.Begin().ReadCurrentRune() == 't');
  CHECK((foundRange.End() - 1).ReadCurrentRune() == 'e');
  foundRange = text.FindFirstOf("lazy");
  CHECK(foundRange.Begin().ReadCurrentRune() == 'l');
  CHECK((foundRange.End() - 1).ReadCurrentRune() == 'y');
  foundRange = text.FindFirstOf("dog");
  CHECK(foundRange.Begin().ReadCurrentRune() == 'd');
  CHECK((foundRange.End() - 1).ReadCurrentRune() == 'g');
  foundRange = text.FindFirstOf("the quick brown fox jumps over the lazy dog");
  CHECK(foundRange.Begin().ReadCurrentRune() == 't');
  CHECK((foundRange.End() - 1).ReadCurrentRune() == 'g');
  
  // character no find
  CHECK(text.FindFirstOf('F').Empty());
  CHECK(text.FindFirstOf('5').Empty());
  CHECK(text.FindFirstOf('!').Empty());

  // range no find
  CHECK(text.FindFirstOf("four").Empty());
  CHECK(text.FindFirstOf("char").Empty());
  CHECK(text.FindFirstOf("word").Empty());

  // before the beginning
  CHECK(text.FindFirstOf(" the quick").Empty());

  // past the end
  CHECK(text.FindFirstOf("lazy dogs").Empty());

  // too many characters
  CHECK(text.FindFirstOf("the quick brown fox jumps over the lazy doge").Empty());

  // not enough characters
  CHECK(text.FindFirstOf("").Empty());

  // no text
  CHECK(String("").FindFirstOf("anything").Empty());

  // no anything
  CHECK(String("").FindFirstOf("").Empty());
}

TEST(FindLastOf)
{
  WindowsDebugTimer timer("FindLastOfSingle");
  //           0000000000111111111122222222223333333333444
  //           0123456789012345678901234567890123456789012
  String text("the quick brown fox jumps over the lazy dog");

  // character find
  CHECK(text.FindLastOf('a') == 'a');
  CHECK(text.FindLastOf('b') == 'b');
  CHECK(text.FindLastOf('c') == 'c');
  CHECK(text.FindLastOf('d') == 'd');
  CHECK(text.FindLastOf('e') == 'e');
  CHECK(text.FindLastOf('f') == 'f');
  CHECK(text.FindLastOf('g') == 'g');
  CHECK(text.FindLastOf('h') == 'h');
  CHECK(text.FindLastOf('i') == 'i');
  CHECK(text.FindLastOf('j') == 'j');
  CHECK(text.FindLastOf('k') == 'k');
  CHECK(text.FindLastOf('l') == 'l');
  CHECK(text.FindLastOf('m') == 'm');
  CHECK(text.FindLastOf('n') == 'n');
  CHECK(text.FindLastOf('o') == 'o');
  CHECK(text.FindLastOf('p') == 'p');
  CHECK(text.FindLastOf('q') == 'q');
  CHECK(text.FindLastOf('r') == 'r');
  CHECK(text.FindLastOf('s') == 's');
  CHECK(text.FindLastOf('t') == 't');
  CHECK(text.FindLastOf('u') == 'u');
  CHECK(text.FindLastOf('v') == 'v');
  CHECK(text.FindLastOf('w') == 'w');
  CHECK(text.FindLastOf('x') == 'x');
  CHECK(text.FindLastOf('y') == 'y');
  CHECK(text.FindLastOf('z') == 'z');

  // range find
  CHECK(text.FindLastOf("dog") == 'd');
  CHECK(text.FindLastOf("the") == 't');
  CHECK(text.FindLastOf("quick") == 'q');
  CHECK(text.FindLastOf("brown") == 'b');
  CHECK(text.FindLastOf("fox") == 'f');
  CHECK(text.FindLastOf("jumps") == 'j');
  CHECK(text.FindLastOf("over") == 'o');
  CHECK(text.FindLastOf("the") == 't');
  CHECK(text.FindLastOf("lazy") == 'l');
  CHECK(text.FindLastOf("the quick brown fox jumps over the lazy dog") == 't');

  // character no find
  CHECK(text.FindLastOf('F').Empty());
  CHECK(text.FindLastOf('5').Empty());
  CHECK(text.FindLastOf('!').Empty());

  // range no find
  CHECK(text.FindLastOf("four").Empty());
  CHECK(text.FindLastOf("char").Empty());
  CHECK(text.FindLastOf("word").Empty());

  // before the beginning
  CHECK(text.FindLastOf(" the quick").Empty());

  // past the end
  CHECK(text.FindLastOf("lazy dogs").Empty());

  // too many characters
  CHECK(text.FindLastOf("the quick brown fox jumps over the lazy doge").Empty());

  // not enough characters
  CHECK(text.FindLastOf("").Empty());

  // no text
  CHECK(String("").FindLastOf("anything").Empty());

  // no anything
  CHECK(String("").FindLastOf(("")).Empty());
}

TEST(FindFirstRangeOf)
{
  WindowsDebugTimer timer("FindFirstRangeOf");
  //           0000000000111111111122222222223333333333444
  //           0123456789012345678901234567890123456789012
  String text("the quick brown fox jumps over the lazy dog");
  StringRange textRange(text);

  StringRange jumpsRange = text.FindFirstOf("jumps");
  StringIterator startPos = textRange.Begin() + 20;
  StringIterator endPos = textRange.Begin() + 25;
  CHECK(jumpsRange.Begin() == startPos);
  CHECK(jumpsRange.End() == endPos);

  StringRange overRange = text.FindFirstOf("over");
  startPos = textRange.Begin() + 26;
  endPos = textRange.Begin() + 30;
  CHECK(overRange.Begin() == startPos);
  CHECK(overRange.End() == endPos);
  
  StringRange theRange = text.FindFirstOf("the");
  startPos = textRange.Begin();
  endPos = textRange.Begin() + 3;
  CHECK(theRange.Begin() == startPos);
  CHECK(theRange.End() == endPos);
  
  StringRange lazyRange = text.FindFirstOf("lazy");
  startPos = textRange.Begin() + 35;
  endPos = textRange.Begin() + 39;
  CHECK(lazyRange.Begin() == startPos);
  CHECK(lazyRange.End() == endPos);
}

TEST(FindLastRangeOf)
{
  WindowsDebugTimer timer("FindLastRangeOf");
  //           0000000000111111111122222222223333333333444
  //           0123456789012345678901234567890123456789012
  String text("the quick brown fox jumps over the lazy dog");
  StringRange textRange(text);

  StringRange foundRange = text.FindLastOf("jumps");
  StringIterator startPos = textRange.Begin() + 20;
  StringIterator endPos = textRange.Begin() + 25;
  CHECK(foundRange.Begin() == startPos);
  CHECK(foundRange.End() == endPos);

  foundRange = text.FindLastOf("over");
  startPos = textRange.Begin() + 26;
  endPos = textRange.Begin() + 30;
  CHECK(foundRange.Begin() == startPos);
  CHECK(foundRange.End() == endPos);

  foundRange = text.FindLastOf("the");
  startPos = textRange.Begin() + 31;
  endPos = textRange.Begin() + 34;
  CHECK(foundRange.Begin() == startPos);
  CHECK(foundRange.End() == endPos);

  foundRange = text.FindLastOf("lazy");
  startPos = textRange.Begin() + 35;
  endPos = textRange.Begin() + 39;
  CHECK(foundRange.Begin() == startPos);
  CHECK(foundRange.End() == endPos);
}

TEST(TrimStart)
{
  WindowsDebugTimer timer("TrimStart");
  String text0("  Some Text ");
  String text1("      Some Text ");
  String trimmed("Some Text ");

  CHECK(text0.TrimStart() == trimmed);
  CHECK(text1.TrimStart() == trimmed);
  CHECK(trimmed.TrimStart() == trimmed);

  String text2("Some  Text");
  StringIterator startIt = text2.Begin() + 4;
  StringIterator endIt = text2.Begin() + 10;
  StringRange subRange = text2.SubString(startIt, endIt);
  CHECK(subRange.TrimStart() == "Text");

  String empty0("    ");
  String empty1("");
  String trimmedEmpty;

  CHECK(empty0.TrimStart() == trimmedEmpty);
  CHECK(empty1.TrimStart() == trimmedEmpty);
  CHECK(trimmedEmpty.TrimStart() == trimmedEmpty);
}

TEST(TrimEnd)
{
  WindowsDebugTimer timer("TrimEnd");
  String text0(" Some Text ");
  String text1(" Some Text    ");
  String trimmed(" Some Text");

  CHECK(text0.TrimEnd() == trimmed);
  CHECK(text1.TrimEnd() == trimmed);
  CHECK(trimmed.TrimEnd() == trimmed);

  String text2("Some  Text");
  StringIterator startIt = text2.Begin();
  StringIterator endIt = text2.Begin() + 6;
  StringRange subRange = text2.SubString(startIt, endIt);
  CHECK(subRange.TrimEnd() == "Some");

  String empty0("    ");
  String empty1("");

  CHECK(empty0.TrimEnd().Empty());
  CHECK(empty1.TrimEnd().Empty());
}

TEST(Trim)
{
  WindowsDebugTimer timer("Trim");
  String text0("  Some Text  ");
  String text1("  Some Text");
  String text2("Some Text  ");
  String trimmed("Some Text");

  CHECK(text0.Trim() == trimmed);
  CHECK(text1.Trim() == trimmed);
  CHECK(text2.Trim() == trimmed);
  CHECK(trimmed.Trim() == trimmed);

  String text3("Some  More  Text");
  StringIterator startIt = text3.Begin() + 4;
  StringIterator endIt = text3.Begin() + 10;
  StringRange subRange = text3.SubString(startIt, endIt);
  CHECK(subRange.Trim() == "More");

  String empty0("    ");
  String empty1("");
  String trimmedEmpty;

  CHECK(empty0.Trim() == trimmedEmpty);
  CHECK(empty1.Trim() == trimmedEmpty);
  CHECK(trimmedEmpty.Trim() == trimmedEmpty);
}

TEST(Contains)
{
  WindowsDebugTimer timer("Contains");
  String text("the quick brown fox jumps over the lazy dog");

  CHECK(text.Contains("the") == true);
  CHECK(text.Contains("quick") == true);
  CHECK(text.Contains("brown") == true);
  CHECK(text.Contains("fox") == true);
  CHECK(text.Contains("jumps") == true);
  CHECK(text.Contains("over") == true);
  CHECK(text.Contains("lazy") == true);
  CHECK(text.Contains("dog") == true);
  CHECK(text.Contains("cat") == false);

  CHECK(text.Contains("k b") == true);
  CHECK(text.Contains(" ") == true);

  // "brown fox"
  StringIterator startIt = text.Begin() + 10;
  StringIterator endIt = text.Begin() + 19;
  StringRange subRange = text.SubString(startIt, endIt);
  CHECK(subRange.Contains("brown") == true);
  CHECK(subRange.Contains("fox") == true);
  CHECK(subRange.Contains("the") == false);
  CHECK(subRange.Contains("jumps") == false);
}

TEST(Join)
{
  WindowsDebugTimer timer("Join");
  Array<String> strings;
  strings.PushBack("This");
  strings.PushBack("is");
  strings.PushBack("a");
  strings.PushBack("test.");
  strings.PushBack("This");
  strings.PushBack("is");
  strings.PushBack("longer");

  CHECK(String::Join(String(" "), strings[0], strings[1]) == "This is");
  CHECK(String::Join(String(" "), strings[0], strings[1], strings[2]) == "This is a");
  CHECK(String::Join(String(" "), strings[0], strings[1], strings[2], strings[3]) == "This is a test.");

  CHECK(String::Join(String(","), strings[0], strings[1], strings[2], strings[3]) == "This,is,a,test.");
  CHECK(String::Join(String(",;"), strings[0], strings[1], strings[2], strings[3]) == "This,;is,;a,;test.");
  CHECK(String::Join(String(" "), strings[0], strings[1]).All().SizeInBytes() == StringRange(" ").SizeInBytes() + StringRange("This").SizeInBytes() + StringRange("is").SizeInBytes());

  CHECK(String::Join(String(" "), &strings[0], strings.Size()) == "This is a test. This is longer");
}

void TestStringSplit(CppUnitLite::TestResult& result_, const char * m_name, String input, StringRange splitChar, StringRange output)
{
  StringSplitRange splitRange = input.Split(splitChar);
  Zero::StringBuilder builder;

  for(; !splitRange.Empty(); splitRange.PopFront())
    builder.Append(splitRange.Front());

  CHECK(builder.ToString() == output);
}

TEST(Split)
{
  WindowsDebugTimer timer("Split");
  TestStringSplit(result_, m_name, "This is a test", " ", "Thisisatest");
  TestStringSplit(result_, m_name, "This,is,a,test", ",", "Thisisatest");
  TestStringSplit(result_, m_name, "This,;is,;a,;test", ",;", "Thisisatest");

  TestStringSplit(result_, m_name, "TA", "T", "A");
  TestStringSplit(result_, m_name, "TA", "A", "T");
  TestStringSplit(result_, m_name, "TA", "TA", "");
}

TEST(StartsWith)
{
  WindowsDebugTimer timer("StartsWith");
  String str = "This is a test";

  CHECK(str.StartsWith("T") == true);
  CHECK(str.StartsWith("This") == true);
  CHECK(str.StartsWith("This is ") == true);
  CHECK(str.StartsWith(str) == true);
  CHECK(str.StartsWith("is") == false);

  StringIterator startIt = str.Begin() + 5;
  StringIterator endIt = str.Begin() + 9;
  StringRange subRange = str.SubString(startIt, endIt);
  CHECK(subRange.StartsWith("is") == true);
  CHECK(subRange.StartsWith("This") == false);
}

TEST(EndsWith)
{
  WindowsDebugTimer timer("EndsWith");
  String str = "This is a test";

  CHECK(str.EndsWith("t") == true);
  CHECK(str.EndsWith("est") == true);
  CHECK(str.EndsWith("a test") == true);
  CHECK(str.EndsWith(str) == true);
  CHECK(str.EndsWith("a") == false);

  StringIterator startIt = str.Begin() + 5;
  StringIterator endIt = str.Begin() + 9;
  StringRange subRange = str.SubString(startIt, endIt);
  CHECK(subRange.EndsWith("a") == true);
  CHECK(subRange.EndsWith("test") == false);
}

TEST(ToLower)
{
  WindowsDebugTimer timer("ToLower");
  String text("THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG?!.-");
  String expected("the quick brown fox jumps over the lazy dog?!.-");

  CHECK(text.ToLower() == expected);

  StringIterator startIt = text.Begin() + 4;
  StringIterator endIt = text.Begin() + 15;
  StringRange subRange = text.SubString(startIt, endIt);
  CHECK(subRange.ToLower() == "quick brown");
}

TEST(ToUpper)
{
  WindowsDebugTimer timer("ToUpper");
  String text("the quick brown fox jumps over the lazy dog?!.-");
  String expected("THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG?!.-");

  CHECK(text.ToUpper() == expected);

  StringIterator startIt = text.Begin() + 4;
  StringIterator endIt = text.Begin() + 15;
  StringRange subRange = text.SubString(startIt, endIt);
  CHECK(subRange.ToUpper() == "QUICK BROWN");
}

TEST(IsAllWhitespace)
{
  WindowsDebugTimer timer("IsAllWhitespace");
  CHECK(String("").IsAllWhitespace() == true);
  CHECK(String("    ").IsAllWhitespace() == true);
  CHECK(String(" \t\r\n ").IsAllWhitespace() == true);
  CHECK(String("     no").IsAllWhitespace() == false);

  String text = "nope\r\n\t";
  StringIterator startIt = text.Begin() + 4;
  StringIterator endIt = text.Begin() + 7;
  StringRange subRange = text.SubString(startIt, endIt);
  CHECK(text.IsAllWhitespace() == false);
  CHECK(subRange.IsAllWhitespace() == true);
}

TEST(Replace)
{
  WindowsDebugTimer timer("Replace");
  //           00000000001111
  //           01234567890123
  String text("This is a test");

  CHECK(text.All().Replace(" ", "-") == "This-is-a-test");
  CHECK(text.All().Replace("-", " ") == text);
  CHECK(text.All().Replace("is", "was") == "Thwas was a test");

  CHECK(text.All().Replace("T", "t") == "this is a test");
  CHECK(text.All().Replace("t", "T") == "This is a TesT");

  //" is a "
  StringIterator startIt = text.Begin() + 4;
  StringIterator endIt = text.Begin() + 10;
  StringRange subRange = text.SubString(startIt, endIt);
  CHECK(subRange.Replace(" ", "-") == "-is-a-");
}

TEST(FindRangeInclusive)
{
  WindowsDebugTimer timer("FindRangeInclusive");
  String text0 = "Array[TemplateType]()";
  String text1 = "[TemplateType]()";
  String text2 = "Array[TemplateType]";
  String text3 = "[TemplateType]";
  String text4 = "TemplateType";
  String text5 = String();
  String text6 = "Array[[TemplateType]]()";
  String text7 = "The 'slow' green turtle";
  
  CHECK(text0.FindRangeInclusive("[", "]") == "[TemplateType]");
  CHECK(text1.FindRangeInclusive("[", "]") == "[TemplateType]");
  CHECK(text2.FindRangeInclusive("[", "]") == "[TemplateType]");
  CHECK(text3.FindRangeInclusive("[", "]") == "[TemplateType]");
  CHECK(text4.FindRangeInclusive("[", "]") == String());
  CHECK(text5.FindRangeInclusive("[", "]") == String());
  CHECK(text6.FindRangeInclusive("[[", "]]") == "[[TemplateType]]");
  CHECK(text7.FindRangeInclusive("'", "'") == "'slow'");

  String text8 = "Array[Array[TemplateType]]";
  StringIterator startIt = text8.Begin() + 6;
  StringIterator endIt = text8.Begin() + 25;
  StringRange subRange = text8.SubString(startIt, endIt);
  CHECK(subRange.FindRangeInclusive("[", "]") == "[TemplateType]");
}

TEST(FindRangeExclusive)
{
  WindowsDebugTimer timer("FindRangeExclusive");
  String text0 = "Array[TemplateType]()";
  String text1 = "[TemplateType]()";
  String text2 = "Array[TemplateType]";
  String text3 = "[TemplateType]";
  String text4 = "TemplateType";
  String text5 = String();
  String text6 = "Array[[TemplateType]]()";

  CHECK(text0.FindRangeExclusive("[", "]") == "TemplateType");
  CHECK(text1.FindRangeExclusive("[", "]") == "TemplateType");
  CHECK(text2.FindRangeExclusive("[", "]") == "TemplateType");
  CHECK(text3.FindRangeExclusive("[", "]") == "TemplateType");
  CHECK(text4.FindRangeExclusive("[", "]") == String());
  CHECK(text5.FindRangeExclusive("[", "]") == String());
  CHECK(text6.FindRangeExclusive("[[", "]]") == "TemplateType");

  String text7 = "Array[Array[TemplateType]]";
  StringIterator startIt = text7.Begin() + 6;
  StringIterator endIt = text7.Begin() + 25;
  StringRange subRange = text7.SubString(startIt, endIt);
  CHECK(subRange.FindRangeExclusive("[", "]") == "TemplateType");
}

TEST(StringIteratorEmpty)
{
  WindowsDebugTimer timer("StringIteratorEmpty");
  String text0 = "123456789123456789123456789123456789";
  StringIterator it = text0.Begin();
  StringIterator it2 = text0.End();
  while (!it.Empty())
  {
    ++it;
  }

  while (!it2.Empty())
  {
    --it;
  }
  //no check, if this test fails it would be an infinite loop
}

TEST(GetPostionTest)
{
  WindowsDebugTimer timer("GetPostionTest");
  String text0 = "This is a test";
  String text1 = "Letters don't matter at all";
  String text2 = "How many of these should I even make?";
  String text3 = "More symbols !@#$%^&*()_+";
  
  StringRange text0test0 = text0.FindFirstOf("T");
  CHECK(text0test0.Begin().GetPosition() == 0);
  StringRange text0test1 = text0.FindFirstOf("a");
  CHECK(text0test1.Begin().GetPosition() == 8);

  StringRange text1test0 = text1.FindFirstOf("d");
  CHECK(text1test0.Begin().GetPosition() == 8);
  StringRange text1test1 = text1.FindFirstOf("\'");
  CHECK(text1test1.Begin().GetPosition() == 11);

  StringRange text2test0 = text2.FindFirstOf("y");
  CHECK(text2test0.Begin().GetPosition() == 7);
  StringRange text2test1 = text2.FindFirstOf("I");
  CHECK(text2test1.Begin().GetPosition() == 25);

  StringRange text3test0 = text3.FindFirstOf("@");
  CHECK(text3test0.Begin().GetPosition() == 14);
  StringRange text3test1 = text3.FindFirstOf("+");
  CHECK(text3test1.Begin().GetPosition() == 24);
}

TEST(UnicodeReadCurrentRuneTest)
{
  WindowsDebugTimer timer("UnicodeReadCurrentRuneTest");
  StringRange unicodeText = Zero::Narrow(L"γρεεκ");

  // check that the front rune is correct
  // while at it test the range operator== shortcut that checks a single item against the front
  
  Rune front = unicodeText.Front();
  CHECK(front == 52915); // γ code point decimal value 52915
  CHECK(unicodeText == 52915); 
  unicodeText.PopFront();

  // move forward and test the next item
  front = unicodeText.Front();
  CHECK(front == 53121); // ρ code point decimal value 53121
  CHECK(unicodeText == 53121);
  unicodeText.PopFront();

  front = unicodeText.Front();
  CHECK(front == 52917); // ε code point decimal value 52917
  CHECK(unicodeText == 52917);

  // second string test
  StringRange unicodeText1 = Zero::Narrow(L"ąčęėįšųū");
  
  front = unicodeText1.Front();
  CHECK(front == 50309); // ą code point decimal value 50309
  CHECK(unicodeText1 == 50309);
  unicodeText1.PopFront();

  front = unicodeText1.Front();
  CHECK(front == 50317); // č code point decimal value 50317
  CHECK(unicodeText1 == 50317);
  unicodeText1.PopFront();

  front = unicodeText1.Front();
  CHECK(front == 50329); // ę code point decimal value 50329
  CHECK(unicodeText1 == 50329);
  unicodeText1.PopFront();

  front = unicodeText1.Front();
  CHECK(front == 50327); // ė code point decimal value 50327
  CHECK(unicodeText1 == 50327);
  unicodeText1.PopFront();
}

TEST(UnicodeCompare)
{
  //NOTE: you cannot do unicode text within the macros for this test framework
  WindowsDebugTimer timer("UnicodeCompare");

  StringRange greekStr     = Zero::Narrow(L"δσδφγη");
  StringRange greekStrOrig = Zero::Narrow(L"δσδφγη");

  StringRange greekAlpha = Zero::Narrow(L"ασδφγη");
  StringRange greekOmega = Zero::Narrow(L"ωσδφγη");

  CHECK(greekStr.CompareTo(greekAlpha) == 1);
  CHECK(greekStr.CompareTo(greekStrOrig) == 0);
  CHECK(greekStr.CompareTo(greekOmega) == -1);


  StringRange lithuanian = Zero::Narrow(L"ūųšįėęčą");
  StringRange lithuanianOrig = Zero::Narrow(L"ūųšįėęčą");
  StringRange lithuanianShorter = Zero::Narrow(L"ūųšį");

  CHECK(lithuanianShorter.CompareTo(lithuanianOrig) == -1);
  CHECK(lithuanian.CompareTo(lithuanianOrig) == 0);
  CHECK(lithuanian.CompareTo(lithuanianShorter) == 1);
}

TEST(UnicodeFindFirstOf)
{
  WindowsDebugTimer timer("UnicodeFindFirstOf");
  //                           0000000000111111111122222222223333333333444
  //                           0123458901234567890123456789012345678901234
  String text = Zero::Narrow(L"ąčę ėįšų ū ςερτυ θιοπα σδφγηξκλ΄ζχψωβνμ αω");
  StringRange textRange(text);

  String first = Zero::Narrow(L"ėįšų");
  StringRange firstRange = text.FindFirstOf(first);
  StringIterator startPos = textRange.Begin() + 4;
  StringIterator endPos = textRange.Begin() + 8;
  CHECK(firstRange.Begin() == startPos);
  CHECK(firstRange.End() == endPos);

  String second = Zero::Narrow(L"ςερτυ");
  StringRange secondRange = text.FindFirstOf(second);
  startPos = textRange.Begin() + 11;
  endPos = textRange.Begin() + 16;
  CHECK(secondRange.Begin() == startPos);
  CHECK(secondRange.End() == endPos);

  String third = Zero::Narrow(L"σδφγηξκλ");
  StringRange thirdRange = text.FindFirstOf(third);
  startPos = textRange.Begin() + 23;
  endPos = textRange.Begin() + 31;
  CHECK(thirdRange.Begin() == startPos);
  CHECK(thirdRange.End() == endPos);

  String fourth = Zero::Narrow(L"αω");
  StringRange fourthRange = text.FindFirstOf(fourth);
  startPos = textRange.Begin() + 40;
  endPos = textRange.End();
  CHECK(fourthRange.Begin() == startPos);
  CHECK(fourthRange.End() == endPos);
}

TEST(UnicodeFindLastOf)
{
  WindowsDebugTimer timer("UnicodeFindLastOf");
  //                           0000000000111111111122222222223333333333444
  //                           0123456789012345678901234567890123456789012
  String text = Zero::Narrow(L"ąčę ėįšų ū ςερτυ θιοπα σδφγηξκλ΄ζχψωβνμ αω");
  StringRange textRange(text);

  String first = Zero::Narrow(L"ėįšų");
  StringRange firstRange = text.FindLastOf(first);
  StringIterator startPos = textRange.Begin() + 4;
  StringIterator endPos = textRange.Begin() + 8;
  CHECK(firstRange.Begin() == startPos);
  CHECK(firstRange.End() == endPos);

  String second = Zero::Narrow(L"ςερτυ");
  StringRange secondRange = text.FindLastOf(second);
  startPos = textRange.Begin() + 11;
  endPos = textRange.Begin() + 16;
  CHECK(secondRange.Begin() == startPos);
  CHECK(secondRange.End() == endPos);

  String third = Zero::Narrow(L"σδφγηξκλ");
  StringRange thirdRange = text.FindLastOf(third);
  startPos = textRange.Begin() + 23;
  endPos = textRange.Begin() + 31;
  CHECK(thirdRange.Begin() == startPos);
  CHECK(thirdRange.End() == endPos);

  String fourth = Zero::Narrow(L"αω");
  StringRange fourthRange = text.FindLastOf(fourth);
  startPos = textRange.Begin() + 40;
  endPos = textRange.End();
  CHECK(fourthRange.Begin() == startPos);
  CHECK(fourthRange.End() == endPos);
}

TEST(UnicodeTrimStart)
{
  WindowsDebugTimer timer("UnicodeTrimStart");
  String text0 = Zero::Narrow(L"  σδφγηξκλ ėįšų ");
  String text1 = Zero::Narrow(L"      σδφγηξκλ ėįšų ");
  String trimmed = Zero::Narrow(L"σδφγηξκλ ėįšų ");

  CHECK(text0.TrimStart() == trimmed);
  CHECK(text0.TrimStart() == trimmed);
  CHECK(text1.TrimStart() == trimmed);
  CHECK(trimmed.TrimStart() == trimmed);

  String text2 = Zero::Narrow(L"σδφγηξκλ  ėįšų");
  String trimmed2 = Zero::Narrow(L"ėįšų");
  StringIterator startIt = text2.Begin() + 8;
  StringIterator endIt = text2.Begin() + 14;
  StringRange subRange = text2.SubString(startIt, endIt);
  CHECK(subRange.TrimStart() == trimmed2);

  //unicode spaces don't work with windows since they don't do proper UTF8 support, dumb windows
     //unicode spaces test
//   String empty0("/x00/x0A/x00/x0A");
//   String empty1("");
//   String trimmedEmpty;
// 
//   CHECK(empty0.TrimStart() == trimmedEmpty);
//   CHECK(empty1.TrimStart() == trimmedEmpty);
//   CHECK(trimmedEmpty.TrimStart() == trimmedEmpty);
}

TEST(UnicodeTrimEnd)
{
  WindowsDebugTimer timer("UnicodeTrimEnd");
  String text0 = Zero::Narrow(L" σδφγηξκλ ėįšų ");
  String text1 = Zero::Narrow(L" σδφγηξκλ ėįšų           ");
  String trimmed = Zero::Narrow(L" σδφγηξκλ ėįšų");

  CHECK(text0.TrimEnd() == trimmed);
  CHECK(text1.TrimEnd() == trimmed);
  CHECK(trimmed.TrimEnd() == trimmed);

  String text2 = Zero::Narrow(L"σδφγηξκλ  ėįšų");
  String trimmed2 = Zero::Narrow(L"σδφγηξκλ");
  StringIterator startIt = text2.Begin();
  StringIterator endIt = text2.Begin() + 10;
  StringRange subRange = text2.SubString(startIt, endIt);
  CHECK(subRange.TrimEnd() == trimmed2);

  //unicode spaces don't work with windows since they don't do proper UTF8 support, dumb windows
  //unicode spaces test
  //   String empty0("/x00/x0A/x00/x0A");
  //   String empty1("");
  //   String trimmedEmpty;
  // 
  //   CHECK(empty0.TrimEnd() == trimmedEmpty);
  //   CHECK(empty1.TrimEnd() == trimmedEmpty);
  //   CHECK(trimmedEmpty.TrimEnd() == trimmedEmpty);
}

TEST(UnicodeTrim)
{
  WindowsDebugTimer timer("UnicodeTrim");
  String text0 = Zero::Narrow(L"  σδφγηξκλ ėįšų  ");
  String text1 = Zero::Narrow(L"  σδφγηξκλ ėįšų");
  String text2 = Zero::Narrow(L"σδφγηξκλ ėįšų  ");
  String trimmed = Zero::Narrow(L"σδφγηξκλ ėįšų");

  CHECK(text0.Trim() == trimmed);
  CHECK(text1.Trim() == trimmed);
  CHECK(text2.Trim() == trimmed);
  CHECK(trimmed.Trim() == trimmed);

  String text3 = Zero::Narrow(L"σδφγηξκλ  ȸɎɤʀ  ėįšų");
  String trimmed3 = Zero::Narrow(L"ȸɎɤʀ");
  StringIterator startIt = text3.Begin() + 8;
  StringIterator endIt = text3.Begin() + 14;
  StringRange subRange = text3.SubString(startIt, endIt);
  CHECK(subRange.Trim() == trimmed3);

  //unicode spaces don't work with windows since they don't do proper UTF8 support, dumb windows
  //unicode spaces test
  //   String empty0("/x00/x0A/x00/x0A");
  //   String empty1("");
  //   String trimmedEmpty;
  // 
  //   CHECK(empty0.TrimEnd() == trimmedEmpty);
  //   CHECK(empty1.TrimEnd() == trimmedEmpty);
  //   CHECK(trimmedEmpty.TrimEnd() == trimmedEmpty);
}

TEST(UnicodeContains)
{
  WindowsDebugTimer timer("UnicodeContains");
  String text = Zero::Narrow(L"σδφγηξκλ ȸɎɤʀ ėįšų ԽՊ֏؇ ػڱ۩߷ ऽॉफ़ ওਊਖ਼૱ఔ ുെൈ ฮฯ฿ຊໜ");

  StringRange compare1 = Zero::Narrow(L"σδφγηξκλ");
  StringRange compare2 = Zero::Narrow(L"ȸɎɤʀ");
  StringRange compare3 = Zero::Narrow(L"ėįšų");
  StringRange compare4 = Zero::Narrow(L"ԽՊ֏؇");
  StringRange compare5 = Zero::Narrow(L"ػڱ۩߷");
  StringRange compare6 = Zero::Narrow(L"ऽॉफ़");
  StringRange compare7 = Zero::Narrow(L"ওਊਖ਼૱ఔ");
  StringRange compare8 = Zero::Narrow(L"ുെൈ");
  StringRange compare9 = Zero::Narrow(L"ฮฯ฿ຊໜ");
  StringRange compare10 = Zero::Narrow(L"σȸėԽػڱऽওുฮ");

  CHECK(text.Contains(compare1) == true);
  CHECK(text.Contains(compare2) == true);
  CHECK(text.Contains(compare3) == true);
  CHECK(text.Contains(compare4) == true);
  CHECK(text.Contains(compare5) == true);
  CHECK(text.Contains(compare6) == true);
  CHECK(text.Contains(compare7) == true);
  CHECK(text.Contains(compare8) == true);
  CHECK(text.Contains(compare9) == true);
  CHECK(text.Contains(compare10) == false);

  StringRange contains0 = Zero::Narrow(L"؇ ػ");
  CHECK(text.Contains(contains0) == true);
  CHECK(text.Contains(" ") == true);
}

TEST(UnicodeJoin)
{
  WindowsDebugTimer timer("UnicodeJoin");
  Array<String> strings;
  strings.PushBack(Zero::Narrow(L"σδφγηξκλ"));
  strings.PushBack(Zero::Narrow(L"ȸɎɤʀ"));
  strings.PushBack(Zero::Narrow(L"ėįšų"));
  strings.PushBack(Zero::Narrow(L"ԽՊ֏؇."));
  strings.PushBack(Zero::Narrow(L"ػڱ۩߷"));
  strings.PushBack(Zero::Narrow(L"ऽॉफ़"));
  strings.PushBack(Zero::Narrow(L"ওਊਖ਼૱ఔ"));

  StringRange joined1 = Zero::Narrow(L"σδφγηξκλ ȸɎɤʀ");
  StringRange joined2 = Zero::Narrow(L"σδφγηξκλ ȸɎɤʀ ėįšų");
  StringRange joined3 = Zero::Narrow(L"σδφγηξκλ ȸɎɤʀ ėįšų ԽՊ֏؇.");
  CHECK(String::Join(String(" "), strings[0], strings[1]) == joined1);
  CHECK(String::Join(String(" "), strings[0], strings[1], strings[2]) == joined2);
  CHECK(String::Join(String(" "), strings[0], strings[1], strings[2], strings[3]) == joined3);

  StringRange joined4 = Zero::Narrow(L"σδφγηξκλ,ȸɎɤʀ,ėįšų,ԽՊ֏؇.");
  StringRange joined5 = Zero::Narrow(L"σδφγηξκλ,;ȸɎɤʀ,;ėįšų,;ԽՊ֏؇.");
  CHECK(String::Join(String(","), strings[0], strings[1], strings[2], strings[3]) == joined4);
  CHECK(String::Join(String(",;"), strings[0], strings[1], strings[2], strings[3]) == joined5);
  CHECK(String::Join(String(" "), strings[0], strings[1]).All().SizeInBytes() == joined1.SizeInBytes());

  StringRange joined6 = Zero::Narrow(L"σδφγηξκλ ȸɎɤʀ ėįšų ԽՊ֏؇. ػڱ۩߷ ऽॉफ़ ওਊਖ਼૱ఔ");
  CHECK(String::Join(String(" "), &strings[0], strings.Size()) == joined6);
}

TEST(UnicodeSplit)
{
  WindowsDebugTimer timer("UnicodeSplit");
  StringRange preSplit1 = Zero::Narrow(L"σδφγηξκλ ȸɎɤʀ ėįšų ԽՊ֏؇");
  StringRange preSplit2 = Zero::Narrow(L"σδφγηξκλ,ȸɎɤʀ,ėįšų,ԽՊ֏؇");
  StringRange preSplit3 = Zero::Narrow(L"σδφγηξκλ,;ȸɎɤʀ,;ėįšų,;ԽՊ֏؇");
  StringRange afterSplit = Zero::Narrow(L"σδφγηξκλȸɎɤʀėįšųԽՊ֏؇");

  TestStringSplit(result_, m_name, preSplit1, " ", afterSplit);
  TestStringSplit(result_, m_name, preSplit2, ",", afterSplit);
  TestStringSplit(result_, m_name, preSplit3, ",;", afterSplit);

  StringRange preSplit4 = Zero::Narrow(L"φԽ");
  StringRange split1 = Zero::Narrow(L"φ");
  StringRange split2 = Zero::Narrow(L"Խ");

  TestStringSplit(result_, m_name, preSplit4, split1, split2);
  TestStringSplit(result_, m_name, preSplit4, split2, split1);
  TestStringSplit(result_, m_name, preSplit4, preSplit4, "");
}

TEST(UnicodeStartsWith)
{
  WindowsDebugTimer timer("UnicodeStartsWith");
  String str = Zero::Narrow(L"σδφγηξκλ ȸɎɤʀ ėįšų ԽՊ֏؇");

  StringRange start1 = Zero::Narrow(L"σ");
  StringRange start2 = Zero::Narrow(L"σδφγηξκλ");
  StringRange start3 = Zero::Narrow(L"σδφγηξκλ ȸɎɤʀ");
  StringRange doesntStartWith1 = Zero::Narrow(L"ԽՊ֏؇");

  CHECK(str.StartsWith(start1) == true);
  CHECK(str.StartsWith(start2) == true);
  CHECK(str.StartsWith(start3) == true);
  CHECK(str.StartsWith(str) == true);
  CHECK(str.StartsWith(doesntStartWith1) == false);

  StringRange start4 = Zero::Narrow(L"ėįšų");
  StringRange doesntStartWith2 = Zero::Narrow(L"σδφγηξκλ");

  StringIterator startIt = str.Begin() + 14;
  StringIterator endIt = str.End();
  StringRange subRange = str.SubString(startIt, endIt);
  CHECK(subRange.StartsWith(start4) == true);
  CHECK(subRange.StartsWith(doesntStartWith2) == false);
}

TEST(UnicodeEndsWith)
{
  WindowsDebugTimer timer("UnicodeEndsWith");
  String str = Zero::Narrow(L"σδφγηξκλ ȸɎɤʀ ėįšų ԽՊ֏؇");

  StringRange end1 = Zero::Narrow(L"؇");
  StringRange end2 = Zero::Narrow(L"ԽՊ֏؇");
  StringRange end3 = Zero::Narrow(L"ėįšų ԽՊ֏؇");
  StringRange doesntEndWith1 = Zero::Narrow(L"σδφγηξκλ");

  CHECK(str.EndsWith(end1) == true);
  CHECK(str.EndsWith(end2) == true);
  CHECK(str.EndsWith(end3) == true);
  CHECK(str.EndsWith(str) == true);
  CHECK(str.EndsWith(doesntEndWith1) == false);

  StringRange end4 = Zero::Narrow(L"ėįšų");
  StringRange doesntEndWith2 = Zero::Narrow(L"ȸɎɤʀ");

  StringIterator startIt = str.Begin() + 9;
  StringIterator endIt = str.Begin() + 18;
  StringRange subRange = str.SubString(startIt, endIt);
  CHECK(subRange.EndsWith(end4) == true);
  CHECK(subRange.EndsWith(doesntEndWith2) == false);
}

TEST(UnicodeReplace)
{
  WindowsDebugTimer timer("UnicodeReplace");
  //                           00000000001111
  //                           01234567890123
  String text = Zero::Narrow(L"σδφγηξκλ ȸɎɤʀ ėįšų ԽՊ֏؇");

  StringRange replaced1    = Zero::Narrow(L"σδφγηξκλ-ȸɎɤʀ-ėįšų-ԽՊ֏؇");
  StringRange toReplace1   = Zero::Narrow(L"κλ");
  StringRange replaceWith1 = Zero::Narrow(L"ȸɎɤʀ");
  StringRange replaced2    = Zero::Narrow(L"σδφγηξȸɎɤʀ ȸɎɤʀ ėįšų ԽՊ֏؇");

  CHECK(text.All().Replace(" ", "-") == replaced1);
  CHECK(text.All().Replace("-", " ") == text);
  CHECK(text.All().Replace(toReplace1, replaceWith1) == replaced2);

  //
  StringRange replaced3 = Zero::Narrow(L"-ȸɎɤʀ-ėįšų-");

  StringIterator startIt = text.Begin() + 8;
  StringIterator endIt = text.Begin() + 19;
  StringRange subRange = text.SubString(startIt, endIt);
  CHECK(subRange.Replace(" ", "-") == replaced3);
}

TEST(UnicodeFindRangeInclusive)
{
  WindowsDebugTimer timer("UnicodeFindRangeInclusive");
  String text0 = Zero::Narrow(L"ᑄσδφγηξȸɎɤʀᑁ ȸɎɤʀ ėįšų ԽՊ֏؇");
  String text1 = Zero::Narrow(L"ȸɎɤʀ ėįšų ᑄσδφγηξȸɎɤʀᑁ ԽՊ֏؇");
  String text2 = Zero::Narrow(L" ȸɎɤʀėįšųԽՊ֏؇ᑄσδφγηξȸɎɤʀᑁ");
  String text3 = Zero::Narrow(L"ᑄσδφγηξȸɎɤʀᑁ");
  String text4 = Zero::Narrow(L"σδφγηξȸɎɤʀ");
  String text5 = String();
  String text6 = Zero::Narrow(L"ᑄᑄσδφγηξȸɎɤʀᑁᑁ ȸɎɤʀ ėįšų ԽՊ֏؇");
  String text7 = Zero::Narrow(L"The 'ԽՊ֏؇' green turtle");

  String inclusiveStart = Zero::Narrow(L"ᑄ");
  String inclusiveEnd = Zero::Narrow(L"ᑁ");
  String inclusiveResult = Zero::Narrow(L"ᑄσδφγηξȸɎɤʀᑁ");
  String doubleInclusiveStart = Zero::Narrow(L"ᑄᑄ");
  String doubleInclusiveEnd = Zero::Narrow(L"ᑁᑁ");
  String doubleInclusiveResult = Zero::Narrow(L"ᑄᑄσδφγηξȸɎɤʀᑁᑁ");
  String inclusiveResult2 = Zero::Narrow(L"'ԽՊ֏؇'");

  CHECK(text0.FindRangeInclusive(inclusiveStart, inclusiveEnd) == inclusiveResult);
  CHECK(text1.FindRangeInclusive(inclusiveStart, inclusiveEnd) == inclusiveResult);
  CHECK(text2.FindRangeInclusive(inclusiveStart, inclusiveEnd) == inclusiveResult);
  CHECK(text3.FindRangeInclusive(inclusiveStart, inclusiveEnd) == inclusiveResult);
  CHECK(text4.FindRangeInclusive(inclusiveStart, inclusiveEnd) == String());
  CHECK(text5.FindRangeInclusive(inclusiveStart, inclusiveEnd) == String());
  CHECK(text6.FindRangeInclusive(doubleInclusiveStart, doubleInclusiveEnd) == doubleInclusiveResult);
  CHECK(text7.FindRangeInclusive("'", "'") == inclusiveResult2);
}

TEST(UnicodeFindRangeExclusive)
{
  WindowsDebugTimer timer("UnicodeFindRangeExclusive");
  String text0 = Zero::Narrow(L"ᑄσδφγηξȸɎɤʀᑁ ȸɎɤʀ ėįšų ԽՊ֏؇");
  String text1 = Zero::Narrow(L"ȸɎɤʀ ėįšų ᑄσδφγηξȸɎɤʀᑁ ԽՊ֏؇");
  String text2 = Zero::Narrow(L" ȸɎɤʀėįšųԽՊ֏؇ᑄσδφγηξȸɎɤʀᑁ");
  String text3 = Zero::Narrow(L"ᑄσδφγηξȸɎɤʀᑁ");
  String text4 = Zero::Narrow(L"σδφγηξȸɎɤʀ");
  String text5 = String();
  String text6 = Zero::Narrow(L"ᑄᑄσδφγηξȸɎɤʀᑁᑁ ȸɎɤʀ ėįšų ԽՊ֏؇");

  String exclusiveStart = Zero::Narrow(L"ᑄ");
  String exclusiveEnd = Zero::Narrow(L"ᑁ");
  String exclusiveResult = Zero::Narrow(L"σδφγηξȸɎɤʀ");
  String doubleExclusiveStart = Zero::Narrow(L"ᑄᑄ");
  String doubleExclusiveEnd = Zero::Narrow(L"ᑁᑁ");

  CHECK(text0.FindRangeExclusive(exclusiveStart, exclusiveEnd) == exclusiveResult);
  CHECK(text1.FindRangeExclusive(exclusiveStart, exclusiveEnd) == exclusiveResult);
  CHECK(text2.FindRangeExclusive(exclusiveStart, exclusiveEnd) == exclusiveResult);
  CHECK(text3.FindRangeExclusive(exclusiveStart, exclusiveEnd) == exclusiveResult);
  CHECK(text4.FindRangeExclusive(exclusiveStart, exclusiveEnd) == String());
  CHECK(text5.FindRangeExclusive(exclusiveStart, exclusiveEnd) == String());
  CHECK(text6.FindRangeExclusive(doubleExclusiveStart, doubleExclusiveEnd) == exclusiveResult);
}

class ThreadData
{
public:
  Zero::ThreadLock mLock;
  Zero::Array<String> mStrings;
  String mGlobalString;
};

ThreadData& GetThreadData()
{
  static ThreadData data;
  return data;
}

static const size_t StringCount = 10000;
static const size_t ThreadCount = 30;

unsigned long ThreadTest(void*)
{
  ThreadData& data = GetThreadData();

  for (size_t i = 0; i < StringCount; ++i)
  {
    size_t j = i % 100;
    {
      String result = String::Format("%d", j);

      data.mLock.Lock();
      data.mStrings.PushBack(result);
      data.mLock.Unlock();
    }

    data.mLock.Lock();
    data.mStrings.PopBack();
    data.mStrings.PushBack(Zero::BuildString(String::Format("%d", j), String::Format("%d", j)));
    data.mLock.Unlock();

    String::Format("%d", j);
    String::Format("__%d", j);
  }

  return 0;
}

TEST(ThreadSafety)
{
  WindowsDebugTimer timer("ThreadSafety");
  Zero::Thread threads[ThreadCount];

  const String poolA("__0");
  const String poolB("__0");
  const String poolC(String::Format("__%d", 0));
  const String poolD("__1");

#if defined(ZeroStringPooling)
  CHECK(poolA.GetNode() == poolB.GetNode());
  CHECK(poolA.GetNode() == poolC.GetNode());
  CHECK(poolA.GetNode() != poolD.GetNode());
  CHECK(poolA.GetNode()->RefCount == 3);
  CHECK(poolD.GetNode()->RefCount == 1);
#endif

  Array<String> premades;
  for (size_t i = 0; i < 20; ++i)
    premades.PushBack(String::Format("%d", i));

  for (size_t i = 0; i < ThreadCount; ++i)
  {
    threads[i].Initialize(ThreadTest, nullptr, "Test");
    threads[i].Resume();
  }

  for (size_t i = 0; i < ThreadCount; ++i)
  {
    threads[i].WaitForCompletion();
  }


  ThreadData& data = GetThreadData();

  CHECK(data.mStrings.Size() == StringCount * ThreadCount);
  data.mStrings.Clear();
  CHECK(data.mStrings.Empty());
}
