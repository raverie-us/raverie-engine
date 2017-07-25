///////////////////////////////////////////////////////////////////////////////
///
/// \file Graph.hpp
/// Declaration of the Memory Graph.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Memory.hpp"
#include "Containers/InList.hpp"
#include "String/FixedString.hpp"

namespace Zero
{

namespace Memory
{

struct ZeroShared Stats
{
  enum StatFlags
  {
    ShowActive     = 1,
    ShowDedicated  = 2,
    ShowPeak       = 4,
    ShowBytes      = 8,
    ShowTotal      = 16,
    ShowLocal      = 32,
    ShowCount      = 64
  };

  MemCounterType Allocations;
  MemCounterType Active;
  MemCounterType BytesAllocated;
  MemCounterType BytesDedicated;
  MemCounterType PeakAllocated;

  Stats();

  template<typename Vistor>
  void Visit(Vistor& vistor, size_t flags);
  void Accumulate(const Stats& right);
};

///Base Memory graph node. All allocators are derived from this class for
///runtime memory statics collection and debugging. Class provides a graph
///structure for hierarchical grouping of memory and the ability to name allocators.
class ZeroShared Graph : public LinkBase
{
public:
  UseStaticMemory();

  cstr GetName(){return Name.c_str();}
  FixedString<32> Name;
  Graph* mParent;
  Stats mData;

  Graph(cstr name, Graph* parent);

  void DeltaDedicated(MemCounterType bytes)
  {
    mData.BytesDedicated+=bytes;
  }

  void AddAllocation(MemCounterType bytes)
  {
    ++mData.Active;
    ++mData.Allocations;
    mData.BytesAllocated+=bytes;
    if(mData.BytesAllocated > mData.PeakAllocated)
      mData.PeakAllocated = mData.BytesAllocated;
  }

  void RemoveAllocation(MemCounterType bytes)
  {
    --mData.Active;
    mData.BytesAllocated-=bytes;
  }

  typedef InListBaseLink<Graph>::range RangeType;
  RangeType GetChildren(){return Children.All();}

  InListBaseLink<Graph> Children;
  void PrintHelper(size_t tabs, size_t flags, cstr name);
  void PrintHeader(size_t flags);
  void Compute(Stats& data);
  void PrintGraph(size_t flags);
  void Print(size_t tabs, size_t flags);

  virtual void CleanUp();
  virtual ~Graph();
private:
  //Can not copy memory managers.
  Graph(const Graph&);
  void operator=(const Graph&);
};


class Heap;
class Root: public Graph
{
public:
  Root(cstr name, Graph* parent)
    :Graph(name, parent)
  {
  }

  static Root* RootGraph;
  static Heap* GloblHeap;
  static Heap* StaticHeap;

  static void Initialize();
  static void Shutdown();
  static void PrintAll();
};

ZeroShared Heap* GetGlobalHeap();
Heap* GetNamedHeap(cstr name);
Root* GetRoot();
Heap* GetStaticHeap();
void Shutdown();
void DumpMemoryDebuggerStats(cstr projectName);

class ZeroShared StandardMemory
{
public:
  static inline void MemCopy(void* dest, void* source, size_t numberOfBytes){memcpy(dest, source, numberOfBytes);}
  static inline void MemMove(void* dest, void* source, size_t numberOfBytes){memmove(dest, source, numberOfBytes);}
};

}//namespace Memory
}//namespace Zero
