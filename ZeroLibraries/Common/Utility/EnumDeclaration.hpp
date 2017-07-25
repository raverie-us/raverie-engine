///////////////////////////////////////////////////////////////////////////////
///
/// \file EnumDeclaration.hpp
/// Declaration of enum declaration macros.
///
/// Authors: Joshua Claeys, Joshua Davis, Auto-Generated
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

//Used to determine whether or not the enum is going to be used as a bit field
//or an index.  As a bit field, each index needs to be shifted.
#define _BitField() 1 << 
#define _Indexed() 
#define _AddNone(name) namespace name { enum {None}; }  

#define _ExpandNames1(name,mode,value1)                                       \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0};                                          \
  enum {Size = 1};                                                            \
  static const cstr Names[] = {#value1, NULL};                                \
  static const uint Values[] = {value1};                                      \
  }                                                                           \

#define _ExpandNames2(name,mode,value1,value2)                                \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0, value2 = mode 1};                         \
  enum {Size = 2};                                                            \
  static const cstr Names[] = {#value1, #value2, NULL};                       \
  static const uint Values[] = {value1, value2};                              \
  }                                                                           \

#define _ExpandNames3(name,mode,value1,value2,value3)                         \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0, value2 = mode 1, value3 = mode 2};        \
  enum {Size = 3};                                                            \
  static const cstr Names[] = {#value1, #value2, #value3, NULL};              \
  static const uint Values[] = {value1, value2, value3};                      \
  }                                                                           \

#define _ExpandNames4(name,mode,value1,value2,value3,value4)                  \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0, value2 = mode 1, value3 = mode 2,         \
                   value4 = mode 3};                                          \
  enum {Size = 4};                                                            \
  static const cstr Names[] = {#value1, #value2, #value3, #value4, NULL};     \
  static const uint Values[] = {value1, value2, value3, value4};              \
  }                                                                           \

#define _ExpandNames5(name,mode,value1,value2,value3,value4,value5)           \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0, value2 = mode 1, value3 = mode 2,         \
                   value4 = mode 3, value5 = mode 4};                         \
  enum {Size = 5};                                                            \
  static const cstr Names[] = {#value1, #value2, #value3, #value4, #value5,   \
                               NULL};                                         \
  static const uint Values[] = {value1, value2, value3, value4, value5};      \
  }                                                                           \

#define _ExpandNames6(name,mode,value1,value2,value3,value4,value5,value6)    \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0, value2 = mode 1, value3 = mode 2,         \
                   value4 = mode 3, value5 = mode 4, value6 = mode 5};        \
  enum {Size = 6};                                                            \
  static const cstr Names[] = {#value1, #value2, #value3, #value4, #value5,   \
                               #value6, NULL};                                \
  static const uint Values[] = {value1, value2, value3, value4, value5, value6\
                                };                                            \
  }                                                                           \

#define _ExpandNames7(name,mode,value1,value2,value3,value4,value5,value6,    \
                      value7)                                                 \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0, value2 = mode 1, value3 = mode 2,         \
                   value4 = mode 3, value5 = mode 4, value6 = mode 5,         \
                   value7 = mode 6};                                          \
  enum {Size = 7};                                                            \
  static const cstr Names[] = {#value1, #value2, #value3, #value4, #value5,   \
                               #value6, #value7, NULL};                       \
  static const uint Values[] = {value1, value2, value3, value4, value5,       \
                                value6, value7};                              \
  }                                                                           \

#define _ExpandNames8(name,mode,value1,value2,value3,value4,value5,value6,    \
                      value7,value8)                                          \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0, value2 = mode 1, value3 = mode 2,         \
                   value4 = mode 3, value5 = mode 4, value6 = mode 5,         \
                   value7 = mode 6, value8 = mode 7};                         \
  enum {Size = 8};                                                            \
  static const cstr Names[] = {#value1, #value2, #value3, #value4, #value5,   \
                               #value6, #value7, #value8, NULL};              \
  static const uint Values[] = {value1, value2, value3, value4, value5,       \
                                value6, value7, value8};                      \
  }                                                                           \

#define _ExpandNames9(name,mode,value1,value2,value3,value4,value5,value6,    \
                      value7,value8,value9)                                   \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0, value2 = mode 1, value3 = mode 2,         \
                   value4 = mode 3, value5 = mode 4, value6 = mode 5,         \
                   value7 = mode 6, value8 = mode 7, value9 = mode 8};        \
  enum {Size = 9};                                                            \
  static const cstr Names[] = {#value1, #value2, #value3, #value4, #value5,   \
                               #value6, #value7, #value8, #value9, NULL};     \
  static const uint Values[] = {value1, value2, value3, value4, value5,       \
                                value6, value7, value8, value9};              \
  }                                                                           \

#define _ExpandNames10(name,mode,value1,value2,value3,value4,value5,value6,   \
                       value7,value8,value9,value10)                          \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0, value2 = mode 1, value3 = mode 2,         \
                   value4 = mode 3, value5 = mode 4, value6 = mode 5,         \
                   value7 = mode 6, value8 = mode 7, value9 = mode 8,         \
                   value10 = mode 9};                                         \
  enum {Size = 10};                                                           \
  static const cstr Names[] = {#value1, #value2, #value3, #value4, #value5,   \
                               #value6, #value7, #value8, #value9, #value10,  \
                               NULL};                                         \
  static const uint Values[] = {value1, value2, value3, value4, value5,       \
                                value6, value7, value8, value9, value10};     \
  }                                                                           \

#define _ExpandNames11(name,mode,value1,value2,value3,value4,value5,value6,   \
                       value7,value8,value9,value10,value11)                  \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0, value2 = mode 1, value3 = mode 2,         \
                   value4 = mode 3, value5 = mode 4, value6 = mode 5,         \
                   value7 = mode 6, value8 = mode 7, value9 = mode 8,         \
                   value10 = mode 9, value11 = mode 10};                      \
  enum {Size = 11};                                                           \
  static const cstr Names[] = {#value1, #value2, #value3, #value4, #value5,   \
                               #value6, #value7, #value8, #value9, #value10,  \
                               #value11, NULL};                               \
  static const uint Values[] = {value1, value2, value3, value4, value5,       \
                                value6, value7, value8, value9, value10,      \
                                value11};                                     \
  }                                                                           \

