/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_RANGE_HPP
#define ZILCH_RANGE_HPP

namespace Zilch
{
  template <typename T>
  class ZeroSharedTemplate Range
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    Range() :
      Index(0)
    {
    }

    Range* GetAll()
    {
      return this;
    }

    void MoveNext()
    {
      ++this->Index;
    }

    T& GetCurrent()
    {
      return this->Elements[this->Index];
    }

    bool IsEmpty()
    {
      return this->Index >= this->Elements.Size();
    }

    bool IsNotEmpty()
    {
      return !this->IsEmpty();
    }

    void Reset()
    {
      this->Index = 0;
    }

    void Clear()
    {
      this->Index = 0;
      this->Elements.Clear();
    }

    void Add(const T& element)
    {
      this->Elements.PushBack(element);
    }

    void Add(const T* element)
    {
      if (element == nullptr)
        return;

      this->Elements.PushBack(*element);
    }

    // This is only avalable to emulate usage of the Array container
    void PushBack(const T& element)
    {
      this->Elements.PushBack(element);
    }

  public:
    // The elements within the array we hold on to
    Array<T> Elements;

    // The index we use with the range
    size_t Index;
  };
}

#endif
