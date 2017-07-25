///////////////////////////////////////////////////////////////////////////////
///
/// \file Array.hpp
/// Declaration of the Binder for STL vector container.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Containers/ContainerCommon.hpp"
#include <vector>
#include "Memory/ZeroAllocator.hpp"

namespace Zero
{

template<typename type>
class StdVector : public std::vector<type, Zallocator<type> >
{
public:
  typedef StdVector <type> this_type;
  typedef IteratorRange<this_type> range;

  typedef type value_type;
  typedef type* pointer;
  typedef const type* const_pointer;
  typedef type& reference;
  typedef const type& const_reference;
  typedef ptrdiff_t difference_type;

  range All()
  {
    return range(Begin(), End());
  }

  pointer Data()
  {
    return &(*this)[0];
  }

};

}//namespace Zero