#define _ExpandNames12(name,mode,value1,value2,value3,value4,value5,value6,   \
                       value7,value8,value9,value10,value11,value12)          \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0, value2 = mode 1, value3 = mode 2,         \
                   value4 = mode 3, value5 = mode 4, value6 = mode 5,         \
                   value7 = mode 6, value8 = mode 7, value9 = mode 8,         \
                   value10 = mode 9, value11 = mode 10, value12 = mode 11};   \
  enum {Size = 12};                                                           \
  static const cstr Names[] = {#value1, #value2, #value3, #value4, #value5,   \
                               #value6, #value7, #value8, #value9, #value10,  \
                               #value11, #value12, NULL};                     \
  static const uint Values[] = {value1, value2, value3, value4, value5,       \
                                value6, value7, value8, value9, value10,      \
                                value11, value12};                            \
  }                                                                           \

#define _ExpandNames13(name,mode,value1,value2,value3,value4,value5,value6,   \
                       value7,value8,value9,value10,value11,value12,value13)  \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0, value2 = mode 1, value3 = mode 2,         \
                   value4 = mode 3, value5 = mode 4, value6 = mode 5,         \
                   value7 = mode 6, value8 = mode 7, value9 = mode 8,         \
                   value10 = mode 9, value11 = mode 10, value12 = mode 11,    \
                   value13 = mode 12};                                        \
  enum {Size = 13};                                                           \
  static const cstr Names[] = {#value1, #value2, #value3, #value4, #value5,   \
                               #value6, #value7, #value8, #value9, #value10,  \
                               #value11, #value12, #value13, NULL};           \
  static const uint Values[] = {value1, value2, value3, value4, value5,       \
                                value6, value7, value8, value9, value10,      \
                                value11, value12, value13};                   \
  }                                                                           \

#define _ExpandNames14(name,mode,value1,value2,value3,value4,value5,value6,   \
                       value7,value8,value9,value10,value11,value12,value13,  \
                       value14)                                               \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0, value2 = mode 1, value3 = mode 2,         \
                   value4 = mode 3, value5 = mode 4, value6 = mode 5,         \
                   value7 = mode 6, value8 = mode 7, value9 = mode 8,         \
                   value10 = mode 9, value11 = mode 10, value12 = mode 11,    \
                   value13 = mode 12, value14 = mode 13};                     \
  enum {Size = 14};                                                           \
  static const cstr Names[] = {#value1, #value2, #value3, #value4, #value5,   \
                               #value6, #value7, #value8, #value9, #value10,  \
                               #value11, #value12, #value13, #value14, NULL}; \
  static const uint Values[] = {value1, value2, value3, value4, value5,       \
                                value6, value7, value8, value9, value10,      \
                                value11, value12, value13, value14};          \
  }                                                                           \

#define _ExpandNames15(name,mode,value1,value2,value3,value4,value5,value6,   \
                       value7,value8,value9,value10,value11,value12,value13,  \
                       value14,value15)                                       \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0, value2 = mode 1, value3 = mode 2,         \
                   value4 = mode 3, value5 = mode 4, value6 = mode 5,         \
                   value7 = mode 6, value8 = mode 7, value9 = mode 8,         \
                   value10 = mode 9, value11 = mode 10, value12 = mode 11,    \
                   value13 = mode 12, value14 = mode 13, value15 = mode 14};  \
  enum {Size = 15};                                                           \
  static const cstr Names[] = {#value1, #value2, #value3, #value4, #value5,   \
                               #value6, #value7, #value8, #value9, #value10,  \
                               #value11, #value12, #value13, #value14,        \
                               #value15, NULL};                               \
  static const uint Values[] = {value1, value2, value3, value4, value5,       \
                                value6, value7, value8, value9, value10,      \
                                value11, value12, value13, value14, value15}; \
  }                                                                           \

#define _ExpandNames16(name,mode,value1,value2,value3,value4,value5,value6,   \
                       value7,value8,value9,value10,value11,value12,value13,  \
                       value14,value15,value16)                               \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0, value2 = mode 1, value3 = mode 2,         \
                   value4 = mode 3, value5 = mode 4, value6 = mode 5,         \
                   value7 = mode 6, value8 = mode 7, value9 = mode 8,         \
                   value10 = mode 9, value11 = mode 10, value12 = mode 11,    \
                   value13 = mode 12, value14 = mode 13, value15 = mode 14,   \
                   value16 = mode 15};                                        \
  enum {Size = 16};                                                           \
  static const cstr Names[] = {#value1, #value2, #value3, #value4, #value5,   \
                               #value6, #value7, #value8, #value9, #value10,  \
                               #value11, #value12, #value13, #value14,        \
                               #value15, #value16, NULL};                     \
  static const uint Values[] = {value1, value2, value3, value4, value5,       \
                                value6, value7, value8, value9, value10,      \
                                value11, value12, value13, value14, value15,  \
                                value16};                                     \
  }                                                                           \

#define _ExpandNames17(name,mode,value1,value2,value3,value4,value5,value6,   \
                       value7,value8,value9,value10,value11,value12,value13,  \
                       value14,value15,value16,value17)                       \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0, value2 = mode 1, value3 = mode 2,         \
                   value4 = mode 3, value5 = mode 4, value6 = mode 5,         \
                   value7 = mode 6, value8 = mode 7, value9 = mode 8,         \
                   value10 = mode 9, value11 = mode 10, value12 = mode 11,    \
                   value13 = mode 12, value14 = mode 13, value15 = mode 14,   \
                   value16 = mode 15, value17 = mode 16};                     \
  enum {Size = 17};                                                           \
  static const cstr Names[] = {#value1, #value2, #value3, #value4, #value5,   \
                               #value6, #value7, #value8, #value9, #value10,  \
                               #value11, #value12, #value13, #value14,        \
                               #value15, #value16, #value17, NULL};           \
  static const uint Values[] = {value1, value2, value3, value4, value5,       \
                                value6, value7, value8, value9, value10,      \
                                value11, value12, value13, value14, value15,  \
                                value16, value17};                            \
  }                                                                           \

