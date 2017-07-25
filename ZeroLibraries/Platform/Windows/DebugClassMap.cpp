///////////////////////////////////////////////////////////////////////////////
///
/// \file DebugClassMap.cpp
/// Implementation of the file class for Windows.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
// Only compile this on the MSVC compiler (not supported by other compilers on Windows)
#ifdef _MSC_VER

// Dbghelp has warnings in VS2015 (unnamed typedef)
#pragma warning(disable : 4091)
#define _NO_CVCONST_H
#include <dbghelp.h>

#pragma comment(lib, "dbghelp.lib")

namespace Zero
{

enum DataKind
{
  DataIsUnknown,
  DataIsLocal,
  DataIsStaticLocal,
  DataIsParam,
  DataIsObjectPtr,
  DataIsFileStatic,
  DataIsGlobal,
  DataIsMember,
  DataIsStaticMember,
  DataIsConstant
};

class TypeModule
{
public:
  UINT mModuleBase;
  HANDLE mProcess;

  TypeModule()
  {
    mModuleBase = 0;
    mProcess = GetCurrentProcess();
  }

  String GetSymbolName(uint classIndex)
  {
    wchar_t* text = L"";
    VerifyWin(SymGetTypeInfo(mProcess, mModuleBase, (ULONG)classIndex, TI_GET_SYMNAME, &text));

    char* asciiText = (char*)alloca(MAX_SYM_NAME);
    ConvertUnicodeToAscii(asciiText, MAX_SYM_NAME, text, wcslen(text));

    return asciiText;
  }

  uint GetDataKind(uint classIndex)
  {
    DWORD dataKind = 0;
    VerifyWin(SymGetTypeInfo(mProcess, mModuleBase, (ULONG)classIndex, TI_GET_DATAKIND, &dataKind));
    return dataKind;
  }

  uint GetTag(uint classIndex)
  {
    DWORD tag = 0;
    SymGetTypeInfo(mProcess, mModuleBase, (ULONG)classIndex, TI_GET_SYMTAG, &tag );
    return tag;
  }

  uint GetOffset(uint index)
  {
    DWORD offset;
    VerifyWin(SymGetTypeInfo(mProcess, mModuleBase, (ULONG)index, TI_GET_OFFSET, &offset));
    return offset;
  }

  uint GetClassParent(uint index)
  {
    DWORD classIndex = 0; 
    VerifyWin(SymGetTypeInfo(mProcess, mModuleBase, (ULONG)index, TI_GET_CLASSPARENTID, &classIndex ));
    return classIndex;
  }

  uint GetSize(uint index)
  {
    DWORD size;
    VerifyWin(SymGetTypeInfo(mProcess, mModuleBase, (ULONG)index, TI_GET_LENGTH, &size ));
    return size;
  }

  uint GetType(uint index)
  {
    DWORD typeId;
    VerifyWin(SymGetTypeInfo(mProcess, mModuleBase, (ULONG)index, TI_GET_TYPEID, &typeId ));
    return typeId;
  }

  uint GetChildCount(uint index)
  {
    DWORD count;
    VerifyWin(SymGetTypeInfo(mProcess, mModuleBase, (ULONG)index, TI_GET_CHILDRENCOUNT, &count ));
    return count;
  }
};

struct MemberVariable
{
  FixedString<100> Name;
  uint Offset;
  uint String;
  uint Size;
};

struct ClassMap
{
  ClassMap()
    :IsValid(true)
  {
  }

  FixedString<100> Name;
  bool IsValid;
  uint TypeIndex;
  ClassMap* BaseClass;
  Array<MemberVariable> mVariables;
};

//Class used to clean up class maps.
struct ClassStorage
{
  HashMap<String, ClassMap*> Classes;

  ClassStorage()
  {
    AddInvalid("Zero::LinearAxisJoint");
  }

  void AddInvalid(StringParam fullClassName)
  {
    ClassMap* notFound = new ClassMap();
    notFound->IsValid = false;
    Classes.Insert(fullClassName, notFound);
  }

