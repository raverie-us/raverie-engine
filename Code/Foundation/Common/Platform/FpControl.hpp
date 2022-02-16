// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

/// Temporarily changes the fpu exceptions mask so that fpu exceptions
/// will happen. After the current scope of this block, the old
/// exceptions mask will be replaced. Use sparingly as changing the
/// mask is not a cheap operation.
struct ZeroShared ScopeFpuExceptionsEnabler
{
  ScopeFpuExceptionsEnabler();
  ~ScopeFpuExceptionsEnabler();

  ZeroDeclarePrivateData(ScopeFpuExceptionsEnabler, 8);
};

/// Temporarily disables all fpu exception masks. Used primarily
/// before calling out into external programs such as directX
/// or CG.
struct ZeroShared ScopeFpuExceptionsDisabler
{
  ScopeFpuExceptionsDisabler();
  ~ScopeFpuExceptionsDisabler();

  ZeroDeclarePrivateData(ScopeFpuExceptionsDisabler, 8);
};

/// System to store the mask and active flag for floating point exceptions.
struct ZeroShared FpuControlSystem
{
  static uint DefaultMask;
  static bool Active;
};

} // namespace Zero

#define ZFpExceptions 1

#ifdef ZFpExceptions

#  define FpuExceptionsEnabler() ScopeFpuExceptionsEnabler __LocalScopedFpuExceptionsEnabler;

#  define FpuExceptionsDisabler() ScopeFpuExceptionsDisabler __LocalScopedFpuExceptionsDisabler;

#  ifdef ZeroDebug
#    define FpuExceptionsEnablerDebug() ScopeFpuExceptionsEnabler __LocalScopedFpuExceptionsEnabler;
#  else
#    define FpuExceptionsEnablerDebug()                                                                                \
      do                                                                                                               \
      {                                                                                                                \
      } while (0)
#  endif

#else

#  define FpuExceptionsEnabler()                                                                                       \
    do                                                                                                                 \
    {                                                                                                                  \
    } while (0)

#  define FpuExceptionsDisabler()                                                                                      \
    do                                                                                                                 \
    {                                                                                                                  \
    } while (0)

#  define FpuExceptionsEnablerDebug()                                                                                  \
    do                                                                                                                 \
    {                                                                                                                  \
    } while (0)

#endif