#define _ExpandNames18(name,mode,value1,value2,value3,value4,value5,value6,   \
                       value7,value8,value9,value10,value11,value12,value13,  \
                       value14,value15,value16,value17,value18)               \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0, value2 = mode 1, value3 = mode 2,         \
                   value4 = mode 3, value5 = mode 4, value6 = mode 5,         \
                   value7 = mode 6, value8 = mode 7, value9 = mode 8,         \
                   value10 = mode 9, value11 = mode 10, value12 = mode 11,    \
                   value13 = mode 12, value14 = mode 13, value15 = mode 14,   \
                   value16 = mode 15, value17 = mode 16, value18 = mode 17};  \
  enum {Size = 18};                                                           \
  static const cstr Names[] = {#value1, #value2, #value3, #value4, #value5,   \
                               #value6, #value7, #value8, #value9, #value10,  \
                               #value11, #value12, #value13, #value14,        \
                               #value15, #value16, #value17, #value18, NULL}; \
  static const uint Values[] = {value1, value2, value3, value4, value5,       \
                                value6, value7, value8, value9, value10,      \
                                value11, value12, value13, value14, value15,  \
                                value16, value17, value18};                   \
  }                                                                           \

#define _ExpandNames19(name,mode,value1,value2,value3,value4,value5,value6,   \
                       value7,value8,value9,value10,value11,value12,value13,  \
                       value14,value15,value16,value17,value18,value19)       \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0, value2 = mode 1, value3 = mode 2,         \
                   value4 = mode 3, value5 = mode 4, value6 = mode 5,         \
                   value7 = mode 6, value8 = mode 7, value9 = mode 8,         \
                   value10 = mode 9, value11 = mode 10, value12 = mode 11,    \
                   value13 = mode 12, value14 = mode 13, value15 = mode 14,   \
                   value16 = mode 15, value17 = mode 16, value18 = mode 17,   \
                   value19 = mode 18};                                        \
  enum {Size = 19};                                                           \
  static const cstr Names[] = {#value1, #value2, #value3, #value4, #value5,   \
                               #value6, #value7, #value8, #value9, #value10,  \
                               #value11, #value12, #value13, #value14,        \
                               #value15, #value16, #value17, #value18,        \
                               #value19, NULL};                               \
  static const uint Values[] = {value1, value2, value3, value4, value5,       \
                                value6, value7, value8, value9, value10,      \
                                value11, value12, value13, value14, value15,  \
                                value16, value17, value18, value19};          \
  }                                                                           \

#define _ExpandNames20(name,mode,value1,value2,value3,value4,value5,value6,   \
                       value7,value8,value9,value10,value11,value12,value13,  \
                       value14,value15,value16,value17,value18,value19,       \
                       value20)                                               \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0, value2 = mode 1, value3 = mode 2,         \
                   value4 = mode 3, value5 = mode 4, value6 = mode 5,         \
                   value7 = mode 6, value8 = mode 7, value9 = mode 8,         \
                   value10 = mode 9, value11 = mode 10, value12 = mode 11,    \
                   value13 = mode 12, value14 = mode 13, value15 = mode 14,   \
                   value16 = mode 15, value17 = mode 16, value18 = mode 17,   \
                   value19 = mode 18, value20 = mode 19};                     \
  enum {Size = 20};                                                           \
  static const cstr Names[] = {#value1, #value2, #value3, #value4, #value5,   \
                               #value6, #value7, #value8, #value9, #value10,  \
                               #value11, #value12, #value13, #value14,        \
                               #value15, #value16, #value17, #value18,        \
                               #value19, #value20, NULL};                     \
  static const uint Values[] = {value1, value2, value3, value4, value5,       \
                                value6, value7, value8, value9, value10,      \
                                value11, value12, value13, value14, value15,  \
                                value16, value17, value18, value19, value20}; \
  }                                                                           \

#define _ExpandNames21(name,mode,value1,value2,value3,value4,value5,value6,   \
                       value7,value8,value9,value10,value11,value12,value13,  \
                       value14,value15,value16,value17,value18,value19,       \
                       value20,value21)                                       \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0, value2 = mode 1, value3 = mode 2,         \
                   value4 = mode 3, value5 = mode 4, value6 = mode 5,         \
                   value7 = mode 6, value8 = mode 7, value9 = mode 8,         \
                   value10 = mode 9, value11 = mode 10, value12 = mode 11,    \
                   value13 = mode 12, value14 = mode 13, value15 = mode 14,   \
                   value16 = mode 15, value17 = mode 16, value18 = mode 17,   \
                   value19 = mode 18, value20 = mode 19, value21 = mode 20};  \
  enum {Size = 21};                                                           \
  static const cstr Names[] = {#value1, #value2, #value3, #value4, #value5,   \
                               #value6, #value7, #value8, #value9, #value10,  \
                               #value11, #value12, #value13, #value14,        \
                               #value15, #value16, #value17, #value18,        \
                               #value19, #value20, #value21, NULL};           \
  static const uint Values[] = {value1, value2, value3, value4, value5,       \
                                value6, value7, value8, value9, value10,      \
                                value11, value12, value13, value14, value15,  \
                                value16, value17, value18, value19, value20,  \
                                value21};                                     \
  }                                                                           \

#define _ExpandNames22(name,mode,value1,value2,value3,value4,value5,value6,   \
                       value7,value8,value9,value10,value11,value12,value13,  \
                       value14,value15,value16,value17,value18,value19,       \
                       value20,value21,value22)                               \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0, value2 = mode 1, value3 = mode 2,         \
                   value4 = mode 3, value5 = mode 4, value6 = mode 5,         \
                   value7 = mode 6, value8 = mode 7, value9 = mode 8,         \
                   value10 = mode 9, value11 = mode 10, value12 = mode 11,    \
                   value13 = mode 12, value14 = mode 13, value15 = mode 14,   \
                   value16 = mode 15, value17 = mode 16, value18 = mode 17,   \
                   value19 = mode 18, value20 = mode 19, value21 = mode 20,   \
                   value22 = mode 21};                                        \
  enum {Size = 22};                                                           \
  static const cstr Names[] = {#value1, #value2, #value3, #value4, #value5,   \
                               #value6, #value7, #value8, #value9, #value10,  \
                               #value11, #value12, #value13, #value14,        \
                               #value15, #value16, #value17, #value18,        \
                               #value19, #value20, #value21, #value22, NULL}; \
  static const uint Values[] = {value1, value2, value3, value4, value5,       \
                                value6, value7, value8, value9, value10,      \
                                value11, value12, value13, value14, value15,  \
                                value16, value17, value18, value19, value20,  \
                                value21, value22};                            \
  }                                                                           \

