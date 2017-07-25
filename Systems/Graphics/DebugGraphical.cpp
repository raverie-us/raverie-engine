#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(DebugGraphical, builder, type)
{
  ZeroBindInterface(Graphical);
  type->AddAttribute(ObjectAttributes::cHidden);
}

void DebugGraphical::Initialize(CogInitializer& initializer)
{
  Graphical::Initialize(initializer);
  // Don't process component shader inputs
  GetReceiver()->Disconnect(Events::ShaderInputsModified);
}

Aabb DebugGraphical::GetLocalAabb()
{
  // stub
  return Aabb();
}

void DebugGraphical::ExtractFrameData(FrameNode& frameNode, FrameBlock& frameBlock)
{
  frameNode.mRenderingType = RenderingType::Streamed;
  frameNode.mCoreVertexType = CoreVertexType::Streamed;

  frameNode.mMaterialRenderData = mMaterial->mRenderData;
  frameNode.mMeshRenderData = nullptr;
  frameNode.mTextureRenderData = TextureManager::Find("White")->mRenderData;

  frameNode.mLocalToWorld = Mat4::cIdentity;
  frameNode.mLocalToWorldNormal = Mat3::cIdentity;

  frameNode.mObjectWorldPosition = Vec3::cZero;

  frameNode.mBoneMatrixRange = IndexRange(0, 0);

  frameNode.mBorderThickness = 1.0f;
}

void DebugGraphical::AddToSpace()
{
  mGraphicsSpace->AddDebugGraphical(this);
}

ZilchDefineType(DebugGraphicalPrimitive, builder, type)
{
  ZeroBindComponent();
}

void DebugGraphicalPrimitive::ExtractViewData(ViewNode& viewNode, ViewBlock& viewBlock, FrameBlock& frameBlock)
{
  FrameNode& frameNode = frameBlock.mFrameNodes[viewNode.mFrameNodeIndex];
  Array<StreamedVertex>& streamedVertices = frameBlock.mRenderQueues->mStreamedVertices;

  viewNode.mLocalToView = Mat4::cIdentity;
  viewNode.mLocalToViewNormal = Mat3::cIdentity;
  viewNode.mLocalToPerspective = viewBlock.mViewToPerspective;

  viewNode.mStreamedVertexType = mPrimitiveType;
  viewNode.mStreamedVertexStart = streamedVertices.Size();
  viewNode.mStreamedVertexCount = 0;

  Debug::DebugVertexArray vertices;
  Debug::DebugViewData viewData =
  {
    viewBlock.mEyePosition,
    viewBlock.mEyeDirection,
    viewBlock.mEyeUp,
    viewBlock.mFieldOfView,
    viewBlock.mOrthographicSize,
    viewBlock.mOrthographic,
  };

  forRange (Debug::DebugDrawObjectAny& debugObject, mDebugObjects.All())
  {
    debugObject->GetVertices(viewData, vertices);
  }

  forRange (Debug::Vertex& vertex, vertices.All())
  {
    StreamedVertex streamed;
    streamed.mPosition = Math::TransformPoint(viewBlock.mWorldToView, vertex.mPosition);
    streamed.mColor = vertex.mColor;
    streamedVertices.PushBack(streamed);
  }

  viewNode.mStreamedVertexCount = streamedVertices.Size() - viewNode.mStreamedVertexStart;
}


ZilchDefineType(DebugGraphicalThickLine, builder, type)
{
  ZeroBindComponent();
}

void DebugGraphicalThickLine::ExtractFrameData(FrameNode& frameNode, FrameBlock& frameBlock)
{
  DebugGraphicalPrimitive::ExtractFrameData(frameNode, frameBlock);
  frameNode.mBorderThickness = 2.0f;
}

ZilchDefineType(DebugGraphicalText, builder, type)
{
  ZeroBindComponent();
}

void DebugGraphicalText::ExtractViewData(ViewNode& viewNode, ViewBlock& viewBlock, FrameBlock& frameBlock)
{
  FrameNode& frameNode = frameBlock.mFrameNodes[viewNode.mFrameNodeIndex];
  Array<StreamedVertex>& streamedVertices = frameBlock.mRenderQueues->mStreamedVertices;

  viewNode.mStreamedVertexType = mPrimitiveType;
  viewNode.mStreamedVertexStart = streamedVertices.Size();
  viewNode.mStreamedVertexCount = 0;

  viewNode.mLocalToView = Mat4::cIdentity;
  viewNode.mLocalToViewNormal = Mat3::cIdentity;
  viewNode.mLocalToPerspective = viewBlock.mViewToPerspective;

  RenderFont* font = FontManager::GetDefault()->GetRenderFont(64);
  frameNode.mTextureRenderData = font->mTexture->mRenderData;

  forRange (Debug::DebugDrawObjectAny& debugObject, mDebugObjects.All())
  {
    Debug::Text* debugText = (Debug::Text*)&*debugObject;
    TextAlign::Enum align = TextAlign::Left;

    float pixelScale = debugText->mTextHeight / 64.0f;
    Vec2 startLocation = Vec2(0, 0);
    Vec2 widths = font->MeasureText(debugText->mText, pixelScale) * 0.5f;
    if (debugText->GetCentered())
    {
      startLocation += Vec2(-widths.x, widths.y);
      align = TextAlign::Center;
    }

    uint debugTextStart = streamedVertices.Size();

    FontProcessor fontProcessor(frameBlock.mRenderQueues, &viewNode, debugText->mColor);
    ProcessTextRange(fontProcessor, font, debugText->mText, startLocation, align, Vec2(1, -1) * pixelScale, widths * 2.0f);

    uint debugTextEnd = streamedVertices.Size();

    Mat3 rotation = Math::ToMatrix3(debugText->mRotation);
    if (debugText->GetViewAligned())
      rotation = Math::ToMatrix3(viewBlock.mWorldToView).Transposed();

    float viewScale = 1.0f;
    if (debugText->GetViewScaled())
      viewScale = (debugText->mPosition - viewBlock.mEyePosition).Length() * Debug::cViewScale;

    // Local to View transform is different for each debug text object
    Mat4 localToView = viewBlock.mWorldToView * Math::BuildTransform(debugText->mPosition, rotation, Vec3(1, 1, 1) * viewScale);
    for (uint i = debugTextStart; i < debugTextEnd; ++i)
      streamedVertices[i].mPosition = Math::TransformPoint(localToView, streamedVertices[i].mPosition + debugText->mViewScaleOffset);
  }
}

} // namespace Zero
