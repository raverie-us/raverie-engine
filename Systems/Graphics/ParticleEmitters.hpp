///////////////////////////////////////////////////////////////////////////////
///
/// \file ParticleEmitters.hpp
/// Declaration of the Particle emitters.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//--------------------------------------------------- Spherical Particle Emitter

/// Emits particles inside a sphere
class SphericalParticleEmitter : public ParticleEmitterShared
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SphericalParticleEmitter();
  ~SphericalParticleEmitter();

  //Component Interface
  void Initialize(CogInitializer& initializer) override;
  void Serialize(Serializer& stream) override;

  //ParticleEmitter Interface
  int EmitParticles(ParticleList* particleList, float dt, 
                    Mat4Ref transform, Vec3Param velocity, float timeAlive) override;
private:
};

//--------------------------------------------------------- Box Particle Emitter
/// Emits particles inside a box 
class BoxParticleEmitter : public ParticleEmitterShared
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  BoxParticleEmitter();
  ~BoxParticleEmitter();

  //Component Interface
  void Initialize(CogInitializer& initializer) override;
  void Serialize(Serializer& stream) override;

  //ParticleEmitter Interface
  int EmitParticles(ParticleList* particleList, float dt, 
                    Mat4Ref transform, Vec3Param velocity, float timeAlive) override;
};

//------------------------------------------------------- Mesh Particle Emitter
DeclareEnum3(MeshEmitMode, Vertex, Edge, Face);

/// Emits particles on a mesh surface
class MeshParticleEmitter : public ParticleEmitterShared
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  MeshParticleEmitter();
  ~MeshParticleEmitter();

  //Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  //ParticleEmitter Interface
  int EmitParticles(ParticleList* particleList, float dt, 
                    Mat4Ref transform, Vec3Param velocity, float timeAlive) override;

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
  typedef Pair<uint,uint> Edge;
  Math::WeightedProbabilityTable<Edge> mEdgeTable;
};

} // namespace Zero