#define _ExpandNames23(name,mode,value1,value2,value3,value4,value5,value6,   \
                       value7,value8,value9,value10,value11,value12,value13,  \
                       value14,value15,value16,value17,value18,value19,       \
                       value20,value21,value22,value23)                       \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0, value2 = mode 1, value3 = mode 2,         \
                   value4 = mode 3, value5 = mode 4, value6 = mode 5,         \
                   value7 = mode 6, value8 = mode 7, value9 = mode 8,         \
                   value10 = mode 9, value11 = mode 10, value12 = mode 11,    \
                   value13 = mode 12, value14 = mode 13, value15 = mode 14,   \
                   value16 = mode 15, value17 = mode 16, value18 = mode 17,   \
                   value19 = mode 18, value20 = mode 19, value21 = mode 20,   \
                   value22 = mode 21, value23 = mode 22};                     \
  enum {Size = 23};                                                           \
  static const cstr Names[] = {#value1, #value2, #value3, #value4, #value5,   \
                               #value6, #value7, #value8, #value9, #value10,  \
                               #value11, #value12, #value13, #value14,        \
                               #value15, #value16, #value17, #value18,        \
                               #value19, #value20, #value21, #value22,        \
                               #value23, NULL};                               \
  static const uint Values[] = {value1, value2, value3, value4, value5,       \
                                value6, value7, value8, value9, value10,      \
                                value11, value12, value13, value14, value15,  \
                                value16, value17, value18, value19, value20,  \
                                value21, value22, value23};                   \
  }                                                                           \

#define _ExpandNames24(name,mode,value1,value2,value3,value4,value5,value6,   \
                       value7,value8,value9,value10,value11,value12,value13,  \
                       value14,value15,value16,value17,value18,value19,       \
                       value20,value21,value22,value23,value24)               \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0, value2 = mode 1, value3 = mode 2,         \
                   value4 = mode 3, value5 = mode 4, value6 = mode 5,         \
                   value7 = mode 6, value8 = mode 7, value9 = mode 8,         \
                   value10 = mode 9, value11 = mode 10, value12 = mode 11,    \
                   value13 = mode 12, value14 = mode 13, value15 = mode 14,   \
                   value16 = mode 15, value17 = mode 16, value18 = mode 17,   \
                   value19 = mode 18, value20 = mode 19, value21 = mode 20,   \
                   value22 = mode 21, value23 = mode 22, value24 = mode 23};  \
  enum {Size = 24};                                                           \
  static const cstr Names[] = {#value1, #value2, #value3, #value4, #value5,   \
                               #value6, #value7, #value8, #value9, #value10,  \
                               #value11, #value12, #value13, #value14,        \
                               #value15, #value16, #value17, #value18,        \
                               #value19, #value20, #value21, #value22,        \
                               #value23, #value24, NULL};                     \
  static const uint Values[] = {value1, value2, value3, value4, value5,       \
                                value6, value7, value8, value9, value10,      \
                                value11, value12, value13, value14, value15,  \
                                value16, value17, value18, value19, value20,  \
                                value21, value22, value23, value24};          \
  }                                                                           \

#define _ExpandNames25(name,mode,value1,value2,value3,value4,value5,value6,   \
                       value7,value8,value9,value10,value11,value12,value13,  \
                       value14,value15,value16,value17,value18,value19,       \
                       value20,value21,value22,value23,value24,value25)       \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0, value2 = mode 1, value3 = mode 2,         \
                   value4 = mode 3, value5 = mode 4, value6 = mode 5,         \
                   value7 = mode 6, value8 = mode 7, value9 = mode 8,         \
                   value10 = mode 9, value11 = mode 10, value12 = mode 11,    \
                   value13 = mode 12, value14 = mode 13, value15 = mode 14,   \
                   value16 = mode 15, value17 = mode 16, value18 = mode 17,   \
                   value19 = mode 18, value20 = mode 19, value21 = mode 20,   \
                   value22 = mode 21, value23 = mode 22, value24 = mode 23,   \
                   value25 = mode 24};                                        \
  enum {Size = 25};                                                           \
  static const cstr Names[] = {#value1, #value2, #value3, #value4, #value5,   \
                               #value6, #value7, #value8, #value9, #value10,  \
                               #value11, #value12, #value13, #value14,        \
                               #value15, #value16, #value17, #value18,        \
                               #value19, #value20, #value21, #value22,        \
                               #value23, #value24, #value25, NULL};           \
  static const uint Values[] = {value1, value2, value3, value4, value5,       \
                                value6, value7, value8, value9, value10,      \
                                value11, value12, value13, value14, value15,  \
                                value16, value17, value18, value19, value20,  \
                                value21, value22, value23, value24, value25}; \
  }                                                                           \

