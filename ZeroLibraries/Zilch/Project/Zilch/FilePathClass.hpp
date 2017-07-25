/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_FILE_PATH_HPP
#define ZILCH_FILE_PATH_HPP

namespace Zilch
{
  class ZeroShared FilePathClass
  {
  public:
    ZilchDocument(FilePathClass,
    "A helper class for building file paths and extracting information from file paths");
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    ZilchDocument(CombineDirectories,
    "Combines directory paths and directories names together (empty entries are skipped). "
    "This will always include a directory separator at the end of the result.\n"
    "Example: ('Content', 'Powerups') results in 'Content\\Powerups\\'\n"
    "Example: ('Content\\', 'Powerups\\') results in 'Content\\Powerups\\'\n"
    "Example: ('Content\\', '', 'Powerups') results in 'Content\\Powerups\\'\n"
    "Example: ('C:\\Sandbox\\', 'Content') results in 'C:\\Sandbox\\Content\\'\n");
    static String CombineDirectories(StringParam dir0, StringParam dir1);
    static String CombineDirectories(StringParam dir0, StringParam dir1, StringParam dir2);
    static String CombineDirectories(StringParam dir0, StringParam dir1, StringParam dir2, StringParam dir3);
    static String CombineDirectories(StringParam dir0, StringParam dir1, StringParam dir2, StringParam dir3, StringParam dir4);
    
    ZilchDocument(CombineDirectoriesAndFile,
    "Combines directory paths, directories names, and a single file name together (empty entries are skipped). "
    "Because we are combining a file name at the end, this will not result in a trailing directory separator.\n"
    "Example: ('Content\\Powerups\\', 'Recharge.png') results in 'Content\\Powerups\\Recharge.png'\n"
    "Example: ('Content\\Powerups', '', 'Recharge.png') results in 'Content\\Powerups\\Recharge.png'\n"
    "Example: ('Content', 'Powerups', 'Recharge.png') results in 'Content\\Powerups\\Recharge.png'\n"
    "Example: ('C:\\Sandbox\\', 'Content\\Player.png') results in 'C:\\Sandbox\\Content\\Player.png'\n");
    static String CombineDirectoriesAndFile(StringParam dir0, StringParam fileName);
    static String CombineDirectoriesAndFile(StringParam dir0, StringParam dir1, StringParam fileName);
    static String CombineDirectoriesAndFile(StringParam dir0, StringParam dir1, StringParam dir2, StringParam fileName);
    static String CombineDirectoriesAndFile(StringParam dir0, StringParam dir1, StringParam dir2, StringParam dir3, StringParam fileName);
    
    ZilchDocument(DirectorySeparator,
    "Gets the character(s) used for separating directories and files. "
    "This value is often different depending on the operating system (generally either '/' or '\\')\n");
    static String GetDirectorySeparator();
    
    ZilchDocument(ChangeExtension,
    "Changes the extension of a path (with file name at the end) to a new extension. "
    "If the file has no extension, then this will automatically add the extension to the end. "
    "The extension is allowed to contain a leading dot '.' character (or not). "
    "The path is also allowed to contain a trailing dot '.' character (or not).\n"
    "Example: ('Content\\Player.png', 'jpg') results in 'Content\\Player.jpg'\n"
    "Example: ('Content\\Player', 'jpg') results in 'Content\\Player.jpg'\n"
    "Example: ('Content\\Player.', '.jpg') results in 'Content\\Player.jpg'\n");
    static String ChangeExtension(StringParam path, StringParam extension);
    
    ZilchDocument(GetExtensionWithDot,
    "Returns only the extension of a file (everything after the last dot, including the dot). "
    "If the file has no extension then this will return an empty string.\n"
    "Example: ('Content\\Player.png') results in '.png'\n"
    "Example: ('Content\\Player.') results in ''\n"
    "Example: ('Parent.Directory\\Log') results in ''\n");
    static String GetExtensionWithDot(StringParam path);
    
    ZilchDocument(GetExtensionWithoutDot,
    "Returns only the extension of a file (everything after the last dot, not including the dot). "
    "If the file has no extension then this will return an empty string.\n"
    "Example: ('Content\\Player.png') results in 'png'\n"
    "Example: ('Content\\Player.') results in ''\n"
    "Example: ('Parent.Directory\\Log') results in ''\n");
    static String GetExtensionWithoutDot(StringParam path);

    ZilchDocument(GetFileNameWithExtension,
    "Returns only the file portion of a path (everything past the last separator including the extension).\n"
    "Example: ('Content\\Player.png') results in 'Player.png'\n"
    "Example: ('Content\\Powerups\\') results in ''\n"
    "Example: ('Content\\Powerups') results in 'Powerups'\n");
    static String GetFileNameWithExtension(StringParam path);
    
    ZilchDocument(GetFileNameWithoutExtension,
    "Returns only the file portion of a path (everything past the last separator excluding the extension).\n"
    "Example: ('Content\\Player.png') results in 'Player'\n"
    "Example: ('Content\\Powerups\\') results in ''\n"
    "Example: ('Content\\Powerups') results in 'Powerups'\n");
    static String GetFileNameWithoutExtension(StringParam path);

