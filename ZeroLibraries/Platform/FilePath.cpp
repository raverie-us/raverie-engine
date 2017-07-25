///////////////////////////////////////////////////////////////////////////////
///
/// \file FilePath.cpp
/// Implementation of the FilePath class.
///
/// Authors: Joshua Davis
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

const Rune cExtensionDelimiter = Rune('.');

String FilePath::Combine(StringRange path0, StringRange path1)
{
  const StringRange* paths[2];
  paths[0] = &path0;
  paths[1] = &path1;
  return Combine(paths, 2, StringRange());
}

String FilePath::Combine(StringRange path0, StringRange path1, StringRange path2)
{
  const StringRange* paths[3];
  paths[0] = &path0;
  paths[1] = &path1;
  paths[2] = &path2;
  return Combine(paths, 3, StringRange());
}

String FilePath::Combine(StringRange path0, StringRange path1, StringRange path2, StringRange path3)
{
  const StringRange* paths[4];
  paths[0] = &path0;
  paths[1] = &path1;
  paths[2] = &path2;
  paths[3] = &path3;
  return Combine(paths, 4, StringRange());
}

String FilePath::Combine(StringRange path0, StringRange path1, StringRange path2, StringRange path3, StringRange path4)
{
  const StringRange* paths[5];
  paths[0] = &path0;
  paths[1] = &path1;
  paths[2] = &path2;
  paths[3] = &path3;
  paths[4] = &path4;
  return Combine(paths, 5, StringRange());
}

String FilePath::CombineWithExtension(StringRange path, StringRange fileName, StringRange ext)
{
  if(!ext.Empty())
    ErrorIf(ext.Front() != '.', "CombineWithExtension assumes that the passed in extension has the '.'");

  const StringRange* paths[2];
  paths[0] = &path;
  paths[1] = &fileName;
  return Combine(paths, 2, ext);
}

StringRange FilePath::GetExtension(StringRange path)
{
  return GetPathInfo(path).Extension;
}

StringRange FilePath::GetFileName(StringRange path)
{
  StringRange found = path.FindLastOf(cDirectorySeparatorRune);
  if(found.Empty())
    return path;

  return StringRange(found.End(), path.End());
}

StringRange FilePath::GetFileNameWithoutExtension(StringRange path)
{
  return GetPathInfo(path).FileName;
}

StringRange FilePath::GetDirectoryName(StringRange path)
{
  StringRange directoryPath = GetDirectoryPath(path);

  StringRange found = directoryPath.FindLastOf(cDirectorySeparatorRune);
  if(found.Empty())
    return directoryPath;

  return StringRange(found.End(), directoryPath.End());
}

StringRange FilePath::GetDirectoryPath(StringRange path)
{
  StringRange found = path.FindLastOf(cDirectorySeparatorRune);
  if(found.Empty())
    return StringRange();

  return path.SubString(path.Begin(), found.Begin());
}

String FilePath::Normalize(StringRange path)
{
  StringBuilder buffer;

  // Copy over path data
  StringIterator it = path.Begin();
  StringIterator end = path.End();
  while (it < end)
  {
    Rune current = *it;

    // If it's either slash, use the operating system specific slash
    bool isDirectorySeparator = (current == '/' || current == '\\');
    if (isDirectorySeparator)
      buffer.Append(cDirectorySeparatorRune);
    else
      buffer.Append(current);

    ++it;

    // Skip duplicate separators unless it is at the beginning
    // for network paths
    if (isDirectorySeparator && it != (path.Begin() + 1))
    {
      while (it < end && (*it == '/' || *it == '\\'))
        ++it;
    }
  }

  // We strip out trailing separators as well
  // (we can only have 1 at the end due to the normalization part of the code)
  if (buffer.GetSize() > 0 && buffer[buffer.GetSize() - 1] == cDirectorySeparatorRune)
  {
    buffer[buffer.GetSize() - 1] = '\0';
    return buffer.ToString();
  }

  return buffer.ToString();
}

FilePathInfo FilePath::GetPathInfo(StringRange path)
{
  StringRange fileExt;
  FilePathInfo info;

  //find the directory separator
  StringRange found = path.FindLastOf(cDirectorySeparatorRune);
  if(!found.Empty())
  {
    //extract the directory
    info.Folder = path.SubString(path.Begin(), found.Begin());
    //what's left is the file and extension
    fileExt = path.SubString(found.End(), path.End());
  }
  else
    fileExt = path;

  //MAINLY BELOW HERE
  //now find the extension separator
  found = fileExt.FindLastOf(Rune('.'));
  if(!found.Empty())
  {
    info.FileName = StringRange(fileExt.Begin(), found.Begin());
    info.Extension = StringRange(found.End(), fileExt.End());
  }
  else
    info.FileName = fileExt;

  return info;
}

String FilePath::Combine(const StringRange** paths, uint count, StringRange extension)
{
  if(count == 0)
    return String();

  //count the total size needed to combine these paths
  size_t pathSize = 0;
  for(uint i = 0; i < count; ++i)
    pathSize += paths[i]->SizeInBytes();
  //just assume that no path has the ending separator,
  //then we need extra space for each / and then one for the null
  pathSize += count;
  //also account for the extension's length (assumed it Contains the '.')
  pathSize += extension.SizeInBytes();

  char* stringData = (char*)alloca(pathSize);
  size_t currentIndex = 0;
  for(uint i = 0; i < count - 1; ++i)
  {
    const StringRange& path = *paths[i];

    //if this string was empty, don't do anything (don't add extra slashes or anything weird)
    if(path.Empty())
      continue;

    //copy the string over
    size_t bytes = path.SizeInBytes();
    memcpy(stringData + currentIndex, path.Data(), bytes);

    //check the last character in the string, if it
    //is not a / then put one at the last position
    char* separator = stringData + currentIndex + bytes - 1;
    if(*separator != cDirectorySeparatorRune)
    {
      *(separator + 1) = cDirectorySeparatorRune.value;
      bytes += 1;
    }
    currentIndex += bytes;
  }

  //copy over the last string, but don't put a slash at the end
  const StringRange& path = *paths[count - 1];
  if(!path.Empty())
  {
    size_t bytes = path.SizeInBytes();
    memcpy(stringData + currentIndex, path.Data(), bytes);
    currentIndex += bytes;
  }
  //if there's an extension Append that as well
  if(!extension.Empty())
  {
    size_t bytes = extension.SizeInBytes();
    memcpy(stringData + currentIndex, extension.Data(), bytes);
    currentIndex += bytes;
  }

  //add the null terminator
  stringData[currentIndex] = 0;

  return String(stringData, currentIndex);
}

}//namespace Zero
