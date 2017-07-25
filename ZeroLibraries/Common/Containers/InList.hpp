///////////////////////////////////////////////////////////////////////////////
///
/// \file InList.hpp
/// Definition of the Intrusively linked list container.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Diagnostic/Diagnostic.hpp"
#include "ContainerCommon.hpp"

namespace Zero
{

const int ObjListPtrDebugValue = 0xFFFFDEAD;

#ifndef DEBUGLINKS
#if ZeroDebug
#define DEBUGLINKS 1
#else
#define DEBUGLINKS 0
#endif
#endif

template<typename type>
struct ZeroSharedTemplate Link
{
#if DEBUGLINKS
  Link() : Next((type*)ObjListPtrDebugValue) , Prev((type*)ObjListPtrDebugValue) {};
#endif
  type* Next;
  type* Prev;
};

#if defined(_MSC_VER) && _MSC_VER <= 1600
///Intrusive is used for debugging in Visual Studio
#define IntrusiveLink(objectType, linkName) \
union{ \
  struct{Link<objectType> linkName;}; \
  struct{objectType* Next; objectType* Prev;}; \
}
#else
#define IntrusiveLink(objectType, linkName) \
  Link<objectType> linkName
#endif

class ZeroShared LinkBase
{
public:
  IntrusiveLink(LinkBase, link);
};

template<typename Parent, typename Member>
inline ptrdiff_t PointerToMemberOffset(const Member Parent::* ptrToMember)
{
  return *(unsigned int*)(void*)&ptrToMember;
}

typedef const char * const cstrc;

cstrc cBadRemoveError = "Prev object next pointer does not match current object."
          "Most likely the prev object has been deleted or improperly removed.";
cstrc cBadLinkInsertError = "Link value is not set to debug value. "
"Probably a double Insert or not yet removed from another list.";

cstrc cBadRemovedAlready = "Object has already been erased or was never added.";

///Intrusively linked list container. 
///Does not own the objects in Contains (they are implicitly pointers)
///Objects can link and unlink without using the container.
template<typename type, typename refType, Link<type> type::* PtrToMember = &type::link>
class ZeroSharedTemplate BaseInList
{
public:
  //Should InList be value type or pointer to value typed?
  //pointer to value type make the interface the same as vector<type*>
  //value_typed make the iterator act just like a pointer which is useful.
  typedef type value_type;
  typedef type* pointer;

  typedef type& reference;
  typedef const type* const_pointer;
  typedef const type& const_reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type; 
  typedef BaseInList<type, refType, PtrToMember> this_type;

  typedef refType& sub_reference;

  static const size_t cInvalidIndex = size_t(-1);

  BaseInList()
  {
    SetEmpty();
  }

  ~BaseInList()
  {
    //ObjList does not own the object it Contains so this is a no op
  }

  class range
  {
  public:
    typedef refType value_type;
    typedef sub_reference reference;
    typedef sub_reference FrontResult;

    ///Default range is an empty range. Can only be checked for empty.
    range()
      : begin(nullptr) , end(nullptr)
    {
    }

    range(pointer pbegin, pointer pend)
      : begin(pbegin) , end(pend)
    {
    }

    sub_reference Front(){return *static_cast<refType*>(begin);}
    sub_reference Back(){return *static_cast<refType*>(Prev(end));}

    void PopFront()
    {
      ErrorIf(Empty(),"Popped empty range.");
      begin = Next(begin);
    }

    void PopBack()
    {
      ErrorIf(Empty(),"Popped empty range.");
      end = Prev(end);
    }

    bool Empty(){return begin==end;}

    void SpliceFront(pointer dest)
    {
      begin = Splice(dest, begin);
    }

    range& All() { return *this; }
    const range& All() const { return *this; }

    pointer begin;
    pointer end;
  };

  // The exact same interface as range, except when you PopFront and popBack are swapped, as well as front and back
  // This means that 'begin' and 'end' are actually the exact same as a normal range's begin/end, but we iterate the end here
  class reverse_range
  {
  public:
    typedef refType value_type;

    ///Default range is an empty range. Can only be checked for empty.
    reverse_range()
      : begin(nullptr) , end(nullptr)
    {
    }

    reverse_range(pointer pbegin, pointer pend)
      : begin(pbegin) , end(pend)
    {
    }

    sub_reference Front(){return *static_cast<refType*>(Prev(end));}
    sub_reference Back(){return *static_cast<refType*>(begin);}

    void PopFront()
    {
      ErrorIf(Empty(),"Popped empty range.");
      end = Prev(end);
    }

    void PopBack()
    {
      ErrorIf(Empty(),"Popped empty range.");
      begin = Next(begin);
    }

    bool Empty(){return begin==end;}

    reverse_range& All() { return *this; }
    const reverse_range& All() const { return *this; }

