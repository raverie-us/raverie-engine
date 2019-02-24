// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

Material* CreateMaterialFromGraphMaterial(SceneGraphMaterial* sceneMaterial)
{
  String graphMaterialName = ConvertToValidName(sceneMaterial->Name);
  // String diffuseTexture =
  // sceneMaterial->Attributes.FindValue("DiffuseTexture", String());

  Material* material = (Material*)MaterialManager::FindOrNull(graphMaterialName);

  // if(material != NULL)
  //{
  //  //Already loaded
  //  sceneMaterial->LoadedMaterial = material;
  //  return material;
  //}

  // String shader = sceneMaterial->Attributes.FindValue("ShaderModel",
  // "Lambert"); if(shader == "Phong")
  //{
  //  float SpecularExponent  = 16.0f;
  //  float SpecularScalar = 0.25f;
  //  //ToValue(sceneMaterial->Attributes.FindValue("SpecularExponent", "40"),
  //  SpecularExponent);
  //  //ToValue(sceneMaterial->Attributes.FindValue("SpecularScalar", "1"),
  //  SpecularScalar); material = MakeNewPhongMaterial(SpecularScalar,
  //  SpecularExponent);
  //}
  // else
  //  material = MakeNewMaterial();

  ////Add textures
  // if(!AddTextureBlock(material, BlockType::Diffuse, diffuseTexture))
  //  AddTextureBlock(material, BlockType::Diffuse, "DefaultTexture");

  // AddTextureBlock(material, BlockType::Normal,
  // sceneMaterial->Attributes.FindValue("NormalMapTexture", String()));
  // AddTextureBlock(material, BlockType::Specular,
  // sceneMaterial->Attributes.FindValue("SpecularMapTexture", String()));

  // material->Initialize();

  // ContentLibrary* library = Z::gEditor->mProjectLibrary;
  // ResourceAdd resourceAdd;
  // resourceAdd.Library = library;
  // resourceAdd.Name = graphMaterialName;
  // resourceAdd.SourceResource = material;
  // AddNewResource(MaterialManager::GetInstance(), resourceAdd);

  // if(!resourceAdd.WasSuccessful())
  //  SafeDelete(material);

  // sceneMaterial->LoadedMaterial = material;

  return material;
}

Material* FindOrCreateMaterial(SceneGraphSource* source, StringParam materialName)
{
  SceneGraphMaterial* sceneMaterial = source->MaterialsByName.FindValue(materialName, NULL);

  if (sceneMaterial == NULL)
    return MaterialManager::GetDefault();

  if (sceneMaterial->LoadedMaterial)
    return sceneMaterial->LoadedMaterial;

  return CreateMaterialFromGraphMaterial(sceneMaterial);
}

Material* FindMeshNodeMaterial(SceneGraphSource* source, SceneGraphNode* node)
{
  ContentLibrary* library = Z::gEditor->mProjectLibrary;

  Material* material = NULL;

  if (!node->Materials.Empty())
  {
    // if(node->Materials.Size() > 1)
    //{
    //  //Multi Materials
    //  String multiName = BuildString(source->Name, node->NodeName, "Multi");

    //  Array<Material*> materials;

    //  forRange(String materialName, node->Materials.All())
    //    materials.PushBack( FindOrCreateMaterial(source, materialName) );

    //  return FindOrCreateMultiMaterial(library, multiName, materials);
    //}
    // else if(node->Materials.Size() == 1)
    {
      return FindOrCreateMaterial(source, node->Materials[0]);
    }
  }

  return NULL;
}

template <typename MeshType, typename ManagerType>
MeshType* GetNodeMesh(StringParam sourceName, StringParam meshNodeName)
{
  MeshType* mesh = nullptr;
  if (!meshNodeName.Empty())
  {
    String fullMeshName = BuildString(sourceName, "_", meshNodeName);
    // Check for a name that's generated when there are multiple meshes from a
    // scene file.
    mesh = ManagerType::FindOrNull(fullMeshName);
    // If that name isn't found, just check for the scene name (when there is
    // only one mesh).
    if (mesh == nullptr)
      mesh = ManagerType::FindOrNull(sourceName);
  }
  return mesh;
}

