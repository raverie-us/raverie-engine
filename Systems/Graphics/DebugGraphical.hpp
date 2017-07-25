#pragma once

namespace Zero
{

class DebugGraphical : public Graphical
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  void Initialize(CogInitializer& initializer) override;

  Aabb GetLocalAabb() override;
  void ExtractFrameData(FrameNode& frameNode, FrameBlock& frameBlock) override;
  void AddToSpace() override;

  // Don't process component shader inputs
  void ComponentAdded(BoundType* typeId, Component* component) override {}
  void ComponentRemoved(BoundType* typeId, Component* component) override {}

  PrimitiveType::Enum mPrimitiveType;

  Array<Debug::DebugDrawObjectAny> mDebugObjects;
};

class DebugGraphicalPrimitive : public DebugGraphical
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  void ExtractViewData(ViewNode& viewNode, ViewBlock& viewBlock, FrameBlock& frameBlock) override;
};

class DebugGraphicalThickLine : public DebugGraphicalPrimitive
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  void ExtractFrameData(FrameNode& frameNode, FrameBlock& frameBlock) override;
};

class DebugGraphicalText : public DebugGraphical
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  void ExtractViewData(ViewNode& viewNode, ViewBlock& viewBlock, FrameBlock& frameBlock) override;
};

} // namespace Zero
