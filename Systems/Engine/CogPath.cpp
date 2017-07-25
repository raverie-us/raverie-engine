///////////////////////////////////////////////////////////////////////////////
///
/// \file CogPath.cpp
/// Implementation of CogPath class.
///
/// Authors: Trevor Sundberg
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Operation.hpp"

static bool cCogPathDeprecate = false;

namespace Zero
{
namespace Events
{
  DefineEvent(CogPathCogChanged);
}

static const char* cRootSpecifier   = ":";
static const char* cPathSeparator   = "/";
static const char* cSelfSpecifier   = ".";
static const char* cParentSpecifier = "..";

static const char cRootSpecifierChar    = cRootSpecifier[0];
static const char cPathSeparatorChar    = cPathSeparator[0];
static const char cSelfSpecifierChar    = cSelfSpecifier[0];
static const char cParentSpecifierChar  = cParentSpecifier[0];

ZilchDefineType(CogPathMetaSerialization, builder, type)
{
}

DeclareEnum2(CogPathErrorCode, DidNotFind, ParseError)

bool IsValidCogPathNameCharacters(Rune r)
{
  return IsAlphaNumeric(r) || r == '_' || r == ' ';
}

ZilchDefineType(CogPath, builder, type)
{
  ZilchBindDefaultCopyDestructor();
  ZilchBindConstructor(StringParam);

  type->Add(new CogPathMetaSerialization());
  type->CreatableInScript = true;
  type->AddAttribute(PropertyAttributes::cAsPropertyUseCustomSerialization);

  ZeroBindDocumented();
  ZilchBindGetterSetter(Path);
  ZilchBindGetterSetter(Cog);

  // METAREFACTOR This should be a property, but we need a custom Cog display
  ZilchBindGetterSetter(DirectCog);

  ZilchBindGetterSetter(RelativeTo);
  ZilchBindGetterSetterProperty(ErrorOnResolveToNull);
  ZilchBindGetterSetterProperty(ErrorOnPathCantCompute);
  ZilchBindGetterSetterProperty(ErrorOnDirectLinkFail);
  ZilchBindGetterSetterProperty(UpdateCogOnPathChange);
  ZilchBindGetterSetterProperty(UpdateCogOnInitialize);
  ZilchBindGetterSetterProperty(UpdatePathOnCogChange);

  ZilchBindMethod(Refresh);
  ZilchBindMethod(RefreshIfNull);
  ZilchBindMethod(Clone);

  ZeroBindEvent(Events::CogPathCogChanged, CogPathEvent);

  ZilchBindOverloadedMethod(Resolve, (Cog* (*)(Cog*, StringParam)));
  ZilchBindOverloadedMethod(ComputePath, (String(*)(Cog*, Cog*, CogPathPreference::Enum)));

  ZilchBindGetterSetterProperty(PathPreference0);
  ZilchBindGetterSetterProperty(PathPreference1);
  ZilchBindGetterSetterProperty(PathPreference2);
}

bool ValidateMetaIsCog(Cog* cog, Status& status)
{
  if (cog == nullptr)
    return true;

  // This must be directly equal to, not IsA!
  BoundType* meta = ZilchVirtualTypeId(cog);
  if(meta != ZilchTypeId(Cog))
  {
    status.State = StatusState::Failure;
    status.Context = StatusCodeCogPath::OnlySupportsCogTypes;
    status.Message = String::Format("Cog paths can only point at true Cog types (attempted to point at a '%s')", meta->Name.c_str());
    return false;
  }
  return true;
};

// Because we're starting a relative path, we want to specify the
// separator \ only if we've already written our first value
bool hitFirst = false;
void AppendSeparatorIfNeeded(bool& hitFirst, StringBuilder& builder)
{
  if(hitFirst)
  {
    builder.Append(cPathSeparator);
  }
  hitFirst = true;
}

String CogPath::ComputePath(Status& status, Cog* from, Cog* to, CogPathPreference::Enum pref)
{
  if(pref == CogPathPreference::CogRelative)
  {
    // If we don't have an object that we're attempting to compute the path from, then it has to be absolute
    if(from == nullptr)
    {
      status.State = StatusState::Failure;
      status.Context = StatusCodeCogPath::RelativeToNotSet;
      static const String message = "The cog path's 'RelativeTo' field was not set, so a relative path could not be computed (generally 'RelativeTo' is set to the owning Cog)";
      status.Message = message;
      return String();
    }

    // The empty string can represent a null cog
    if(to == nullptr)
      return String();

    // We only support true cog types, not spaces, viewports, game sessions, etc
    if(!(ValidateMetaIsCog(from, status) && ValidateMetaIsCog(to, status)))
      return String();

    // If the objects are the same, we emit a special character that means ourself
    if(from == to)
    {
      return cSelfSpecifier;
    }

    Space* fromSpace = from->GetSpace();
    Space* toSpace = to->GetSpace();

    // If the objects lie in different spaces, there's nothing we can do to specify relative (it must be absolute)
    if(fromSpace != toSpace)
    {
      status.State = StatusState::Failure;
      status.Context = StatusCodeCogPath::CogsInDifferentSpaces;
      static const String message = "The two cogs were in different spaces so a relative path could not be computed";
      status.Message = message;
      return String();
    }

    Cog* fromRoot = from->FindRoot();
    Cog* toRoot = to->FindRoot();

    // If the cogs don't share a relative root path (other than the space)
    // then the path just has to be relative to the space
    if(fromRoot != toRoot)
    {
      status.State = StatusState::Failure;
      status.Context = StatusCodeCogPath::CogsDoNotShareRoots;
      static const String message = "The cogs do not share a root cog so a relative path could not be computed";
      status.Message = message;
      return String();
    }
  
    // We now know they are in the same space and have common ancestry, lets figure out the relative path!
    Array<Cog*> reversePathFrom;
    Array<Cog*> reversePathTo;

    while(from != nullptr)
    {
      reversePathFrom.PushBack(from);
      from = from->GetParent();
    }

    while(to != nullptr)
    {
      reversePathTo.PushBack(to);
      to = to->GetParent();
    }

    // Reverse the array in place to save memory, just rename the variable and don't use the old ones
    Reverse(reversePathFrom.Begin(), reversePathFrom.End());
    Reverse(reversePathTo.Begin(), reversePathTo.End());
    Array<Cog*>& pathFrom = reversePathFrom;
    Array<Cog*>& pathTo = reversePathTo;

    // Get the minimum size between the two relative paths (the shortest of both branches)
    size_t minSize = Math::Min(pathFrom.Size(), pathTo.Size());

    // Assume we diverge at the end of the from or to path (may be a directly up or down)
    // If this ever diverges anywhere else, then 
    size_t divergePoint = minSize;

    // Traverse the path from the root down
    for(size_t i = 0; i < minSize; ++i)
    {
      Cog* pathCogFrom = pathFrom[i];
      Cog* pathCogTo = pathTo[i];

      // If the paths ever diverge, this is where we start the relative path from!
      if(pathCogFrom != pathCogTo)
      {
        divergePoint = i;
        break;
      }
    }
  
    StringBuilder builder;

    // Because we're starting a relative path, we want to specify the
    // separator \ only if we've already written our first value
    bool hitFirst = false;

    // We need to add 'up one' or '..' for every child in the 'from path' past the diverge point
    for(size_t i = divergePoint; i < pathFrom.Size(); ++i)
    {
      AppendSeparatorIfNeeded(hitFirst, builder);
      builder.Append(cParentSpecifier);
    }

    // We need to add children references from the diverge point down in the 'to path'
    for(size_t i = divergePoint; i < pathTo.Size(); ++i)
    {
      Cog* pathCogTo = pathTo[i];

      AppendSeparatorIfNeeded(hitFirst, builder);

      String cogName = pathCogTo->GetName();
      if(cogName.Empty())
      {
        status.State = StatusState::Failure;
        status.Context = StatusCodeCogPath::CogNotNamed;
        String cogDescription = pathCogTo->GetDescription();
        if (!status.IgnoreMessage)
          status.Message = String::Format("%s was not named and therefore a path could not be created with it", cogDescription.c_str());
        return String();
      }

      builder.Append(cogName);
    }

    return builder.ToString();
  }
  else
  {
    // The empty string can represent a null cog
    if(to == nullptr)
      return String();

    StringBuilder builder;

    // We only support true cog types, not spaces, viewports, game sessions, etc
    if(!(ValidateMetaIsCog(from, status) && ValidateMetaIsCog(to, status)))
      return String();
  
    Space* space = to->GetSpace();

    // We only Append the name of the space if we want a true absolute path
    // Otherwise we can get the path relative to the current space
    if(pref == CogPathPreference::Absolute)
    {
      String spaceName = space->GetName();
  
      if(!spaceName.Empty())
      {
        builder.Append(spaceName);
      }
      else
      {
        status.State = StatusState::Failure;
        status.Context = StatusCodeCogPath::SpaceNotNamed;
        static const String message = "The space was not named, so we could not get an absolute path";
        status.Message = message;
        return String();
      }
    }
    else if (from == nullptr || from->GetSpace() != space)
    {
      status.State = StatusState::Failure;
      status.Context = StatusCodeCogPath::CogsInDifferentSpaces;
      static const String message = "The two cogs were in different spaces so a relative path could not be computed";
      status.Message = message;
      return String();
    }
    builder.Append(cRootSpecifier);

    Array<Cog*> reversePath;
    reversePath.Reserve(5);

    while(to != nullptr)
    {
      reversePath.PushBack(to);
      to = to->GetParent();
    }

    // Traverse the path from the root down
    for(int i = reversePath.Size() - 1; i >= 0; --i)
    {
      Cog* pathCog = reversePath[i];

      builder.Append(cPathSeparator);
    
      String cogName = pathCog->GetName();
      if(cogName.Empty())
      {
        status.State = StatusState::Failure;
        status.Context = StatusCodeCogPath::CogNotNamed;
        String cogDescription = pathCog->GetDescription();
        status.Message = String::Format("%s was not named and therefore a path could not be created with it", cogDescription.c_str());
        return String();
      }

      builder.Append(cogName);
    }

    return builder.ToString();
  }
}

String CogPath::ComputePath(Status& status, Cog* from, Cog* to, CogPathPreference::Enum pref0, CogPathPreference::Enum pref1, CogPathPreference::Enum pref2)
{
  String newPath;
  String errorMessage;

  CogPathPreference::Enum preferences[] =
  {
    pref0,
    pref1,
    pref2
  };
  size_t preferenceCount = sizeof(preferences) / sizeof(preferences[0]);
  
  for (size_t i = 0; i < preferenceCount; ++i)
  {
    status.SetSucceeded();

    CogPathPreference::Enum preference = preferences[i];
    newPath = ComputePath(status, from, to, preference);

    if (status.Succeeded())
      break;

    if (!status.IgnoreMessage)
    {
      if (errorMessage.Empty())
        errorMessage = BuildString(CogPathPreference::Names[i], ": ", status.Message);
      else
        errorMessage = BuildString(errorMessage, "\n", CogPathPreference::Names[i], ": ", status.Message);
    }
  }

  if(status.Failed())
    status.Message = errorMessage;
  return newPath;
}

cstr CogPathToken::GetFixedTokenText(CogPathTokenType::Enum fixedToken)
{
  switch(fixedToken)
  {
    case CogPathTokenType::Eof:
      return "<Eof>";
    case CogPathTokenType::Separator:
      return "/";
    case CogPathTokenType::Self:
      return ".";
    case CogPathTokenType::Parent:
      return "..";
    case CogPathTokenType::CurrentSpace:
      return ":";
    case CogPathTokenType::NamedCog:
      return "<NamedCog>";
    case CogPathTokenType::NamedSpace:
      return "<NamedSpace>:";
    default:
      return "<Invalid>";
  }
}

CogPathToken::CogPathToken() :
  mType(CogPathTokenType::Invalid)
{
}

CogPathTokenizer::CogPathTokenizer() :
  mPosition(0)
{
}

CogPathElement::CogPathElement() :
  mType(CogPathElementType::NamedCog)
{
}

CogPathCompiled::CogPathCompiled() :
  mRootType(CogPathRootType::Null)
{
}

bool CogPathParser::Accept(CogPathTokenType::Enum type, String* output)
{
  if(mToken.mType == type)
  {
    if (output != nullptr)
      *output = mToken.mText;
    mTokenizer.ReadToken(mToken);
    return true;
  }

  return false;
}

bool CogPathParser::Expect(CogPathTokenType::Enum type, String* output, Status& status, StringParam error)
{
  if(Accept(type, output))
    return true;

  status.State = StatusState::Failure;
  status.Message = error;
  return false;
}

bool CogPathParser::ParseSeparatedElement(Status& status, CogPathCompiled& output)
{
  if(this->Accept(CogPathTokenType::Separator, nullptr))
  {
    return this->ParseElement(status, output);
  }
  return false;
}

bool CogPathParser::ParseElement(Status& status, CogPathCompiled& output)
{
  String cogName;

  if(this->Accept(CogPathTokenType::NamedCog, &cogName))
  {
    CogPathElement& element = output.mElements.PushBack();
    element.mType = CogPathElementType::NamedCog;
    element.mValue = cogName;
  }
  else if(this->Accept(CogPathTokenType::Parent, nullptr))
  {
    // If we have no elements yet, and the output type is not 'relative', then it is not legal to traverse a parent!
    if(output.mElements.Empty() && output.mRootType != CogPathRootType::Relative)
    {
      status.State = StatusState::Failure;
      static const String message = "The '..' was used too many times and attempted to traverse above the space itself";
      status.Message = message;
      return false;
    }

    // This is an optimization / normalization of paths
    // If we're running the parent operator, and the element above us is a named cog,
    // we can just remove the element above instead of inserting another parent after it
    if(!output.mElements.Empty() && output.mElements.Back().mType == CogPathElementType::NamedCog)
    {
      output.mElements.PopBack();
    }
    else
    {
      CogPathElement& element = output.mElements.PushBack();
      element.mType = CogPathElementType::Parent;
    }
  }
  // Otherwise, the last thing we can parse is the self '.' (which does nothing!)
  else if(this->Accept(CogPathTokenType::Self, nullptr) == false)
  {
    // We didn't parse the self '.', so this is an error
    status.State = StatusState::Failure;
    static const String message = "Expected a Cog name, a parent '..', or a self '.'";
    status.Message = message;
    return false;
  }

  return true;
}

bool CogPathParser::ParsePath(Status& status, CogPathCompiled& output)
{
  // If our only token is an eof (nothing in our string) this is a special case that means null
  if(Accept(CogPathTokenType::Eof, nullptr))
  {
    output.mRootType = CogPathRootType::Null;
    return true;
  }

  // We can be a named space
  if(Accept(CogPathTokenType::NamedSpace, &output.mSpaceName))
  {
    output.mRootType = CogPathRootType::NamedSpace;
  }
  else if(Accept(CogPathTokenType::CurrentSpace, nullptr))
  {
    output.mRootType = CogPathRootType::CurrentSpace;
  }
  else
  {
    output.mRootType = CogPathRootType::Relative;
    if(this->ParseElement(status, output) == false)
    {
      return false;
    }
  }

  // Parse the rest of the elements
  while(this->ParseSeparatedElement(status, output)) {};

  // If we end up with no elements, and we're not relative, then things are bad!
  // Relative with no is OK because it means 'self'
  if(output.mElements.Empty() && output.mRootType != CogPathRootType::Relative)
  {
    status.State = StatusState::Failure;
    static const String message = "A space ':' or 'Name:' must be followed by a '/' and at least one "
                                  "Cog name (cannot be reversed by a following parent '..' operator!)";
    status.Message = message;
    return false;
  }

  return true;
}

void CogPathParser::Parse(Status& status, CogPathCompiled& output)
{
  // Read a single token so we can start deciding what to do
  mTokenizer.ReadToken(mToken);
  ParsePath(status, output);
  
  // We MUST see an Eof here (note that even after reading an Eof, our tokenizer is defined to keep returning Eof)
  if(Accept(CogPathTokenType::Eof, nullptr) == false)
  {
    status.State = StatusState::Failure;

    bool tokenHasName = mToken.mType == CogPathTokenType::NamedCog   ||
                        mToken.mType == CogPathTokenType::NamedSpace ||
                        mToken.mType == CogPathTokenType::Invalid;

    if(tokenHasName)
    {
      status.Message = String::Format("The path was malformed (hit an unexpected '%s' '%s' token)",
        CogPathTokenType::Names[mToken.mType], mToken.mText.c_str());
    }
    else
    {
      status.Message = String::Format("The path was malformed (hit an unexpected '%s' '%s' token)",
        CogPathTokenType::Names[mToken.mType], CogPathToken::GetFixedTokenText(mToken.mType));
    }
  }
}

void CogPathTokenizer::ReadToken(CogPathToken& tokenOut)
{
  tokenOut.mText.Clear();
  tokenOut.mType = CogPathTokenType::Invalid;
  
  StringIterator start = mPath.Begin() + mPosition;
  StringIterator it = start;
  Rune r = it.ReadCurrentRune();

  if(r == '\0')
  {
    // In this one case, we don't bother to advance the token position (keeps the tokenizer safe!)
    tokenOut.mType = CogPathTokenType::Eof;
  }
  else if(r == '/')
  {
    mPosition += UTF8::EncodedCodepointLength(mPath.Data()[mPosition]);
    tokenOut.mType = CogPathTokenType::Separator;
  }
  else if(r == '.')
  {
    mPosition += UTF8::EncodedCodepointLength(mPath.Data()[mPosition]);
    ++it;
    tokenOut.mType = CogPathTokenType::Self;

    r = it.ReadCurrentRune();
    
    // We need to keep looking to see if it's a parent
    if (r == '.')
    {
      mPosition += UTF8::EncodedCodepointLength(mPath.Data()[mPosition]);
      tokenOut.mType = CogPathTokenType::Parent;
    }
  }
  else if(r == ':')
  {
    mPosition += UTF8::EncodedCodepointLength(mPath.Data()[mPosition]);
    tokenOut.mType = CogPathTokenType::CurrentSpace;
  }
  else if(IsValidCogPathNameCharacters(r))
  {
    mPosition += UTF8::EncodedCodepointLength(mPath.Data()[mPosition]);
    ++it;
    tokenOut.mType = CogPathTokenType::NamedCog;

    for(;;)
    {
      r = it.ReadCurrentRune();

      if(IsValidCogPathNameCharacters(r))
      {
        // Still a cog!
        mPosition += UTF8::EncodedCodepointLength(mPath.Data()[mPosition]);
        ++it;
      }
      else if(r == ':')
      {
        mPosition += UTF8::EncodedCodepointLength(mPath.Data()[mPosition]);
        ++it;
        tokenOut.mType = CogPathTokenType::NamedSpace;
        // No more outgoing edges!
        break;
      }
      else
      {
        break;
      }
    }

    // Store the string on the token
    if(tokenOut.mType == CogPathTokenType::NamedSpace)
    {
      // We strip out the : at the end of the space name
      tokenOut.mText = mPath.SubString(start, --it);
    }
    else
    {
      tokenOut.mText = mPath.SubString(start, it);
    }
  }
  else
  {
    tokenOut.mText = String(r);
  }
}

Cog* CogPath::Resolve(Status& status, Cog* startFrom, const CogPathCompiled& path)
{
  if(path.mRootType == CogPathRootType::Null)
    return nullptr;

  // We cannot resolve a relative path unless we have a starting cog
  if(startFrom == nullptr && path.mRootType != CogPathRootType::NamedSpace)
  {
    status.State = StatusState::Failure;
    static const String message = "Attempting to resolve a relative path without a starting cog/space";
    if (!status.IgnoreMessage)
      status.Message = message;
    return nullptr;
  }

  Space* startSpace = nullptr;

  if(path.mRootType == CogPathRootType::CurrentSpace)
  {
    startSpace = startFrom->GetSpace();
  }
  else if(path.mRootType == CogPathRootType::NamedSpace)
  {
    GameSession* game = nullptr;
    if(startFrom)
      game = startFrom->GetGameSession();

    if(!game)
    {
      status.State = StatusState::Failure;
      static const String message = "The path used a named space, but there was no GameSession active to lookup spaces by name";
      if (!status.IgnoreMessage)
        status.Message = message;
      return nullptr;
    }

    startSpace = game->FindSpaceByName(path.mSpaceName);

    if(!startSpace)
    {
      status.State = StatusState::Failure;
      if (!status.IgnoreMessage)
        status.Message = String::Format("A space by the name of '%s' could not be found", path.mSpaceName.c_str());
      return nullptr;
    }
  }

  Cog* resolvedCog = nullptr;

  size_t elementIndex = 0;

  if(startSpace)
  {
    // Accessing the front element should always be valid here, since
    // a path that starts from a space MUST have at least one element
    ErrorIf(path.mElements.Empty(), "The path is invalid if the elements are empty and it starts from a space");
    const CogPathElement& root = path.mElements.Front();
    ErrorIf(root.mType != CogPathElementType::NamedCog, "The first element in a path that starts from a space MUST be a named cog");

    // Attempt to resolve the cog
    resolvedCog = startSpace->FindLastRootObjectByName(root.mValue);

    // Set the element index to 1 since we just read/processed the front element
    elementIndex = 1;

    if(!resolvedCog)
    {
      status.State = StatusState::Failure;
      if (!status.IgnoreMessage)
        status.Message = String::Format("The root cog by the name of '%s' could not be found in the space", root.mValue.c_str());
      return nullptr;
    }
  }
  else
  {
    // Since we're relative, start with the given cog
    resolvedCog = startFrom;
    ErrorIf(path.mRootType != CogPathRootType::Relative, "If we don't have a space here, we must be looking relative");
  }

  while(elementIndex < path.mElements.Size())
  {
    const CogPathElement& element = path.mElements[elementIndex];
    Cog* previousCog = resolvedCog;

    switch(element.mType)
    {
      case CogPathElementType::NamedCog:
        resolvedCog = resolvedCog->FindChildByName(element.mValue);
        if(!resolvedCog)
        {
          status.State = StatusState::Failure;
          if (!status.IgnoreMessage)
            status.Message = String::Format("The cog '%s' had no child named '%s'",
              previousCog->GetDescription().c_str(), element.mValue.c_str());
          return nullptr;
        }
        break;

      case CogPathElementType::Parent:
        resolvedCog = resolvedCog->GetParent();
        if(!resolvedCog)
        {
          status.State = StatusState::Failure;
          if (!status.IgnoreMessage)
            status.Message = String::Format("The cog '%s' has no parent",
              previousCog->GetDescription().c_str());
          return nullptr;
        }
        break;

      default:
        Error("Unhandled case in traversing a cog path");
        break;
    }

    ErrorIf(!resolvedCog, "We should have exited out earlier if we failed to find a cog");
    ++elementIndex;
  }

  return resolvedCog;
}

Cog* CogPath::Resolve(Status& status, Cog* startFrom, StringParam path)
{
  CogPathParser parser;
  parser.mTokenizer.mPath = path;

  CogPathCompiled finalPath;
  parser.Parse(status, finalPath);

  // If we ever failed during tokenizing or parsing...
  if(status.Failed())
  {
    status.Context = CogPathErrorCode::ParseError;
    return nullptr;
  }

  // We set the error code assuming that if an error occurs below, it will be this kind
  // Technically the status could still return success, even with this error code set (which is what we want)
  status.Context = CogPathErrorCode::DidNotFind;
  return Resolve(status, startFrom, finalPath);
}

CogPathNode::CogPathNode()
{
  mFlags.SetFlag(CogPathFlags::UpdateCogOnPathChange);
  mFlags.SetFlag(CogPathFlags::UpdatePathOnCogChange);
  mFlags.SetFlag(CogPathFlags::UpdateCogOnInitialize);

  mPathPreference0 = CogPathPreference::CogRelative;
  mPathPreference1 = CogPathPreference::SpaceRelative;
  mPathPreference2 = CogPathPreference::Absolute;
}

CogPath::CogPath() :
  mSharedNode(new CogPathNode())
{
}

CogPath::CogPath(StringParam path) :
  mSharedNode(new CogPathNode())
{
  mSharedNode->mPath = path;
}

Component* CogPath::QueryComponentId(BoundType* typeId)
{
  Cog* cog = GetCog();
  if(cog == nullptr)
    return nullptr;

  return cog->QueryComponentType(typeId);
}

CogPath CogPath::Clone()
{
  CogPath newPath;
  CogPathNode& clonedNode = newPath.mSharedNode;
  CogPathNode& ourNode = this->mSharedNode;

  clonedNode.mFlags = ourNode.mFlags;
  
  clonedNode.mPathPreference0 = ourNode.mPathPreference0;
  clonedNode.mPathPreference1 = ourNode.mPathPreference1;
  clonedNode.mPathPreference2 = ourNode.mPathPreference2;
  
  clonedNode.mResolvedCog = ourNode.mResolvedCog;
  clonedNode.mPath = ourNode.mPath;
  clonedNode.mRelativeTo = ourNode.mRelativeTo;
  return newPath;
}

Cog* CogPath::RestoreLink(CogInitializer& initializer, Component* component, StringParam propertyName)
{
  return RestoreLink(initializer, component->GetOwner(), component, propertyName);
}

Cog* CogPath::RestoreLink(CogInitializer& initializer, Cog* owner, StringParam propertyName)
{
  return RestoreLink(initializer, owner, nullptr, propertyName);
}

Cog* CogPath::RestoreLink(CogInitializer& initializer, Cog* owner, Component* component, StringParam propertyName)
{
  ErrorIf(owner == nullptr, "Invalid owner given to CogPath's RestoreLink (needed for relative paths)");
  if (owner != nullptr)
    mSharedNode->mRelativeTo = owner;

  Zero::RestoreLink(&mSharedNode->mResolvedCog, initializer.Context, component, propertyName, GetErrorOnDirectLinkFail());
  Cog* newCog = mSharedNode->mResolvedCog;
  if (newCog)
  {
    CogPathEvent toSend(*this);
    DispatchEvent(Events::CogPathCogChanged, &toSend);
  }
  else if (GetUpdateCogOnInitialize())
  {
    Refresh();
  }

  // If we have a resolved cog, but no path...
  if (GetUpdatePathOnCogChange() && mSharedNode->mPath.Empty())
  {
    Cog* cog = mSharedNode->mResolvedCog;
    if (cog != nullptr)
    {
      Status status;
      status.IgnoreMessage = true;
      mSharedNode->mPath = ComputePath(status, mSharedNode->mRelativeTo, cog, mSharedNode->mPathPreference0, mSharedNode->mPathPreference1, mSharedNode->mPathPreference2);
    }
  }

  return mSharedNode->mResolvedCog;
}

EventDispatcher* CogPath::GetDispatcherObject()
{
  return mSharedNode->mEvents.GetDispatcherObject();
}

EventReceiver* CogPath::GetReceiverObject()
{
  return mSharedNode->mEvents.GetReceiverObject();
}

void CogPath::DispatchEvent(StringParam eventId, Event* event)
{
  mSharedNode->mEvents.DispatchEvent(eventId, event);
}

Cog* CogPath::GetCogOrNull(CogPath* path)
{
  if(path == nullptr)
    return nullptr;

  return path->GetCog();
}

void CogPath::SetDirectCog(Cog* cog)
{
  SetCog(cog);
}

Cog* CogPath::GetDirectCog()
{
  return mSharedNode->mResolvedCog;
}

void CogPath::SetCog(Cog* to)
{
  Cog* previousCog = mSharedNode->mResolvedCog;
  mSharedNode->mResolvedCog = to;

  if(GetUpdatePathOnCogChange())
  {
    Cog* from = mSharedNode->mRelativeTo;
  
    Status status;
    status.IgnoreMessage = (GetErrorOnPathCantCompute() == false);
    String newPath = ComputePath(status, from, to, mSharedNode->mPathPreference0, mSharedNode->mPathPreference1, mSharedNode->mPathPreference2);

    if(OperationQueue::IsListeningForSideEffects() && !OperationQueue::sSideEffectContextStack.Empty())
      OperationQueue::RegisterSideEffect(this, "Path", mSharedNode->mPath);

    if(status.Succeeded())
    {
      mSharedNode->mPath = newPath;
    }
    else
    {
      mSharedNode->mPath.Clear();
      if (!status.Message.Empty())
        DoNotifyException("Cog Path", status.Message);
    }

    // Before dispatching an event and allowing other modifications to happen, we need to pop our
    // local context to allow for properties outside of the context of this CogPath to be made
    if (OperationQueue::IsListeningForSideEffects())
      OperationQueue::PopSubPropertyContext();
  }

  if(previousCog != to)
  {
    CogPathEvent toSend(*this);
    DispatchEvent(Events::CogPathCogChanged, &toSend);
  }
}

Cog* CogPath::GetCog()
{
  Cog* cog = mSharedNode->mResolvedCog;
  if(cog)
    return cog;

  Refresh();
  return mSharedNode->mResolvedCog;
}

void CogPath::SetPath(StringParam path)
{
  mSharedNode->mPath = path;

  if (GetUpdateCogOnPathChange())
    Refresh();
}

StringParam CogPath::GetPath()
{
  return mSharedNode->mPath;
}

Cog* CogPath::GetRelativeTo()
{
  return mSharedNode->mRelativeTo;
}

void CogPath::SetRelativeTo(Cog* cog)
{
  mSharedNode->mRelativeTo = cog;
}

bool CogPath::GetErrorOnResolveToNull()
{
  return mSharedNode->mFlags.IsSet(CogPathFlags::ErrorOnResolveToNull);
}

void CogPath::SetErrorOnResolveToNull(bool state)
{
  mSharedNode->mFlags.SetState(CogPathFlags::ErrorOnResolveToNull, state);
}

bool CogPath::GetErrorOnPathCantCompute()
{
  return mSharedNode->mFlags.IsSet(CogPathFlags::ErrorOnPathCantCompute);
}

void CogPath::SetErrorOnPathCantCompute(bool state)
{
  mSharedNode->mFlags.SetState(CogPathFlags::ErrorOnPathCantCompute, state);
}

bool CogPath::GetErrorOnDirectLinkFail()
{
  return mSharedNode->mFlags.IsSet(CogPathFlags::ErrorOnDirectLinkFail);
}

void CogPath::SetErrorOnDirectLinkFail(bool state)
{
  mSharedNode->mFlags.SetState(CogPathFlags::ErrorOnDirectLinkFail, state);
}

bool CogPath::GetUpdateCogOnPathChange()
{
  return mSharedNode->mFlags.IsSet(CogPathFlags::UpdateCogOnPathChange);
}

void CogPath::SetUpdateCogOnPathChange(bool state)
{
  mSharedNode->mFlags.SetState(CogPathFlags::UpdateCogOnPathChange, state);
}

bool CogPath::GetUpdatePathOnCogChange()
{
  return mSharedNode->mFlags.IsSet(CogPathFlags::UpdatePathOnCogChange);
}

void CogPath::SetUpdatePathOnCogChange(bool state)
{
  mSharedNode->mFlags.SetState(CogPathFlags::UpdatePathOnCogChange, state);
}

bool CogPath::GetUpdateCogOnInitialize()
{
  return mSharedNode->mFlags.IsSet(CogPathFlags::UpdateCogOnInitialize);
}

void CogPath::SetUpdateCogOnInitialize(bool state)
{
  mSharedNode->mFlags.SetState(CogPathFlags::UpdateCogOnInitialize, state);
}

CogPathPreference::Enum CogPath::GetPathPreference0()
{
  return mSharedNode->mPathPreference0;
}

void CogPath::SetPathPreference0(CogPathPreference::Enum value)
{
  mSharedNode->mPathPreference0 = value;
}

CogPathPreference::Enum CogPath::GetPathPreference1()
{
  return mSharedNode->mPathPreference1;
}

void CogPath::SetPathPreference1(CogPathPreference::Enum value)
{
  mSharedNode->mPathPreference1 = value;
}

CogPathPreference::Enum CogPath::GetPathPreference2()
{
  return mSharedNode->mPathPreference2;
}

void CogPath::SetPathPreference2(CogPathPreference::Enum value)
{
  mSharedNode->mPathPreference2 = value;
}

bool CogPath::Refresh()
{
  Cog* relativeTo = mSharedNode->mRelativeTo;
  Status status;

  Cog* previousCog = mSharedNode->mResolvedCog;
  mSharedNode->mResolvedCog = CogPath::Resolve(status, relativeTo, mSharedNode->mPath);

  if(status.Failed())
  {
    // If there was a parse error with the path...
    if (status.Context == CogPathErrorCode::ParseError)
    {
      DoNotifyException("Cog Path", status.Message);
    }
    // Otherwise, if the object could not be resolved...
    else if (GetErrorOnResolveToNull() && !mSharedNode->mFlags.IsSet(CogPathFlags::ResolvedNullErrorOccurred))
    {
      mSharedNode->mFlags.SetFlag(CogPathFlags::ResolvedNullErrorOccurred);
      String message = BuildString(status.Message, "\nYou can disable 'ErrorOnResolveToNull' to stop this message from occurring");
      DoNotifyException("Cog Path", message);
    }
  }
  else
  {
    mSharedNode->mFlags.ClearFlag(CogPathFlags::ResolvedNullErrorOccurred);
  }

  if(previousCog != mSharedNode->mResolvedCog)
  {
    CogPathEvent toSend(*this);
    DispatchEvent(Events::CogPathCogChanged, &toSend);
    return true;
  }

  return false;
}

Cog* CogPath::Resolve(Cog* startFrom, StringParam path)
{
  Status status;
  status.IgnoreMessage = true;
  return Resolve(status, startFrom, path);
}

String CogPath::ComputePath(Cog* from, Cog* to, CogPathPreference::Enum pref)
{
  Status status;
  status.IgnoreMessage = true;
  return ComputePath(status, from, to, pref);
}

bool CogPath::RefreshIfNull()
{
  Cog* oldCog = mSharedNode->mResolvedCog;
  Cog* newCog = GetCog();
  return newCog != oldCog;
}

ZilchDefineType(CogPathEvent, builder, type)
{
  ZilchBindFieldProperty(mCogPath);
}

CogPathEvent::CogPathEvent()
{
}

CogPathEvent::CogPathEvent(CogPath& path)
{
  mCogPath = path;
}

namespace Serialization
{
  bool Policy<CogPath>::Serialize(Serializer& stream, cstr fieldName, CogPath& path)
  {
    CogPathNode& value = path.mSharedNode;

    // Support for the old version of CogPaths, which just used strings (only for loading)
    // We also support changing CogIds into CogPaths
    if(stream.GetMode() == SerializerMode::Loading && stream.GetClass() == SerializerClass::DataTreeLoader)
    {
      DataTreeLoader& loader = (DataTreeLoader&)stream;
      DataNode* parent = loader.GetCurrent();
      DataNode* node = parent->FindChildWithName(fieldName);

      if (node != nullptr && node->mNodeType == DataNodeType::Value)
      {
        // If the old type name was 'CogPath', then the string portion was just the path
        if(node->mTypeName == "CogPath" || node->mTypeName == "string")
        {
          value.mPath = node->mTextValue;
          return true;
        }
        // If this is a cog-id then use the policy to de-serialize it
        else if (node->mTypeName == "uint")
        {
          // Also see if this is just a cog-id being upgraded
          if (Policy<CogId>::Serialize(stream, fieldName, value.mResolvedCog))
            return true;
        }
      }
    }

    // Cog paths save full objects which include the path, any flags we set, and the actual cog we resolved to
    // It may or may not be possible to resolve the cog, but we attempt to save it anyways
    // Note that this means we must have OnAllObjectsCreated call on the CogPath, just like a serialized CogId
    if (stream.Start("CogPath", fieldName, StructureType::Object))
    {
      stream.SerializeFieldDefault("Path", value.mPath, String());
      SerializeBits(stream, value.mFlags, CogPathFlags::Names, 0,
        CogPathFlags::UpdateCogOnPathChange | CogPathFlags::UpdatePathOnCogChange | CogPathFlags::UpdateCogOnInitialize);
      stream.SerializeFieldDefault("ResolvedCog", value.mResolvedCog, CogId());
      if (!stream.EnumField("CogPathPreference", "PathPreference0", (uint&)value.mPathPreference0, ZilchTypeId(CogPathPreference::Enum)))
        value.mPathPreference0 = CogPathPreference::CogRelative;
      if (!stream.EnumField("CogPathPreference", "PathPreference1", (uint&)value.mPathPreference1, ZilchTypeId(CogPathPreference::Enum)))
        value.mPathPreference1 = CogPathPreference::SpaceRelative;
      if (!stream.EnumField("CogPathPreference", "PathPreference2", (uint&)value.mPathPreference2, ZilchTypeId(CogPathPreference::Enum)))
        value.mPathPreference2 = CogPathPreference::Absolute;
      stream.End("CogPath", StructureType::Object);
      
      // Never load the 'error already occurred' flag, as we just use that so we don't spam the user with exceptions
      if(stream.GetMode() == SerializerMode::Loading)
        value.mFlags.ClearFlag(CogPathFlags::ResolvedNullErrorOccurred);
      return true;
    }
    return false;
  }
}

bool CogPathMetaSerialization::SerializeReferenceProperty(BoundType* meta, cstr fieldName, Any& value, Serializer& serializer)
{
  // If we are saving a cog path
  if (serializer.GetMode() == SerializerMode::Saving)
  {
    CogPath* pathInVariant = value.Get<CogPath*>();

    // If the cog path is not valid inside the variant we got, that either means we didn't yet have the variant, or it was of another type
    if (pathInVariant == nullptr)
      value = CogPath();

    // Save out the cog path
    pathInVariant = value.Get<CogPath*>();
    Serialization::Policy<CogPath>::Serialize(serializer, fieldName, *pathInVariant);
    return true;
  }
  // Otherwise, we are loading up a cog path!
  else
  {
    // If we already have the cog path, then we really just want to directly serialize into that one
    // This should even work if its on a script object since the internals are reference counted/shared
    CogPath* pathInVariant = value.Get<CogPath*>();
    if(pathInVariant == nullptr)
    {
      // Allocate the CogPath for them if they don't have it already.
      pathInVariant = new CogPath( );
      value = pathInVariant;
    }
    else
    {
      // For now, we always clone the path when loading to make sure we never share instances of a path
      value = pathInVariant->Clone( );
      pathInVariant = value.Get<CogPath*>( );
    }

    // Now run our serialization on the path
    Serialization::Policy<CogPath>::Serialize(serializer, fieldName, *pathInVariant);

    return true;
  }
}

void CogPathMetaSerialization::SetDefault(Type* type, Any& any)
{
  any = CogPath();
}

bool CogPathMetaSerialization::ConvertFromString(StringParam input, Any& output)
{
  output = CogPath(input);
  return true;
}

}