void UpdateMeshes(SceneGraphSource* source, SceneGraphNode* sourceNode, Cog* object, UpdateFlags::Type flags)
{
  if (!(flags & UpdateFlags::Meshes))
    return;

  // Mesh
  Mesh* mesh = GetNodeMesh<Mesh, MeshManager>(source->Name, sourceNode->MeshName);
  if (mesh)
  {
    // Get the material for this node
    Material* material = NULL;

    if (flags & UpdateFlags::Materials)
      material = FindMeshNodeMaterial(source, sourceNode);

    if (mesh->mBones.Empty() == false)
    {
      SkinnedModel* skinnedModel = HasOrAdd<SkinnedModel>(object);
      skinnedModel->SetMesh(mesh);

      if (sourceNode->SkeletonRootNodePath.Empty() == false)
        skinnedModel->SetSkeletonPath(sourceNode->SkeletonRootNodePath);
      else
        skinnedModel->SetSkeletonPath(CogPath("."));

      if (material)
        skinnedModel->SetMaterial(material);
    }
    else
    {
      Model* model = HasOrAdd<Model>(object);
      model->SetMesh(mesh);

      if (material)
        model->SetMaterial(material);
    }
  }

  // Physics Mesh
  PhysicsMesh* physicsMesh = GetNodeMesh<PhysicsMesh, PhysicsMeshManager>(source->Name, sourceNode->PhysicsMeshName);
  if (physicsMesh)
  {
    MeshCollider* meshCollider = HasOrAdd<MeshCollider>(object);
    meshCollider->SetPhysicsMesh(physicsMesh);
  }

  // Convex Mesh
  ConvexMesh* convexMesh = GetNodeMesh<ConvexMesh, ConvexMeshManager>(source->Name, sourceNode->PhysicsMeshName);
  if (convexMesh)
  {
    ConvexMeshCollider* meshCollider = HasOrAdd<ConvexMeshCollider>(object);
    meshCollider->SetConvexMesh(convexMesh);
  }
}

Cog* CreateCogFromGraph(Space* space,
                        SceneGraphSource* source,
                        SceneGraphNode* sourceNode,
                        Cog* parentObject,
                        Cog* object,
                        UpdateFlags::Type flags,
                        bool isBone)
{
  // If this is the root node used the name of the source name
  String nodeName = parentObject ? sourceNode->NodeName : source->Name;

  // Does this child object already exists
  if (object == NULL && parentObject)
    object = parentObject->FindChildByName(nodeName);

  // No child with that name so create one
  if (object == NULL)
  {
    // Create a object with just transform
    object =
        space->CreateAt(CoreArchetypes::Transform, sourceNode->Translation, sourceNode->Rotation, sourceNode->Scale);
    object->ClearArchetype();

    // If parent object is provided attach to it
    if (parentObject)
      object->AttachToPreserveLocal(parentObject);
  }
  else
  {
    if (parentObject != NULL && (flags & UpdateFlags::Transforms))
    {
      Transform* transform = object->has(Transform);
      transform->SetLocalTranslation(sourceNode->Translation);
      transform->SetLocalRotation(sourceNode->Rotation);
      transform->SetLocalScale(sourceNode->Scale);
    }
  }

  object->SetName(nodeName);

  if (isBone)
  {
    Bone* bone = HasOrAdd<Bone>(object);
  }

  if (sourceNode->IsSkeletonRoot)
  {
    Skeleton* skeleton = HasOrAdd<Skeleton>(object);
    // Make all of Skeleton's child objects a Bone
    isBone = true;
  }

  // Update meshes resources
  UpdateMeshes(source, sourceNode, object, flags);

  // Update children
  for (uint i = 0; i < sourceNode->Children.Size(); ++i)
  {
    SceneGraphNode* childGraphNode = sourceNode->Children[i];
    CreateCogFromGraph(space, source, childGraphNode, object, NULL, flags, isBone);
  }

  return object;
}

