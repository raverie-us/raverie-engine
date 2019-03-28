// MIT Licensed (see LICENSE.md).
#pragma once
#include "Precompiled.hpp"

namespace Zero
{

/// Encodes a string to be safely passed as a parameter
/// in a Url using Percent-encoding.
String UrlParamEncode(StringParam string);

/// Decodes a string from a Url using Percent-encoding.
String UrlParamDecode(StringParam string);

} // namespace Zero
