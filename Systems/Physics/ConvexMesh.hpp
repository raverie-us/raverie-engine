///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Benjamin Strukus, Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//------------------------------------------------------------------ ConvexMesh
/// A convex mesh meant for use with dynamic rigid bodies. Computes efficient
/// contact information compared to a regular physics mesh. This mesh also
/// defines a volume which means mass properties can be computed.
class ConvexMesh : public GenericPhysicsMesh
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  
  //-------------------------------------------------------------------Resource Interface
  void Serialize(Serializer& stream) override;
  void Initialize();
  void OnResourceModified() override;

  HandleOf<Resource> Clone() override;
  /// Creates a clone of this mesh for run-time modifications.
  HandleOf<ConvexMesh> RuntimeClone();
  /// Creates a ConvexMesh for run-time modifications.
  static HandleOf<ConvexMesh> CreateRuntime();
  
  //-------------------------------------------------------------------Internal
  /// Finds the first triangle hit by the local-space ray.
  bool CastRay(const Ray& localRay, ProxyResult& result, BaseCastFilter& filter);
  void Draw(Mat4Param transform);
  
  void BuildFromPointSet(const Vec3Array& points);
};

//---------------------------------------------------------- Convex Mesh Manager
class ConvexMeshManager : public ResourceManager
{
public:
  DeclareResourceManager(ConvexMeshManager, ConvexMesh);
  ConvexMeshManager(BoundType* resourceType);

  void UpdateAndNotifyModifiedResources();

  typedef HandleOf<ConvexMesh> ConvexMeshReference;
  Array<ConvexMeshReference> mModifiedMeshes;
};

}//namespace Zero