void CopyChildIds(Cog* newArchetypeCog, Cog* oldArchetypeCog)
{
  newArchetypeCog->mChildId = oldArchetypeCog->mChildId;

  forRange (Cog& newChild, newArchetypeCog->GetChildren())
  {
    if (Cog* oldChild = oldArchetypeCog->FindChildByName(newChild.GetName()))
    {
      newChild.mChildId = oldChild->mChildId;

      CopyChildIds(&newChild, oldChild);
    }
  }
}

void DoEditorSideGeometryImporting(GeometryContent* geometryContent,
                                   Cog* object,
                                   UpdateFlags::Type flags,
                                   StringParam contentOutputPath)
{
  // Load the graph file with details of materials to generate
  String graphFile = FilePath::CombineWithExtension(contentOutputPath, geometryContent->Filename, ".graph.data");
  if (!FileExists(graphFile))
    return;

  GeneratedArchetype* genArchetype = geometryContent->has(GeneratedArchetype);
  if (!genArchetype)
    return;

  ResourceId archetypeId = genArchetype->mResourceId;

  // Find the archetype
  Archetype* archetype = ArchetypeManager::FindOrNull(archetypeId);

  // Temporary, do not allow GeneratedArchetype to replace existing Archetype.
  if (archetype != nullptr)
    return;

  UniquePointer<SceneGraphSource> sceneGraph = new SceneGraphSource();
  LoadFromDataFile(*sceneGraph, graphFile);
  sceneGraph->MapNames();

  // Create a space that will be used to build the object in
  Space* space = Z::gFactory->CreateSpace(CoreArchetypes::DefaultSpace, CreationFlags::Editing, nullptr);

  String baseName = geometryContent->GetName();
  sceneGraph->Name = baseName;

  object = CreateCogFromGraph(space, sceneGraph, sceneGraph->Root, NULL, object, flags, false);

  // If there is animations attach a animGraph component and Assign animations
  AnimationBuilder* animations = geometryContent->has(AnimationBuilder);

  if (archetype)
  {
    Cog* oldArchetypeCog = space->Create(archetype);
    CopyChildIds(object, oldArchetypeCog);
  }

  if (animations)
  {
    AnimationGraph* animGraph = HasOrAdd<AnimationGraph>(object);

    // Assign the first animation
    if (animations->mAnimations.Size() > 0)
    {
      Animation* animation = (Animation*)Z::gResources->GetResource(animations->mAnimations[0].mResourceId);
      if (animation)
      {
        if (object->AddComponentByType(ZilchTypeId(SimpleAnimation)))
        {
          SimpleAnimation* playAnimation = object->has(SimpleAnimation);
          playAnimation->SetAnimation(animation);
        }
      }
    }
  }

  if (flags & UpdateFlags::Archetype)
  {
    // If archetype exists update it
    if (archetype)
    {
      object->SetArchetype(archetype);
      object->UploadToArchetype();
      Z::gEngine->RebuildArchetypes(archetype);
    }
    else
    {
      archetype = ArchetypeManager::GetInstance()->MakeNewArchetypeWith(object, baseName, archetypeId);
    }
  }

  space->Destroy();
}

void UpdateToContent(Cog* object, UpdateFlags::Type flags)
{
  Archetype* archetype = object->GetArchetype();

  if (archetype == NULL)
    return;

  ContentLibrary* library = Z::gEditor->mProjectLibrary;
  forRange (ContentItem* item, library->GetContentItems())
  {
    GeneratedArchetype* generated = item->has(GeneratedArchetype);
    if (generated && generated->mResourceId == archetype->mResourceId)
    {
      Resource* res = Z::gResources->GetResource(generated->mResourceId);
      if (res->mResourceLibrary)
        DoEditorSideGeometryImporting(
            Type::DynamicCast<GeometryContent*>(item), object, flags, res->mResourceLibrary->Location);
      else
        ErrorIf(true,
                "Resource Library not present, location of ContentItem's "
                "generated resource location unknown\n");
    }
  }
}