#define _ExpandNames26(name,mode,value1,value2,value3,value4,value5,value6,   \
                       value7,value8,value9,value10,value11,value12,value13,  \
                       value14,value15,value16,value17,value18,value19,       \
                       value20,value21,value22,value23,value24,value25,       \
                       value26)                                               \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0, value2 = mode 1, value3 = mode 2,         \
                   value4 = mode 3, value5 = mode 4, value6 = mode 5,         \
                   value7 = mode 6, value8 = mode 7, value9 = mode 8,         \
                   value10 = mode 9, value11 = mode 10, value12 = mode 11,    \
                   value13 = mode 12, value14 = mode 13, value15 = mode 14,   \
                   value16 = mode 15, value17 = mode 16, value18 = mode 17,   \
                   value19 = mode 18, value20 = mode 19, value21 = mode 20,   \
                   value22 = mode 21, value23 = mode 22, value24 = mode 23,   \
                   value25 = mode 24, value26 = mode 25};                     \
  enum {Size = 26};                                                           \
  static const cstr Names[] = {#value1, #value2, #value3, #value4, #value5,   \
                               #value6, #value7, #value8, #value9, #value10,  \
                               #value11, #value12, #value13, #value14,        \
                               #value15, #value16, #value17, #value18,        \
                               #value19, #value20, #value21, #value22,        \
                               #value23, #value24, #value25, #value26, NULL}; \
  static const uint Values[] = {value1, value2, value3, value4, value5,       \
                                value6, value7, value8, value9, value10,      \
                                value11, value12, value13, value14, value15,  \
                                value16, value17, value18, value19, value20,  \
                                value21, value22, value23, value24, value25,  \
                                value26};                                     \
  }                                                                           \

#define _ExpandNames27(name,mode,value1,value2,value3,value4,value5,value6,   \
                       value7,value8,value9,value10,value11,value12,value13,  \
                       value14,value15,value16,value17,value18,value19,       \
                       value20,value21,value22,value23,value24,value25,       \
                       value26,value27)                                       \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0, value2 = mode 1, value3 = mode 2,         \
                   value4 = mode 3, value5 = mode 4, value6 = mode 5,         \
                   value7 = mode 6, value8 = mode 7, value9 = mode 8,         \
                   value10 = mode 9, value11 = mode 10, value12 = mode 11,    \
                   value13 = mode 12, value14 = mode 13, value15 = mode 14,   \
                   value16 = mode 15, value17 = mode 16, value18 = mode 17,   \
                   value19 = mode 18, value20 = mode 19, value21 = mode 20,   \
                   value22 = mode 21, value23 = mode 22, value24 = mode 23,   \
                   value25 = mode 24, value26 = mode 25, value27 = mode 26};  \
  enum {Size = 27};                                                           \
  static const cstr Names[] = {#value1, #value2, #value3, #value4, #value5,   \
                               #value6, #value7, #value8, #value9, #value10,  \
                               #value11, #value12, #value13, #value14,        \
                               #value15, #value16, #value17, #value18,        \
                               #value19, #value20, #value21, #value22,        \
                               #value23, #value24, #value25, #value26,        \
                               #value27, NULL};                               \
  static const uint Values[] = {value1, value2, value3, value4, value5,       \
                                value6, value7, value8, value9, value10,      \
                                value11, value12, value13, value14, value15,  \
                                value16, value17, value18, value19, value20,  \
                                value21, value22, value23, value24, value25,  \
                                value26, value27};                            \
  }                                                                           \

#define _ExpandNames28(name,mode,value1,value2,value3,value4,value5,value6,   \
                       value7,value8,value9,value10,value11,value12,value13,  \
                       value14,value15,value16,value17,value18,value19,       \
                       value20,value21,value22,value23,value24,value25,       \
                       value26,value27,value28)                               \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0, value2 = mode 1, value3 = mode 2,         \
                   value4 = mode 3, value5 = mode 4, value6 = mode 5,         \
                   value7 = mode 6, value8 = mode 7, value9 = mode 8,         \
                   value10 = mode 9, value11 = mode 10, value12 = mode 11,    \
                   value13 = mode 12, value14 = mode 13, value15 = mode 14,   \
                   value16 = mode 15, value17 = mode 16, value18 = mode 17,   \
                   value19 = mode 18, value20 = mode 19, value21 = mode 20,   \
                   value22 = mode 21, value23 = mode 22, value24 = mode 23,   \
                   value25 = mode 24, value26 = mode 25, value27 = mode 26,   \
                   value28 = mode 27};                                        \
  enum {Size = 28};                                                           \
  static const cstr Names[] = {#value1, #value2, #value3, #value4, #value5,   \
                               #value6, #value7, #value8, #value9, #value10,  \
                               #value11, #value12, #value13, #value14,        \
                               #value15, #value16, #value17, #value18,        \
                               #value19, #value20, #value21, #value22,        \
                               #value23, #value24, #value25, #value26,        \
                               #value27, #value28, NULL};                     \
  static const uint Values[] = {value1, value2, value3, value4, value5,       \
                                value6, value7, value8, value9, value10,      \
                                value11, value12, value13, value14, value15,  \
                                value16, value17, value18, value19, value20,  \
                                value21, value22, value23, value24, value25,  \
                                value26, value27, value28};                   \
  }                                                                           \

#define _ExpandNames29(name,mode,value1,value2,value3,value4,value5,value6,   \
                       value7,value8,value9,value10,value11,value12,value13,  \
                       value14,value15,value16,value17,value18,value19,       \
                       value20,value21,value22,value23,value24,value25,       \
                       value26,value27,value28,value29)                       \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0, value2 = mode 1, value3 = mode 2,         \
                   value4 = mode 3, value5 = mode 4, value6 = mode 5,         \
                   value7 = mode 6, value8 = mode 7, value9 = mode 8,         \
                   value10 = mode 9, value11 = mode 10, value12 = mode 11,    \
                   value13 = mode 12, value14 = mode 13, value15 = mode 14,   \
                   value16 = mode 15, value17 = mode 16, value18 = mode 17,   \
                   value19 = mode 18, value20 = mode 19, value21 = mode 20,   \
                   value22 = mode 21, value23 = mode 22, value24 = mode 23,   \
                   value25 = mode 24, value26 = mode 25, value27 = mode 26,   \
                   value28 = mode 27, value29 = mode 28};                     \
  enum {Size = 29};                                                           \
  static const cstr Names[] = {#value1, #value2, #value3, #value4, #value5,   \
                               #value6, #value7, #value8, #value9, #value10,  \
                               #value11, #value12, #value13, #value14,        \
                               #value15, #value16, #value17, #value18,        \
                               #value19, #value20, #value21, #value22,        \
                               #value23, #value24, #value25, #value26,        \
                               #value27, #value28, #value29, NULL};           \
  static const uint Values[] = {value1, value2, value3, value4, value5,       \
                                value6, value7, value8, value9, value10,      \
                                value11, value12, value13, value14, value15,  \
                                value16, value17, value18, value19, value20,  \
                                value21, value22, value23, value24, value25,  \
                                value26, value27, value28, value29};          \
  }                                                                           \

