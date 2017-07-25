///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
  DeclareEvent(CastFilterCallback);
}//namespace Events

struct CogId;
class Cog;
class Collider;
class CollisionGroup;

/// Controls custom cast filtering via the CastFilterEvent. Allows custom filter logic per object.
/// <param name="Accept">Always accept this object for testing.</param>
/// <param name="Reject">Always reject this object for testing.</param>
/// <param name="DefaultBehavior">Run the rest of the filtering logic on the cast filter.</param>
DeclareEnum3(CastFilterState, Accept, Reject, DefaultBehavior);

//-------------------------------------------------------------------CastFilterEvent
/// Allows a user to filter out an object during any cast in physics.
struct CastFilterEvent : public Event
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  CastFilterEvent();
  /// The object being tested in this cast.
  Cog* GetObject();

  /// When filtering should we accept or reject this cog?
  /// Alternatively we can let the default cast filter logic run.
  CastFilterState::Enum mFilterState;

  Collider* mCollider;
};

//-------------------------------------------------------------------CastFilter
/// Filter for casting operations in physics. Allows basic filtering such as
/// static or dynamic objects, advanced filters such as collision groups, and
/// custom filters via an event callback.
struct CastFilter : public BaseCastFilter
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  CastFilter();

  bool IsValid(void* clientData) override;

  /// Should this cast behave like it belongs to a collision group?
  /// Uses the current space's CollisionTable for filtering logic.
  CollisionGroup* GetCollisionGroup();
  void SetCollisionGroup(CollisionGroup* group);

  /// A cog to ignore during casts
  Cog* GetIgnoreCog() const;
  void SetIgnoreCog(Cog* cog);

  CollisionGroup* mFilterGroup;

  /// An object to invoke a callback on (via the callback name)
  /// to see if an object in a cast should be skipped.
  Object* mCallbackObject;
  /// The name of the event to invoke on the callback object.
  String mCallbackEventName;
};

//-------------------------------------------------------------------CastResult
/// A result from a cast operation on a PhysicsSpace.
struct CastResult
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  CastResult();
  CastResult(const CastResult& rhs);
  ~CastResult(){}

  void operator=(const CastResult& rhs);

  /// Returns the id of the hit cog.
  CogId GetCogId();
  /// The collider hit by the cast.
  Collider* GetCollider();
  /// The cog hit by the cast.
  Cog* GetObjectHit();
  /// Returns the local-space position that the object was hit. The point index is used
  /// to get the first or last point of intersection. Invalid on volume casts.
  Vec3 GetLocalPosition(uint pointIndex);
  /// Returns the world-space position that the object was hit. Invalid on a volume cast.
  Vec3 GetWorldPosition();
  /// The normal of the object at the intersection point (world space).
  /// Invalid on a volume cast.
  Vec3 GetNormal();
  /// The distance from the ray/segment start to the point of intersection.
  /// Invalid on a volume cast.
  real GetDistance();

  Collider* mObjectHit;
  Vec3 mPoints[2];
  Vec3 mContactNormal;
  real mTime;
  uint mShapeIndex;
};

#ifdef SupportsStaticAsserts
static_assert(sizeof(CastResult)==sizeof(ProxyResult), 
  "Size of CastResult must be the same size of ProxyResult.");
#endif

typedef Array<CastResult> CastResultArray;

//-------------------------------------------------------------------CastResults
///The public interface to ray casting.
class CastResults
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  typedef CastResultArray::range range;
  typedef CastFilter Filter;

  static CastFilter mDefaultFilter;

  ///Default the amount of objects to retrieve to 1.
  CastResults(uint amount = 1, BaseCastFilter& filter = mDefaultFilter);
  CastResults(const CastResults& rhs);
  ~CastResults(){}

  ///Returns a reference to the cast result at the given index.
  CastResult& operator[](uint index);
  const CastResult& operator[](uint index) const;
  
  ///Returns the amount of objects retrieved.
  uint Size() const;

  ///Returns whether or not there were any objects found.
  bool Empty();

  ///Returns a range of all objects the ray hit (within the max).
  range All();
  
  ///Clears
  void Clear();

  ///Resizes the number of results
  void Resize(uint amount);

  ///Returns the capacity (maximum amount of objects returned).
  uint Capacity() const;

private:
  ///The physics space needs access to the results class, 
  ///but it should not be available to the public.
  friend class PhysicsSpace;

  ///When we pass mResults to the broad phase, it will fill up mArray with
  ///Broad Phase Proxies instead of Collider's.  This walks through them and replaces
  ///the pointer with a pointer to the Collider.
  void ConvertToColliders();

  ///Holds the results
  CastResultArray mArray;
  ///Has a pointer to the results.  Manages the insertion of ray cast contacts.
  ProxyCastResults mResults;
};

//-------------------------------------------------------------------CastResultsRange
struct CastResultsRange
{
  typedef CastResult value_type;
  typedef CastResult& return_type;
  typedef CastResult& FrontResult;
  CastResultsRange(){}
  CastResultsRange(const CastResults& castResults);
  CastResultsRange(const CastResultsRange& rhs);

  bool Empty();
  CastResult& Front();
  void PopFront();
  uint Size();

  CastResultArray::range mRange;
  CastResultArray mArray;
};

}//namespace Zero