void BuildSoundCues(ResourcePackage* package, AudioOptions* options)
{
  ResourceListing& resources = package->Resources;

  if (options->mGenerateCue == AudioCueImport::PerSound)
  {
    // Create a sound cue for each sound in the package
    for (uint i = 0; i < resources.Size(); ++i)
    {
      ResourceEntry& entry = resources[i];
      if (entry.Type == "Sound" || entry.Type == "StreamedSound" || entry.Type == "AutoStreamedSound")
      {
        // Attempt to create a new sound cue
        ResourceAdd resourceAdd;
        resourceAdd.Library = Z::gEditor->mProjectLibrary;
        resourceAdd.Name = entry.Name;
        AddNewResource(SoundCueManager::GetInstance(), resourceAdd);

        // If it was successfully created, add the sound entry
        if (resourceAdd.WasSuccessful())
        {
          SoundCue* cue = (SoundCue*)(resourceAdd.SourceResource);

          // Add the sound
          Sound* sound = SoundManager::GetInstance()->Find(entry.mResourceId);
          cue->AddSoundEntry(sound, 1.0f);
          if (cue->mContentItem)
            cue->mContentItem->SaveContent();
        }
      }
    }
  }
  else if (options->mGenerateCue == AudioCueImport::Grouped)
  {
    // Attempt to create a new sound cue
    ResourceAdd resourceAdd;
    resourceAdd.Library = Z::gEditor->mProjectLibrary;
    resourceAdd.Name = options->mGroupCueName;
    AddNewResource(SoundCueManager::GetInstance(), resourceAdd);

    // If it was successful, add each sound that was loaded as an entry in the
    // cue
    if (resourceAdd.WasSuccessful())
    {
      SoundCue* cue = (SoundCue*)(resourceAdd.SourceResource);
      for (uint i = 0; i < resources.Size(); ++i)
      {
        ResourceEntry& entry = resources[i];
        if (entry.Type == "Sound" || entry.Type == "StreamedSound" || entry.Type == "AutoStreamedSound")
        {
          // Add the sound
          Sound* sound = SoundManager::GetInstance()->Find(entry.mResourceId);
          cue->AddSoundEntry(sound, 1.0f);
        }
      }

      if (cue->mContentItem)
        cue->mContentItem->SaveContent();
    }
  }
}

void DoEditorSideImporting(ResourcePackage* package, ImportOptions* options)
{
  if (Z::gEngine->IsReadOnly())
  {
    DoNotifyWarning("Content", "Cannot add content items in read-only mode");
    return;
  }

  forRange (ContentItem* item, package->EditorProcessing.All())
  {
    TextureContent* textureContent = item->has(TextureContent);
    if (textureContent)
    {
      Array<String> files;

      for (size_t i = 0; i < textureContent->mTextures.Size(); ++i)
      {
        String fileName = FilePath::Combine(package->Location, textureContent->mTextures[i].mName);
        files.PushBack(fileName);
      }

      // we have textures we pulled from the scene file that need loading
      ContentLibrary* library = Z::gEditor->mProjectLibrary;
      ImportOptions* options = new ImportOptions();
      options->Initialize(files, library);
      RunGroupImport(*options);
    }

    // Does an archetype need to be generated for this content item?
    GeneratedArchetype* genArchetype = item->has(GeneratedArchetype);
    if (genArchetype)
    {
      // Find the archetype associated with this content item may not be
      // generated yet
      Archetype* archetype =
          (Archetype*)ArchetypeManager::GetInstance()->ResourceIdMap.FindValue(genArchetype->mResourceId, NULL);

      // Check to see if the archetype is generated and is up to date
      bool needToBuild = archetype == NULL || genArchetype->NeedToBuildArchetype(archetype->mContentItem);
      if (!needToBuild)
        continue;

      // Build or update the archetype
      UpdateFlags::Type flags =
          UpdateFlags::Archetype | UpdateFlags::Materials | UpdateFlags::Meshes | UpdateFlags::Transforms;
      DoEditorSideGeometryImporting(Type::DynamicCast<GeometryContent*>(item), NULL, flags, package->Location);
    }
    else
      continue;
  }

  // Build sound cue(s)
  if (options && options->mAudioOptions)
    BuildSoundCues(package, options->mAudioOptions);
}

} // namespace Zero