#define _ExpandNames30(name,mode,value1,value2,value3,value4,value5,value6,   \
                       value7,value8,value9,value10,value11,value12,value13,  \
                       value14,value15,value16,value17,value18,value19,       \
                       value20,value21,value22,value23,value24,value25,       \
                       value26,value27,value28,value29,value30)               \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0, value2 = mode 1, value3 = mode 2,         \
                   value4 = mode 3, value5 = mode 4, value6 = mode 5,         \
                   value7 = mode 6, value8 = mode 7, value9 = mode 8,         \
                   value10 = mode 9, value11 = mode 10, value12 = mode 11,    \
                   value13 = mode 12, value14 = mode 13, value15 = mode 14,   \
                   value16 = mode 15, value17 = mode 16, value18 = mode 17,   \
                   value19 = mode 18, value20 = mode 19, value21 = mode 20,   \
                   value22 = mode 21, value23 = mode 22, value24 = mode 23,   \
                   value25 = mode 24, value26 = mode 25, value27 = mode 26,   \
                   value28 = mode 27, value29 = mode 28, value30 = mode 29};  \
  enum {Size = 30};                                                           \
  static const cstr Names[] = {#value1, #value2, #value3, #value4, #value5,   \
                               #value6, #value7, #value8, #value9, #value10,  \
                               #value11, #value12, #value13, #value14,        \
                               #value15, #value16, #value17, #value18,        \
                               #value19, #value20, #value21, #value22,        \
                               #value23, #value24, #value25, #value26,        \
                               #value27, #value28, #value29, #value30, NULL}; \
  static const uint Values[] = {value1, value2, value3, value4, value5,       \
                                value6, value7, value8, value9, value10,      \
                                value11, value12, value13, value14, value15,  \
                                value16, value17, value18, value19, value20,  \
                                value21, value22, value23, value24, value25,  \
                                value26, value27, value28, value29, value30}; \
  }                                                                           \

#define _ExpandNames31(name,mode,value1,value2,value3,value4,value5,value6,   \
                       value7,value8,value9,value10,value11,value12,value13,  \
                       value14,value15,value16,value17,value18,value19,       \
                       value20,value21,value22,value23,value24,value25,       \
                       value26,value27,value28,value29,value30,value31)       \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0, value2 = mode 1, value3 = mode 2,         \
                   value4 = mode 3, value5 = mode 4, value6 = mode 5,         \
                   value7 = mode 6, value8 = mode 7, value9 = mode 8,         \
                   value10 = mode 9, value11 = mode 10, value12 = mode 11,    \
                   value13 = mode 12, value14 = mode 13, value15 = mode 14,   \
                   value16 = mode 15, value17 = mode 16, value18 = mode 17,   \
                   value19 = mode 18, value20 = mode 19, value21 = mode 20,   \
                   value22 = mode 21, value23 = mode 22, value24 = mode 23,   \
                   value25 = mode 24, value26 = mode 25, value27 = mode 26,   \
                   value28 = mode 27, value29 = mode 28, value30 = mode 29,   \
                   value31 = mode 30};                                        \
  enum {Size = 31};                                                           \
  static const cstr Names[] = {#value1, #value2, #value3, #value4, #value5,   \
                               #value6, #value7, #value8, #value9, #value10,  \
                               #value11, #value12, #value13, #value14,        \
                               #value15, #value16, #value17, #value18,        \
                               #value19, #value20, #value21, #value22,        \
                               #value23, #value24, #value25, #value26,        \
                               #value27, #value28, #value29, #value30,        \
                               #value31, NULL};                               \
  static const uint Values[] = {value1, value2, value3, value4, value5,       \
                                value6, value7, value8, value9, value10,      \
                                value11, value12, value13, value14, value15,  \
                                value16, value17, value18, value19, value20,  \
                                value21, value22, value23, value24, value25,  \
                                value26, value27, value28, value29, value30,  \
                                value31};                                     \
  }                                                                           \