  ~ClassStorage()
  {
    DeleteObjectsInContainer(Classes);
  }
} gClassStorage;


ClassMap* BuildMap(TypeModule& types, uint typeIndex)
{
  String name = types.GetSymbolName(typeIndex);

  ClassMap* classMap = gClassStorage.Classes.FindValue(name, NULL);

  //Locally cache the class map of members to avoid constantly
  //have to class api functions.
  if(classMap)
    return classMap;

  classMap = new ClassMap();

  classMap->BaseClass = NULL;

  classMap->TypeIndex = typeIndex;

  gClassStorage.Classes.Insert(name, classMap);

  classMap->Name = name;

  DWORD childCount = (DWORD)types.GetChildCount(typeIndex);

  DWORD findChildrenSize = sizeof(TI_FINDCHILDREN_PARAMS) + childCount*sizeof(ULONG); 

  TI_FINDCHILDREN_PARAMS* findChildren = (TI_FINDCHILDREN_PARAMS*)alloca(findChildrenSize); 

  memset( findChildren, 0, findChildrenSize ); 

  findChildren->Count = childCount; 

  VerifyWin(SymGetTypeInfo(types.mProcess, types.mModuleBase, (ULONG)typeIndex, TI_FINDCHILDREN, findChildren));

  for(uint i=0;i<childCount;++i)
  {
    uint childTypeIndex = findChildren->ChildId[i];
    DWORD symTag = (DWORD)types.GetTag(childTypeIndex);

    if(symTag == SymTagBaseClass)
    {
      classMap->BaseClass = BuildMap(types, childTypeIndex);
    }
    else if(symTag == SymTagData)
    {
      uint kind = types.GetDataKind(childTypeIndex);
      String symbolName = types.GetSymbolName(childTypeIndex);

      if(kind == DataIsMember)
      {
        uint offset = types.GetOffset(childTypeIndex);

        uint classType = types.GetType(childTypeIndex);

        uint size = types.GetSize(classType);

        MemberVariable& member = classMap->mVariables.PushBack();

        member.Name = symbolName;
        member.Offset = offset;
        member.Size = size;
      }
    }
  }

  return classMap;
}

void Check(TypeModule& typeModule, cstr className, byte* classMemory, uint typeIndex)
{ 

  //Get the class map
  ClassMap* classMap = BuildMap(typeModule, typeIndex);

  //Check all member variables for uninitialized memory
  forRange(MemberVariable& var, classMap->mVariables.All())
  {
    const byte DebugByte = 0xCD;
    bool good = false;
    uint dwords = var.Size;
    for(uint i=0;i<var.Size;++i)
    {
      if(classMemory[var.Offset + i] != DebugByte)
      {
        good = true;
        break;
      }
    }

    if(!good)
    {
      ErrorIf(true, "On Class '%s' member '%s::%s' was not initialized ", className, classMap->Name.c_str(), var.Name.c_str());
    }
  }

  if(classMap->BaseClass)
    Check(typeModule, className, classMemory, classMap->BaseClass->TypeIndex);
}

bool SymbolsLoaded = false;
bool CanScanSymbols = true;

void CheckClassMemory(cstr className, byte* classMemory)
{
  if(!CanScanSymbols)
    return;

  HANDLE process = GetCurrentProcess();

  if(!SymbolsLoaded)
  {
    SymbolsLoaded = true;
    DWORD Options = SymGetOptions(); 

    Options |= SYMOPT_DEBUG & SYMOPT_UNDNAME; 

    SymSetOptions( Options ); 

    UINT moduleBase = 0;

    if( !SymInitialize(process, NULL, TRUE ) )
    {
      return; 
    }
  }

  TypeModule typeModule;

  String fullClassName =  BuildString(Zero::String("Zero::"), Zero::String(className));
  String constructorFunc = BuildString(fullClassName, Zero::String("::"), Zero::String(className));
  uint moduleBase = 0;

  uint totalSize = sizeof(SYMBOL_INFO) + (MAX_SYM_NAME - 1) * sizeof(CHAR);
  SYMBOL_INFO* symbolInfo = (SYMBOL_INFO*)alloca(totalSize);
  ZeroMemory(symbolInfo, totalSize);
  symbolInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
  symbolInfo->MaxNameLen = MAX_SYM_NAME;

  ClassMap* classMap =  gClassStorage.Classes.FindValue(fullClassName, NULL);

  //Check for invalid class to avoid stalls
  if(classMap && !classMap->IsValid)
  {
    SymCleanup(process);
    return;
  }

  //dbghelp used to by only able to look up functions and data so classes can not be looked up (as far as I know)
  //so look up the class by its constructor. Until this is called the class is not around...
  uint success = SymFromName(process, constructorFunc.c_str(), symbolInfo);

  if(!success)
  {
    gClassStorage.AddInvalid(fullClassName);
    ErrorIf(true, "Could not find class %s. This causes a stall on load.", fullClassName.c_str());
    SymCleanup(process);
    return;
  }

  DWORD typeId = 0;
  BOOL test = SymGetTypeInfo(process, moduleBase, symbolInfo->TypeIndex, TI_GET_TYPE, &typeId);

  //It seems that many versions of dbghelp do not work need to investigate why
  if(!test)
  {
    CanScanSymbols = false;
    SymCleanup(process);
    return;
  }

  //With the index of the constructor use the parent id to find is class id;
  DWORD typeIndex = 0; 
  VerifyWin(SymGetTypeInfo(process, moduleBase, symbolInfo->TypeIndex, TI_GET_CLASSPARENTID, &typeIndex));

  String foundClassName = typeModule.GetSymbolName(typeIndex);
  //Now check the classes memory
  Check(typeModule, className, classMemory, typeIndex);
  SymCleanup(process);
}

}
#endif
