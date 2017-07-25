///////////////////////////////////////////////////////////////////////////////
///
/// \file CogPath.hpp
/// Declaration of CogPath class.
///
/// Authors: TrevorSundberg
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
namespace Events
{
  /// Occurs when any change is made to the cog path that could affect which object it points at
  DeclareEvent(CogPathCogChanged);
}

class Cog;

DeclareEnum6(StatusCodeCogPath, OnlySupportsCogTypes, SpaceNotNamed, CogNotNamed, CogsInDifferentSpaces, RelativeToNotSet, CogsDoNotShareRoots);

DeclareEnum8(CogPathTokenType, Invalid, Eof, Separator, Self, Parent, CurrentSpace, NamedCog, NamedSpace);

class CogPathToken
{
public:
  CogPathToken();

  static cstr GetFixedTokenText(CogPathTokenType::Enum fixedToken);
  String mText;
  CogPathTokenType::Enum mType;
};

class CogPathTokenizer
{
public:
  CogPathTokenizer();
  void ReadToken(CogPathToken& tokenOut);

  String mPath;
  size_t mPosition;
};

DeclareEnum4(CogPathRootType, NamedSpace, CurrentSpace, Relative, Null);

DeclareEnum2(CogPathElementType, NamedCog, Parent);

class CogPathElement
{
public:
  CogPathElement();

  CogPathElementType::Enum mType;
  String mValue;
};

class CogPathCompiled
{
public:
  CogPathCompiled();

  // The name of the space to start from (empty if we're accessing children or the same space)
  String mSpaceName;

  // Whether this path is defined starting from a named space, the current space, or relative
  CogPathRootType::Enum mRootType;

  // The chain (or trail) of things we traverse
  // We use empty strings to represent moving up a parent,
  // and filled strings to mean moving down a child of that name
  // Also note, if the elements are empty and the root type is Relative, this means 'self'
  // All other forms of empty elements with non relative root types are NOT legal
  Array<CogPathElement> mElements;
};

class CogPathParser
{
public:
  
  // Call this to parse a full path out!
  void Parse(Status& status, CogPathCompiled& output);
  
  // Internal
  bool Accept(CogPathTokenType::Enum type, String* output);
  bool Expect(CogPathTokenType::Enum type, String* output, Status& status, StringParam error);
  
  bool ParseSeparatedElement(Status& status, CogPathCompiled& output);
  bool ParseElement(Status& status, CogPathCompiled& output);
  bool ParsePath(Status& status, CogPathCompiled& output);

  CogPathTokenizer mTokenizer;
  CogPathToken mToken;
};

DeclareBitField7(CogPathFlags,
  ErrorOnResolveToNull,
  ErrorOnPathCantCompute,
  ErrorOnDirectLinkFail,
  UpdateCogOnPathChange,
  UpdatePathOnCogChange,
  UpdateCogOnInitialize,
  ResolvedNullErrorOccurred);
DeclareEnum3(CogPathPreference, CogRelative, SpaceRelative, Absolute);

class CogPathNode : public ReferenceCountedEventObject
{
public:
  CogPathNode();

  EventObject mEvents;
  BitField<CogPathFlags::Enum> mFlags;

  CogPathPreference::Enum mPathPreference0;
  CogPathPreference::Enum mPathPreference1;
  CogPathPreference::Enum mPathPreference2;

  CogId mResolvedCog;
  String mPath;
  CogId mRelativeTo;
};

