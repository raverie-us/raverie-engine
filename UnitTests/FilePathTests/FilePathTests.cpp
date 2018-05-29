///////////////////////////////////////////////////////////////////////////////
///
///  \file BlockArray.cpp
///  Unit tests for the block array.
///  
///  Authors: Joshua Davis
///  Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Common/Precompiled.hpp"

typedef unsigned int uint;
#include "Math/Vector3.hpp"

#include "CppUnitLite2/CppUnitLite2.h"
#include "String/String.hpp"
#include "Platform/FilePath.hpp"
#include "String/StringBuilder.hpp"
#include "Platform/FileSystem.hpp"

typedef Zero::StringRange StringRange;
typedef Zero::String String;

#define ZFS "\\"

void CheckCombine(CppUnitLite::TestResult& result_, const char * m_name, StringRange str0, StringRange str1, StringRange str2, StringRange expected)
{
  String path = Zero::FilePath::Combine(str0, str1, str2);
  CHECK_STRING_EQUAL(path.c_str(), String(expected).c_str());
}

#define TestCombine(str0, str1, str2, expected) \
  CheckCombine(result_, m_name, str0, str1, str2, expected)

void CheckCombineWithExtension(CppUnitLite::TestResult& result_, const char * m_name, StringRange str0, StringRange str1, StringRange str2, StringRange expected)
{
  String path = Zero::FilePath::CombineWithExtension(str0, str1, str2);
  CHECK_STRING_EQUAL(path.c_str(), String(expected).c_str());
}

#define TestCombineWithExtension(str0, str1, str2, expected) \
  CheckCombineWithExtension(result_, m_name, str0, str1, str2, expected)

TEST(FilePath_Combine1)
{
  TestCombine(   "Z:",    "Path", "Folder", "Z:"ZFS"Path"ZFS"Folder");
  TestCombine("Z:"ZFS,    "Path", "Folder", "Z:"ZFS"Path"ZFS"Folder");
  TestCombine(   "Z:", "Path"ZFS, "Folder", "Z:"ZFS"Path"ZFS"Folder");
  TestCombine("Z:"ZFS, "Path"ZFS, "Folder", "Z:"ZFS"Path"ZFS"Folder");
}

TEST(FilePath_CombineWithExtension)
{
  TestCombineWithExtension(   "Path", "FileName", ".Ext", "Path"ZFS"FileName.Ext");
  TestCombineWithExtension("Path"ZFS, "FileName", ".Ext", "Path"ZFS"FileName.Ext");
}

//------------------------------------------------------------------
#define TestStringRange(actual, expected)\
  CHECK_STRING_EQUAL(String(actual).c_str(), String(expected).c_str())

void CheckPathInfo(CppUnitLite::TestResult& result_, const char * m_name, StringRange fullPath, StringRange dirPath,
                     StringRange dirName, StringRange fileNameWithExt, StringRange fileName, StringRange ext)
{
  TestStringRange(dirPath        , Zero::FilePath::GetDirectoryPath(fullPath));
  TestStringRange(dirName        , Zero::FilePath::GetDirectoryName(fullPath));
  TestStringRange(fileNameWithExt, Zero::FilePath::GetFileName(fullPath));
  TestStringRange(fileName       , Zero::FilePath::GetFileNameWithoutExtension(fullPath));
  TestStringRange(ext            , Zero::FilePath::GetExtension(fullPath));

  Zero::FilePathInfo info = Zero::FilePath::GetPathInfo(fullPath);
  TestStringRange(dirPath , info.Folder);
  TestStringRange(fileName, info.FileName);
  TestStringRange(ext     , info.Extension);
}

#define TestPathInfo(fullPath, dirPath, dirName, fileNameWithExt, fileName, ext)\
  CheckPathInfo(result_, m_name, fullPath, dirPath, dirName, fileNameWithExt, fileName, ext)

TEST(FilePath_GetDirectoryName)
{
  TestPathInfo("Z:\\Path\\Dir\\File.Ext", "Z:\\Path\\Dir",  "Dir", "File.Ext", "File", "Ext");
  TestPathInfo("Z:\\Path\\Dir\\File."   , "Z:\\Path\\Dir",  "Dir",    "File.", "File",    "");
  TestPathInfo("Z:\\Path\\Dir\\.Ext"    , "Z:\\Path\\Dir",  "Dir",     ".Ext",     "", "Ext");
  TestPathInfo("Z:\\Path\\File.Ext"     , "Z:\\Path"     , "Path", "File.Ext", "File", "Ext");
  TestPathInfo("Z:\\Dir\\File.Ext"      , "Z:\\Dir"      ,  "Dir", "File.Ext", "File", "Ext");
  TestPathInfo("Path\\Dir\\File.Ext"    , "Path\\Dir"    ,  "Dir", "File.Ext", "File", "Ext");

  TestPathInfo("Z:\\Path\\Dir\\."       , "Z:\\Path\\Dir",  "Dir",        ".",     "",    "");
  TestPathInfo("Z:\\Path\\File."        , "Z:\\Path"     , "Path",    "File.", "File",    "");
  TestPathInfo("Z:\\Path\\Dir\\.Ext"    , "Z:\\Path\\Dir",  "Dir",     ".Ext",     "", "Ext");

  TestPathInfo("File.Ext"               ,              "",     "", "File.Ext", "File", "Ext");
  TestPathInfo(".Ext"                   ,              "",     "",     ".Ext",     "", "Ext");
  TestPathInfo("File."                  ,              "",     "",    "File.", "File",    "");
  TestPathInfo("."                      ,              "",     "",        ".",     "",    "");

  TestPathInfo("\\"                     ,              "",     "",         "",     "",    "");
  TestPathInfo(""                       ,              "",     "",         "",     "",    "");
}

//------------------------------------------------------------------
void CheckNormalizePath(CppUnitLite::TestResult& result_, const char * m_name, StringRange fullPath, StringRange expected)
{
  String normalizedPath = Zero::FilePath::Normalize(fullPath);
  CHECK_STRING_EQUAL(normalizedPath.c_str(), String(expected).c_str());
}

#define TestNormalize(fullPath, expected) \
  CheckNormalizePath(result_, m_name, fullPath, expected)

TEST(FilePath_NormalizePath)
{
  TestNormalize("Z:\\Path\\Dir\\File.Ext", "Z:\\Path\\Dir\\File.Ext");

  TestNormalize("Z:\\\\Path\\\\Dir\\\\File.Ext", "Z:\\Path\\Dir\\File.Ext");
  TestNormalize("Z:\\Path\\\\Dir\\File.Ext", "Z:\\Path\\Dir\\File.Ext");
  TestNormalize("Z:\\\\Path\\Dir\\File.Ext", "Z:\\Path\\Dir\\File.Ext");
  TestNormalize("Z:\\Path\\Dir\\\\File.Ext", "Z:\\Path\\Dir\\File.Ext");

  TestNormalize("Z:\\Path\\Dir\\", "Z:\\Path\\Dir");
  TestNormalize("Z:\\Path\\Dir\\\\", "Z:\\Path\\Dir");

  //network path
  TestNormalize("\\\\Path\\Dir", "\\\\Path\\Dir");

  TestNormalize("\\", "");
  TestNormalize("", "");
}

