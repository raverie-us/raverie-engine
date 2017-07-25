///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#ifdef ExterningMemoryTracker
#define DllDeclaration extern "C" __declspec(dllexport)
#else
#define DllDeclaration extern "C" __declspec(dllimport)
#endif

DllDeclaration void* AllocateMemory(size_t size, int framesToSkip);
DllDeclaration void DeallocateMemory(void* memory);

enum OutputMode{VerySleepy_0_90};

DllDeclaration void OutputActiveAllocations(const char* outputPath, int outputMode);