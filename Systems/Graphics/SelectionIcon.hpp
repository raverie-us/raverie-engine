#pragma once

namespace Zero
{

/// Utility selection behavior for objects in the editor.
class SelectionIcon : public Graphical
{
public:
  // Does not bind Graphical as an interface because this component
  // is only for special behavior that needs to draw.
  ZilchDeclareDerivedTypeExplicit(SelectionIcon, Component, TypeCopyMode::ReferenceType);

  static const float cBaseScale;

  // Component Interface

  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void OnDestroy(uint flags = 0) override;

  // Graphical Interface

  Aabb GetLocalAabb() override;
  void ExtractFrameData(FrameNode& frameNode, FrameBlock& frameBlock) override;
  void ExtractViewData(ViewNode& viewNode, ViewBlock& viewBlock, FrameBlock& frameBlock) override;
  void MidPhaseQuery(Array<GraphicalEntry>& entries, Camera& camera, Frustum* frustum) override;
  bool TestRay(GraphicsRayCast& rayCast, CastInfo& castInfo) override;
  bool TestFrustum(const Frustum& frustum, CastInfo& castInfo) override;
  void AddToSpace() override;
  String GetDefaultMaterialName() override;

  // Properties

  /// Sprite based image to use for icon, will not animate.
  SpriteSource* GetSpriteSource();
  void SetSpriteSource(SpriteSource* spriteSource);
  HandleOf<SpriteSource> mSpriteSource;

  /// Scalar for how big the icon should appear in the viewport.
  float mViewScale;

  /// If collider/graphical or other selection logic should be disabled and only selectable via this icon.
  bool GetOverrideSelections();
  void SetOverrideSelections(bool overrideSelections);
  bool mOverrideSelections;
  
  // Internal

  float GetRadius(Camera* camera);
  float GetRadius(ViewBlock& viewBlock);
  Vec3 GetWorldTranslation();
  void SetSelectionFlag(bool selectionLimited);
};

} // namespace Zero
