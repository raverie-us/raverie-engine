///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2014-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareBitField3(TriangleInfoFlags, V0V1Convex, V2V0Convex, V1V2Convex);

//-------------------------------------------------------------------MeshTriangleInfo
struct MeshTriangleInfo
{
  MeshTriangleInfo()
  {
    mEdgeV0V1Angle = mEdgeV1V2Angle = mEdgeV2V0Angle = Math::cTwoPi;
    mEdgeFlags.Clear();
  }

  real mEdgeV0V1Angle;
  real mEdgeV1V2Angle;
  real mEdgeV2V0Angle;

  BitField<TriangleInfoFlags::Enum> mEdgeFlags;
};

typedef HashMap<uint, MeshTriangleInfo> TriangleInfoMap;

//-------------------------------------------------------------------GenericPhysicsMesh
/// Base class of mesh type physics resources. Stores the actual mesh
/// (no optimization structures) and information about the mesh such as mass and inertia.
class GenericPhysicsMesh : public Resource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  typedef Array<Vec3> VertexArray;
  typedef Array<uint> IndexArray;

  GenericPhysicsMesh();

  //-------------------------------------------------------------------Resource Interface
  void Save(StringParam filename) override;
  void Serialize(Serializer& stream) override;
  void Initialize();
  void Unload() override;
  void ResourceModified() override;

  //------------------------------------------------------------------- GenericPhysicsMesh Interface
  /// Called when a mesh is modified. This is typically done when the vertices/indices are modified in script.
  virtual void OnResourceModified() = 0;
  /// Uploads a new set of points and indices, then calls ForceRebuild.
  virtual void Upload(const Vec3Array& points, const IndexArray& indices);
  /// Recomputes mass, volume, the local space aabb, and internal edge information.
  /// If a mesh wants to do something special (such as a mid-phase) then it should override this.
  /// Note: Inertia is not calculated as it cannot be (non-uniformly) scaled afterwards.
  virtual void ForceRebuild();
  /// Rebuild a mid-phase if it is needed. This will happen before internal edge data is generated.
  virtual void RebuildMidPhase() {};
  /// Generates the voronoi region data for internal edge catching prevention.
  /// Virtual so that any mesh can use its appropriate mid-phase for optimization.
  virtual void GenerateInternalEdgeData();
  

  /// The vertex buffer data of this mesh.
  PhysicsMeshVertexData* GetVertices();
  /// The index buffer data of this mesh.
  PhysicsMeshIndexData* GetIndices();

  /// The number of triangles in the mesh.
  uint GetTriangleCount();
  /// Returns the triangle at an index. Note: the index is not from the index buffer,
  /// it is the triangle number index (e.g. index 4 is triangle number 3 because of 0 based indexing).
  Triangle GetTriangle(uint index);

  /// Is the mesh currently dirty (modified without having been updated).
  bool GetModified();
  /// Is the mesh in an invalid state? This happens if the user modifies the mesh such
  /// that the indices and vertices are mismatched. In this case the mesh should enter a dormant state.
  bool GetValid();
  /// Check if the mesh is valid. Optionally throw a script exception if it is invalid.
  bool Validate(bool throwExceptionIfInvalid);
  /// Rebuild all extra mesh information if it is currently modified. This includes things like the
  /// center of mass, volume, aabb, edge info and more (some derived types may have a mid-phase, etc...)
  void UpdateAndNotifyIfModified();
  
  void DrawEdges(Mat4Param transform, ByteColor color);
  void DrawFaces(Mat4Param transform, ByteColor color);
  void DrawFaceNormals(Mat4Param transform, ByteColor color);

  //-------------------------------------------------------------------Internal
  /// RayCasts against the given triangle (the index is needed for the result's shape index)
  /// and fills out the appropriate info (t, points, normals, etc...)
  bool CastRayTriangle(const Ray& localRay, const Triangle& tri, int triIndex, ProxyResult& result, BaseCastFilter& filter);
  /// RayCasts a local-space ray against the underlying vertex/index buffer. No mid-phase or optimizations are performed.
  bool CastRayGeneric(const Ray& localRay, ProxyResult& result, BaseCastFilter& filter);
  
  void Support(const Vec3Array points, Vec3Param localDirection, Vec3Ptr support) const;
  /// Basic Minkowski space support function. Just checks all of the stored vertices in O(n) time.
  void Support(Vec3Param localDirection, Vec3Ptr support) const;

  /// Creates the triangle defined by the 3 indices in the index buffer starting at the provided index.
  /// This should rarely be used and is mostly a helper for GetTriangle().
  Triangle GetTriangleFromIndexBufferIndex(uint index);
  
  /// Returns the info map used for correcting internal edges
  TriangleInfoMap* GetInfoMap();

  /// Compute the volume of the mesh in local space.
  void ComputeLocalVolume();
  /// Computes the center of mass of the mesh in local space.
  void ComputeLocalCenterOfMass();

  /// Computes and returns the center of mass in LocalScaled-space.
  Vec3 ComputeScaledCenterOfMass(Vec3Param worldScale);
  /// Computes and returns the volume in LocalScaled-space.
  real ComputeScaledVolume(Vec3Param worldScale);
  /// Computes the LocalScaled-space inertia tensor. The inertia tensor cannot be computed
  /// and stored in local space then scaled later when there is non-uniform scale
  /// (this might actually be wrong, double check this later).
  Mat3 ComputeScaledInvInertiaTensor(Vec3Param worldScale, real worldMass);


  void CopyTo(GenericPhysicsMesh* destination);

  // Legacy
  const Vec3Array& GetVertexArray() const;
  const IndexArray& GetIndexArray() const;

  // @JoshD: Merge into bitfield
  bool mIsValid;
  bool mModified;

  /// All vertices of the mesh.
  VertexArray mVertices;
  /// The indices into the vertex array. Every 3 indices is a triangle
  IndexArray mIndices;

  /// Aabb in model space.
  Aabb mLocalAabb;
  /// Center of mass of the mesh in model space.
  Vec3 mLocalCenterOfMass;
  /// Volume of the mesh in model space.
  real mLocalVolume;

  TriangleInfoMap mInfoMap;
  PhysicsMeshVertexData mVertexData;
  PhysicsMeshIndexData mIndexData;
};

}//namespace Zero
