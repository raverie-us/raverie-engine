// SciTE - Scintilla based Text Editor
/** @file PropSetSimple.cxx
 ** A Java style properties file module.
 **/
// Copyright 1998-2010 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

// Maintain a dictionary of properties

#include "Precompiled.hpp"
#include "../ScintillaPrecompiled.hpp"

#ifdef _MSC_VER
// Visual C++ doesn't like unreachable code in its own headers.
#pragma warning(disable: 4018 4100 4245 4511 4512 4663 4702)
#endif

#ifdef SCI_NAMESPACE
using namespace Scintilla;
#endif

typedef ScMap<ScString, ScString> mapss;

PropSetSimple::PropSetSimple() {
	mapss *props = new mapss;
	impl = static_cast<void *>(props);
}

PropSetSimple::~PropSetSimple() {
	mapss *props = static_cast<mapss *>(impl);
	delete props;
	impl = 0;
}

void PropSetSimple::Set(const char *key, const char *val, int lenKey, int lenVal) {
	mapss *props = static_cast<mapss *>(impl);
	if (!*key)	// Empty keys are not supported
		return;
	if (lenKey == -1)
		lenKey = static_cast<int>(strlen(key));
	if (lenVal == -1)
		lenVal = static_cast<int>(strlen(val));
	(*props)[ScString(key, lenKey)] = ScString(val, lenVal);
}

static bool IsASpaceCharacter(unsigned int ch) {
    return (ch == ' ') || ((ch >= 0x09) && (ch <= 0x0d));
}

void PropSetSimple::Set(const char *keyVal) {
	while (IsASpaceCharacter(*keyVal))
		keyVal++;
	const char *endVal = keyVal;
	while (*endVal && (*endVal != '\n'))
		endVal++;
	const char *eqAt = strchr(keyVal, '=');
	if (eqAt) {
		Set(keyVal, eqAt + 1, static_cast<int>(eqAt-keyVal), 
			static_cast<int>(endVal - eqAt - 1));
	} else if (*keyVal) {	// No '=' so assume '=1'
		Set(keyVal, "1", static_cast<int>(endVal-keyVal), 1);
	}
}

void PropSetSimple::SetMultiple(const char *s) {
	const char *eol = strchr(s, '\n');
	while (eol) {
		Set(s);
		s = eol + 1;
		eol = strchr(s, '\n');
	}
	Set(s);
}

const char *PropSetSimple::Get(const char *key) const {
	mapss *props = static_cast<mapss *>(impl);
	mapss::const_iterator keyPos = props->find(ScString(key));
	if (keyPos != props->end()) {
		return keyPos->second.c_str();
	} else {
		return "";
	}
}

// There is some inconsistency between GetExpanded("foo") and Expand("$(foo)").
// A solution is to keep a stack of variables that have been expanded, so that
// recursive expansions can be skipped.  For now I'll just use the C++ stack
// for that, through a recursive function and a simple chain of pointers.

struct VarChain {
	VarChain(const char *var_=NULL, const VarChain *link_=NULL): var(var_), link(link_) {}

	bool Contains(const char *testVar) const {
		return (var && (0 == strcmp(var, testVar)))
			|| (link && link->Contains(testVar));
	}

	const char *var;
	const VarChain *link;
};

static int ExpandAllInPlace(const PropSetSimple &props, ScString &withVars, int maxExpands, const VarChain &blankVars) {
	size_t varStart = withVars.find("$(");
	while ((varStart != ScString::npos) && (maxExpands > 0)) {
		size_t varEnd = withVars.find(")", varStart+2);
		if (varEnd == ScString::npos) {
			break;
		}

		// For consistency, when we see '$(ab$(cde))', expand the inner variable first,
		// regardless whether there is actually a degenerate variable named 'ab$(cde'.
		size_t innerVarStart = withVars.find("$(", varStart+2);
		while ((innerVarStart != ScString::npos) && (innerVarStart > varStart) && (innerVarStart < varEnd)) {
			varStart = innerVarStart;
			innerVarStart = withVars.find("$(", varStart+2);
		}

		ScString var(withVars.c_str(), varStart + 2, varEnd - varStart - 2);
		ScString val = props.Get(var.c_str());

		if (blankVars.Contains(var.c_str())) {
			val = ""; // treat blankVar as an empty string (e.g. to block self-reference)
		}

		if (--maxExpands >= 0) {
			maxExpands = ExpandAllInPlace(props, val, maxExpands, VarChain(var.c_str(), &blankVars));
		}

		withVars.erase(varStart, varEnd-varStart+1);
		withVars.insert(varStart, val.c_str(), val.length());

		varStart = withVars.find("$(");
	}

	return maxExpands;
}

char *PropSetSimple::Expanded(const char *key) const {
	ScString val = Get(key);
	ExpandAllInPlace(*this, val, 100, VarChain(key));
	char *ret = new char [val.size() + 1];
	strcpy(ret, val.c_str());
	return ret;
}

int PropSetSimple::GetExpanded(const char *key, char *result) const {
	char *val = Expanded(key);
	const int n = static_cast<int>(strlen(val));
	if (result) {
		strcpy(result, val);
	}
	delete []val;
	return n;	// Not including NUL
}

int PropSetSimple::GetInt(const char *key, int defaultValue) const {
	char *val = Expanded(key);
	if (val) {
		int retVal = val[0] ? atoi(val) : defaultValue;
		delete []val;
		return retVal;
	}
	return defaultValue;
}