    pointer begin;
    pointer end;
  };

  range All()
  {
    return range(Next(GetHeader()), GetHeader());
  }

  reverse_range ReverseAll()
  {
    return reverse_range(Next(GetHeader()), GetHeader());
  }

  class iterator
  {
  public:
    friend class BaseInList;
    iterator(){};
    iterator(pointer ptr) : mPtr(ptr) {}
    void operator--(){mPtr = Prev(mPtr);}
    void operator++(){mPtr = Next(mPtr);}
    sub_reference Front(){return *static_cast<refType*>(mPtr);}
    refType* operator->(){return static_cast<refType*>(mPtr);}
    refType* operator*(){return static_cast<refType*>(mPtr);}
    bool operator==(const iterator& it){return mPtr == it.mPtr;}
    bool operator!=(const iterator& it){return mPtr != it.mPtr;}
    operator bool(){return mPtr != nullptr;}
    operator pointer(){return mPtr;}
  private:
    pointer mPtr;
  };

  iterator Begin(){return iterator(Next(GetHeader()));};
  iterator End(){return iterator(GetHeader());}
  iterator ReverseBegin(){return iterator(Prev(GetHeader()));};
  //End is the same as forward because of sentinel node)
  iterator ReverseEnd(){return iterator(GetHeader());}

  ///True if the list does not contain any nodes.
  bool Empty()
  {
    return mHeader.Next == GetHeader();
  }

  ///This function only works in debug mode (because in release
  ///clear doesn't set the next/prev pointers for each node).
  static inline bool VerifyUnlinkedDebugOnly(pointer element)
  {
#if DEBUGLINKS
    pointer prev = Prev(element);
    pointer next = Next(element);
    if(prev == (type*)ObjListPtrDebugValue && next == (pointer)ObjListPtrDebugValue)
      return true;
    else
      return false;
#else
    return true;
#endif
  }

  static inline iterator Unlink(pointer element)
  {
    pointer prev = Prev(element);
    pointer next = Next(element);

#if DEBUGLINKS
    ErrorIf(prev == (pointer)ObjListPtrDebugValue, cBadRemovedAlready);
    ErrorIf(next == (pointer)ObjListPtrDebugValue, cBadRemovedAlready);
#endif

    ErrorIf(Next(prev) != element, cBadLinkInsertError);
    ErrorIf(Prev(next) != element, cBadLinkInsertError);

    Next(prev) = next;
    Prev(next) = prev;

#if DEBUGLINKS
    Prev(element) = (pointer)ObjListPtrDebugValue;
    Next(element) = (pointer)ObjListPtrDebugValue;
#endif
    return next;
  }

  ///Inserts object in list BEFORE where.
  void InsertBefore(pointer where, pointer obj)
  {
    Insert(where, obj);
  }

  ///Inserts object in list BEFORE where.
  void Insert(pointer where, pointer obj)
  {

#if DEBUGLINKS
    ErrorIf(Next(obj) != (type*)ObjListPtrDebugValue, cBadLinkInsertError);
    ErrorIf(Prev(obj) != (type*)ObjListPtrDebugValue, cBadLinkInsertError);
#endif

    Next(obj) = where;
    Prev(obj) = Prev(where);
    Next(Prev(where)) = obj;
    Prev(where) = obj;
  }

  ///Inserts object in list after where.
  void InsertAfter(pointer where, pointer obj)
  {
#if DEBUGLINKS
    ErrorIf(Next(obj) != (type*)ObjListPtrDebugValue, cBadLinkInsertError);
    ErrorIf(Prev(obj) != (type*)ObjListPtrDebugValue, cBadLinkInsertError);
#endif

    Next(obj) = Next(where);
    Prev(obj) = where;
    Prev(Next(where)) = obj;
    Next(where) = obj;
  }

  /// Inserts before the given index
  void InsertAt(size_t index, pointer obj)
  {
    size_t currIndex = 0;
    iterator it = Begin();

    do 
    {
      if(currIndex == index)
      {
        InsertBefore(it, obj);
        return;
      }
      
      ++currIndex;
      ++it;
    } while (it != End());

    PushBack(obj);
  }

  ///Adds the object into the lists sorted position
  template<typename Comparer>
  void SortedInsert(pointer obj, Comparer comparer)
  {
    iterator begin = Begin();
    SortedInsertInternal(begin, obj, comparer);
  }

  ///Adds the object to the end of the list.
  void PushBack(pointer obj)
  {
    pointer header = GetHeader();
    Insert(header, obj);
  };

  void PopBack()
  {
    Unlink(&Back());
  };

  ///Adds the object to the beginning of the list
  void PushFront(pointer obj)
  {
    InsertAfter(GetHeader(), obj);
  };

