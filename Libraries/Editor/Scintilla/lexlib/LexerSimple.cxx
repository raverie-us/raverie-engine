// Scintilla source code edit control
/** @file LexerSimple.cxx
 ** A simple lexer with no state.
 **/
// Copyright 1998-2010 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include "Precompiled.hpp"
#include "../ScintillaPrecompiled.hpp"

#ifdef SCI_NAMESPACE
using namespace Scintilla;
#endif

LexerSimple::LexerSimple(const LexerModule *module_) : module(module_) {
	for (int wl = 0; wl < module->GetNumWordLists(); wl++) {
		if (!wordLists.empty())
			wordLists += "\n";
		wordLists += module->GetWordListDescription(wl);
	}
}

const char * SCI_METHOD LexerSimple::DescribeWordListSets() {
	return wordLists.c_str();
}

void SCI_METHOD LexerSimple::Lex(unsigned int startPos, int lengthDoc, int initStyle, IDocument *pAccess) {
	Accessor astyler(pAccess, &props);
	module->Lex(startPos, lengthDoc, initStyle, keyWordLists, astyler);
	astyler.Flush();
}

void SCI_METHOD LexerSimple::Fold(unsigned int startPos, int lengthDoc, int initStyle, IDocument *pAccess) {
	if (props.GetInt("fold")) {
		Accessor astyler(pAccess, &props);
		module->Fold(startPos, lengthDoc, initStyle, keyWordLists, astyler);
		astyler.Flush();
	}
}