#define _ExpandNames32(name,mode,value1,value2,value3,value4,value5,value6,   \
                       value7,value8,value9,value10,value11,value12,value13,  \
                       value14,value15,value16,value17,value18,value19,       \
                       value20,value21,value22,value23,value24,value25,       \
                       value26,value27,value28,value29,value30,value31,       \
                       value32)                                               \
  namespace name                                                              \
  {                                                                           \
  typedef uint Type;                                                          \
  static const cstr EnumName = #name;                                         \
  enum       Enum {value1 = mode 0, value2 = mode 1, value3 = mode 2,         \
                   value4 = mode 3, value5 = mode 4, value6 = mode 5,         \
                   value7 = mode 6, value8 = mode 7, value9 = mode 8,         \
                   value10 = mode 9, value11 = mode 10, value12 = mode 11,    \
                   value13 = mode 12, value14 = mode 13, value15 = mode 14,   \
                   value16 = mode 15, value17 = mode 16, value18 = mode 17,   \
                   value19 = mode 18, value20 = mode 19, value21 = mode 20,   \
                   value22 = mode 21, value23 = mode 22, value24 = mode 23,   \
                   value25 = mode 24, value26 = mode 25, value27 = mode 26,   \
                   value28 = mode 27, value29 = mode 28, value30 = mode 29,   \
                   value31 = mode 30, value32 = mode 31};                     \
  enum {Size = 32};                                                           \
  static const cstr Names[] = {#value1, #value2, #value3, #value4, #value5,   \
                               #value6, #value7, #value8, #value9, #value10,  \
                               #value11, #value12, #value13, #value14,        \
                               #value15, #value16, #value17, #value18,        \
                               #value19, #value20, #value21, #value22,        \
                               #value23, #value24, #value25, #value26,        \
                               #value27, #value28, #value29, #value30,        \
                               #value31, #value32, NULL};                     \
  static const uint Values[] = {value1, value2, value3, value4, value5,       \
                                value6, value7, value8, value9, value10,      \
                                value11, value12, value13, value14, value15,  \
                                value16, value17, value18, value19, value20,  \
                                value21, value22, value23, value24, value25,  \
                                value26, value27, value28, value29, value30,  \
                                value31, value32};                            \
  }                                                                           \


#define DeclareEnum1(name,v1)                                                 \
        _ExpandNames1(name,_Indexed(),v1)
#define DeclareEnum2(name,v1,v2)                                              \
        _ExpandNames2(name,_Indexed(),v1,v2)
#define DeclareEnum3(name,v1,v2,v3)                                           \
        _ExpandNames3(name,_Indexed(),v1,v2,v3)
#define DeclareEnum4(name,v1,v2,v3,v4)                                        \
        _ExpandNames4(name,_Indexed(),v1,v2,v3,v4)
#define DeclareEnum5(name,v1,v2,v3,v4,v5)                                     \
        _ExpandNames5(name,_Indexed(),v1,v2,v3,v4,v5)
#define DeclareEnum6(name,v1,v2,v3,v4,v5,v6)                                  \
        _ExpandNames6(name,_Indexed(),v1,v2,v3,v4,v5,v6)
#define DeclareEnum7(name,v1,v2,v3,v4,v5,v6,v7)                               \
        _ExpandNames7(name,_Indexed(),v1,v2,v3,v4,v5,v6,v7)
#define DeclareEnum8(name,v1,v2,v3,v4,v5,v6,v7,v8)                            \
        _ExpandNames8(name,_Indexed(),v1,v2,v3,v4,v5,v6,v7,v8)
#define DeclareEnum9(name,v1,v2,v3,v4,v5,v6,v7,v8,v9)                         \
        _ExpandNames9(name,_Indexed(),v1,v2,v3,v4,v5,v6,v7,v8,v9)
#define DeclareEnum10(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10)                    \
        _ExpandNames10(name,_Indexed(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10)
#define DeclareEnum11(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11)                \
        _ExpandNames11(name,_Indexed(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11)
#define DeclareEnum12(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12)            \
        _ExpandNames12(name,_Indexed(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12)
#define DeclareEnum13(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13)        \
        _ExpandNames13(name,_Indexed(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,\
                       v13)
#define DeclareEnum14(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14)    \
        _ExpandNames14(name,_Indexed(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,\
                       v13,v14)
#define DeclareEnum15(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15)\
        _ExpandNames15(name,_Indexed(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,\
                       v13,v14,v15)
#define DeclareEnum16(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,\
                      v16)                                                    \
        _ExpandNames16(name,_Indexed(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,\
                       v13,v14,v15,v16)
#define DeclareEnum17(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,\
                      v16,v17)                                                \
        _ExpandNames17(name,_Indexed(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,\
                       v13,v14,v15,v16,v17)
#define DeclareEnum18(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,\
                      v16,v17,v18)                                            \
        _ExpandNames18(name,_Indexed(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,\
                       v13,v14,v15,v16,v17,v18)
#define DeclareEnum19(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,\
                      v16,v17,v18,v19)                                        \
        _ExpandNames19(name,_Indexed(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,\
                       v13,v14,v15,v16,v17,v18,v19)
#define DeclareEnum20(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,\
                      v16,v17,v18,v19,v20)                                    \
        _ExpandNames20(name,_Indexed(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,\
                       v13,v14,v15,v16,v17,v18,v19,v20)
#define DeclareEnum21(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,\
                      v16,v17,v18,v19,v20,v21)                                \
        _ExpandNames21(name,_Indexed(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,\
                       v13,v14,v15,v16,v17,v18,v19,v20,v21)
#define DeclareEnum22(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,\
                      v16,v17,v18,v19,v20,v21,v22)                            \
        _ExpandNames22(name,_Indexed(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,\
                       v13,v14,v15,v16,v17,v18,v19,v20,v21,v22)
#define DeclareEnum23(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,\
                      v16,v17,v18,v19,v20,v21,v22,v23)                        \
        _ExpandNames23(name,_Indexed(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,\
                       v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23)
#define DeclareEnum24(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,\
                      v16,v17,v18,v19,v20,v21,v22,v23,v24)                    \
        _ExpandNames24(name,_Indexed(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,\
                       v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24)
#define DeclareEnum25(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,\
                      v16,v17,v18,v19,v20,v21,v22,v23,v24,v25)                \
        _ExpandNames25(name,_Indexed(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,\
                       v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25)
#define DeclareEnum26(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,\
                      v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26)            \
        _ExpandNames26(name,_Indexed(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,\
                       v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,   \
                       v26)
#define DeclareEnum27(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,\
                      v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27)        \
        _ExpandNames27(name,_Indexed(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,\
                       v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,   \
                       v26,v27)
#define DeclareEnum28(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,\
                      v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28)    \
        _ExpandNames28(name,_Indexed(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,\
                       v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,   \
                       v26,v27,v28)
#define DeclareEnum29(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,\
                      v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29)\
        _ExpandNames29(name,_Indexed(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,\
                       v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,   \
                       v26,v27,v28,v29)
#define DeclareEnum30(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,\
                      v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,\
                      v30)                                                    \
        _ExpandNames30(name,_Indexed(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,\
                       v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,   \
                       v26,v27,v28,v29,v30)
#define DeclareEnum31(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,\
                      v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,\
                      v30,v31)                                                \
        _ExpandNames31(name,_Indexed(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,\
                       v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,   \
                       v26,v27,v28,v29,v30,v31)
#define DeclareEnum32(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,\
                      v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,v28,v29,\
                      v30,v31,v32)                                            \
        _ExpandNames32(name,_Indexed(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,\
                       v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,   \
                       v26,v27,v28,v29,v30,v31,v32)

#define DeclareBitField1(name,v1)                                             \
        _ExpandNames1(name,_BitField(),v1)                                    \
        _AddNone(name)
#define DeclareBitField2(name,v1,v2)                                          \
        _ExpandNames2(name,_BitField(),v1,v2)                                 \
        _AddNone(name)
#define DeclareBitField3(name,v1,v2,v3)                                       \
        _ExpandNames3(name,_BitField(),v1,v2,v3)                              \
        _AddNone(name)
#define DeclareBitField4(name,v1,v2,v3,v4)                                    \
        _ExpandNames4(name,_BitField(),v1,v2,v3,v4)                           \
        _AddNone(name)
#define DeclareBitField5(name,v1,v2,v3,v4,v5)                                 \
        _ExpandNames5(name,_BitField(),v1,v2,v3,v4,v5)                        \
        _AddNone(name)
#define DeclareBitField6(name,v1,v2,v3,v4,v5,v6)                              \
        _ExpandNames6(name,_BitField(),v1,v2,v3,v4,v5,v6)                     \
        _AddNone(name)
#define DeclareBitField7(name,v1,v2,v3,v4,v5,v6,v7)                           \
        _ExpandNames7(name,_BitField(),v1,v2,v3,v4,v5,v6,v7)                  \
        _AddNone(name)
#define DeclareBitField8(name,v1,v2,v3,v4,v5,v6,v7,v8)                        \
        _ExpandNames8(name,_BitField(),v1,v2,v3,v4,v5,v6,v7,v8)               \
        _AddNone(name)
#define DeclareBitField9(name,v1,v2,v3,v4,v5,v6,v7,v8,v9)                     \
        _ExpandNames9(name,_BitField(),v1,v2,v3,v4,v5,v6,v7,v8,v9)            \
        _AddNone(name)
#define DeclareBitField10(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10)                \
        _ExpandNames10(name,_BitField(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10)       \
        _AddNone(name)
#define DeclareBitField11(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11)            \
        _ExpandNames11(name,_BitField(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11)   \
        _AddNone(name)
#define DeclareBitField12(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12)        \
        _ExpandNames12(name,_BitField(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,   \
                       v12)                                                   \
        _AddNone(name)
#define DeclareBitField13(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13)    \
        _ExpandNames13(name,_BitField(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,   \
                       v12,v13)                                               \
        _AddNone(name)
#define DeclareBitField14(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14)\
        _ExpandNames14(name,_BitField(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,   \
                       v12,v13,v14)                                           \
        _AddNone(name)
#define DeclareBitField15(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,\
                          v15)                                                \
        _ExpandNames15(name,_BitField(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,   \
                       v12,v13,v14,v15)                                       \
        _AddNone(name)
#define DeclareBitField16(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,\
                          v15,v16)                                            \
        _ExpandNames16(name,_BitField(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,   \
                       v12,v13,v14,v15,v16)                                   \
        _AddNone(name)
#define DeclareBitField17(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,\
                          v15,v16,v17)                                        \
        _ExpandNames17(name,_BitField(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,   \
                       v12,v13,v14,v15,v16,v17)                               \
        _AddNone(name)
#define DeclareBitField18(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,\
                          v15,v16,v17,v18)                                    \
        _ExpandNames18(name,_BitField(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,   \
                       v12,v13,v14,v15,v16,v17,v18)                           \
        _AddNone(name)
#define DeclareBitField19(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,\
                          v15,v16,v17,v18,v19)                                \
        _ExpandNames19(name,_BitField(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,   \
                       v12,v13,v14,v15,v16,v17,v18,v19)                       \
        _AddNone(name)
#define DeclareBitField20(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,\
                          v15,v16,v17,v18,v19,v20)                            \
        _ExpandNames20(name,_BitField(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,   \
                       v12,v13,v14,v15,v16,v17,v18,v19,v20)                   \
        _AddNone(name)
#define DeclareBitField21(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,\
                          v15,v16,v17,v18,v19,v20,v21)                        \
        _ExpandNames21(name,_BitField(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,   \
                       v12,v13,v14,v15,v16,v17,v18,v19,v20,v21)               \
        _AddNone(name)
#define DeclareBitField22(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,\
                          v15,v16,v17,v18,v19,v20,v21,v22)                    \
        _ExpandNames22(name,_BitField(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,   \
                       v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22)           \
        _AddNone(name)
#define DeclareBitField23(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,\
                          v15,v16,v17,v18,v19,v20,v21,v22,v23)                \
        _ExpandNames23(name,_BitField(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,   \
                       v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23)       \
        _AddNone(name)
#define DeclareBitField24(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,\
                          v15,v16,v17,v18,v19,v20,v21,v22,v23,v24)            \
        _ExpandNames24(name,_BitField(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,   \
                       v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24)   \
        _AddNone(name)
#define DeclareBitField25(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,\
                          v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25)        \
        _ExpandNames25(name,_BitField(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,   \
                       v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,   \
                       v25)                                                   \
        _AddNone(name)
#define DeclareBitField26(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,\
                          v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26)    \
        _ExpandNames26(name,_BitField(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,   \
                       v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,   \
                       v25,v26)                                               \
        _AddNone(name)
#define DeclareBitField27(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,\
                          v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27)\
        _ExpandNames27(name,_BitField(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,   \
                       v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,   \
                       v25,v26,v27)                                           \
        _AddNone(name)
#define DeclareBitField28(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,\
                          v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,\
                          v28)                                                \
        _ExpandNames28(name,_BitField(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,   \
                       v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,   \
                       v25,v26,v27,v28)                                       \
        _AddNone(name)
#define DeclareBitField29(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,\
                          v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,\
                          v28,v29)                                            \
        _ExpandNames29(name,_BitField(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,   \
                       v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,   \
                       v25,v26,v27,v28,v29)                                   \
        _AddNone(name)
#define DeclareBitField30(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,\
                          v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,\
                          v28,v29,v30)                                        \
        _ExpandNames30(name,_BitField(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,   \
                       v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,   \
                       v25,v26,v27,v28,v29,v30)                               \
        _AddNone(name)
#define DeclareBitField31(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,\
                          v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,\
                          v28,v29,v30,v31)                                    \
        _ExpandNames31(name,_BitField(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,   \
                       v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,   \
                       v25,v26,v27,v28,v29,v30,v31)                           \
        _AddNone(name)
#define DeclareBitField32(name,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,\
                          v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25,v26,v27,\
                          v28,v29,v30,v31,v32)                                \
        _ExpandNames32(name,_BitField(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,   \
                       v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,   \
                       v25,v26,v27,v28,v29,v30,v31,v32)                       \
        _AddNone(name)

