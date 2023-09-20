// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{
GeometryImporter::GeometryImporter(StringParam inputFile, StringParam outputPath, StringParam metaFile) :
    mScene(nullptr),
    mInputFile(inputFile),
    mOutputPath(outputPath),
    mMetaFile(metaFile),
    mBaseMeshName(FilePath::GetFileNameWithoutExtension(inputFile)),
    mUniquifyingIndex(0)
{
  if (metaFile.Empty())
    mMetaFile = BuildString(inputFile, ".meta");
}

GeometryImporter::~GeometryImporter()
{
  delete mGeometryContent;
}

uint GeometryImporter::SetupAssimpPostProcess()
{
  MeshBuilder* meshBuilder = mGeometryContent->has(MeshBuilder);
  if (meshBuilder == nullptr)
    return 0;

  uint flags = 0;
  flags |= aiProcess_JoinIdenticalVertices;
  // always turn polygons into triangle
  flags |= aiProcess_Triangulate;
  // always generate normals if none are present
  flags |= aiProcess_GenSmoothNormals;

  int removeFlags = 0;

  if (meshBuilder->mInvertUvYAxis)
  {
    flags |= aiProcess_FlipUVs;
  }

  if (meshBuilder->mGenerateSmoothNormals)
  {
    // if generate normals is checked we want to remove any existing normals and
    // generate our own
    flags |= aiProcess_RemoveComponent;
    removeFlags |= aiComponent_NORMALS;
    mAssetImporter.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, meshBuilder->mSmoothingAngleDegreesThreshold);
  }

  if (meshBuilder->mGenerateTangentSpace)
  {
    flags |= aiProcess_CalcTangentSpace;
    flags |= aiProcess_RemoveComponent;
    removeFlags |= aiComponent_TANGENTS_AND_BITANGENTS;
  }

  if (meshBuilder->mFlipWindingOrder)
  {
    flags |= aiProcess_FlipWindingOrder;
  }

  mAssetImporter.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, removeFlags);

  return flags;
}

