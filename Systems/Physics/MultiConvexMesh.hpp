///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2014-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareBitField2(MultiConvexMeshFlags, NewlyCreatedInEditor, EditType2D);

class MultiConvexMesh;
class SubConvexMesh;

//-------------------------------------------------------------------MultiConvexMeshVertexData
class MultiConvexMeshVertexData : public BoundMeshData<MultiConvexMesh, Vec3>
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
};

//-------------------------------------------------------------------MultiConvexMeshIndexData
class MultiConvexMeshIndexData : public BoundMeshData<MultiConvexMesh, uint>
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
};

//-------------------------------------------------------------------MultiConvexMeshSubMeshData
class MultiConvexMeshSubMeshData : public BoundMeshData<MultiConvexMesh, SubConvexMesh*>
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  typedef MultiConvexMeshSubMeshData SelfType;
  typedef BoundMeshDataRange<SelfType> RangeType;

  /// Create and add a new SubConvexMesh. Returns the new mesh for modification.
  SubConvexMesh* Add();
  /// Remove the sub-mesh at the given index.
  void RemoveAt(int arrayIndex);
  /// Clears all sub-meshes.
  void Clear();
  RangeType All();
};

//-------------------------------------------------------------------SubConvexMesh
/// Contains the indices of a convex mesh. The triangle indices are the primary method
/// to configure this (required for mass computations). The regular indices are used
/// for debug drawing and to reduce intersection tests by removing duplicate points. 
/// If left empty, Indices will be auto-filled from the triangle indices.
class SubConvexMesh : public SafeId32Object
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  typedef Array<Vec3> VertexArray;
  typedef const Array<Vec3>& VertexArrayParam;
  typedef Array<uint> IndexArray;

  SubConvexMesh();
  void Serialize(Serializer& stream);

  void SetOwner(MultiConvexMesh* owner);
  bool Validate(VertexArrayParam verts);
  bool ValidateInternal(VertexArrayParam verts);
  /// Compute the mesh indices (for debug drawing in 2d and support functions) from the triangle indices.
  void ComputeUniqueIndices();

  void ComputeCenterOfMassAndVolume(VertexArrayParam verts);
  Mat3 ComputeInertiaTensor(VertexArrayParam verts, Vec3Param centerOfMass, Vec3Param scale);
  Aabb ComputeAabb(VertexArrayParam verts);

  // Gjk/Mpr interface functions
  /// Find the point furthest in the given direction.
  void Support(VertexArrayParam verts, Vec3Param direction, Vec3Ptr support);
  /// Some center point of the mesh (the center of mass in this case).
  Vec3 GetCenter();

  /// Determines if a local-space ray hits this mesh.
  bool CastRay(const Ray& localRay, MultiConvexMesh* mesh, ProxyResult& result, BaseCastFilter& filter);

  Triangle GetTriangleIndexed(VertexArrayParam verts, uint index);
  Triangle GetTriangle(VertexArrayParam verts, uint index);
  uint GetTriangleCount();

  /// The vertex indices on the main mesh used to generate the convex hull.
  MultiConvexMeshIndexData* GetIndices();
  /// The vertex indices on the main mesh used to generate triangle indices
  /// for computing mass information and debug drawing. More indices are needed for
  /// determining triangles than for generating the convex mesh.
  MultiConvexMeshIndexData* GetTriangleIndices();

  void Draw(VertexArrayParam verts, Mat4Param transform, bool drawEdges, bool drawFaces);
  void Draw2d(VertexArrayParam verts, Mat4Param transform, bool drawEdges, bool drawFaces);
  void DrawFaces(VertexArrayParam verts, Mat4Param transform, ByteColor color);
  void DrawEdges(VertexArrayParam verts, Mat4Param transform, ByteColor color);
  void DrawEdges2d(VertexArrayParam verts, Mat4Param transform, ByteColor color);

  /// The indices into the vertices array that represent what points are in the convex mesh.
  IndexArray mIndices;
  /// The indices for the triangles used for debug drawing.
  IndexArray mTriangleIndices;

  Aabb mAabb;
  Vec3 mCenterOfMass;
  real mVolume;
  /// Is this sub-mesh incorrectly configured. Typically means that the indices don't point to
  /// valid vertices. Also the number of triangle indices could be incorrect (multiple of 3).
  bool mValid;

  /// The MultiConvexMesh that owns this sub-mesh.
  MultiConvexMesh* mMesh;
  MultiConvexMeshIndexData mIndexData;
  MultiConvexMeshIndexData mTriangleIndexData;
};

