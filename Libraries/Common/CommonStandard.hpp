// MIT Licensed (see LICENSE.md).
#pragma once

// The first thing we do is detect the platform.
// The next thing we do is define macros that all platforms may use
// We also ignore any compiler or platform specific warnings here
#include "Platform.hpp"

#include <algorithm>
#include <ctype.h>
#include <cstddef>
#include <utility>
#include <typeinfo>
#include <new>
#include <stdarg.h>
#include <climits>
#include <string>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stddef.h>
#include <setjmp.h>
#include <ctime>
#include <limits>
#include <unordered_map>
#include <memory>
#include <vector>
#include <cmath>
#include <limits>
#include <cstdio>
#include <cstdlib>
#include <cfloat>
#include <malloc.h>

using namespace std;

namespace Zero
{

class ZeroNoImportExport CommonLibrary
{
public:
  static void Initialize();
  static void Shutdown();
};

} // namespace Zero

#include "MaxSizeof.hpp"

static const constexpr size_t MaxPrimtiveSize = MaxSizeof4(void*, uintptr_t, uintmax_t, double);
typedef double MaxAlignmentType;
#define ZeroAlignCount(SizeBytes) ((SizeBytes + sizeof(::MaxAlignmentType) - 1) / sizeof(::MaxAlignmentType))

#include "Standard.hpp"
#include "Typedefs.hpp"
#include "Time.hpp"
#include "Misc.hpp"
#include "Guid.hpp"
#include "TypeTraits.hpp"
#include "BitTypes.hpp"
#include "BitMath.hpp"
#include "Atomic.hpp"
#include "InList.hpp"
#include "ArrayMap.hpp"
#include "ArraySet.hpp"
#include "BlockArray.hpp"
#include "ByteBuffer.hpp"
#include "CyclicArray.hpp"
#include "OwnedArray.hpp"
#include "SortedArray.hpp"
#include "UnsortedMap.hpp"
#include "OrderedHashMap.hpp"
#include "OrderedHashSet.hpp"
#include "Console.hpp"
#include "Diagnostic.hpp"
#include "Algorithm.hpp"
#include "Allocator.hpp"
#include "Array.hpp"
#include "BitStream.hpp"
#include "ContainerCommon.hpp"
#include "Hashing.hpp"
#include "HashMap.hpp"
#include "HashSet.hpp"
#include "SlotMap.hpp"
#include "Block.hpp"
#include "Graph.hpp"
#include "Heap.hpp"
#include "LocalStackAllocator.hpp"
#include "Memory.hpp"
#include "Pool.hpp"
#include "Stack.hpp"
#include "ZeroAllocator.hpp"
#include "Permuter.hpp"
#include "Rune.hpp"
#include "String.hpp"
#include "FixedString.hpp"
#include "CharacterTraits.hpp"
#include "StringRange.hpp"
#include "StringConversion.hpp"
#include "StringBuilder.hpp"
#include "StringUtility.hpp"
#include "ToString.hpp"
#include "BitStream.hpp"
#include "Regex.hpp"
#include "BitField.hpp"
#include "ByteEnum.hpp"
#include "ConditionalRange.hpp"
#include "EnumDeclaration.hpp"
#include "Guid.hpp"
#include "IdSequence.hpp"
#include "TextStream.hpp"
#include "HalfFloat.hpp"
#include "VirtualAny.hpp"
#include "IdStore.hpp"
#include "ItemCache.hpp"
#include "Log2.hpp"
#include "Status.hpp"
#include "ForEachRange.hpp"
#include "Misc.hpp"
#include "Standard.hpp"
#include "Typedefs.hpp"
#include "UintNType.hpp"
#include "UniquePointer.hpp"
#include "Functor.hpp"
#include "HalfFloat.hpp"
#include "Callback.hpp"
#include "PointerCast.hpp"
#include "Lexer.hpp"
#include "Guid.hpp"
#include "NativeType.hpp"
#include "Variant.hpp"
#include "Determinism.hpp"
#include "SpinLock.hpp"
#include "Web.hpp"
#include "Stream.hpp"
#include "Singleton.hpp"

namespace Math
{
#include "Typedefs.hpp"
} // namespace Math

#include "Reals.hpp"
#include "MatrixStorage.hpp"
#include "Vector2.hpp"
#include "Vector3.hpp"
#include "Vector4.hpp"
#include "Matrix2.hpp"
#include "Matrix3.hpp"
#include "Matrix4.hpp"
#include "IntVector2.hpp"
#include "IntVector3.hpp"
#include "IntVector4.hpp"
#include "BoolVector2.hpp"
#include "BoolVector3.hpp"
#include "BoolVector4.hpp"
#include "Quaternion.hpp"
#include "Math.hpp"
#include "SharedVectorFunctions.hpp"

#include "Random.hpp"
#include "ByteColor.hpp"
#include "Curve.hpp"
#include "DecomposedMatrix4.hpp"
#include "EulerOrder.hpp"
#include "EulerAngles.hpp"
#include "Numerical.hpp"

#include "VectorHashPolicy.hpp"
#include "WeightedProbabilityTable.hpp"

#include "GenericVector.hpp"
#include "ExtendableMath.hpp"
#include "ErrorCallbacks.hpp"
#include "BlockVector3.hpp"
#include "IndexPolicies.hpp"
#include "JacobiSolver.hpp"
#include "ConjugateGradient.hpp"
#include "SimpleCgPolicies.hpp"
#include "GaussSeidelSolver.hpp"

#include "MathToString.hpp"

// Currently the SIMD extensions are not technically platform agnostic and need
// to be revisited. It may be acceptable to include the intrinsic headers on
// multiple platforms, but it's unknown.
#if defined(USESSE)
#  include "SimMath.hpp"
#  include "SimVectors.hpp"
#  include "SimMatrix3.hpp"
#  include "SimMatrix4.hpp"
#  include "SimConversion.hpp"
#endif

namespace Zero
{
#include "BasicNativeTypesMath.inl"
#include "MathImports.hpp"
} // namespace Zero

#include "Rect.hpp"
#include "Image.hpp"

#include "PrivateImplementation.hpp"
#include "Utilities.hpp"
#include "OsHandle.hpp"
#include "Thread.hpp"
#include "ThreadSync.hpp"
#include "CrashHandler.hpp"
#include "Debug.hpp"
#include "DebugSymbolInformation.hpp"
#include "ExternalLibrary.hpp"
#include "File.hpp"
#include "FileEvents.hpp"
#include "FilePath.hpp"
#include "FileSystem.hpp"
#include "FpControl.hpp"
#include "Lock.hpp"
#include "Process.hpp"
#include "Registry.hpp"
#include "Resolution.hpp"
#include "SocketEnums.hpp"
#include "SocketConstants.hpp"
#include "Socket.hpp"
#include "IpAddress.hpp"
#include "Timer.hpp"
#include "TimerBlock.hpp"
#include "DirectoryWatcher.hpp"
#include "Peripherals.hpp"
#include "ExecutableResource.hpp"
#include "Main.hpp"
#include "MainLoop.hpp"
#include "Shell.hpp"
#include "ComPort.hpp"
#include "Intrinsics.hpp"
#include "Browser.hpp"
#include "RendererEnumerations.hpp"
#include "Renderer.hpp"
#include "Audio.hpp"
#include "CallStack.hpp"
#include "WebRequest.hpp"

#include "ThreadableLoop.hpp"