GeometryProcessorCodes::Enum GeometryImporter::ProcessModelFiles()
{
  if (!FileExists(mInputFile))
  {
    ZPrint("Missing model file '%s'\n", mInputFile.c_str());
    return Raverie::GeometryProcessorCodes::Failed;
  }

  if (!FileExists(mMetaFile))
  {
    ZPrint("Missing meta file '%s'\n", mMetaFile.c_str());
    return Raverie::GeometryProcessorCodes::Failed;
  }

  // Load from meta file
  mGeometryContent = new GeometryContent(mInputFile);
  LoadFromDataFile(*mGeometryContent, mMetaFile);

  // Set the flags for post process we want to run
  uint flags = SetupAssimpPostProcess();

  // Load the file into Assimp. We must use memory because their
  // file functions do not call into our File wrappers. Also
  // when file paths contain unicode characters Assimp fails to
  // read the file.
  DataBlock block = ReadFileIntoDataBlock(mInputFile.c_str());
  mScene = mAssetImporter.ReadFileFromMemory(block.Data, block.Size, flags);
  if (!mScene) {
    String extension = FilePath::GetExtension(mInputFile);
    mScene = mAssetImporter.ReadFileFromMemory(block.Data, block.Size, flags, extension.c_str());
  }
  zDeallocate(block.Data);
  ZPrint("Processing model: %s\n", FilePath::GetFileNameWithoutExtension(mInputFile).Data());

  // An error has occurred, no scene imported
  if (!mScene)
  {
    String error(mAssetImporter.GetErrorString());
    error = ProcessAssimpErrorMessage(error);
    ZPrint("Geometry Processor Error: %s\n", error.c_str());
    return Raverie::GeometryProcessorCodes::Failed;
  }

  if (SceneEmpty())
    return GeometryProcessorCodes::NoContent;

  CollectNodeData();
  bool metaUpdated = UpdateBuilderMetaData();

  mGeometryContent->has(GeometryImport)->ComputeTransforms();

  // Assimp stores all bone data on the meshes so if there are no meshes, there
  // are no skeletons
  if (mScene->HasMeshes())
  {
    // We need the hierarchy data to travel through it marking what nodes
    // comprise the skeleton
    SkeletonProcessor skeletonProcess(mHierarchyDataMap, mMeshDataMap, mRootNodeName);
    skeletonProcess.ProcessSkeletonHierarchy(mScene);
  }

  // The pivot processor needs the skeleton data to properly collapse pivots
  // with animations so run this step after the SkeletonProcessor
  if (mGeometryContent->has(GeometryImport)->mCollapsePivots)
  {
    PivotProcessor pivotProcessor(mHierarchyDataMap, mRootNodeName, mAnimationRedirectMap);
    pivotProcessor.ProccessAndCollapsePivots();
  }

  // process the data into our format and export the files
  MeshBuilder* meshBuilder = mGeometryContent->has(MeshBuilder);
  if (meshBuilder && mScene->HasMeshes())
  {
    MeshProcessor meshProcessor(meshBuilder, mMeshDataMap);
    meshProcessor.ExtractAndProcessMeshData(mScene);
    meshProcessor.ExportMeshData(mOutputPath);
  }

  PhysicsMeshBuilder* physicsMeshBuilder = mGeometryContent->has(PhysicsMeshBuilder);
  if (physicsMeshBuilder)
  {
    PhysicsMeshProcessor physicsMeshProcessor(physicsMeshBuilder, mMeshDataMap);
    physicsMeshProcessor.BuildPhysicsMesh(mOutputPath);
  }

  TextureContent* textureContent = mGeometryContent->has(TextureContent);
  if (mScene->HasTextures() && textureContent)
  {
    TextureProcessor textureProcessor(textureContent, mOutputPath, mInputFile);
    textureProcessor.ExtractAndImportTextures(mScene);
  }

  AnimationBuilder* animationBuilder = mGeometryContent->has(AnimationBuilder);
  if (animationBuilder && mScene->HasAnimations())
  {
    animationBuilder->Name = mBaseMeshName;
    AnimationProcessor animationProcessor(animationBuilder, mHierarchyDataMap, mAnimationRedirectMap);
    animationProcessor.ExtractAndProcessAnimationData(mScene);
    animationProcessor.ExportAnimationData(mOutputPath);
  }

  // we need to build a scene graph such that on import the engine can create an
  // archetype for this mesh file and any animations attached to it if it has
  // any
  GeneratedArchetype* generatedArchetype = mGeometryContent->has(GeneratedArchetype);
  if (generatedArchetype)
  {
    ArchetypeProcessor archetypeProcessor(generatedArchetype, mHierarchyDataMap);
    archetypeProcessor.BuildSceneGraph(mRootNodeName);
    archetypeProcessor.ExportSceneGraph(FilePath::GetFileName(mInputFile), mOutputPath);
  }

  if ((metaUpdated || animationBuilder || generatedArchetype) && !textureContent)
    return GeometryProcessorCodes::LoadGraph;

  if (!(metaUpdated || animationBuilder) && textureContent)
    return GeometryProcessorCodes::LoadTextures;

  if ((metaUpdated || animationBuilder || generatedArchetype) && textureContent)
    return GeometryProcessorCodes::LoadGraphAndTextures;

  return GeometryProcessorCodes::Success;
}

bool GeometryImporter::SceneEmpty()
{
  // nothing we import is present, the scene is empty
  if (!mScene->HasAnimations() && !mScene->HasMeshes() && !mScene->HasTextures())
    return true;

  return false;
}

void GeometryImporter::CollectNodeData()
{
  aiNode* rootNode = mScene->mRootNode;
  // If the root scene node has 1 child, no meshes, and its transform is the
  // identity matrix then don't include it in the model hierarchy as this is
  // commonly a hidden node within Maya
  if ((rootNode->mNumChildren == 1) && (rootNode->mNumMeshes == 0) && (rootNode->mTransformation.IsIdentity()))
    rootNode = rootNode->mChildren[0];

  // Set the root node name and loop through the scenes nodes children and find
  // the ones with a mesh attached and store them along with their name for
  // process and export
  mRootNodeName = CleanAssetName(rootNode->mName.C_Str());
  ExtractDataFromNodesRescursive(rootNode, "");

  // Find all nodes with animations if they are present
  // and collapse pivots has been enabled
  AnimationBuilder* animationBuilder = mGeometryContent->has(AnimationBuilder);
  if (animationBuilder && mScene->HasAnimations() && mGeometryContent->has(GeometryImport)->mCollapsePivots)
    FindAnimationNodes();

  ComputeMeshTransforms();
}

