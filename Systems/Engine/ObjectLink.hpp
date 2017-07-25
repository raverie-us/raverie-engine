///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
  DeclareEvent(ObjectLinkChanged);
  DeclareEvent(ObjectLinkPointChanged);
}//namespace Events

class ObjectLink;
class ObjectLinkAnchor;
class ObjectLinkRange;

//-------------------------------------------------------------------ObjectLinkEdge
/// A directed edge between a cog and an object link.
class ObjectLinkEdge
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ObjectLinkEdge();

  bool IsValid() const;

  // Property interface

  /// The cog that this edge on the object link is connected to.
  Cog* GetSelfCog();
  /// The other cog that the object link is connected to (the one not part of this edge).
  Cog* GetOtherCog();
  /// The object link that this edge is connected to.
  ObjectLink* GetObjectLink();  

private:
  friend class ObjectLinkAnchor;
  friend class ObjectLink;
  friend class ObjectLinkRange;

  IntrusiveLink(ObjectLinkEdge, EdgeLink);

  void Set(ObjectLink* link, Cog* cog);
  void Clear();
  
  HandleOf<ObjectLink> mObjectLinkHandle;
  HandleOf<ObjectLinkAnchor> mSelfAnchorHandle;
};

//-------------------------------------------------------------------ObjectLinkRange
/// A range to iterate over all ObjectLinkEdges on a cog.
/// An edge allows you to go between on object link and both cogs in the link.
class ObjectLinkRange
{
public:
  typedef ObjectLinkRange self_type;
  typedef ObjectLinkEdge value_type;
  typedef ObjectLinkEdge& FrontResult;

  ObjectLinkRange();
  ObjectLinkRange(Cog* cog);
  ObjectLinkRange(ObjectLinkAnchor* anchor);

  bool Empty();
  void PopFront();
  ObjectLinkEdge& Front();

  typedef InList<ObjectLinkEdge, &ObjectLinkEdge::EdgeLink> EdgeList;
  typedef EdgeList::range EdgeRange;

private:
  EdgeList::range mRange;
};

//-------------------------------------------------------------------ObjectLinkAnchor
/// Component used to keep track of what ObjectLinks a cog has. This component
/// is added dynamically whenever a ObjectLink is added to a cog that did not
/// contain an ObjectLink before. This can be used to traverse across linked
/// objects and find the entire "island" of ObjectLinks.
class ObjectLinkAnchor : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Component Interface
  void OnDestroy(uint flags) override;
  bool ShouldSerialize() override { return false; }

  /// The range of object link edges connected to this anchor.
  ObjectLinkRange GetObjectLinks();

  // Unlink this anchor from all object links
  void ClearLinks();

  typedef ObjectLinkRange::EdgeList EdgeList;
  EdgeList mEdges;
};

//-------------------------------------------------------------------ObjectLink
/// Forms a link between two positions on two objects. ObjectLinks are used primarily by physics
/// to represent joints, but can also be used by graphics, gameplay, etc...
/// to represent some connection between two objects.
class ObjectLink : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ObjectLink();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void OnAllObjectsCreated(CogInitializer& initializer) override;
  void OnDestroy(uint flags) override;

  // Unlink both from both objects.
  void Unlink();

  /// CogPath to object A.
  CogPath GetObjectAPath();
  void SetObjectAPath(CogPath& path);
  /// CogPath to object B.
  CogPath GetObjectBPath();
  void SetObjectBPath(CogPath& path);

  /// The first object that is being connected to. Set this to null to clear the link.
  Cog* GetObjectA();
  void SetObjectA(Cog* cog);
  /// The second object that is being connected to. Set this to null to clear the link.
  Cog* GetObjectB();
  void SetObjectB(Cog* cog);

  /// The Point on Object A in local space.
  Vec3 GetLocalPointA();
  void SetLocalPointA(Vec3 newPoint);
  /// The Point on Object B in local space.
  Vec3 GetLocalPointB();
  void SetLocalPointB(Vec3 newPoint);

  /// The point on object A in world space.
  Vec3 GetWorldPointA();
  void SetWorldPointA(Vec3Param worldPoint);
  /// The point on object B in world space.
  Vec3 GetWorldPointB();
  void SetWorldPointB(Vec3Param worldPoint);  

  // Returns a position used to represent the link. If both objects have at transform
  // this will be the center of the two points in world space. If only one object has a
  // transform then this will be the world point on that object. Otherwise this will just
  // be the center of the local points relative to the origin.
  Vec3 GetWorldPosition();

  // Internals

  enum ObjectIndex { IndexA = 0, IndexB = 1 };

  void OnObjectAChanged(Event* e);
  void OnObjectBChanged(Event* e);

  Cog* GetCog(ObjectIndex index);
  void SetCog(Cog* newCog, ObjectIndex index);
  void LinkCog(Cog* newCog, ObjectIndex index);

  void SetCogAInternal(Cog* cog);
  void SetCogBInternal(Cog* cog);
  void SetCogInternal(Cog* cog, ObjectIndex index);

  Transform* GetTransform(ObjectIndex index);

  // Get/Set a point in local space given an object index (internal)
  Vec3 GetLocalPoint(ObjectIndex index);
  void SetLocalPoint(Vec3Param localPoint, ObjectIndex index);
  // Get/Set a point in world space given an object index (internal)
  Vec3 GetWorldPoint(ObjectIndex index);
  void SetWorldPoint(Vec3Param worldPoint, ObjectIndex index);

  CogPath mObjectPaths[2];
  Vec3 mBodyPoints[2];
  ObjectLinkEdge mEdges[2];

  bool mValid;
};

//-------------------------------------------------------------------ObjectLinkEvent
/// An event sent when an object link changes one of its link edges.
class ObjectLinkEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ObjectLinkEvent();

  void Set(uint edgeIndex, Cog* oldCog, Cog* newCog);

  /// The new cog being set.
  Cog* NewCog;
  /// The old cog being overridden.
  Cog* OldCog;
  /// The index on the edge that is being overridden.
  uint EdgeId;
};

//-------------------------------------------------------------------ObjectLinkPointChangedEvent
class ObjectLinkPointChangedEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ObjectLinkPointChangedEvent();

  void Set(uint edgeIndex, Vec3Param oldPoint, Vec3Param newPoint);

  Vec3 mOldLocalPoint;
  Vec3 mNewLocalPoint;
  /// The index on the edge that is being overridden.
  uint mEdgeId;
};

}//namespace Zero