  void PopFront()
  {
    Unlink(&Front());
  };

  ///Removes all the objects from the list. It will
  ///unlink the individual objects internal pointers
  ///only in debug mode.
  void Clear()
  {
#if DEBUGLINKS
    iterator it = Begin();
    while(it != End())
    {
      it = Erase(it);
    }
#endif
    SetEmpty();
  };

  // O(N) walk to find the given elements index
  size_t FindIndex(pointer element)
  {
    size_t index = 0;
    iterator it = Begin();
    do 
    {
      if((pointer)it == element)
        return index;
      ++index;
      ++it;
    } while (it != End());

    return cInvalidIndex;
  }

  sub_reference Front()
  {
    return *static_cast<refType*>(Next(GetHeader()));
  }

  sub_reference Back()
  {
    return *static_cast<refType*>(Prev(GetHeader()));
  }

  //Swaps one list elements with another this also must
  //fix the prev and next pointers of the first and
  //last objects located in the list
  void Swap(this_type& other)
  {
    Zero::Swap(mHeader.Next, other.mHeader.Next);
    Zero::Swap(mHeader.Prev, other.mHeader.Prev);

    //If the list contained pointers fix them
    if(mHeader.Next != other.GetHeader())
    {
      Prev(mHeader.Next) = GetHeader();
      Next(mHeader.Prev) = GetHeader();
    }
    else
    {
      SetEmpty();
    }

    if(other.mHeader.Next != GetHeader())
    {
      Prev(other.mHeader.Next) = other.GetHeader();
      Next(other.mHeader.Prev) = other.GetHeader();
    }
    else
    {
      other.SetEmpty();
    }
  }

  ///Unlinks the object from the list. Does not destroy it.
  //returns the next item in the list.
  iterator Erase(iterator it)
  {
    return Unlink(it);
  }

  iterator EraseAndGetPrev(iterator it)
  {
    iterator prev = Prev(it);
    Erase(it);
    return prev;
  }

  //Function for iterating over the list when performing destructive
  //operations (operations that would break the next pointer) such
  //as erase and delete
  template<typename unaryFunction>
  void SafeForEach(iterator begin, iterator end, unaryFunction op)
  {
    while(begin != end)
    {
      iterator next = Next(begin);
      op(begin);
      begin = next;
    }
  }

  void Erase(range eraseRange)
  {
    iterator begin = eraseRange.begin;
    iterator end = eraseRange.end;
    while(begin != end)
      begin = Erase(begin);
  }

  ///Splices the range with the provided 
  ///into the list BEFORE where.
  static void Splice(iterator where, range right)
  {
    ErrorIf(right.Empty(), "Cannot splice and empty range.");

    Next(Prev(right.begin)) = right.end;
    Next(Prev(right.end)) = where;
    Next(Prev(where)) = right.begin;
    pointer prevNode = Prev(where);
    Prev(where) = Prev(right.end);
    Prev(right.end) = Prev(right.begin);
    Prev(right.begin) = prevNode;
  }

  ///Splices a single object from 'from'
  static iterator Splice(iterator where, iterator from)
  {
    pointer next = Next(from);
    Splice(where, range(from, next));
    return next;
  }

  //Splices entire list leaving right empty
  void Splice(iterator where, this_type& right)
  {
    Splice(where, right.All());
  }

  template<typename Comparer>
  void Merge(this_type& right, Comparer comparer)
  {
    range leftR = All();
    range rightR = right.All();

    while(!leftR.Empty() && !rightR.Empty())
    {
      if(comparer(rightR.Front(), leftR.Front()))
      {
        //right is 'greater' than left
        //move a right node into list
        rightR.SpliceFront(leftR.begin);
      }
      else
      {
        leftR.PopFront();
      }
    }

    if(!rightR.Empty())
      Splice(leftR.begin, rightR);
  }


  template<typename Comparer>
  void Sort(Comparer comparer)
  {
    //If there is more than two nodes sort the list
    if(mHeader.Next != mHeader.Prev)
    {
      const size_t cMaxBins = 25;
      this_type temp;
      this_type bins[cMaxBins+1];
      size_t maxBin = 0;

      while(!Empty())
      {
        //Move a single elements onto the temp list
        temp.Splice(temp.Begin() , range(Begin(), Next(Begin())));

        size_t curBin = 0;
        for(; curBin < maxBin && !bins[curBin].Empty();++curBin)
        {
          bins[curBin].Merge(temp, comparer);
          bins[curBin].Swap(temp);
        }

        if(curBin == cMaxBins)
          bins[curBin-1].Merge(temp, comparer);
        else
        {
          bins[curBin].Swap(temp);
          if(curBin == maxBin)
            ++maxBin;
        }
      }

      //All elements are now in bins merge all bins together
      for(size_t mergeBin = 1; mergeBin < maxBin; ++mergeBin)
      {
        //Merge previous into current bin
        bins[mergeBin].Merge(bins[mergeBin-1], comparer);
      }

      //move back to list
      Splice(Begin(), bins[maxBin-1]);

    }

  }