class CogPath : public Object
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  CogPath();
  CogPath(StringParam path);

  // Events
  EventDispatcher* GetDispatcherObject() override;
  EventReceiver* GetReceiverObject() override;
  void DispatchEvent(StringParam eventId, Event* event);

  /// A safe and simple way to get a cog from a cog path (the cog path itself can be null)
  static Cog* GetCogOrNull(CogPath* path);

  /// Component interface
  Component* QueryComponentId(BoundType* typeId);

  /// Type safe way of accessing components.
  template<typename type>
  type* Has();

  /// Creates a new copy of a cog path (since cog paths are reference counted and shared)
  CogPath Clone();

  /// Use to Restore links in serialization
  Cog* RestoreLink(CogInitializer& initializer, Component* component, StringParam propertyName);
  Cog* RestoreLink(CogInitializer& initializer, Cog* owner, StringParam propertyName);
  Cog* RestoreLink(CogInitializer& initializer, Cog* owner, Component* component, StringParam propertyName);
  
  /// Setting the cog manually may recompute the path if the option is set
  /// Getting the cog will return whatever cog we already resolved, or null (it will not attempt to resolve)
  void SetDirectCog(Cog* cog);
  Cog* GetDirectCog();

  /// Setting the cog manually may recompute the path if the option is set
  /// Getting the cog will attempt to resolve the cog if we don't already have one (or if the path options is set, it will always resolve)
  void SetCog(Cog* cog);
  Cog* GetCog();

  /// Setting the path will invalidate the object until the next call to GetCog
  void SetPath(StringParam path);
  StringParam GetPath();

  /// The cog that we compute paths relative to.
  Cog* GetRelativeTo();
  void SetRelativeTo(Cog* cog);

  /// Is an exception thrown if you try to access the Cog when it's invalid or not found?
  bool GetErrorOnResolveToNull();
  void SetErrorOnResolveToNull(bool state);

  /// Is it an exception/notification if the path to an object cannot be computed?
  bool GetErrorOnPathCantCompute();
  void SetErrorOnPathCantCompute(bool state);
  
  /// Is it an exception/notification if a direct link to the object cannot be resolved?
  bool GetErrorOnDirectLinkFail();
  void SetErrorOnDirectLinkFail(bool state);

  /// When we set the cog path, should we try and resolve the object (this also detects parse errors)
  bool GetUpdateCogOnPathChange();
  void SetUpdateCogOnPathChange(bool state);

  /// When we set the cog, should we try and recompute a path to the object?
  bool GetUpdatePathOnCogChange();
  void SetUpdatePathOnCogChange(bool state);

  /// Whether the cog path attempts to resolve an object when the object is fully initialized
  bool GetUpdateCogOnInitialize();
  void SetUpdateCogOnInitialize(bool state);

  CogPathPreference::Enum GetPathPreference0();
  void SetPathPreference0(CogPathPreference::Enum value);
  CogPathPreference::Enum GetPathPreference1();
  void SetPathPreference1(CogPathPreference::Enum value);
  CogPathPreference::Enum GetPathPreference2();
  void SetPathPreference2(CogPathPreference::Enum value);

  /// Returns true if the object changes, false otherwise
  bool Refresh();

  /// Returns true if the object changes, false otherwise
  bool RefreshIfNull();

  /// Resolves a cog from a path and a a relative object (or null for absolute paths)
  /// Returns null if it fails to find the cog, and will not throw an exception or assert
  static Cog* Resolve(Cog* startFrom, StringParam path);

  /// Computes a path from one object to another (or an absolute path if specified - 'from' can be null)
  /// If computing the path fails, this will return an empty string
  static String ComputePath(Cog* from, Cog* to, CogPathPreference::Enum pref);

  static String ComputePath(Status& status, Cog* from, Cog* to, CogPathPreference::Enum pref0, CogPathPreference::Enum pref1, CogPathPreference::Enum pref2);
  static String ComputePath(Status& status, Cog* from, Cog* to, CogPathPreference::Enum pref);

  static Cog* Resolve(Status& status, Cog* startFrom, StringParam path);
  static Cog* Resolve(Status& status, Cog* startFrom, const CogPathCompiled& path);

  HandleOf<CogPathNode> mSharedNode;
};

template<typename type>
inline type* CogPath::Has()
{
  return (type*)QueryComponentId(ZilchTypeId(type));
}

class CogPathEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  CogPathEvent();
  CogPathEvent(CogPath& path);
  CogPath mCogPath;
};

class CogPathMetaSerialization : public MetaSerialization
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  bool SerializeReferenceProperty(BoundType* meta, cstr fieldName, Any& value, Serializer& serializer) override;
  void SetDefault(Type* type, Any& any) override;
  bool ConvertFromString(StringParam input, Any& output) override;
};

namespace Serialization
{
  template<>
  struct Policy<CogPath>
  {
    static bool Serialize(Serializer& stream, cstr fieldName, CogPath& value);
  };
}
}
