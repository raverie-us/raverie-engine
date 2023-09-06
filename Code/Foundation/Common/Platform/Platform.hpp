// MIT Licensed (see LICENSE.md).
#pragma once

// Detect debug or release settings.
#ifdef NDEBUG
#  define ZeroRelease 1
#else
#  define ZeroDebug 1
#endif

// Ignore unknown pragma warnings...
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wpragmas"
// ZilchDeclareType is used on classes that sometimes derive from a base class
// with ZilchGetDerivedType() creating a discrepancy we can't avoid
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
// Many event handlers take an event and do not utilize the parameter or
// variable
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-private-field"
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#pragma clang diagnostic ignored "-Wunused-const-variable"
#pragma clang diagnostic ignored "-Wunused-value"
#pragma clang diagnostic ignored "-Wunused-function"
// Adding a virtual destructor to all classes this appears on results in various
// linker errors for scintilla
#pragma clang diagnostic ignored "-Wdelete-non-virtual-dtor"

// These should be investigated later
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Werror"

#pragma clang diagnostic ignored "-Wlogical-op-parentheses"

// Spirv triggers this
#pragma clang diagnostic ignored "-Wunqualified-std-cast-call"

#undef __STDC__

// ZeroForceInline forces a function to be inlined for optimization purposes
// ZeroNoInline forces a function to not be inlined (only used to debug, and
// unfortunately to fix true linker bugs)

// Helper macros
#define ZeroStringDeref(text) #text
#define ZeroStringize(text) ZeroStringDeref(text)

#define ZeroOffsetOfHelper(structure, op, member)                                                                      \
  (((::size_t) & reinterpret_cast<char const volatile&>((((structure*)(::uintptr_t)1)op member))) - 1)
#define ZeroOffsetOf(structure, member) ZeroOffsetOfHelper(structure, ->, member)

#define ZeroThreadLocal __thread
#define ZeroImport __attribute__((visibility("default")))
#define ZeroExport __attribute__((visibility("default")))
#define ZeroExportC extern "C" __attribute__((visibility("default")))
#define ZeroImportNamed(Name) __attribute__((used)) __attribute__((noinline)) __attribute__((visibility("default"))) __attribute__((__import_name__(#Name))) Name
#define ZeroExportNamed(Name) __attribute__((used)) __attribute__((noinline)) __attribute__((visibility("default"))) __attribute__((__export_name__(#Name))) Name
#define ZeroNoReturn __attribute__((noreturn))

#define ZeroTodo(text)
#define ZeroForceInline inline
#define ZeroDebugBreak()
#define ZeroNoInline

#define ZeroNoImportExport
#define ZeroShared
#define ZeroSharedTemplate

#include <alloca.h>
