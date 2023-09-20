// MIT Licensed (see LICENSE.md).

#pragma once

namespace Raverie
{
// This struct is given back to the user when asking for tokens
class UserToken
{
public:
  // Default constructor
  UserToken();

  // Construct a token for a keyword or symbol (NOT for variable length things
  // such as identifiers)
  UserToken(Grammar::Enum tokenId, CodeLocation* location = nullptr);

  // Constructor for a special type of token
  UserToken(StringParam token, Grammar::Enum tokenId, CodeLocation* location = nullptr);

  // The location is optional (if null is given, this will do nothing)
  void SetLocationAndStartLength(CodeLocation* location);

  // Make it easier to get the c-string for the token
  cstr c_str() const;

  String Token;
  Grammar::Enum TokenId;
  CodeLocation Location;
  size_t Start;
  size_t Length;
};

// A classifcation of tokens (not the specific token, but rather a category)
namespace TokenCategory
{
enum Enum
{
  Keyword,
  Symbol,
  Unknown,
};
}
} // namespace Raverie

// Explicit specializations
namespace Raverie
{
// UserToken should be directly memory movable (so should CodeLocation)
// String would technically just increment a reference and then decrement, so
// skip it!
template <>
struct MoveWithoutDestructionOperator<Raverie::UserToken>
{
  static inline void MoveWithoutDestruction(Raverie::UserToken* dest, Raverie::UserToken* source)
  {
    memcpy(dest, source, sizeof(*source));
  }
};
} // namespace Raverie
