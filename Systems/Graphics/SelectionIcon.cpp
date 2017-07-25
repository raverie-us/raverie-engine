#include "Precompiled.hpp"

namespace Zero
{

const float SelectionIcon::cBaseScale = 0.75f;

ZilchDefineType(SelectionIcon, builder, type)
{
  // Not requiring a Transform component, but this means that
  // Graphical methods that use Transform cannot be called
  //BindDependency(Transform);

  ZeroBindComponent();
  ZeroBindDocumented();
  ZeroBindSetup(SetupMode::DefaultSerialization);

  // Graphical data
  ZilchBindGetterSetterProperty(Visible);
  ZilchBindFieldProperty(mVisibilityEvents);
  ZilchBindGetterSetterProperty(Material);
  ZilchBindGetterSetter(ShaderInputs);

  // SelectionIcon data
  ZilchBindGetterSetterProperty(SpriteSource)->Add(new EditorResource(true, false, "SelectionIcon"));
  ZilchBindFieldProperty(mViewScale);
  ZilchBindGetterSetterProperty(OverrideSelections);
}

void SelectionIcon::Serialize(Serializer& stream)
{
  // Graphical data
  SerializeNameDefault(mVisible, true);
  SerializeNameDefault(mVisibilityEvents, true);
  SerializeResourceNameDefault(mMaterial, MaterialManager, GetDefaultMaterialName());

  // SelectionIcon data
  SerializeResourceNameDefault(mSpriteSource, SpriteSourceManager, "SelectIcon");
  SerializeNameDefault(mViewScale, 1.0f);
  SerializeNameDefault(mOverrideSelections, true);
}

void SelectionIcon::Initialize(CogInitializer& initializer)
{
  Graphical::Initialize(initializer);
  SetSelectionFlag(mOverrideSelections);
}

void SelectionIcon::OnDestroy(uint flags)
{
  Graphical::OnDestroy(flags);
  SetSelectionFlag(false);
}

Aabb SelectionIcon::GetLocalAabb()
{
  return Aabb(Vec3(0.0f), Vec3(0.5f));
}

void SelectionIcon::ExtractFrameData(FrameNode& frameNode, FrameBlock& frameBlock)
{
  frameNode.mBorderThickness = 1.0f;
  frameNode.mRenderingType = RenderingType::Streamed;
  frameNode.mCoreVertexType = CoreVertexType::Streamed;

  frameNode.mMaterialRenderData = mMaterial->mRenderData;
  frameNode.mMeshRenderData = nullptr;
  frameNode.mTextureRenderData = mSpriteSource->mTexture->mRenderData;

  frameNode.mLocalToWorld = Mat4::cIdentity;
  frameNode.mLocalToWorldNormal = Mat3::cIdentity;

  frameNode.mObjectWorldPosition = Vec3::cZero;

  frameNode.mBoneMatrixRange = IndexRange(0, 0);
}

void SelectionIcon::ExtractViewData(ViewNode& viewNode, ViewBlock& viewBlock, FrameBlock& frameBlock)
{
  FrameNode& frameNode = frameBlock.mFrameNodes[viewNode.mFrameNodeIndex];

  MakeLocalToViewAligned(viewNode.mLocalToView, frameNode.mLocalToWorld, viewBlock.mWorldToView, GetWorldTranslation());

  float radius = GetRadius(viewBlock);
  Vec2 center = Vec2(0.0f);
  Vec2 widths = Vec2(radius);

  Vec3 pos0 = Vec3(center, 0) + Vec3(-widths.x, widths.y, 0);
  Vec3 pos1 = Vec3(center, 0) + Vec3(widths.x, -widths.y, 0);

  UvRect rect = mSpriteSource->GetUvRect(0);

  Vec2 uv0 = rect.TopLeft;
  Vec2 uv1 = rect.BotRight;

  Vec2 uvAux0 = Vec2(0, 0);
  Vec2 uvAux1 = Vec2(1, 1);

  viewNode.mStreamedVertexType = PrimitiveType::Triangles;
  viewNode.mStreamedVertexStart = frameBlock.mRenderQueues->mStreamedVertices.Size();
  viewNode.mStreamedVertexCount = 0;

  frameBlock.mRenderQueues->AddStreamedQuad(viewNode, pos0, pos1, uv0, uv1, Vec4(1.0f), uvAux0, uvAux1);
}

void SelectionIcon::MidPhaseQuery(Array<GraphicalEntry>& entries, Camera& camera, Frustum* frustum)
{
  mGraphicalEntryData.mGraphical = this;
  mGraphicalEntryData.mFrameNodeIndex = -1;
  mGraphicalEntryData.mPosition = GetWorldTranslation();
  mGraphicalEntryData.mUtility = 0;

  GraphicalEntry entry;
  entry.mData = &mGraphicalEntryData;
  entry.mSort = 0;

  entries.PushBack(entry);
}

bool SelectionIcon::TestRay(GraphicsRayCast& rayCast, CastInfo& castInfo)
{
  float radius = GetRadius(castInfo.mCameraCog->has(Camera));
  Intersection::IntersectionPoint point;
  Intersection::Type result = Intersection::RaySphere(rayCast.mRay.Start, rayCast.mRay.Direction, GetWorldTranslation(), radius, &point);
  if (result == Intersection::None)
    return false;

  rayCast.mObject = GetOwner();
  rayCast.mT = 0;
  return true;
}

bool SelectionIcon::TestFrustum(const Frustum& frustum, CastInfo& castInfo)
{
  float radius = GetRadius(castInfo.mCameraCog->has(Camera));
  bool result = Overlap(frustum, Sphere(GetWorldTranslation(), radius));
  return result;
}

void SelectionIcon::AddToSpace()
{
  if (mVisible)
    mGraphicsSpace->mGraphicalsNeverCulled.PushBack(this);
  else
    mGraphicsSpace->mGraphicalsAlwaysCulled.PushBack(this);
}

String SelectionIcon::GetDefaultMaterialName()
{
  return "DebugDrawOnTop";
}

SpriteSource* SelectionIcon::GetSpriteSource()
{
  return mSpriteSource;
}

void SelectionIcon::SetSpriteSource(SpriteSource* spriteSource)
{
  if (spriteSource != nullptr)
    mSpriteSource = spriteSource;
}

bool SelectionIcon::GetOverrideSelections()
{
  return mOverrideSelections;
}

void SelectionIcon::SetOverrideSelections(bool overrideSelections)
{
  mOverrideSelections = overrideSelections;
  SetSelectionFlag(mOverrideSelections);
}

float SelectionIcon::GetRadius(Camera* camera)
{
  float viewDistance = Debug::GetViewDistance(GetWorldTranslation(), camera->GetWorldTranslation(), camera->GetWorldDirection());
  bool orthographic = camera->mPerspectiveMode == PerspectiveMode::Orthographic;
  float viewScale = Debug::GetViewScale(viewDistance, camera->mFieldOfView, camera->mSize, orthographic);
  return viewScale * mViewScale * cBaseScale * 0.5f;
}

float SelectionIcon::GetRadius(ViewBlock& viewBlock)
{
  float viewDistance = Debug::GetViewDistance(GetWorldTranslation(), viewBlock.mEyePosition, viewBlock.mEyeDirection);
  float viewScale = Debug::GetViewScale(viewDistance, viewBlock.mFieldOfView, viewBlock.mOrthographicSize, viewBlock.mOrthographic);
  return viewScale * mViewScale * cBaseScale * 0.5f;
}

Vec3 SelectionIcon::GetWorldTranslation()
{
  Vec3 worldTranslation = Vec3::cZero;
  if (Transform* transform = GetOwner()->has(Transform))
    worldTranslation = transform->GetWorldTranslation();
  else if (ObjectLink* objectLink = GetOwner()->has(ObjectLink))
    worldTranslation = objectLink->GetWorldPosition();
  return worldTranslation;
}

void SelectionIcon::SetSelectionFlag(bool selectionLimited)
{
  if (selectionLimited)
    GetOwner()->mFlags.SetFlag(CogFlags::SelectionLimited);
  else
    GetOwner()->mFlags.ClearFlag(CogFlags::SelectionLimited);
}

} // namespace Zero
