///////////////////////////////////////////////////////////////////////////////
/// Dll interface for the memory debugger.
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#ifdef ExterningMemoryDebugger
#define DllDeclaration extern "C" __declspec(dllexport)
#else
#define DllDeclaration extern "C" __declspec(dllimport)
#endif

enum AllocationType
{
  AllocationType_Direct,//ZAllocate called directly, not from new
  AllocationType_Single,//operator new (not new[])
  AllocationType_Array,//operator new[]
};

DllDeclaration void InitializeMemory();
DllDeclaration void InitializeMemoryWithBlockCount(int blockCount);
DllDeclaration void ActivateDebugger();
DllDeclaration void DeactivateDebugger();
DllDeclaration int IsDebuggerActive();

DllDeclaration void* AllocateMemory(size_t size, int allocationType, int framesToSkip);
DllDeclaration int DeallocateMemory(void* memory, int allocationType);
DllDeclaration void BuildVerySleepyStats(const char* projectName);
DllDeclaration void BuildVerySleepyStats_ActiveAllocations(const char* projectName);


//For testing framework!
DllDeclaration size_t GetNumberOfBytesActive();
DllDeclaration void ClearLeaks();