String GeometryImporter::ExtractDataFromNodesRescursive(aiNode* node, String parentName)
{
  uint numChildren = node->mNumChildren;

  // collect the hierarchy data, used for animations if the scene has any
  bool isPivot = IsPivot(node->mName.C_Str());
  String unsantizedName = node->mName.C_Str();
  String nodeName = CleanAssetName(unsantizedName);
  // check to see if this node name is a unique entry, if no append an number to
  // the end to make it unique
  if (mHierarchyDataMap.ContainsKey(nodeName))
    nodeName = BuildString(nodeName, ToString(mUniquifyingIndex++));

  String nodePath = nodeName;
  if (node->mParent)
    nodePath = BuildString(mHierarchyDataMap[parentName].mNodePath, cAnimationPathDelimiterStr, nodeName);

  HierarchyData hierarchyData;
  hierarchyData.mParentNodeName = parentName;
  hierarchyData.mNodeName = nodeName;
  hierarchyData.mNodePath = nodePath;
  hierarchyData.mLocalTransform = AiMat4ToZeroMat4(node->mTransformation);
  hierarchyData.mIsPivot = isPivot;

  // Maya outputs an inverted PostRotation value to the fbx file. We currently
  // do not know why.
  if (unsantizedName.Contains("_$AssimpFbx$_PostRotation"))
    hierarchyData.mLocalTransform.Invert();

  // if the node has a mesh store the mesh keyed to its name
  if (node->mNumMeshes == 1)
  {
    // store one mesh entry on this node
    SingleMeshHierarchyEntry(hierarchyData, node->mMeshes[0]);
  }
  else if (node->mNumMeshes > 1)
  {
    // create a child node for each mesh
    MultipleMeshsHierarchicalEntry(hierarchyData, node);
  }

  mHierarchyDataMap.Insert(nodeName, hierarchyData);

  // walk down all the children
  for (uint i = 0; i < numChildren; ++i)
  {
    String childsName = ExtractDataFromNodesRescursive(node->mChildren[i], nodeName);
    mHierarchyDataMap[nodeName].mChildren.PushBack(childsName);
  }

  return nodeName;
}

void GeometryImporter::SingleMeshHierarchyEntry(HierarchyData& hierarchyData, uint meshIndex)
{
  hierarchyData.mHasMesh = true;

  if (mMeshDataMap.ContainsKey(meshIndex))
  {
    MeshData& meshData = mMeshDataMap[meshIndex];
    hierarchyData.mMeshName = meshData.mMeshName;
    hierarchyData.mPhysicsMeshName = meshData.mPhysicsMeshName;
  }
  else
  {
    MeshData data;
    data.mMeshTransform = hierarchyData.mLocalTransform;
    hierarchyData.mPhysicsMeshName = hierarchyData.mMeshName = data.mMeshName = hierarchyData.mNodeName;
    mMeshDataMap.Insert(meshIndex, data);
  }
}

void GeometryImporter::MultipleMeshsHierarchicalEntry(HierarchyData& hierarchyData, aiNode* node)
{
  String parentNodeName = hierarchyData.mNodeName;

  for (size_t i = 0; i < node->mNumMeshes; ++i)
  {
    HierarchyData childHierarchyData;
    childHierarchyData.mParentNodeName = parentNodeName;
    childHierarchyData.mNodeName = BuildString(parentNodeName, "GeneratedMeshNode", ToString(mUniquifyingIndex++));
    childHierarchyData.mNodePath =
        BuildString(hierarchyData.mNodePath, cAnimationPathDelimiterStr, childHierarchyData.mNodeName);
    childHierarchyData.mLocalTransform = Mat4::cIdentity;

    SingleMeshHierarchyEntry(childHierarchyData, node->mMeshes[i]);

    hierarchyData.mChildren.PushBack(childHierarchyData.mNodeName);
    mHierarchyDataMap.Insert(childHierarchyData.mNodeName, childHierarchyData);
  }
}

