// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

/// Emits particles inside a sphere
class SphericalParticleEmitter : public ParticleEmitterShared
{
public:
  RaverieDeclareType(SphericalParticleEmitter, TypeCopyMode::ReferenceType);

  SphericalParticleEmitter();
  ~SphericalParticleEmitter();

  // Component Interface
  void Initialize(CogInitializer& initializer) override;
  void Serialize(Serializer& stream) override;

  // ParticleEmitter Interface
  int EmitParticles(
      ParticleList* particleList, float dt, Mat4Ref transform, Vec3Param velocity, float timeAlive) override;

private:
};

/// Emits particles inside a box
class BoxParticleEmitter : public ParticleEmitterShared
{
public:
  RaverieDeclareType(BoxParticleEmitter, TypeCopyMode::ReferenceType);

  BoxParticleEmitter();
  ~BoxParticleEmitter();

  // Component Interface
  void Initialize(CogInitializer& initializer) override;
  void Serialize(Serializer& stream) override;

  // ParticleEmitter Interface
  int EmitParticles(
      ParticleList* particleList, float dt, Mat4Ref transform, Vec3Param velocity, float timeAlive) override;
};

DeclareEnum3(MeshEmitMode, Vertex, Edge, Face);

/// Emits particles on a mesh surface
class MeshParticleEmitter : public ParticleEmitterShared
{
public:
  RaverieDeclareType(MeshParticleEmitter, TypeCopyMode::ReferenceType);

  MeshParticleEmitter();
  ~MeshParticleEmitter();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  // ParticleEmitter Interface
  int EmitParticles(
      ParticleList* particleList, float dt, Mat4Ref transform, Vec3Param velocity, float timeAlive) override;

  void GetNextEmitPoint(Vec3Ptr position, Vec3Ptr normal);

  void SetMeshEmitMode(MeshEmitMode::Enum mode);
  MeshEmitMode::Enum GetMeshEmitMode();

  /// Mesh used for this Model.
  Mesh* GetMesh();
  void SetMesh(Mesh* newMesh);

  void Setup();
  void SetupFaceTable();
  void SetupEdgeTable();

private:
  MeshEmitMode::Enum mMeshEmitMode;

  /// Moved the spawn position along the normal scaled by this amount.
  float mNormalExtrude;

  HandleOf<Mesh> mMesh;

  Math::WeightedProbabilityTable<uint> mFaceTable;
  typedef Pair<uint, uint> Edge;
  Math::WeightedProbabilityTable<Edge> mEdgeTable;
};

} // namespace Raverie