  template<typename Comparer>
  void SortedInsertInternal(iterator where, pointer valueInInsert, Comparer comparer)
  {
    while(where != End())
    {
      // dereferenced where will never be null as the empty case where == end 
      // and then when at the end it will never enter the while loop
      if(comparer(*valueInInsert, *(*where)))
        break;
      ++where;
    }
    Insert(where, valueInInsert);
  }

  void SetEmpty()
  {
    mHeader.Next = GetHeader();
    mHeader.Prev = GetHeader();
  }

  inline static pointer& Next(pointer obj){ return ToLink(obj).Next; }
  inline static pointer& Prev(pointer obj){ return ToLink(obj).Prev; }
  
  iterator NextWrap(iterator iter)
  { 
    // Increment the iterator
    iterator i = iter;
    ++i;

    // If it's the end, increment passed the sentinel
    if(i == End())
      ++i;

    return i;
  }

  iterator PrevWrap(iterator iter)
  { 
    // Decrement the iterator
    iterator i = iter;
    --i;

    // If it's the end, decrement passed the sentinel
    if(i == End())
      --i;

    return i;
  }

protected:

  //InList is not copyable (not really a valid operation)
  BaseInList(const BaseInList&){}
  void operator=(const BaseInList&){}

  inline static pointer ToNode(Link<type>& link)
  {return (pointer)((unsigned char*)&link - PointerToMemberOffset(PtrToMember));}
  inline static Link<type>& ToLink(pointer obj)
  {return obj->*PtrToMember; }

  pointer GetHeader(){return ToNode(mHeader);}
  Link<type> mHeader;

};

template<typename type, Link<type> type::* PtrToMember = &type::link>
class ZeroSharedTemplate InList : public BaseInList<type, type, PtrToMember>
{
public:

  InList()
  {

  }

private:

  //InList is not copyable (not really a valid operation)
  InList(const InList&){}
  void operator=(const InList&){}
};

template<typename type, typename baseLinkType = LinkBase>
class ZeroSharedTemplate InListBaseLink : public BaseInList<baseLinkType, type, &baseLinkType::link>
{
public:
  InListBaseLink()
  {

  }

private:

  //InList is not copyable (not really a valid operation)
  InListBaseLink(const InListBaseLink&){}
  void operator=(const InListBaseLink&){}
};

//------------------------------------------------------------------------------
template<typename baseLinkType, typename type>
void EraseAndDeleteBase(baseLinkType* element)
{
  BaseInList<baseLinkType, type>::Unlink(element);
  delete static_cast<type*>(element);
}

template<typename baseLinkType, typename type>
void DeleteObjectsIn(BaseInList<baseLinkType, type>& container)
{
  container.SafeForEach(container.Begin( ), container.End( ), EraseAndDeleteBase<baseLinkType, type>);
}

template<typename baseLinkType, typename type>
void DeleteObjectsInRange(BaseInList<baseLinkType, type>& container,
  typename BaseInList<baseLinkType, type>::iterator& begin,
  typename BaseInList<baseLinkType, type>::iterator& end)
{
  container.SafeForEach(begin, end, EraseAndDeleteBase<baseLinkType, type>);
}

//------------------------------------------------------------------------------
template<typename type, Link<type> type::* PtrToMember>
void EraseAndDelete(type* element)
{
  InList<type, PtrToMember>::Unlink(element);
  delete element;
}

template<typename type, Link<type> type::* PtrToMember>
void DeleteObjectsIn(InList<type, PtrToMember>& container)
{
  container.SafeForEach(container.Begin(), container.End(), EraseAndDelete<type,PtrToMember>);
}

//------------------------------------------------------------------------------
template<typename type, typename baseLinktype = LinkBase>
void EraseAndDeleteBaseLink(baseLinktype* element)
{
  InListBaseLink<type, baseLinktype>::Unlink(element);
  delete static_cast<type*>(element);
}

template<typename type, typename baseLinkType = LinkBase>
void DeleteObjectsIn(InListBaseLink<type, baseLinkType>& container)
{
  container.SafeForEach(container.Begin( ), container.End( ), EraseAndDeleteBaseLink<type, baseLinkType>);
}

//------------------------------------------------------------------------------
template<typename type, Link<type> type::* PtrToMember>
void OnlyDeleteObjectIn(InList<type, PtrToMember>& container)
{
  container.SafeForEach(container.Begin( ), container.End( ), DeleteOp<type>);
}

}//namespace Zero