//-------------------------------------------------------------------MultiConvexMesh
/// Represents a collection of convex meshes that was decomposed from a mesh.
class MultiConvexMesh : public Resource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  MultiConvexMesh();
  ~MultiConvexMesh();

  void Save(StringParam filename) override;
  void Serialize(Serializer& stream) override;
  void Initialize();
  void ResourceModified() override;

  HandleOf<Resource> Clone() override;
  /// Creates a clone of this mesh for run-time modifications.
  HandleOf<MultiConvexMesh> RuntimeClone();
  /// Creates a MultiConvexMesh for run-time modifications.
  static HandleOf<MultiConvexMesh> CreateRuntime();
  void CopyTo(MultiConvexMesh* destination);

  void Draw(Mat4Param transform, bool drawEdges = true, bool drawFaces = true);

  /// Creates and adds a new sub-mesh to this mesh.
  SubConvexMesh* AddSubMesh();
  /// The vertex buffer data of this mesh.
  MultiConvexMeshVertexData* GetVertices();
  /// A collection of sub-convex meshes.
  MultiConvexMeshSubMeshData* GetSubMeshes();
  /// Remove all sub-meshes.
  void ClearSubMeshes();

  /// Is the resource currently modified?
  bool GetModified();
  /// Is the resource correctly setup? Typically involves a mis-match in indices and vertices.
  bool GetValid();
  /// If the user only specifies triangle indices (the regular indices are empty) on a sub-mesh then
  /// extract unique indices from the triangle indices. This needs to be updated later to merge the two index lists.
  void FillEmptyIndices();
  /// Check if the mesh is valid. Optionally throw a script exception if it is invalid.
  bool Validate(bool throwExceptionIfInvalid);
  /// Rebuild all extra mesh information if it is currently modified. This includes
  /// things like the center of mass, volume, aabb, edge info and more.
  void UpdateAndNotifyIfModified();

  /// Returns the volume of the entire multi-convex mesh.
  /// This value can be scaled later so it is computed once.
  real GetVolume();
  /// Scales the cached volume into a world-space scaled volume.
  real GetWorldVolume(Vec3Param worldScale);
  /// Returns the center of mass of the entire multi-convex mesh.
  /// This value can be scaled later so it is computed once.
  Vec3 GetCenterOfMass();
  /// Scales the cached center of mass into a world-space scaled center of mass.
  Vec3 GetWorldCenterOfMass(Vec3Param worldScale);
  /// The inertia tensor cannot be scaled later so it must be recomputed each time.
  Mat3 ComputeInvInertiaTensor(Vec3Param worldScale, real totalMass);

  /// Determines if a local-space ray hits any of the sub-meshes.
  bool CastRay(const Ray& localRay, ProxyResult& result, BaseCastFilter& filter);

  /// Rebuild all cached information for when the mesh has changed (center of mass, volume, aabb).
  void RebuildCachedInfo();

private:
  void ComputeCenterOfMassAndVolume();
  Aabb ComputeAabb();

public:

  typedef Array<Vec3> VertexArray;
  VertexArray mVertices;

  typedef Array<SubConvexMesh*> SubMeshArray;
  SubMeshArray mMeshes;

  // These values are the accumulated values from all the sub-meshes
  Aabb mAabb;
  Vec3 mCenterOfMass;
  real mVolume;

  /// This resource is not specific to 2d, but it does need to know what kind of resource
  /// it was for editing purposes (so we only edit a 2d mesh with the 2d mesh editor).
  /// Not currently used but more of an idea right now.
  BitField<MultiConvexMeshFlags::Enum> mFlags;
  bool mIs2D;
  bool mModified;
  bool mIsValid;

  /// The bound vertex data. This is a safe object that behaves like an array to the users.
  MultiConvexMeshVertexData mVertexData;
  /// The bound sub-mesh data. This is a safe object that behaves like an array to the
  /// users (slightly different interface as they can add/get/clear but not set).
  MultiConvexMeshSubMeshData mSubMeshData;
};

//---------------------------------------------------------- MultiConvexMeshManager
class MultiConvexMeshManager : public ResourceManager
{
public:
  DeclareResourceManager(MultiConvexMeshManager, MultiConvexMesh);

  MultiConvexMeshManager(BoundType* resourceType);
  MultiConvexMesh* CreateNewResourceInternal(StringParam name) override;

  void UpdateAndNotifyModifiedResources();

  typedef HandleOf<MultiConvexMesh> MeshReference;
  Array<MeshReference> mModifiedMeshes;
};

}//namespace Zero
