#include "Precompiled.hpp"

namespace Zero
{

namespace Tags
{
  DefineTag(Graphical);
}

namespace Events
{
  DefineEvent(EnterView);
  DefineEvent(EnterViewAny);
  DefineEvent(ExitView);
  DefineEvent(ExitViewAll);
}

void MakeLocalToViewAligned(Mat4& localToView, Mat4Param localToWorld, Mat4Param worldToView, Vec3Param translation)
{
  // Get just the camera's rotation
  Mat4 viewBasis = worldToView;
  viewBasis.m03 = 0;
  viewBasis.m13 = 0;
  viewBasis.m23 = 0;

  // Remove translation so transform can be rotated
  Mat4 localToWorldRotated = localToWorld;
  localToWorldRotated.m03 = 0;
  localToWorldRotated.m13 = 0;
  localToWorldRotated.m23 = 0;

  // Append inverse camera rotation
  localToWorldRotated = viewBasis.Transposed() * localToWorldRotated;

  // Add translation back on to transform
  localToWorldRotated.m03 = translation.x;
  localToWorldRotated.m13 = translation.y;
  localToWorldRotated.m23 = translation.z;

  // localToWorldRotated will now undo the view rotation,
  // resulting in the Sprite facing the camera
  localToView = worldToView * localToWorldRotated;
}

ZilchDefineType(GraphicalEvent, builder, type)
{
  ZeroBindDocumented();
  ZilchBindFieldProperty(mViewingObject);
}

//-------------------------------------------------------------------- Graphical
ZilchDefineType(Graphical, builder, type)
{
  ZeroBindDocumented();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDependency(Transform);

  ZilchBindGetterSetterProperty(Visible);
  ZilchBindGetterSetterProperty(ViewCulling);
  ZilchBindFieldProperty(mVisibilityEvents);
  ZilchBindGetterSetterProperty(OverrideBoundingBox)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  ZilchBindGetterSetterProperty(LocalAabbCenter)->ZeroFilterEquality(mOverrideBoundingBox, bool, true);
  ZilchBindGetterSetterProperty(LocalAabbHalfExtents)->ZeroFilterEquality(mOverrideBoundingBox, bool, true);
  ZilchBindFieldProperty(mGroupSortValue);
  ZilchBindGetterSetterProperty(Material);
  ZilchBindGetterSetter(ShaderInputs);

  ZilchBindGetter(WorldAabb);

  ZeroBindEvent(Events::EnterView, GraphicalEvent);
  ZeroBindEvent(Events::EnterViewAny, GraphicalEvent);
  ZeroBindEvent(Events::ExitView, GraphicalEvent);
  ZeroBindEvent(Events::ExitViewAll, GraphicalEvent);

  ZeroBindTag(Tags::Graphical);
}

void Graphical::Initialize(CogInitializer& initializer)
{
  mTransform = GetOwner()->has(Transform);
  mGraphicsSpace = initializer.mSpace->has(GraphicsSpace);

  AddToSpace();

  RebuildComponentShaderInputs();

  ConnectThisTo(Z::gEngine->has(GraphicsEngine), Events::ShaderInputsModified, OnShaderInputsModified);
  ConnectThisTo(MaterialManager::GetInstance(), Events::ResourceModified, OnMaterialModified);
}

void Graphical::Serialize(Serializer& stream)
{
  SerializeNameDefault(mVisible, true);
  SerializeNameDefault(mViewCulling, true);
  SerializeNameDefault(mVisibilityEvents, true);
  SerializeNameDefault(mOverrideBoundingBox, false);
  SerializeNameDefault(mLocalAabbCenter, Vec3(0.0f));
  SerializeNameDefault(mLocalAabbHalfExtents, Vec3(1.0f));
  SerializeNameDefault(mGroupSortValue, 0);
  SerializeResourceNameDefault(mMaterial, MaterialManager, GetDefaultMaterialName());
}

void Graphical::OnDestroy(uint flags)
{
  mGraphicsSpace->RemoveGraphical(this);
}

void Graphical::TransformUpdate(TransformUpdateInfo& info)
{
  UpdateBroadPhaseAabb();
}

void Graphical::AttachTo(AttachmentInfo& info)
{
  UpdateBroadPhaseAabb();
}

void Graphical::Detached(AttachmentInfo& info)
{
  UpdateBroadPhaseAabb();
}

void Graphical::MidPhaseQuery(Array<GraphicalEntry>& entries, Camera& camera, Frustum* frustum)
{
  mGraphicalEntryData.mGraphical = this;
  mGraphicalEntryData.mFrameNodeIndex = -1;
  mGraphicalEntryData.mPosition = mTransform->GetWorldTranslation();
  mGraphicalEntryData.mUtility = 0;

  GraphicalEntry entry;
  entry.mData = &mGraphicalEntryData;
  entry.mSort = 0;

  entries.PushBack(entry);
}

bool Graphical::TestRay(GraphicsRayCast& rayCast, CastInfo& castInfo)
{
  Ray ray = rayCast.mRay;
  Obb obb = GetWorldObb();

  Intersection::IntersectionPoint point;
  Intersection::Type result = Intersection::None;

  result = Intersection::RayObb(ray.Start, ray.Direction, obb.Center, obb.HalfExtents, obb.Basis, &point);
  if (result == Intersection::None)
    return false;

  rayCast.mObject = GetOwner();
  rayCast.mT = point.T;
  return true;
}

bool Graphical::TestFrustum(const Frustum& frustum, CastInfo& castInfo)
{
  return Overlap(frustum, GetWorldObb());
}

void Graphical::AddToSpace()
{
  mGraphicsSpace->AddGraphical(this);
}

String Graphical::GetDefaultMaterialName()
{
  return MaterialManager::GetInstance()->DefaultResourceName;
}

bool Graphical::GetVisible()
{
  return mVisible;
}

void Graphical::SetVisible(bool visible)
{
  if (visible == mVisible)
    return;

  mVisible = visible;
  // Re-adding will allow GraphicsSpace to put it in a new list
  mGraphicsSpace->RemoveGraphical(this);
  AddToSpace();
}

bool Graphical::GetViewCulling()
{
  return mViewCulling;
}

void Graphical::SetViewCulling(bool culling)
{
  if (culling == mViewCulling)
    return;

  mViewCulling = culling;
  // Re-adding will allow GraphicsSpace to put it in a new list
  mGraphicsSpace->RemoveGraphical(this);
  AddToSpace();
}

bool Graphical::GetOverrideBoundingBox()
{
  return mOverrideBoundingBox;
}

void Graphical::SetOverrideBoundingBox(bool overrideBoundingBox)
{
  if (overrideBoundingBox == mOverrideBoundingBox)
    return;

  mOverrideBoundingBox = overrideBoundingBox;
  UpdateBroadPhaseAabb();
}

Vec3 Graphical::GetLocalAabbCenter()
{
  return mLocalAabbCenter;
}

void Graphical::SetLocalAabbCenter(Vec3 center)
{
  mLocalAabbCenter = center;
  UpdateBroadPhaseAabb();
}

Vec3 Graphical::GetLocalAabbHalfExtents()
{
  return mLocalAabbHalfExtents;
}

void Graphical::SetLocalAabbHalfExtents(Vec3 halfExtents)
{
  mLocalAabbHalfExtents = Math::Max(halfExtents, Vec3(0.1f));
  UpdateBroadPhaseAabb();
}

Material* Graphical::GetMaterial()
{
  return mMaterial;
}

void Graphical::SetMaterial(Material* material)
{
  if (material != nullptr && material != mMaterial)
  {
    mMaterial = material;

    // Some shader inputs could be using the material to auto-find which fragment to use
    RebuildComponentShaderInputs();
  }
}

ShaderInputs* Graphical::GetShaderInputs()
{
  return mShaderInputs;
}

void Graphical::SetShaderInputs(ShaderInputs* shaderInputs)
{
  mShaderInputs = shaderInputs;
}

Aabb Graphical::GetWorldAabb()
{
  Aabb localAabb = GetLocalAabbInternal();
  localAabb.SetHalfExtents(Math::Max(localAabb.GetHalfExtents(), Vec3(0.1f)));
  return localAabb.TransformAabb(mTransform->GetWorldMatrix());
}

Obb Graphical::GetWorldObb()
{
  Aabb localAabb = GetLocalAabbInternal();
  localAabb.SetHalfExtents(Math::Max(localAabb.GetHalfExtents(), Vec3(0.1f)));
  return localAabb.Transform(mTransform->GetWorldMatrix());
}

Aabb Graphical::GetLocalAabbInternal()
{
 if (mOverrideBoundingBox)
    return Aabb(mLocalAabbCenter, mLocalAabbHalfExtents);
  else
    return GetLocalAabb();
 }

void Graphical::UpdateBroadPhaseAabb()
{
  if (mProxy.ToVoidPointer() != nullptr)
  {
    // Update the information in the visibility broad phase
    BaseBroadPhaseData<Graphical*> data;
    data.mClientData = this;
    data.mAabb = GetWorldAabb();

    mGraphicsSpace->mBroadPhase.UpdateProxy(mProxy, data);
  }
}

void Graphical::OnShaderInputsModified(ShaderInputsEvent* event)
{
  // Valid pointer already checked by GraphicsEngine
  BoundType* modifiedType = event->mType;

  Component* component = GetOwner()->GetComponentByName(modifiedType->Name);
  if (component == nullptr)
    return;

  // Iterate backwards to erase while iterating
  for (uint i = mPropertyShaderInputs.Size() - 1; i < mPropertyShaderInputs.Size(); --i)
  {
    if (mPropertyShaderInputs[i].mComponent == component)
      mPropertyShaderInputs.EraseAt(i);
  }

  AddComponentShaderInputs(component);
}

void Graphical::OnMaterialModified(ResourceEvent* event)
{
  if ((Material*)event->EventResource == mMaterial)
    RebuildComponentShaderInputs();
}

void Graphical::ComponentAdded(BoundType* typeId, Component* component)
{
  AddComponentShaderInputs(component);
}

void Graphical::ComponentRemoved(BoundType* typeId, Component* component)
{
  for (uint i = mPropertyShaderInputs.Size() - 1; i < mPropertyShaderInputs.Size(); --i)
  {
    if (mPropertyShaderInputs[i].mComponent == component)
      mPropertyShaderInputs.EraseAt(i);
  }
}

void Graphical::RebuildComponentShaderInputs()
{
  mPropertyShaderInputs.Clear();

  forRange (Component* component, GetOwner()->GetComponents())
    AddComponentShaderInputs(component);
}

void Graphical::AddComponentShaderInputs(Component* component)
{
  String componentName = ZilchVirtualTypeId(component)->Name;
  Array<ShaderMetaProperty>* shaderProperties = Z::gEngine->has(GraphicsEngine)->mComponentShaderProperties.FindPointer(componentName);

  // No inputs from this component
  if (shaderProperties == nullptr)
    return;

  ZilchShaderGenerator* shaderGenerator = Z::gEngine->has(GraphicsEngine)->mShaderGenerator;

  forRange (ShaderMetaProperty& shaderProperty, shaderProperties->All())
  {
    Property* metaProperty = ZilchVirtualTypeId(component)->GetProperty(shaderProperty.mMetaPropertyName);
    String fragmentName = shaderProperty.mFragmentName;
    String inputName = shaderProperty.mInputName;

    // Use property name if attribute did not specify
    if (inputName.Empty())
      inputName = metaProperty->Name;

    // Look on material for a fragment with matching input if attribute did not specify
    if (fragmentName.Empty())
    {
      forRange (MaterialBlock* materialBlock, mMaterial->mMaterialBlocks.All())
      {
        BoundType* metaType = ZilchVirtualTypeId(materialBlock);
        if (metaType->HasAttribute(ObjectAttributes::cProxy))
          continue;

        forRange (Property* fragmentProperty, metaType->GetProperties())
        {
          if (fragmentProperty->Name != inputName)
            continue;

          if (fragmentProperty->PropertyType != metaProperty->PropertyType)
            continue;

          fragmentName = metaType->Name;
          break;
        }

        // Exit loop when first matching fragment is found
        if (fragmentName.Empty() == false)
          break;
      }
    }

    ShaderInputType::Enum type = MaterialFactory::GetInstance()->GetShaderInputType(metaProperty->PropertyType);
    ShaderInput shaderInput = shaderGenerator->CreateShaderInput(fragmentName, inputName, type, Any());
    if (shaderInput.mShaderInputType == ShaderInputType::Invalid)
      continue;

    PropertyShaderInput input;
    input.mComponent = component;
    input.mMetaProperty = metaProperty;
    input.mShaderInput = shaderInput;

    mPropertyShaderInputs.PushBack(input);
  }
}

} // namespace Zero
