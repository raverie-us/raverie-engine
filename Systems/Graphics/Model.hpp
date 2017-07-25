#pragma once

namespace Zero
{

/// Renders a mesh.
class Model : public Graphical
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Component Interface

  void Initialize(CogInitializer& initializer) override;
  void Serialize(Serializer& stream) override;

  // Graphical Interface

  Aabb GetLocalAabb() override;
  void ExtractFrameData(FrameNode& frameNode, FrameBlock& frameBlock) override;
  void ExtractViewData(ViewNode& viewNode, ViewBlock& viewBlock, FrameBlock& frameBlock) override;
  bool TestRay(GraphicsRayCast& rayCast, CastInfo& castInfo) override;
  bool TestFrustum(const Frustum& frustum, CastInfo& castInfo) override;

  /// Mesh that the graphical will render.
  Mesh* GetMesh();
  void SetMesh(Mesh* newMesh);
  HandleOf<Mesh> mMesh;

  // Internal

  void OnMeshModified(ResourceEvent* event);
};

} // namespace Zero
