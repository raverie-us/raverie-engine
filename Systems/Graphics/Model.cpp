#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------------------ Model
ZilchDefineType(Model, builder, type)
{
  ZeroBindComponent();
  ZeroBindDocumented();
  ZeroBindInterface(Graphical);
  ZeroBindSetup(SetupMode::DefaultSerialization);

  ZilchBindGetterSetterProperty(Mesh);
}

void Model::Initialize(CogInitializer& initializer)
{
  Graphical::Initialize(initializer);
  ConnectThisTo(MeshManager::GetInstance(), Events::ResourceModified, OnMeshModified);
}

void Model::Serialize(Serializer& stream)
{
  Graphical::Serialize(stream);
  SerializeResourceName(mMesh, MeshManager);
}

Aabb Model::GetLocalAabb()
{
  return mMesh->mAabb;
}

void Model::ExtractFrameData(FrameNode& frameNode, FrameBlock& frameBlock)
{
  frameNode.mBorderThickness = 1.0f;
  frameNode.mRenderingType = RenderingType::Static;
  frameNode.mCoreVertexType = CoreVertexType::Mesh;

  frameNode.mMaterialRenderData = mMaterial->mRenderData;
  frameNode.mMeshRenderData = mMesh->mRenderData;
  frameNode.mTextureRenderData = nullptr;

  frameNode.mLocalToWorld = mTransform->GetWorldMatrix();
  frameNode.mLocalToWorldNormal = Math::BuildTransform(mTransform->GetWorldRotation(), mTransform->GetWorldScale());
  frameNode.mLocalToWorldNormal.Invert().Transpose();

  frameNode.mObjectWorldPosition = mTransform->GetWorldTranslation();

  frameNode.mBoneMatrixRange = IndexRange(0, 0);
}

void Model::ExtractViewData(ViewNode& viewNode, ViewBlock& viewBlock, FrameBlock& frameBlock)
{
  FrameNode& frameNode = frameBlock.mFrameNodes[viewNode.mFrameNodeIndex];

  viewNode.mLocalToView = viewBlock.mWorldToView * frameNode.mLocalToWorld;
  viewNode.mLocalToViewNormal = Math::ToMatrix3(viewBlock.mWorldToView) * frameNode.mLocalToWorldNormal;
  viewNode.mLocalToPerspective = viewBlock.mViewToPerspective * viewNode.mLocalToView;
}

bool Model::TestRay(GraphicsRayCast& rayCast, CastInfo& castInfo)
{
  rayCast.mObject = GetOwner();
  return mMesh->TestRay(rayCast, mTransform->GetWorldMatrix());
}

bool Model::TestFrustum(const Frustum& frustum, CastInfo& castInfo)
{
  Frustum localFrustum = frustum.TransformInverse(mTransform->GetWorldMatrix());
  return mMesh->TestFrustum(localFrustum);
}

Mesh* Model::GetMesh()
{
  return mMesh;
}

void Model::SetMesh(Mesh* mesh)
{
  if (mesh == nullptr || mesh == mMesh)
    return;

  mMesh = mesh;
  UpdateBroadPhaseAabb();
}

void Model::OnMeshModified(ResourceEvent* event)
{
  if ((Mesh*)event->EventResource == mMesh)
    UpdateBroadPhaseAabb();
}

} // namespace Zero
