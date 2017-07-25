#pragma once

namespace Zero
{

class MaterialFactory : public SimpleResourceFactory<Material, MaterialBlock>
{
public:
  ZilchDeclareDerivedTypeExplicit(MaterialFactory, MetaComposition, TypeCopyMode::ReferenceType);
  static MaterialFactory* sInstance;
  static MaterialFactory* GetInstance() {return sInstance;}
  MaterialFactory();

  void MoveComponent(HandleParam owner, HandleParam componentToMove, uint destination) override;
  uint GetComponentIndex(HandleParam owner, BoundType* componentType) override;

  bool CanAddComponent(HandleParam owner, BoundType* typeToAdd, AddInfo* info = nullptr) override;

  void UpdateRestrictedComponents(HashMap<LibraryRef, ZilchShaderLibraryRef>& libraries, ZilchFragmentTypeMap& fragmentTypes);

  ShaderInputType::Enum GetShaderInputType(Type* type);
  typedef HashMap<Type*, ShaderInputType::Enum> ShaderInputTypeMap;
  ShaderInputTypeMap mShaderInputTypes;

  // Fragments that are not allowed to be added to a material
  // because they are used automatically in shader permutation generation
  HashSet<BoundType*> mRestrictedComponents;

  // Keeping track of geometry fragments so that using multiple can be disallowed
  HashSet<BoundType*> mGeometryComponents;
};

} // namespace Zero