void GeometryImporter::FindAnimationNodes()
{
  // Search through the animation tree
  aiAnimation** animations = mScene->mAnimations;
  size_t numAnimations = mScene->mNumAnimations;
  for (size_t animIndex = 0; animIndex < numAnimations; ++animIndex)
  {
    aiAnimation* sceneAnimation = animations[animIndex];
    aiNodeAnim** sceneAnimationChannels = sceneAnimation->mChannels;
    size_t numChannels = sceneAnimation->mNumChannels;
    for (size_t channelIndex = 0; channelIndex < numChannels; ++channelIndex)
    {
      aiNodeAnim* sceneChannelNode = sceneAnimationChannels[channelIndex];
      String name = CleanAssetName(sceneChannelNode->mNodeName.C_Str());
      // Mark each animation in the hierarchy as needing to have its
      // animation corrected and remove any animated nodes status
      // as a pivot as it should not be collapsed
      HierarchyData& node = mHierarchyDataMap[name];
      if (node.mIsPivot)
      {
        // If the node has a single value track for translation, rotation, and
        // scale it does not count as an animation and will be collapsed as
        // these values are only used to take the model out of bind pose and are
        // effectively the nodes local transform
        if (sceneChannelNode->mNumPositionKeys > 1 || sceneChannelNode->mNumRotationKeys > 1 ||
            sceneChannelNode->mNumScalingKeys > 1)
        {
          node.mIsPivot = false;
          node.mIsAnimatedPivot = true;
        }
        else if (sceneChannelNode->mNumPositionKeys == 1 || sceneChannelNode->mNumRotationKeys == 1 ||
                 sceneChannelNode->mNumScalingKeys == 1)
        {
          node.mAnimationNode = sceneChannelNode;
        }
      }
    }
  }
}

void GeometryImporter::ComputeMeshTransforms()
{
  forRange (HierarchyData& node, mHierarchyDataMap.Values())
  {
    if (node.mHasMesh == false)
      continue;

    Mat4 meshTransform = node.mLocalTransform;
    String parentName = node.mParentNodeName;

    HierarchyData parentNode = mHierarchyDataMap[parentName];
    while (!parentNode.mParentNodeName.Empty())
    {
      parentNode = mHierarchyDataMap[parentName];
      meshTransform = parentNode.mLocalTransform * meshTransform;
      parentName = parentNode.mParentNodeName;
    }

    forRange (MeshData& meshData, mMeshDataMap.Values())
    {
      if (meshData.mMeshName == node.mMeshName)
      {
        meshData.mMeshTransform = meshTransform;
        break;
      }
    }
  }
}

