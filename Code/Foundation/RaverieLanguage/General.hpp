// MIT Licensed (see LICENSE.md).

#pragma once

namespace Raverie
{
// Defines
#  ifndef RaverieLoop
#    define RaverieLoop for (;;)
#  endif

// Helper macros for stringifiying previous #defines
#  define RaverieStr(Argument) #  Argument
#  define RaverieStringify(Argument) RaverieStr(Argument)

// Don't allow copying of a type
#  define RaverieNoCopy(type)                                                                                            \
  private:                                                                                                             \
    type& operator=(const type&);                                                                                      \
    type(const type&);

// Don't allow copying of a type
#  define RaverieNoDefaultConstructor(type)                                                                              \
  private:                                                                                                             \
    type();

// Don't allow destruction of a type
#  define RaverieNoDestructor(type)                                                                                      \
  private:                                                                                                             \
    ~type();

// Don't allow instantiations of this type
#  define RaverieNoInstantiations(type)                                                                                  \
  private:                                                                                                             \
    type();                                                                                                            \
    ~type();                                                                                                           \
    type& operator=(const type&);                                                                                      \
    type(const type&);

// Macro for figuring out the size of a fixed C-array
#  define RaverieCArrayCount(x) ((sizeof(x) / sizeof(0 [x])) / ((size_t)(!(sizeof(x) % sizeof(0 [x])))))

// A class we use when we're either debugging or refactoring
// This class attempts to act as a placeholder for any other class
template <typename ToEmulate>
class DebugPlaceholder
{
public:
  DebugPlaceholder()
  {
  }

  template <typename T>
  DebugPlaceholder(const T&)
  {
  }

  operator ToEmulate()
  {
    return ToEmulate();
  }

  template <typename T>
  T& operator=(T& value)
  {
    return value;
  }

  template <typename T>
  const T& operator=(const T& value)
  {
    return value;
  }

  template <typename T>
  bool operator==(const T&) const
  {
    return false;
  }

  template <typename T>
  bool operator!=(const T&) const
  {
    return false;
  }
};
} // namespace Raverie

