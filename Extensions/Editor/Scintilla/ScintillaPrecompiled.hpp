#pragma once

#include <assert.h>
#include <map>
#define ScString std::string
#define ScVector std::vector
#define ScMap std::map

//ToDo port for Scintilla- Compiles, but needs to confirm that it works
//#define ScString Zero::String
//#define ScMap Zero::HashMap
//#define ScVector Zero::Array
//#define assert Assert

#define SCI_LEXER

#include "include/Platform.h"
#include "include/SciLexer.h"
#include "include/Scintilla.h"
#include "include/ILexer.h"
#include "include/ScintillaWidget.h"

#include "lexlib/PropSetSimple.h"
#include "lexlib/WordList.h"
#include "lexlib/LexerBase.h"
#include "lexlib/LexerModule.h"
#include "lexlib/LexerNoExceptions.h"
#include "lexlib/LexerSimple.h"
#include "lexlib/OptionSet.h"
#include "lexlib/SparseState.h"
#include "lexlib/LexAccessor.h"
#include "lexlib/Accessor.h"
#include "lexlib/CharacterSet.h"
#include "lexlib/StyleContext.h"

#include "src/CallTip.h"
#include "src/Catalogue.h"
#include "src/SplitVector.h"
#include "src/Partitioning.h"
#include "src/CellBuffer.h"
#include "src/CharClassify.h"
#include "src/RunStyles.h"
#include "src/Decoration.h"
#include "src/ScintillaDocument.h"
#include "src/Selection.h"
#include "src/Style.h"
#include "src/XPM.h"
#include "src/LineMarker.h"
#include "src/Indicator.h"
#include "src/ViewStyle.h"
#include "src/PositionCache.h"
#include "src/AutoComplete.h"
#include "src/ContractionState.h"
#include "src/ExternalLexer.h"
#include "src/FontQuality.h"
#include "src/KeyMap.h"
#include "src/PerLine.h"
#include "src/RESearch.h"
#include "src/ScintillaEditor.h"
#include "src/ScintillaBase.h"
#include "src/SVector.h"
#include "src/UniConversion.h"

#ifdef SCI_NAMESPACE
using namespace Scintilla;
#endif

// Include Editor standard
#include "../Precompiled.hpp"

#include "ScintillaPlatformZero.hpp"