bool GeometryImporter::UpdateBuilderMetaData()
{
  bool metaChanges = false;

  MeshBuilder* meshBuilder = mGeometryContent->has(MeshBuilder);
  if (mScene->HasMeshes() && meshBuilder != nullptr)
  {
    Array<GeometryResourceEntry> meshEntries;

    uint meshCount = mScene->mNumMeshes;
    for (uint i = 0; i < meshCount; ++i)
    {
      if (!mMeshDataMap.ContainsKey(i))
        continue;

      String name = mBaseMeshName;
      if (meshCount > 1)
        name = BuildString(name, "_", mMeshDataMap[i].mMeshName);

      GeometryResourceEntry entry;
      entry.mName = name;

      if (GeometryResourceEntry* previousEntry = meshBuilder->Meshes.FindPointer(entry))
        entry.mResourceId = previousEntry->mResourceId;
      else
        entry.mResourceId = GenerateUniqueId64();

      meshEntries.PushBack(entry);
    }

    if (meshBuilder->Meshes != meshEntries)
    {
      meshBuilder->Meshes = meshEntries;
      metaChanges = true;
    }
  }

  PhysicsMeshBuilder* physicsMeshBuilder = mGeometryContent->has(PhysicsMeshBuilder);
  if (mScene->HasMeshes() && physicsMeshBuilder != nullptr)
  {
    Array<GeometryResourceEntry> physicsMeshEntries;

    uint meshCount = mScene->mNumMeshes;
    for (uint i = 0; i < meshCount; ++i)
    {
      if (!mMeshDataMap.ContainsKey(i))
        continue;

      String name = mBaseMeshName;
      if (meshCount > 1)
        name = BuildString(name, "_", mMeshDataMap[i].mMeshName);

      GeometryResourceEntry entry;
      entry.mName = name;

      if (GeometryResourceEntry* previousEntry = physicsMeshBuilder->Meshes.FindPointer(entry))
        entry.mResourceId = previousEntry->mResourceId;
      else
        entry.mResourceId = GenerateUniqueId64();

      physicsMeshEntries.PushBack(entry);
    }

    if (physicsMeshBuilder->Meshes != physicsMeshEntries)
    {
      physicsMeshBuilder->Meshes = physicsMeshEntries;
      metaChanges = true;
    }
  }

  AnimationBuilder* animBuilder = mGeometryContent->has(AnimationBuilder);
  if (mScene->HasAnimations() && animBuilder != nullptr)
  {
    aiAnimation** animations = mScene->mAnimations;
    uint animationCount = mScene->mNumAnimations;

    Array<GeometryResourceEntry> animEntries;

    if (animBuilder->mClips.Size() != 0)
    {
      forRange (AnimationClip& clip, animBuilder->mClips.All())
      {
        if ((size_t)clip.mAnimationIndex >= animationCount)
          continue;

        String name = BuildString(mBaseMeshName, "_", CleanAssetName(clip.mName));

        GeometryResourceEntry entry;
        entry.mName = name;

        // Ignore attempts to make duplicate names.
        if (animEntries.Contains(entry))
          continue;

        // Get resource id if this name already had one, otherwise make a new
        // one.
        if (GeometryResourceEntry* previousEntry = animBuilder->mAnimations.FindPointer(entry))
          entry.mResourceId = previousEntry->mResourceId;
        else
          entry.mResourceId = GenerateUniqueId64();

        animEntries.PushBack(entry);
      }
    }
    else
    {
      for (uint i = 0; i < animationCount; ++i)
      {
        aiAnimation* animation = animations[i];

        String name = mBaseMeshName;
        if (animationCount > 1)
          name = BuildString(name, "_", CleanAssetName(animation->mName.C_Str()));

        GeometryResourceEntry entry;
        entry.mName = name;

        // Get resource id if this name already had one, otherwise make a new
        // one.
        if (GeometryResourceEntry* previousEntry = animBuilder->mAnimations.FindPointer(entry))
          entry.mResourceId = previousEntry->mResourceId;
        else
          entry.mResourceId = GenerateUniqueId64();

        animEntries.PushBack(entry);
      }
    }

    if (animBuilder->mAnimations != animEntries)
    {
      animBuilder->mAnimations = animEntries;
      metaChanges = true;
    }
  }

  TextureContent* textureContent = mGeometryContent->has(TextureContent);
  if (mScene->HasTextures() && textureContent != nullptr)
  {
    aiTexture** textures = mScene->mTextures;
    uint textureCount = mScene->mNumTextures;

    Array<GeometryResourceEntry> textureEntries;

    for (uint i = 0; i < textureCount; ++i)
    {
      aiTexture* texture = textures[i];

      // We actually expect a 'compressed' texture format
      if (texture->mHeight != 0)
        continue;

      String extension = texture->achFormatHint;
      if (IsSupportedImageLoadExtension(extension))
      {
        GeometryResourceEntry entry;
        entry.mName = BuildString(FilePath::GetFileNameWithoutExtension(mInputFile), ToString(i), ".", extension);

        // Get resource id if this name already had one, otherwise make a new
        // one.
        if (GeometryResourceEntry* previousEntry = textureContent->mTextures.FindPointer(entry))
          entry.mResourceId = previousEntry->mResourceId;
        else
          entry.mResourceId = GenerateUniqueId64();

        textureEntries.PushBack(entry);
      }
    }

    if (textureContent->mTextures != textureEntries)
    {
      textureContent->mTextures = textureEntries;
      metaChanges = true;
    }
  }

  if (metaChanges)
    SaveToDataFile(*mGeometryContent, mMetaFile);
  return metaChanges;
}

String GeometryImporter::ProcessAssimpErrorMessage(StringParam errorMessage)
{
  // This string is output when the FBX file experiences a parsing error, most
  // likely a result of the format but assimp attempts to parse the entire FBX
  // DOM before checking if the format is supported
  if (errorMessage.Contains("FBX-Tokenize"))
    return String("FBX Parsing Error. Supported formats are FBX 2011, FBX 2012 "
                  "and FBX 2013.");

  return errorMessage;
}

} // namespace Raverie
