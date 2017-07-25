/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  ZilchDefineType(FilePathClass, builder, type)
  {
    ZilchFullBindMethod(builder, type, &FilePathClass::CombineDirectories, (String (*)(StringParam, StringParam)), "CombineDirectories", "dir0, dir1")->Description = FilePathClass::CombineDirectoriesDocumentation();
    ZilchFullBindMethod(builder, type, &FilePathClass::CombineDirectories, (String (*)(StringParam, StringParam, StringParam)), "CombineDirectories", "dir0, dir1, dir2")->Description = FilePathClass::CombineDirectoriesDocumentation();
    ZilchFullBindMethod(builder, type, &FilePathClass::CombineDirectories, (String (*)(StringParam, StringParam, StringParam, StringParam)), "CombineDirectories", "dir0, dir1, dir2, dir3")->Description = FilePathClass::CombineDirectoriesDocumentation();
    ZilchFullBindMethod(builder, type, &FilePathClass::CombineDirectories, (String (*)(StringParam, StringParam, StringParam, StringParam, StringParam)), "CombineDirectories", "dir0, dir1, dir2, dir3, dir4")->Description = FilePathClass::CombineDirectoriesDocumentation();

    ZilchFullBindMethod(builder, type, &FilePathClass::CombineDirectoriesAndFile, (String (*)(StringParam, StringParam)), "CombineDirectoriesAndFile", "dir0, fileName")->Description = FilePathClass::CombineDirectoriesAndFileDocumentation();
    ZilchFullBindMethod(builder, type, &FilePathClass::CombineDirectoriesAndFile, (String (*)(StringParam, StringParam, StringParam)), "CombineDirectoriesAndFile", "dir0, dir1, fileName")->Description = FilePathClass::CombineDirectoriesAndFileDocumentation();
    ZilchFullBindMethod(builder, type, &FilePathClass::CombineDirectoriesAndFile, (String (*)(StringParam, StringParam, StringParam, StringParam)), "CombineDirectoriesAndFile", "dir0, dir1, dir2, fileName")->Description = FilePathClass::CombineDirectoriesAndFileDocumentation();
    ZilchFullBindMethod(builder, type, &FilePathClass::CombineDirectoriesAndFile, (String (*)(StringParam, StringParam, StringParam, StringParam, StringParam)), "CombineDirectoriesAndFile", "dir0, dir1, dir2, dir3, fileName")->Description = FilePathClass::CombineDirectoriesAndFileDocumentation();

    //// Bind the array version of Combine
    //Array<Type*> arrayElement;
    //arrayElement.PushBack(ZilchTypeId(String));
    //LibraryArray coreArray;
    //coreArray.PushBack(Core::GetInstance().GetBuilder()->BuiltLibrary);
    //BoundType* arrayOfStrings = builder.InstantiateTemplate("Array", arrayElement, coreArray).Type;
    //Function* combineFunction = builder.AddBoundFunction(type, "Combine", &FilePathClass::Combine, OneParameter(arrayOfStrings, "parts"), ZilchTypeId(String), FunctionOptions::Static);
    //combineFunction->Description = CombineDocumentation();

    ZilchFullBindGetterSetter(builder, type, &FilePathClass::GetDirectorySeparator, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, "DirectorySeparator")->Description = FilePathClass::DirectorySeparatorDocumentation();

    ZilchFullBindMethod(builder, type, &FilePathClass::ChangeExtension, ZilchNoOverload, "ChangeExtension", ZilchNoNames)->Description = FilePathClass::ChangeExtensionDocumentation();
    ZilchFullBindMethod(builder, type, &FilePathClass::GetExtensionWithDot, ZilchNoOverload, "GetExtensionWithDot", ZilchNoNames)->Description = FilePathClass::GetExtensionWithDotDocumentation();
    ZilchFullBindMethod(builder, type, &FilePathClass::GetExtensionWithoutDot, ZilchNoOverload, "GetExtensionWithoutDot", ZilchNoNames)->Description = FilePathClass::GetExtensionWithoutDotDocumentation();
    ZilchFullBindMethod(builder, type, &FilePathClass::GetFileNameWithExtension, ZilchNoOverload, "GetFileNameWithExtension", ZilchNoNames)->Description = FilePathClass::GetFileNameWithExtensionDocumentation();
    ZilchFullBindMethod(builder, type, &FilePathClass::GetFileNameWithoutExtension, ZilchNoOverload, "GetFileNameWithoutExtension", ZilchNoNames)->Description = FilePathClass::GetFileNameWithoutExtensionDocumentation();

    ZilchFullBindMethod(builder, type, &FilePathClass::AddTrailingDirectorySeparator, ZilchNoOverload, "AddTrailingDirectorySeparator", ZilchNoNames)->Description = FilePathClass::AddTrailingDirectorySeparatorDocumentation();
    ZilchFullBindMethod(builder, type, &FilePathClass::RemoveTrailingDirectorySeparator, ZilchNoOverload, "RemoveTrailingDirectorySeparator", ZilchNoNames)->Description = FilePathClass::RemoveTrailingDirectorySeparatorDocumentation();

    ZilchFullBindMethod(builder, type, &FilePathClass::GetDirectoryPath, ZilchNoOverload, "GetDirectoryPath", ZilchNoNames)->Description = FilePathClass::GetDirectoryPathDocumentation();
    ZilchFullBindMethod(builder, type, &FilePathClass::GetDirectoryName, ZilchNoOverload, "GetDirectoryName", ZilchNoNames)->Description = FilePathClass::GetDirectoryNameDocumentation();
    ZilchFullBindMethod(builder, type, &FilePathClass::GetCanonicalizedPathFromAbsolutePath, ZilchNoOverload, "GetCanonicalizedPathFromAbsolutePath", ZilchNoNames)->Description = FilePathClass::GetCanonicalizedPathFromAbsolutePathDocumentation();
    ZilchFullBindMethod(builder, type, &FilePathClass::GetComparablePathFromAbsolutePath, ZilchNoOverload, "GetComparablePathFromAbsolutePath", ZilchNoNames)->Description = FilePathClass::GetComparablePathFromAbsolutePathDocumentation();
    ZilchFullBindMethod(builder, type, &FilePathClass::IsRelative, ZilchNoOverload, "IsRelative", ZilchNoNames)->Description = FilePathClass::IsRelativeDocumentation();

    ZilchFullBindGetterSetter(builder, type, &FilePathClass::GetWorkingDirectory, ZilchNoOverload, &FilePathClass::SetWorkingDirectory, ZilchNoOverload, "WorkingDirectory")->Description = FilePathClass::WorkingDirectoryDocumentation();
    ZilchFullBindGetterSetter(builder, type, &FilePathClass::GetTemporaryDirectory, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, "TemporaryDirectory")->Description = FilePathClass::TemporaryDirectoryDocumentation();
    ZilchFullBindGetterSetter(builder, type, &FilePathClass::GetUserLocalDirectory, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, "UserLocalDirectory")->Description = FilePathClass::UserLocalDirectoryDocumentation();

    ZilchFullBindGetterSetter(builder, type, &FilePathClass::GetUserDocumentsDirectory, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, "UserDocumentsDirectory")->Description = FilePathClass::UserDocumentsDirectoryDocumentation();
    ZilchFullBindGetterSetter(builder, type, &FilePathClass::GetExecutableDirectory, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, "ExecutableDirectory")->Description = FilePathClass::ExecutableDirectoryDocumentation();
    ZilchFullBindGetterSetter(builder, type, &FilePathClass::GetExecutableFile, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, "ExecutableFile")->Description = FilePathClass::ExecutableFileDocumentation();
  }

  //***************************************************************************
  String FilePathClass::CombineDirectories(StringParam dir0, StringParam dir1)
  {
    return AddTrailingDirectorySeparator(Zero::FilePath::Combine(dir0, dir1));
  }

  //***************************************************************************
  String FilePathClass::CombineDirectories(StringParam dir0, StringParam dir1, StringParam dir2)
  {
    return AddTrailingDirectorySeparator(Zero::FilePath::Combine(dir0, dir1, dir2));
  }

  //***************************************************************************
  String FilePathClass::CombineDirectories(StringParam dir0, StringParam dir1, StringParam dir2, StringParam dir3)
  {
    return AddTrailingDirectorySeparator(Zero::FilePath::Combine(dir0, dir1, dir2, dir3));
  }

  //***************************************************************************
  String FilePathClass::CombineDirectories(StringParam dir0, StringParam dir1, StringParam dir2, StringParam dir3, StringParam dir4)
  {
    return AddTrailingDirectorySeparator(Zero::FilePath::Combine(dir0, dir1, dir2, dir3, dir4));
  }

  //***************************************************************************
  String FilePathClass::CombineDirectoriesAndFile(StringParam dir0, StringParam fileName)
  {
    return Zero::FilePath::Combine(dir0, fileName);
  }

  //***************************************************************************
  String FilePathClass::CombineDirectoriesAndFile(StringParam dir0, StringParam dir1, StringParam fileName)
  {
    return Zero::FilePath::Combine(dir0, dir1, fileName);
  }

  //***************************************************************************
  String FilePathClass::CombineDirectoriesAndFile(StringParam dir0, StringParam dir1, StringParam dir2, StringParam fileName)
  {
    return Zero::FilePath::Combine(dir0, dir1, dir2, fileName);
  }

  //***************************************************************************
  String FilePathClass::CombineDirectoriesAndFile(StringParam dir0, StringParam dir1, StringParam dir2, StringParam dir3, StringParam fileName)
  {
    return Zero::FilePath::Combine(dir0, dir1, dir2, dir3, fileName);
  }

  //***************************************************************************
  String FilePathClass::GetDirectorySeparator()
  {
    static String DirectorySeparator(Zero::cDirectorySeparatorCstr);
    return DirectorySeparator;
  }

  //***************************************************************************
  String FilePathClass::ChangeExtension(StringParam path, StringParam extension)
  {
    // First we get the last dot in the path (it may not exist)
    StringRange pathLastDot = path.FindLastOf('.');
    StringRange pathRangeWithoutExtension = path.All();

    // If the last dot exists, then adjust the string range to not include it at all
    /*if (0 != StringRange::InvalidIndex) {}*/
      //pathRangeWithoutExtension.end = pathRangeWithoutExtension.begin + pathLastDot;

    // Check if the first character is a .'. so we know whether the user passed in '.jpeg' vs just 'jpeg'
    StringRange extensionRangeWithoutLeadingDot = extension.All();
    if (extension.Empty() == false && extension.Front() == '.')
      extensionRangeWithoutLeadingDot.IncrementByRune();

    // Concatenate the path and extension together (both should not have the dot, so add it in ourselves)
    return BuildString(pathRangeWithoutExtension, ".", extensionRangeWithoutLeadingDot);
  }

  //***************************************************************************
  String FilePathClass::GetExtensionWithDot(StringParam path)
  {
    // This gets the extension without the dot '.'
    // If there is no extension, our function is defined to return nothing
    StringRange extension = Zero::FilePath::GetExtension(path);
    if (extension.Empty())
      return extension;

    // Prepend the dot and return the extension
    return BuildString(".", extension);
  }

  //***************************************************************************
  String FilePathClass::GetExtensionWithoutDot(StringParam path)
  {
    // This gets the extension without the dot '.' always
    return Zero::FilePath::GetExtension(path);
  }

  //***************************************************************************
  String FilePathClass::GetFileNameWithExtension(StringParam path)
  {
    return Zero::FilePath::GetFileName(path);
  }

  //***************************************************************************
  String FilePathClass::GetFileNameWithoutExtension(StringParam path)
  {
    return Zero::FilePath::GetFileNameWithoutExtension(path);
  }
  
  //***************************************************************************
  String FilePathClass::AddTrailingDirectorySeparator(StringParam path)
  {
    // First check if the path already ends in a directory separator, if not add it in
    StringRange directorySeparator(Zero::cDirectorySeparatorCstr);
    if (path.EndsWith(directorySeparator))
      return path;
    else
      return BuildString(path, Zero::cDirectorySeparatorCstr);
  }
  
  //***************************************************************************
  String FilePathClass::RemoveTrailingDirectorySeparator(StringParam path)
  {
    // First check if the path already ends in a directory separator, if it does then remove it
    StringRange directorySeparator(Zero::cDirectorySeparatorCstr);
    if (path.EndsWith(directorySeparator))
    {
      StringRange pathRange = path.All();
     //--pathRange.end; TODO DANE this is not properly supported
      return pathRange;
    }
    else
    {
      return path;
    }
  }

  //***************************************************************************
  String FilePathClass::GetDirectoryPath(StringParam path)
  {
    // This function does not include the trailing directory separator
    StringRange directoryPath = Zero::FilePath::GetDirectoryPath(path);

    // Increment the end to include the path separator (if its within the range of the original path we passed in)
    if (directoryPath.Begin() >= path.Begin() && directoryPath.End() < path.End())
      ++directoryPath.End();

    return directoryPath;
  }

  //***************************************************************************
  String FilePathClass::GetDirectoryName(StringParam path)
  {
    return Zero::FilePath::GetDirectoryName(path);
  }

  //***************************************************************************
  String FilePathClass::GetCanonicalizedPathFromAbsolutePath(StringParam absolutePath)
  {
    // We want to check if it has an ending separator because our FilePathClass::Normalize removes it (but we want it to be added back in)
    bool hasEndingSeparator = false;
    if (absolutePath.SizeInBytes() > 0 && (absolutePath.Back() == '\\' || absolutePath.Back() == '/'))
      hasEndingSeparator = true;

    // Our path normalizationg changes all slashes to be the OS slashes, and removes redudant slashes
    String normalized = Zero::FilePath::Normalize(absolutePath);
    if (hasEndingSeparator)
      normalized = BuildString(normalized, Zero::cDirectorySeparatorCstr);

    // Let the operating system specific behavior canonicalize the path
    String canonicalized = Zero::CanonicalizePath(normalized);
    return canonicalized;
  }

  //***************************************************************************
  String FilePathClass::GetComparablePathFromAbsolutePath(StringParam path)
  {
    // First do path normaliziation and canonicalization
    String comparablePath = GetCanonicalizedPathFromAbsolutePath(path);

    // If the current file system is case insensative, then technically "Player.png" should compare the same as "player.PNG"
    // To make strings properly comparable, we make them all lowercase
    if (Zero::cFileSystemCaseInsensitive)
      comparablePath = comparablePath.ToLower();
    return comparablePath;
  }
  
  //***************************************************************************
  bool FilePathClass::IsRelative(StringParam path)
  {
    // Return that empty paths are relative (it doesn't really matter what we do here)
    if (path.Empty())
      return true;

    // If we have the unix 'root directory', then its not relative
    if (path.Front() == '/')
      return false;

    // Look for a windows network or UNC path
    if (path.StartsWith("\\\\"))
      return false;

    // If we find a ':' character then we assume its rooted because that is otherwise an invalid character
    // This may fail on older operating systems (we'll have to let this run wild and see what people say)
    if (!path.FindFirstOf(':').Empty())
      return false;

    // Otherwise, assume its relative if we got here!
    return true;
  }

  //***************************************************************************
  String FilePathClass::GetWorkingDirectory()
  {
    return AddTrailingDirectorySeparator(Zero::GetWorkingDirectory());
  }

  //***************************************************************************
  void FilePathClass::SetWorkingDirectory(StringParam path)
  {
    Zero::SetWorkingDirectory(path);
  }

  //***************************************************************************
  String FilePathClass::GetTemporaryDirectory()
  {
    return AddTrailingDirectorySeparator(Zero::GetTemporaryDirectory());
  }

  //***************************************************************************
  String FilePathClass::GetUserLocalDirectory()
  {
    return AddTrailingDirectorySeparator(Zero::GetUserLocalDirectory());
  }

  //***************************************************************************
  String FilePathClass::GetUserDocumentsDirectory()
  {
    return AddTrailingDirectorySeparator(Zero::GetUserDocumentsDirectory());
  }

  //***************************************************************************
  String FilePathClass::GetExecutableDirectory()
  {
    return AddTrailingDirectorySeparator(Zero::GetApplicationDirectory());
  }

  //***************************************************************************
  String FilePathClass::GetExecutableFile()
  {
    return Zero::GetApplication();
  }
}
