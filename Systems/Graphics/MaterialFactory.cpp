#include "Precompiled.hpp"

namespace Zero
{

MaterialFactory* MaterialFactory::sInstance = nullptr;

ZilchDefineType(MaterialFactory, builder, type)
{
  type->AddAttribute(ObjectAttributes::cHidden);
}

MaterialFactory::MaterialFactory()
{
  sInstance = this;

  MaterialFactory::ShaderInputTypeMap& shaderInputTypes = MaterialFactory::sInstance->mShaderInputTypes;
  shaderInputTypes[ZilchTypeId(bool)] = ShaderInputType::Bool;
  shaderInputTypes[ZilchTypeId(int)] = ShaderInputType::Int;
  shaderInputTypes[ZilchTypeId(IntVec2)] = ShaderInputType::IntVec2;
  shaderInputTypes[ZilchTypeId(IntVec3)] = ShaderInputType::IntVec3;
  shaderInputTypes[ZilchTypeId(IntVec4)] = ShaderInputType::IntVec4;
  shaderInputTypes[ZilchTypeId(float)] = ShaderInputType::Float;
  shaderInputTypes[ZilchTypeId(Vec2)] = ShaderInputType::Vec2;
  shaderInputTypes[ZilchTypeId(Vec3)] = ShaderInputType::Vec3;
  shaderInputTypes[ZilchTypeId(Vec4)] = ShaderInputType::Vec4;
  shaderInputTypes[ZilchTypeId(Mat3)] = ShaderInputType::Mat3;
  shaderInputTypes[ZilchTypeId(Mat4)] = ShaderInputType::Mat4;
  shaderInputTypes[ZilchTypeId(Texture)] = ShaderInputType::Texture;

  ErrorIf(shaderInputTypes.FindPointer(nullptr) != nullptr, "A MetaType was not found.");
}

void MaterialFactory::MoveComponent(HandleParam instance, HandleParam componentToMove, uint destination)
{
  uint indexToMove = GetComponentIndex(instance, componentToMove.StoredType);

  Material* material = instance.Get<Material*>();
  Material::MaterialBlockArray& materialBlocks = material->mMaterialBlocks;

  MaterialBlock* block = materialBlocks[indexToMove];
  materialBlocks.EraseAt(indexToMove);

  if (indexToMove < destination)
    materialBlocks.InsertAt(destination - 1, HandleOf<MaterialBlock>(block));
  else
    materialBlocks.InsertAt(destination, HandleOf<MaterialBlock>(block));

  material->mCompositionChanged = true;
  material->SendModified();
}

uint MaterialFactory::GetComponentIndex(HandleParam instance, BoundType* typeId)
{
  Material* resource = instance.Get<Material*>();
  return resource->GetBlockIndex(typeId);
}

bool MaterialFactory::CanAddComponent(HandleParam owner, BoundType* typeToAdd, AddInfo* info)
{
  // If component is a restricted type
  if (mRestrictedComponents.Contains(typeToAdd))
  {
    if (info)
    {
      info->Reason = "CoreVertex, RenderPass, and PostProcess fragments cannot be added to materials, " \
                     "they are used for auto generated shader permutations.";
    }
    return false;
  }

  // We can only have one geometry fragment on the composition
  bool addingGeometry = mGeometryComponents.Contains(typeToAdd);
  forRange(Handle component, AllComponents(owner))
  {
    if(addingGeometry && mGeometryComponents.Contains(component.StoredType))
    {
      if(info)
      {
        info->BlockingComponent = component;
        info->Reason = String::Format("Geometry Fragment %s already present on Material.", typeToAdd->Name.c_str());
      }
      return false;
    }
  }

  return MetaComposition::CanAddComponent(owner, typeToAdd, info);
}

void MaterialFactory::UpdateRestrictedComponents(HashMap<LibraryRef, ZilchShaderLibraryRef>& libraries, ZilchFragmentTypeMap& fragmentTypes)
{
  mRestrictedComponents.Clear();
  mGeometryComponents.Clear();

  forRange (LibraryRef wrapperLibrary, libraries.Keys())
  {
    forRange (BoundType* boundType, wrapperLibrary->BoundTypes.Values())
    {
      ZilchFragmentType::Enum fragmentType = fragmentTypes.FindValue(boundType->Name, ZilchFragmentType::Fragment);

      if (fragmentType != ZilchFragmentType::Fragment)
        mRestrictedComponents.Insert(boundType);

      if (boundType->HasAttribute("Geometry") != nullptr)
        mGeometryComponents.Insert(boundType);
    }
  }
}

ShaderInputType::Enum MaterialFactory::GetShaderInputType(Type* type)
{
  return mShaderInputTypes.FindValue(type, ShaderInputType::Invalid);
}

} // namespace Zero