    ZilchDocument(AddTrailingDirectorySeparator,
    "Pass in a directory path with or without the separator and this will add it at the end (if needed).\n"
    "Example: ('Content\\Powerups') results in 'Content\\Powerups\\'\n"
    "Example: ('Content\\Powerups\\') results in 'Content\\Powerups\\'\n");
    static String AddTrailingDirectorySeparator(StringParam path);

    ZilchDocument(RemoveTrailingDirectorySeparator,
    "Pass in a directory path with or without the separator and this will remove it from the end (if needed).\n"
    "Example: ('Content\\Powerups') results in 'Content\\Powerups'\n"
    "Example: ('Content\\Powerups\\') results in 'Content\\Powerups'\n");
    static String RemoveTrailingDirectorySeparator(StringParam path);

    ZilchDocument(GetDirectoryPath,
    "If a file path is passed in, this will return the parent directory. "
    "If a directory path is passed in (ending in a separator), this will return the directy back with no modifications. "
    "A directory path without a trailing separator is abiguous with a file that has no extension. "
    "This will always include a directory separator at the end of the result. "
    "In this case, we always assume it is a file and therefore get the parent directory's name.\n"
    "Example: ('Content\\Powerups\\Recharge.png') results in 'Content\\Powerups\\'\n"
    "Example: ('Content\\Powerups\\') results in 'Content\\Powerups\\'\n"
    "Example: ('Content\\Powerups') results in 'Content\\'\n"
    "Example: ('Content') results in ''\n");
    static String GetDirectoryPath(StringParam path);

    ZilchDocument(GetDirectoryName,
    "If a file path is passed in, this will return the name of the parent directory. "
    "If a directory path is passed in (ending in a separator), this will return the name of the directory. "
    "A directory path without a trailing separator is abiguous with a file that has no extension. "
    "In this case, we always assume it is a file and therefore get the parent directory's name.\n"
    "Example: ('Content\\Powerups\\Recharge.png') results in 'Powerups'\n"
    "Example: ('Content\\Powerups\\') results in 'Powerups'\n"
    "Example: ('Content\\Powerups') results in 'Content'\n"
    "Example: ('Content') results in ''\n");
    static String GetDirectoryName(StringParam path);

    ZilchDocument(GetCanonicalizedPathFromAbsolutePath,
    "Changes all directory separators to be the current operating system directory separator, removes duplicate separators, and removes '..' and '.' from the paths. "
    "Canonicalized is only guaranteed to work on absolute paths. "
    "This behavior is operating system dependent and may call the related OS functions.\n"
    "Example: ('C:/Sandbox//Engine/../Content/./Player.png') results in 'C:\\Sandbox\\Content\\Player.png'\n");
    static String GetCanonicalizedPathFromAbsolutePath(StringParam absolutePath);

    ZilchDocument(GetComparablePathFromAbsolutePath,
    "First this normalizes the path, then if the operating system is case insensative, it will make the path all lowercase so that it compares.\n"
    "Example: ('C:\\Sandbox\\Engine\\..\\Content\\.\\Player.png') results in 'c:\\sandbox\\content\\player.png'\n");
    static String GetComparablePathFromAbsolutePath(StringParam path);

    ZilchDocument(IsRelative,
    "Returns true if a path has no root (such as a volume/hard drive specifier, or unix like systems a beginning slash). "
    "Even a beginning slash that means 'relative to the current working directory volume' is still relative. "
    "Empty paths will return that they are relative.\n"
    "Example: ('C:\\Sandbox\\Engine\\..\\Content\\.\\Player.png') results in 'false'\n"
    "Example: ('Sandbox') results in 'true'\n"
    "Example: ('Content\\Powerups\\Recharge.png') results in 'true'\n"
    "Example: ('/usr/Content/Player.png') results in 'false'\n");
    static bool IsRelative(StringParam path);
    
    ZilchDocument(WorkingDirectory,
    "A directory that all relative paths start resolving from. "
    "In general the changing of the working directory is discouraged because it may affect assumptions of the host application. "
    "This will always include a directory separator at the end of the result.\n");
    static String GetWorkingDirectory();
    static void SetWorkingDirectory(StringParam path);
    
    ZilchDocument(TemporaryDirectory,
    "Temporary files should be placed here. "
    "This will always include a directory separator at the end of the result.\n");
    static String GetTemporaryDirectory();
    
    ZilchDocument(UserLocalDirectory,
    "Application saved information should be placed here (read/write/create permissions should be allowed). "
    "This will always include a directory separator at the end of the result.\n");
    static String GetUserLocalDirectory();
    
    ZilchDocument(UserDocumentsDirectory,
    "User saved data that the user can backup or modify should be placed here (read/write/create permissions should be allowed). "
    "This will always include a directory separator at the end of the result.\n");
    static String GetUserDocumentsDirectory();
    
    ZilchDocument(ExecutableDirectory,
    "The directory the executable lives with in (exe, elf...). "
    "This will always include a directory separator at the end of the result.\n");
    static String GetExecutableDirectory();
    
    ZilchDocument(ExecutableFile,
    "A path directly to the executable itself (exe, elf...).\n");
    static String GetExecutableFile();
  };
}

#endif
