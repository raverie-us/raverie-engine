///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Davis
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Disable a warning for noexcept when exception handling is off
#ifdef _MSC_VER
#pragma warning(disable:4577)
#endif
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

#include "Utility/Standard.hpp"

namespace Zero
{

class ZeroNoImportExport CommonLibrary
{
public:
  static void Initialize();
  static void Shutdown();
};

}//namespace Zero

#include "Containers/InList.hpp"
#include "Containers/ArrayMap.hpp"
#include "Containers/ArraySet.hpp"
#include "Containers/BitStream.hpp"
#include "Containers/BlockArray.hpp"
#include "Containers/ByteBuffer.hpp"
#include "Containers/CyclicArray.hpp"
#include "Containers/OwnedArray.hpp"
#include "Containers/SortedArray.hpp"
#include "Containers/UnsortedMap.hpp"
#include "Containers/OrderedHashMap.hpp"
#include "Containers/OrderedHashSet.hpp"
#include "Diagnostic/Console.hpp"
#include "Diagnostic/Diagnostic.hpp"
#include "Containers/Algorithm.hpp"
#include "Containers/Allocator.hpp"
#include "Containers/Array.hpp"
#include "Containers/TypeTraits.hpp"
#include "Containers/ContainerCommon.hpp"
#include "Containers/Hashing.hpp"
#include "Containers/HashMap.hpp"
#include "Containers/HashSet.hpp"
#include "Containers/SlotMap.hpp"
#include "Memory/Block.hpp"
#include "Memory/Graph.hpp"
#include "Memory/Heap.hpp"
#include "Memory/LocalStackAllocator.hpp"
#include "Memory/Memory.hpp"
#include "Memory/Pool.hpp"
#include "Memory/Stack.hpp"
#include "Memory/ZeroAllocator.hpp"
#include "NullPtr.hpp"
#include "Regex/Regex.hpp"
#include "String/Rune.hpp"
#include "String/CharacterTraits.hpp"
#include "String/FixedString.hpp"
#include "String/StringRange.hpp"
#include "String/StringBuilder.hpp"
#include "String/StringConversion.hpp"
#include "String/StringUtility.hpp"
#include "String/ToString.hpp"
#include "Time.hpp"
#include "Utility/BitField.hpp"
#include "Utility/BitMath.hpp"
#include "Utility/BitTypes.hpp"
#include "Utility/ByteEnum.hpp"
#include "Utility/ConditionalRange.hpp"
#include "Utility/EnumDeclaration.hpp"
#include "Utility/Guid.hpp"
#include "Utility/IdSequence.hpp"
#include "Utility/TextStream.hpp"
#include "Utility/HalfFloat.hpp"
#include "Utility/MaxSizeof.hpp"
#include "VirtualAny.hpp"
#include "Utility/IdStore.hpp"
#include "Utility/ItemCache.hpp"
#include "Utility/Log2.hpp"
#include "Utility/Status.hpp"
#include "Utility/ForEachRange.hpp"
#include "Utility/Atomic.hpp"
#include "Utility/Misc.hpp"
#include "Utility/Standard.hpp"
#include "Utility/Typedefs.hpp"
#include "Utility/UintN.hpp"
#include "Utility/UniquePointer.hpp"
#include "Utility/Functor.hpp"
#include "Utility/HalfFloat.hpp"
#include "Utility/Callback.hpp"
#include "Utility/PointerCast.hpp"
#include "Lexer/Lexer.hpp"
#include "Utility/Guid.hpp"
#include "Utility/NativeType.hpp"
#include "Utility/Variant.hpp"
#include "Utility/Determinism.hpp"
#include "Utility/SpinLock.hpp"
#include "Singleton.hpp"
#include "Platform/PlatformSelector.hpp"
#include "Platform/PrivateImplementation.hpp"
#include "Platform/Utilities.hpp"
#include "Platform/OsHandle.hpp"
#include "Platform/Thread.hpp"
#include "Platform/ThreadSync.hpp"
#include "Platform/CrashHandler.hpp"
#include "Platform/Debug.hpp"
#include "Platform/DebugSymbolInformation.hpp"
#include "Platform/ExternalLibrary.hpp"
#include "Platform/File.hpp"
#include "Platform/FileEvents.hpp"
#include "Platform/FilePath.hpp"
#include "Platform/FileSystem.hpp"
#include "Platform/FpControl.hpp"
#include "Platform/Lock.hpp"
#include "Platform/PlatformSelector.hpp"
#include "Platform/Process.hpp"
#include "Platform/Registry.hpp"
#include "Platform/Resolution.hpp"
#include "Platform/SocketEnums.hpp"
#include "Platform/SocketConstants.hpp"
#include "Platform/Socket.hpp"
#include "Platform/IpAddress.hpp"
#include "Platform/Timer.hpp"
#include "Platform/TimerBlock.hpp"
#include "Platform/DirectoryWatcher.hpp"
#include "Platform/Peripherals.hpp"
#include "Platform/ExecutableResource.hpp"
#include "Platform/CommandLineSupport.hpp"
