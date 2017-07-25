///////////////////////////////////////////////////////////////////////////////
///
/// \file ProxyCast.hpp
/// Declaration of classes for Ray Casting.
/// 
/// Authors: Joshua Claeys
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareBitField8(BaseCastFilterFlags,
  IgnoreStatic,
  IgnoreDynamic,
  IgnoreKinematic,
  IgnoreGhost,
  IgnoreChildren,
  /// Determines whether or not to refine the cast. If this is false,
  /// it will use an approximation (using AABB's or bounding spheres).
  /// If it is true, it will check the actual geometry of the object.
  /// This is initially set.
  Refine,
  /// Fills out the contact normal in the cast result if set.
  GetContactNormal,
  /// Ignores a cast if it starts inside of the object
  IgnoreInternalCasts);

//------------------------------------------------------------------ Cast Filter
/// Used to filter objects during cast operations.
struct BaseCastFilter
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  BaseCastFilter();
  BaseCastFilter(const BaseCastFilter& rhs);
  virtual ~BaseCastFilter(){}

  #define FlagSetter(Name)                                                   \
  bool Get##Name() { return IsSet(BaseCastFilterFlags::Name); }              \
  void Set##Name(bool state) { SetState(BaseCastFilterFlags::Name, state); }

  FlagSetter(IgnoreStatic);
  FlagSetter(IgnoreDynamic);
  FlagSetter(IgnoreKinematic);
  FlagSetter(IgnoreGhost);
  FlagSetter(IgnoreChildren);


  #undef FlagSetter

  /// Checks if a flag is set.
  bool IsSet(BaseCastFilterFlags::Enum flag);
  /// Sets the passed in flag.
  void Set(BaseCastFilterFlags::Enum flag);
  /// Sets the passed in flag to the desired state
  void SetState(BaseCastFilterFlags::Enum flag, bool state);
  /// Clears the passed in flag.
  void ClearFlag(BaseCastFilterFlags::Enum flag);
  /// Sets an object to ignore during raycasts.
  void IgnoreCog(void* cog);

  /// Certain combinations of flags are not valid, therefore we must check for
  /// them and throw an error.
  void ValidateFlags();

  virtual bool IsValid(void* clientData) = 0;

  void* mIgnoredCog;

  BitField<BaseCastFilterFlags::Enum> mFlags;
};

template <size_t size1, size_t size2>
struct Max
{
  static const size_t Result = (size1 > size2) ? size1 : size2;
};

//-------------------------------------------------------------------- Cast Data
/// This is used to avoid code duplication for different casts into 
/// a broad phase.  Used to store data for Ray, Segment, Aabb, and Sphere casts.
/// NOTE: This structure is getting a little big due to the frustum.
/// Right now, it only exists once per cast and is passed around by
/// reference (never copied).
struct CastData
{
  /// NOTE: 'vec' can be either a direction or an end point.  It will be treated 
  /// differently depending on what is called (Ray or Segment cast).
  CastData(Vec3Param start, Vec3Param vec);
  CastData(const Ray& ray);
  CastData(const Segment& segment);
  CastData(const Aabb& aabb);
  CastData(const Sphere& sphere);
  CastData(const Frustum& frustum);

  Ray& GetRay();
  const Ray& GetRay() const;

  Segment& GetSegment();
  const Segment& GetSegment() const;

  Aabb& GetAabb();
  const Aabb& GetAabb() const;

  Sphere& GetSphere();
  const Sphere& GetSphere() const;

  Frustum& GetFrustum();
  const Frustum& GetFrustum() const;

  char bytes[Max<Max<Max<Max<sizeof(Ray), sizeof(Segment)>::Result, sizeof(Aabb)>::Result, sizeof(Sphere)>::Result, sizeof(Frustum)>::Result];
};

typedef const CastData& CastDataParam;

//----------------------------------------------------------------- ProxyResult
/// Stores a single result from a broadphase query.
struct ProxyResult
{
  ProxyResult() {}
  ProxyResult(void* proxy, Vec3Param p1, Vec3Param p2,
              Vec3Param contactNormal, real time);

  void operator=(const ProxyResult& rhs);
  bool operator==(const ProxyResult& rhs);
  bool operator!=(const ProxyResult& rhs);

  void Set(void* proxy, Vec3 points[2], 
           Vec3Param contactNormal, real time);

  void* mObjectHit;

  Vec3 mPoints[2];
  Vec3 mContactNormal;

  union
  {
    /// Time of collision for a ray cast.
    real mTime;

    /// Distance to the center of the volume cast.
    /// For a cone cast, it is the distance from the point of the cone.
    real mDistance;
  };

  /// The index of the sub-item hit in a complex shape. Aka, the triangle
  /// index in the mesh (not the index into the index buffer, it's a
  /// value from 0 to the # of tris)
  uint ShapeIndex;
};

typedef Array<ProxyResult> ProxyCastResultArray;

//------------------------------------------------------- ProxyCastResults
/// Internal wrapper around an array of cast results.
/// Manages adding cast results to the array and keeping them in order
/// (as well as maintaining the correct size).
struct ProxyCastResults
{
  ProxyCastResults(ProxyCastResultArray& array, BaseCastFilter& filter);

  /// Attempts to Insert a proxy.  It is inserted in order by shortest time.
  /// If the array is already filled, it will either replace one already or reject the insertion.
  bool Insert(ProxyResult& proxyResult);
  /// Note: time is either time or distance. 
  bool Insert(void* mObjectHit, Vec3 points[2], 
              Vec3Param normal, real time);
  /// For volume casting (points are defaulted to Vec3(0,0,0)).
  bool Insert(void* mObjectHit, real distance);

  /// Returns the total amount of proxies to be returned for the ray cast.
  uint GetProxyCount() {return Results.Size();}
  /// Returns the current amount of proxies that the cast has collided with.
  uint GetCurrentSize() const {return CurrSize;}
  /// Returns the remaining amount of new proxies to be accepted.
  /// You can still Insert when this returns 0, but it will either deny 
  /// it (if it's time is too long), or replace one with it.
  uint GetRemainingSize() {return Results.Size() - CurrSize;}

  /// Merges two sets of results.  Used for combining separate casts into
  /// Dynamic and Static broad phases for optimizations.
  void Merge(ProxyCastResults& rhs);

  /// Returns whether or not the passed in results contain the same data.
  bool Match(ProxyCastResults& rhs);

  /// For simple access to the correct range type.
  typedef Array<ProxyResult>::range range;

  /// Returns a range of all objects the ray hit (within the specified max).
  range All();

  /// Clears all data.
  void Clear();

  uint CurrSize;
  /// Filters out possible results.
  BaseCastFilter& Filter;
  ProxyCastResultArray& Results;

private:
  bool IsValid(void* mObjectHit);
};

}//namespace Zero
