// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Zero
{

ZilchShaderGenerator* CreateZilchShaderGenerator()
{
  ZilchShaderGenerator* shaderGenerator = new ZilchShaderGenerator();
  shaderGenerator->Initialize();
  return shaderGenerator;
}

ZilchShaderGenerator::ZilchShaderGenerator() : mFragmentsProject("Fragments")
{
}

ZilchShaderGenerator::~ZilchShaderGenerator()
{
}

void ZilchShaderGenerator::Initialize()
{
  ShaderSettingsLibrary::GetInstance().BuildLibrary();

  // Pre map every attribute value for quick processing of sampler shader inputs
  mSamplerAttributeValues["TextureAddressingXClamp"] = SamplerSettings::AddressingX(TextureAddressing::Clamp);
  mSamplerAttributeValues["TextureAddressingXRepeat"] = SamplerSettings::AddressingX(TextureAddressing::Repeat);
  mSamplerAttributeValues["TextureAddressingXMirror"] = SamplerSettings::AddressingX(TextureAddressing::Mirror);
  mSamplerAttributeValues["TextureAddressingYClamp"] = SamplerSettings::AddressingY(TextureAddressing::Clamp);
  mSamplerAttributeValues["TextureAddressingYRepeat"] = SamplerSettings::AddressingY(TextureAddressing::Repeat);
  mSamplerAttributeValues["TextureAddressingYMirror"] = SamplerSettings::AddressingY(TextureAddressing::Mirror);
  mSamplerAttributeValues["TextureFilteringNearest"] = SamplerSettings::Filtering(TextureFiltering::Nearest);
  mSamplerAttributeValues["TextureFilteringBilinear"] = SamplerSettings::Filtering(TextureFiltering::Bilinear);
  mSamplerAttributeValues["TextureFilteringTrilinear"] = SamplerSettings::Filtering(TextureFiltering::Trilinear);
  mSamplerAttributeValues["TextureCompareModeDisabled"] = SamplerSettings::CompareMode(TextureCompareMode::Disabled);
  mSamplerAttributeValues["TextureCompareModeEnabled"] = SamplerSettings::CompareMode(TextureCompareMode::Enabled);
  mSamplerAttributeValues["TextureCompareFuncNever"] = SamplerSettings::CompareFunc(TextureCompareFunc::Never);
  mSamplerAttributeValues["TextureCompareFuncAlways"] = SamplerSettings::CompareFunc(TextureCompareFunc::Always);
  mSamplerAttributeValues["TextureCompareFuncLess"] = SamplerSettings::CompareFunc(TextureCompareFunc::Less);
  mSamplerAttributeValues["TextureCompareFuncLessEqual"] = SamplerSettings::CompareFunc(TextureCompareFunc::LessEqual);
  mSamplerAttributeValues["TextureCompareFuncGreater"] = SamplerSettings::CompareFunc(TextureCompareFunc::Greater);
  mSamplerAttributeValues["TextureCompareFuncGreaterEqual"] =
      SamplerSettings::CompareFunc(TextureCompareFunc::GreaterEqual);
  mSamplerAttributeValues["TextureCompareFuncEqual"] = SamplerSettings::CompareFunc(TextureCompareFunc::Equal);
  mSamplerAttributeValues["TextureCompareFuncNotEqual"] = SamplerSettings::CompareFunc(TextureCompareFunc::NotEqual);

  InitializeSpirV();
}

void ZilchShaderGenerator::InitializeSpirV()
{
  SpirVNameSettings nameSettings;

  nameSettings.mPerspectiveToApiPerspectiveName = "ZeroPerspectiveToApiPerspective";
  nameSettings.mAllowedClassAttributes.Insert("Protected", AttributeInfo());
  nameSettings.mAllowedClassAttributes.Insert("CoreVertex", AttributeInfo());
  nameSettings.mAllowedClassAttributes.Insert("RenderPass", AttributeInfo());
  nameSettings.mAllowedClassAttributes.Insert("PostProcess", AttributeInfo());
  // Temporarily make compute fragments illegal (not supported in zero yet)
  nameSettings.mAllowedClassAttributes.Erase(nameSettings.mComputeAttribute);
  nameSettings.mAllowedFieldAttributes.Insert("Hidden", AttributeInfo());
  nameSettings.mAllowedFieldAttributes.Insert(PropertyAttributes::cGroup, AttributeInfo());
  nameSettings.mAllowedFieldAttributes.Insert(PropertyAttributes::cRange, AttributeInfo());
  nameSettings.mAllowedFieldAttributes.Insert(PropertyAttributes::cSlider, AttributeInfo());

  // Add names to list of allowed attributes in zilch fragments
  forRange (String& attribute, mSamplerAttributeValues.Keys())
    nameSettings.mAllowedFieldAttributes.Insert(attribute, AttributeInfo());

  mSpirVSettings = new ZilchShaderSpirVSettings(nameSettings);
  ZilchShaderSpirVSettings* settings = mSpirVSettings;
  settings->SetMaxSimultaneousRenderTargets(4);
  settings->SetRenderTargetName("Target0", 0);
  settings->SetRenderTargetName("Target1", 1);
  settings->SetRenderTargetName("Target2", 2);
  settings->SetRenderTargetName("Target3", 3);

  // Built-in inputs

  Zilch::BoundType* realType = ZilchTypeId(Zilch::Real);
  Zilch::BoundType* real2Type = ZilchTypeId(Zilch::Real2);
  Zilch::BoundType* real3Type = ZilchTypeId(Zilch::Real3);
  Zilch::BoundType* real4Type = ZilchTypeId(Zilch::Real4);
  Zilch::BoundType* intType = ZilchTypeId(Zilch::Integer);
  Zilch::BoundType* int4Type = ZilchTypeId(Zilch::Integer4);
  Zilch::BoundType* real3x3Type = ZilchTypeId(Zilch::Real3x3);
  Zilch::BoundType* real4x4Type = ZilchTypeId(Zilch::Real4x4);

  // Vertex Attributes
  VertexDefinitionDescription& vertexDefDesc = settings->mVertexDefinitions;
  vertexDefDesc.AddField(real3Type, "LocalPosition");
  vertexDefDesc.AddField(real3Type, "LocalNormal");
  vertexDefDesc.AddField(real3Type, "LocalTangent");
  vertexDefDesc.AddField(real3Type, "LocalBitangent");
  vertexDefDesc.AddField(real2Type, "Uv");
  vertexDefDesc.AddField(real2Type, "UvAux");
  vertexDefDesc.AddField(real4Type, "Color");
  vertexDefDesc.AddField(real4Type, "ColorAux");
  vertexDefDesc.AddField(intType, "BoneIndices");
  vertexDefDesc.AddField(real4Type, "BoneWeights");
  vertexDefDesc.AddField(real4Type, "Aux0");
  vertexDefDesc.AddField(real4Type, "Aux1");
  vertexDefDesc.AddField(real4Type, "Aux2");
  vertexDefDesc.AddField(real4Type, "Aux3");
  vertexDefDesc.AddField(real4Type, "Aux4");
  vertexDefDesc.AddField(real4Type, "Aux5");

  // Setup uniform buffers
  UniformBufferDescription frameData(0);
  frameData.mDebugName = "FrameData";
  frameData.AddField(realType, "LogicTime");
  frameData.AddField(realType, "FrameTime");
  settings->AddUniformBufferDescription(frameData);

  // Setup uniform buffers
  UniformBufferDescription cameraData(1);
  cameraData.mDebugName = "CameraData";
  cameraData.AddField(realType, "NearPlane");
  cameraData.AddField(realType, "FarPlane");
  cameraData.AddField(real2Type, "ViewportSize");
  cameraData.AddField(real2Type, "InverseViewportSize");
  cameraData.AddField(real3Type, "ObjectWorldPosition");
  settings->AddUniformBufferDescription(cameraData);

  // Setup uniform buffers
  UniformBufferDescription transformData(2);
  transformData.mDebugName = "TransformData";
  transformData.AddField(real4x4Type, "LocalToWorld");
  transformData.AddField(real4x4Type, "WorldToLocal");
  transformData.AddField(real4x4Type, "WorldToView");
  transformData.AddField(real4x4Type, "ViewToWorld");
  transformData.AddField(real4x4Type, "LocalToView");
  transformData.AddField(real4x4Type, "ViewToLocal");
  transformData.AddField(real3x3Type, "LocalToViewNormal");
  transformData.AddField(real3x3Type, "ViewToLocalNormal");
  transformData.AddField(real3x3Type, "LocalToWorldNormal");
  transformData.AddField(real3x3Type, "WorldToLocalNormal");
  transformData.AddField(real4x4Type, "LocalToPerspective");
  transformData.AddField(real4x4Type, "ViewToPerspective");
  transformData.AddField(real4x4Type, "PerspectiveToView");
  transformData.AddField(real4x4Type, nameSettings.mPerspectiveToApiPerspectiveName);
  settings->AddUniformBufferDescription(transformData);

  // UniformBufferDescription miscData(3);
  // miscData.mDebugName = "MiscData";
  // settings->mShaderDefinitionSettings.AddBuiltIn("BoneTransforms",
  // "FixedArray[Real4x4, 80]");
  // SpriteSource input
  // settings->mShaderDefinitionSettings.AddBuiltIn("SpriteSource",
  // "Sampler2d");
  //// HeightMap input
  // settings->mShaderDefinitionSettings.AddBuiltIn("HeightMapWeights",
  // "Sampler2d");

  settings->AutoSetDefaultUniformBufferDescription();

  settings->SetHardwareBuiltInName(spv::BuiltInPosition, "ApiPerspectivePosition");

  // Utility

  // Set custom callbacks in both the compositor and entry point code generation
  // for dealing with perspective position vs. api perspective position.
  settings->mCallbackSettings.SetCompositeCallback(&ZilchShaderIRCompositor::ApiPerspectivePositionCallback, nullptr);
  settings->mCallbackSettings.SetAppendCallback(&EntryPointGeneration::PerspectiveTransformAppendVertexCallback,
                                                nullptr);

  settings->Finalize();

  mFrontEndTranslator = new ZilchSpirVFrontEnd();
  mFrontEndTranslator->SetSettings(mSpirVSettings);
  mFrontEndTranslator->Setup();
  // Create the core library and parse it
  ZilchShaderIRCore::InitializeInstance();
  ZilchShaderIRCore& coreLibrary = ZilchShaderIRCore::GetInstance();
  coreLibrary.Parse(mFrontEndTranslator);
  mCoreLibrary = coreLibrary.GetLibrary();

  // Create the intrinsics library and parse it
  ShaderIntrinsicsStaticZilchLibrary::InitializeInstance();
  ShaderIntrinsicsStaticZilchLibrary& shaderIntrinsics = ShaderIntrinsicsStaticZilchLibrary::GetInstance();
  shaderIntrinsics.Parse(mFrontEndTranslator);
  mShaderIntrinsicsLibrary = shaderIntrinsics.GetLibrary();

  // Connect error events
  EventConnect(&mFragmentsProject,
               Zilch::Events::CompilationError,
               &ZilchShaderGenerator::OnZilchFragmentCompilationError,
               this);
  EventConnect(&mFragmentsProject, Zilch::Events::TypeParsed, &ZilchShaderGenerator::OnZilchFragmentTypeParsed, this);
  EventConnect(
      &mFragmentsProject, Events::TranslationError, &ZilchShaderGenerator::OnZilchFragmentTranslationError, this);
  EventConnect(
      &mFragmentsProject, Events::ValidationError, &ZilchShaderGenerator::OnZilchFragmentValidationError, this);
}

LibraryRef BuildWrapperLibrary(ZilchShaderIRLibraryRef fragmentsLibrary)
{
  // @Nate: Maybe put this somewhere else?
  // Build a lookup map to deal with opaque shader types
  HashMap<String, String> lookupMap;
  lookupMap["SampledImage2d"] = "Texture";
  lookupMap["SampledImage3d"] = "Texture3d";
  lookupMap["SampledImageCube"] = "Texture";
  lookupMap["SampledDepthImage2d"] = "Texture";

  // Helper structure for collecting property info.
  struct ShaderPropertyInfo
  {
    String mName;
    ShaderIRFieldMeta* mShaderField;
    BoundType* mBoundType;
    size_t mMemberOffset;
    bool mIsTexture;
  };

  // The Zero editor attributes to check for valid crossover attributes from the
  // shader types.
  AttributeExtensions* zeroAttributes = AttributeExtensions::GetInstance();

  String wrapperLibraryName = BuildString(fragmentsLibrary->mZilchLibrary->Name, "Wrapper");
  LibraryBuilder builder(wrapperLibraryName);

  forRange (ZilchShaderIRType* shaderType, fragmentsLibrary->mTypes.Values())
  {
    // Only add fragment types.
    ShaderIRTypeMeta* shaderTypeMeta = shaderType->mMeta;
    if (shaderTypeMeta == nullptr)
      continue;

    if (shaderTypeMeta->mFragmentType != FragmentType::None)
    {
      size_t baseSize = sizeof(MaterialBlock);
      size_t fragmentSize = baseSize;

      Array<ShaderPropertyInfo> shaderProperties;

      // Calculate the fragment type's size and get info needed to bind
      // properties.
      forRange (ShaderIRFieldMeta* field, shaderTypeMeta->mFields.All())
      {
        if (field->mZilchType->Name.Contains("FixedArray"))
          continue;

        if (field->ContainsAttribute("PropertyInput"))
        {
          // Property type should be a primitive or a texture.
          String propertyTypeName = lookupMap.FindValue(field->mZilchType->Name, field->mZilchType->Name);
          BoundType* type = MetaDatabase::GetInstance()->FindType(propertyTypeName);
          if (type != nullptr)
          {
            ShaderPropertyInfo shaderProperty;
            shaderProperty.mName = field->mZilchName;
            shaderProperty.mShaderField = field;
            shaderProperty.mBoundType = type;

            size_t fieldSize;
            const size_t byteAlignment = sizeof(MaxAlignmentType);

            if (type->IsA(ZilchTypeId(Texture)))
            {
              // Textures are stored by their ID's.
              fieldSize = sizeof(u64);
              // Textures need a different getter/setter.
              shaderProperty.mIsTexture = true;
            }
            else
            {
              fieldSize = type->GetCopyableSize();
              shaderProperty.mIsTexture = false;
            }

            // Compute pad bytes needed for placing this data member on correct
            // alignment.
            size_t pad = (byteAlignment - fragmentSize % byteAlignment) % byteAlignment;
            fragmentSize += pad;

            // Memory offset from start of derived type.
            shaderProperty.mMemberOffset = fragmentSize - baseSize;
            shaderProperties.PushBack(shaderProperty);

            fragmentSize += fieldSize;
          }
        }
      }

      // Pad type out to largest stored primitve.
      size_t largestSize = sizeof(MaxAlignmentType);
      size_t endPad = (largestSize - fragmentSize % largestSize) % largestSize;
      fragmentSize += endPad;

      // Create fragment type.
      BoundType* boundType =
          builder.AddBoundType(shaderTypeMeta->mZilchName, TypeCopyMode::ReferenceType, fragmentSize);
      boundType->BaseType = ZilchTypeId(MaterialBlock);
      // Associate this type to the ZilchFragment resource containing the shader
      // type.
      boundType->Add(new MetaResource((Resource*)shaderTypeMeta->mZilchType->Location.CodeUserData));

      builder.AddBoundDefaultConstructor(boundType, FragmentConstructor);
      builder.AddBoundDestructor(boundType, FragmentDestructor);

      // Allocate one instance of the derived type and store it on the BoundType
      // for initializing fragments to the default values.
      ByteBufferBlock& defaultMemory = boundType->ComplexUserData.CreateObject<ByteBufferBlock>();
      size_t derivedTypeSize = fragmentSize - baseSize;
      defaultMemory.SetData((byte*)zAllocate(derivedTypeSize), derivedTypeSize, true);
      byte* defaultMemoryPtr = defaultMemory.GetBegin();
      memset(defaultMemoryPtr, 0, defaultMemory.Size());

      // Add all found properties to the new type.
      forRange (ShaderPropertyInfo& shaderProperty, shaderProperties.All())
      {
        // Address of the default value for this property.
        byte* defaultElement = defaultMemoryPtr + shaderProperty.mMemberOffset;

        BoundFn getter;
        BoundFn setter;
        if (shaderProperty.mIsTexture)
        {
          getter = FragmentTextureGetter;
          setter = FragmentTextureSetter;

          // Set default value.
          Texture* texture = shaderProperty.mShaderField->mDefaultValueVariant.Get<Texture*>();
          if (texture != nullptr)
            *(u64*)defaultElement = (u64)texture->mResourceId;
        }
        else
        {
          getter = FragmentGetter;
          setter = FragmentSetter;

          // Set default value.
          shaderProperty.mShaderField->mDefaultValueVariant.CopyStoredValueTo(defaultElement);
        }

        // Create property.
        GetterSetter* getterSetter = builder.AddBoundGetterSetter(
            boundType, shaderProperty.mName, shaderProperty.mBoundType, setter, getter, MemberOptions::None);
        // Storing member offset on the property meta for generic getter/setter
        // implementation.
        getterSetter->UserData = (void*)shaderProperty.mMemberOffset;
        getterSetter->Get->UserData = (void*)shaderProperty.mMemberOffset;
        getterSetter->Set->UserData = (void*)shaderProperty.mMemberOffset;
        // Currently just setting the type location, a more specific location
        // for properties still needs to be added.
        getterSetter->NameLocation = shaderProperty.mShaderField->mZilchType->NameLocation;
        getterSetter->Location = shaderProperty.mShaderField->mZilchType->Location;

        // Mark all bound getter/setters as property.
        getterSetter->AddAttribute(PropertyAttributes::cProperty);

        // Add all other valid attributes from shader field.
        forRange (ShaderIRAttribute& shaderAttribute, shaderProperty.mShaderField->mAttributes.All())
        {
          if (zeroAttributes->IsValidPropertyAttribute(shaderAttribute.mAttributeName))
          {
            Attribute* attribute = getterSetter->AddAttribute(shaderAttribute.mAttributeName);
            forRange (ShaderIRAttributeParameter& attributeParam, shaderAttribute.mParameters.All())
            {
              Zilch::AttributeParameter& param = attributeParam.GetZilchAttributeParameter();
              attribute->Parameters.PushBack(param);
            }
          }
        }
      }

      // Set file location for this type.
      boundType->NameLocation = shaderTypeMeta->mZilchType->NameLocation;
      boundType->Location = shaderTypeMeta->mZilchType->Location;
    }
  }

  // Adds the component getters for the fragment types.
  forRange (BoundType* boundType, builder.BoundTypes.Values())
  {
    AttributeStatus status;
    AttributeExtensions::GetInstance()->ProcessType(status, boundType, false);
    if (status.Failed())
      ZPrint(status.Message.c_str());
    MetaLibraryExtensions::ProcessComponent(builder, boundType);
  }

  return builder.CreateLibrary();
}

LibraryRef ZilchShaderGenerator::BuildFragmentsLibrary(Module& dependencies,
                                                       Array<ZilchDocumentResource*>& fragments,
                                                       StringParam libraryName)
{
  mFragmentsProject.Clear();
  mFragmentsProject.mProjectName = libraryName;

  // Add all fragments
  forRange (Resource* resource, fragments.All())
  {
    // Templates shouldn't be compiled. They contain potentially invalid code
    // and identifiers such as RESOURCE_NAME_ that are replaced when a new
    // resource is created from the template
    if (resource->GetResourceTemplate())
      continue;

    ZilchFragment* fragment = (ZilchFragment*)resource;
    mFragmentsProject.AddCodeFromString(fragment->mText, fragment->GetOrigin(), resource);
  }

  // Internal dependencies used to build the internal library
  ZilchShaderIRModuleRef internalDependencies = new ZilchShaderIRModule();
  internalDependencies->PushBack(mCoreLibrary);
  internalDependencies->PushBack(mShaderIntrinsicsLibrary);

  // We gave the engine our "wrapped" library, and it's giving them back as
  // dependencies. For fragment compilation, it expects our internal libraries,
  // so we need to look them up and use those instead
  forRange (Library* dependentLibrary, dependencies.All())
  {
    ZilchShaderIRLibraryRef internalDependency = GetInternalLibrary(dependentLibrary);
    internalDependencies->Append(internalDependency);
  }

  ZilchShaderIRLibraryRef fragmentsLibrary =
      mFragmentsProject.CompileAndTranslate(internalDependencies, mFrontEndTranslator);
  if (fragmentsLibrary == nullptr)
    return nullptr;

  // Write to the complex user data of each shader type the name of the resource
  // they came from. This has to be done as a second pass because complex user
  // currently can't be written per file (we also need per type).
  forRange (ZilchShaderIRType* shaderType, fragmentsLibrary->mTypes.Values())
  {
    ShaderIRTypeMeta* shaderTypeMeta = shaderType->mMeta;
    // Not all types in the library will have meta (such as ref/pointer types)
    if (shaderTypeMeta == nullptr)
      continue;

    Resource* resource = (Resource*)shaderTypeMeta->mZilchType->Location.CodeUserData;
    // If we have a valid user data (some types won't, like arrays that are
    // generated in your library)
    if (resource == nullptr)
      continue;
    FragmentUserData complexUserData(resource->Name);
    shaderTypeMeta->mComplexUserData.WriteObject(complexUserData);
  }

  // Build the wrapper library around the internal library
  LibraryRef library = BuildWrapperLibrary(fragmentsLibrary);

  if (library == nullptr)
    return nullptr;

  // If pending changes cause scripts to not compile, and then fragments are
  // changed again to fix it, will have duplicate pending libraries that should
  // be replaced. Libraries aren't mapped by name so find it manually.
  forRange (LibraryRef pendingLib, mPendingToPendingInternal.Keys())
  {
    if (pendingLib->Name == library->Name)
    {
      mPendingToPendingInternal.Erase(pendingLib);
      break;
    }
  }

  mPendingToPendingInternal.Insert(library, fragmentsLibrary);

  ZilchFragmentTypeMap& fragmentTypes = mPendingFragmentTypes[library];
  fragmentTypes.Clear();
  forRange (BoundType* boundType, library->BoundTypes.Values())
  {
    ZilchShaderIRType* shaderType = fragmentsLibrary->FindType(boundType->Name);
    ShaderIRTypeMeta* shaderTypeMeta = shaderType->mMeta;

    if (shaderTypeMeta->ContainsAttribute("CoreVertex") && shaderTypeMeta->ContainsAttribute("Vertex"))
      fragmentTypes.Insert(boundType->Name, ZilchFragmentType::CoreVertex);
    else if (shaderTypeMeta->ContainsAttribute("RenderPass") && shaderTypeMeta->ContainsAttribute("Pixel"))
      fragmentTypes.Insert(boundType->Name, ZilchFragmentType::RenderPass);
    else if (shaderTypeMeta->ContainsAttribute("PostProcess") && shaderTypeMeta->ContainsAttribute("Pixel"))
      fragmentTypes.Insert(boundType->Name, ZilchFragmentType::PostProcess);
    else if (shaderTypeMeta->ContainsAttribute("Protected"))
      fragmentTypes.Insert(boundType->Name, ZilchFragmentType::Protected);
    else
      fragmentTypes.Insert(boundType->Name, ZilchFragmentType::Fragment);
  }

  return library;
}

bool ZilchShaderGenerator::Commit(ZilchCompileEvent* e)
{
  // Don't need to do anything if there are no pending libraries
  if (mPendingToPendingInternal.Empty())
    return false;

  // Remove all old libraries
  forRange (ResourceLibrary* library, e->mModifiedLibraries.All())
  {
    Library* pendingLibrary = library->mSwapFragment.mPendingLibrary;
    if (pendingLibrary)
    {
      ZilchShaderIRLibraryRef internalPendingLibrary = mPendingToPendingInternal.FindValue(pendingLibrary, nullptr);
      ErrorIf(internalPendingLibrary == nullptr, "Invalid pending library");

      mCurrentToInternal.Erase(library->mSwapFragment.mCurrentLibrary);
      mCurrentToInternal.Insert(pendingLibrary, internalPendingLibrary);
      mPendingToPendingInternal.Erase(pendingLibrary);
    }
  }

  ErrorIf(!mPendingToPendingInternal.Empty(), "We created a new library but it was not given to commit");
  // Clear after assert so it's not repeated or leaked.
  mPendingToPendingInternal.Clear();

  MapFragmentTypes();

  return true;
}

void ZilchShaderGenerator::MapFragmentTypes()
{
  mPendingFragmentTypes.Clear();

  mCoreVertexFragments.Clear();
  mRenderPassFragments.Clear();
  mFragmentTypes.Clear();

  // Find fragments needed for shader permutations
  forRange (LibraryRef wrapperLibrary, mCurrentToInternal.Keys())
  {
    ZilchShaderIRLibraryRef fragmentLibrary = GetInternalLibrary(wrapperLibrary);
    forRange (BoundType* boundType, wrapperLibrary->BoundTypes.Values())
    {
      ZilchShaderIRType* shaderType = fragmentLibrary->FindType(boundType->Name);
      ShaderIRTypeMeta* shaderTypeMeta = shaderType->mMeta;

      if (shaderTypeMeta->ContainsAttribute("CoreVertex") && shaderTypeMeta->ContainsAttribute("Vertex"))
      {
        mCoreVertexFragments.PushBack(boundType->Name);
        mFragmentTypes.Insert(boundType->Name, ZilchFragmentType::CoreVertex);
      }
      else if (shaderTypeMeta->ContainsAttribute("RenderPass") && shaderTypeMeta->ContainsAttribute("Pixel"))
      {
        mRenderPassFragments.PushBack(boundType->Name);
        mFragmentTypes.Insert(boundType->Name, ZilchFragmentType::RenderPass);
      }
      else if (shaderTypeMeta->ContainsAttribute("PostProcess") && shaderTypeMeta->ContainsAttribute("Pixel"))
      {
        mFragmentTypes.Insert(boundType->Name, ZilchFragmentType::PostProcess);
      }
      else if (shaderTypeMeta->ContainsAttribute("Protected"))
      {
        mFragmentTypes.Insert(boundType->Name, ZilchFragmentType::Protected);
      }
      else
      {
        mFragmentTypes.Insert(boundType->Name, ZilchFragmentType::Fragment);
      }
    }
  }
}

bool ZilchShaderGenerator::BuildShaders(ShaderSet& shaders,
                                        HashMap<String, UniqueComposite>& composites,
                                        Array<ShaderEntry>& shaderEntries,
                                        Array<ShaderDefinition>* compositeShaderDefs)
{
  // @Nate: Build a description of the pipeline tools to run.
  // This could be cached and down the line should probably be
  // split up to deal with multiple libraries and caching.
  ShaderPipelineDescription pipelineDescription;
#if !defined(ZeroDebug)
  pipelineDescription.mToolPasses.PushBack(new SpirVSpecializationConstantPass());
  pipelineDescription.mToolPasses.PushBack(new SpirVOptimizerPass());
#endif
  ZeroZilchShaderGlslBackend* backend = new ZeroZilchShaderGlslBackend();
  pipelineDescription.mBackend = backend;

#ifdef WelderTargetOsEmscripten
  backend->mTargetVersion = 300;
  backend->mTargetGlslEs = true;
#endif

  ZilchShaderIRCompositor compositor;

  ZilchShaderIRLibraryRef fragmentsLibrary = GetCurrentInternalProjectLibrary();

  Array<Shader*> shaderArray;
  shaderArray.Append(shaders.All());

  // Value should not be very large to prevent unnecessary memory consumption to
  // compile.
  const size_t compositeBatchCount = 20;

  size_t totalShaderCount = shaderArray.Size();
  for (size_t startIndex = 0; startIndex < totalShaderCount; startIndex += compositeBatchCount)
  {
    ZilchShaderIRProject shaderProject("ShaderProject");

    // All batches are added to this array, get start index for this batch.
    size_t entryStartIndex = shaderEntries.Size();

    size_t endIndex = Math::Min(startIndex + compositeBatchCount, totalShaderCount);
    for (size_t i = startIndex; i < endIndex; ++i)
    {
      Shader* shader = shaderArray[i];
      ZPrint("Compositing %s\n", shader->mName.c_str());

      // Make shader def.
      ZilchShaderIRCompositor::ShaderDefinition shaderDef;
      shaderDef.mShaderName = shader->mName;

      ZilchShaderIRLibrary* lib = fragmentsLibrary;

      // CoreVertex
      ErrorIf(fragmentsLibrary->FindType(shader->mCoreVertex) == nullptr, "No fragment.");
      shaderDef.mFragments.PushBack(fragmentsLibrary->FindType(shader->mCoreVertex));

      // Composition
      if (composites.ContainsKey(shader->mComposite))
      {
        forRange (String fragmentName, composites[shader->mComposite].mFragmentNames.All())
        {
          ErrorIf(fragmentsLibrary->FindType(fragmentName) == nullptr, "No fragment.");
          shaderDef.mFragments.PushBack(fragmentsLibrary->FindType(fragmentName));
        }
      }
      else
      {
        String fragmentName = shader->mComposite;
        ErrorIf(fragmentsLibrary->FindType(fragmentName) == nullptr, "No fragment.");
        shaderDef.mFragments.PushBack(fragmentsLibrary->FindType(fragmentName));
      }

      // ApiPerspective
      ZilchShaderIRType* apiPerspectiveOutput = fragmentsLibrary->FindType("ApiPerspectiveOutput");
      ErrorIf(apiPerspectiveOutput == nullptr, "Missing fragment from core library.");
      shaderDef.mFragments.PushBack(apiPerspectiveOutput);

      // RenderPass
      if (shader->mRenderPass.Empty() == false)
      {
        ErrorIf(fragmentsLibrary->FindType(shader->mRenderPass) == nullptr, "No fragment.");
        shaderDef.mFragments.PushBack(fragmentsLibrary->FindType(shader->mRenderPass));
      }

      // @Nate: Should be separated at some point.
      // Currently only is used to emit errors on unsupported shader stages.
      ShaderCapabilities capabilities;
      compositor.Composite(shaderDef, capabilities, mSpirVSettings);

      // If the user requested it, then add the resulting shader def as an
      // output. Used for debugging purposes to display the zilch composites.
      if (compositeShaderDefs != nullptr)
        compositeShaderDefs->PushBack(shaderDef);

      ZilchShaderIRCompositor::ShaderStageDescription& vertexInfo = shaderDef.mResults[FragmentType::Vertex];
      ZilchShaderIRCompositor::ShaderStageDescription& geometryInfo = shaderDef.mResults[FragmentType::Geometry];
      ZilchShaderIRCompositor::ShaderStageDescription& pixelInfo = shaderDef.mResults[FragmentType::Pixel];

      shaderProject.AddCodeFromString(vertexInfo.mShaderCode, vertexInfo.mClassName, nullptr);
      shaderProject.AddCodeFromString(geometryInfo.mShaderCode, geometryInfo.mClassName, nullptr);
      shaderProject.AddCodeFromString(pixelInfo.mShaderCode, pixelInfo.mClassName, nullptr);

      ShaderEntry entry(shader);
      entry.mVertexShader = vertexInfo.mClassName;
      entry.mGeometryShader = geometryInfo.mClassName;
      entry.mPixelShader = pixelInfo.mClassName;
      shaderEntries.PushBack(entry);

      shader->mSentToRenderer = true;
    }

    ZilchShaderIRModuleRef shaderDependencies = new ZilchShaderIRModule();
    shaderDependencies->PushBack(fragmentsLibrary);

    EventConnect(
        &shaderProject, Zilch::Events::CompilationError, &ZilchShaderGenerator::OnZilchFragmentCompilationError, this);
    ZilchShaderIRLibraryRef shaderLibrary = shaderProject.CompileAndTranslate(shaderDependencies, mFrontEndTranslator);

    if (shaderLibrary == nullptr)
    {
      DoNotifyError("Shader Error", "Failed to build shader library.");
      return false;
    }

    for (size_t i = entryStartIndex; i < shaderEntries.Size(); ++i)
    {
      ShaderEntry& entry = shaderEntries[i];

      ZilchShaderIRType* vertexShader = shaderLibrary->FindType(entry.mVertexShader);
      ZilchShaderIRType* geometryShader = shaderLibrary->FindType(entry.mGeometryShader);
      ZilchShaderIRType* pixelShader = shaderLibrary->FindType(entry.mPixelShader);
      ErrorIf(vertexShader == nullptr || pixelShader == nullptr, "Invalid shader entry");

      bool success = true;
      Array<TranslationPassResultRef> vertexPipelineResults;
      success &= CompilePipeline(vertexShader, pipelineDescription, vertexPipelineResults);

      Array<TranslationPassResultRef> geometryPipelineResults;
      if (geometryShader != nullptr)
        success &= CompilePipeline(geometryShader, pipelineDescription, geometryPipelineResults);

      Array<TranslationPassResultRef> pixelPipelineResults;
      success &= CompilePipeline(pixelShader, pipelineDescription, pixelPipelineResults);

      if (!success)
        return false;

      entry.mVertexShader = vertexPipelineResults.Back()->mByteStream.ToString();
      if (geometryShader != nullptr)
        entry.mGeometryShader = geometryPipelineResults.Back()->mByteStream.ToString();
      entry.mPixelShader = pixelPipelineResults.Back()->mByteStream.ToString();
    }
  }

  return true;
}

bool ZilchShaderGenerator::CompilePipeline(ZilchShaderIRType* shaderType,
                                           ShaderPipelineDescription& pipeline,
                                           Array<TranslationPassResultRef>& pipelineResults)
{
  if (shaderType == nullptr)
    return false;

  ShaderTranslationPassResult* binaryBackendData = new ShaderTranslationPassResult();
  pipelineResults.PushBack(binaryBackendData);

  // Convert from the in-memory format of spir-v to actual binary (array of
  // words)
  ShaderByteStreamWriter byteWriter(&binaryBackendData->mByteStream);
  ZilchShaderSpirVBinaryBackend binaryBackend;
  binaryBackend.TranslateType(shaderType, byteWriter, binaryBackendData->mReflectionData);

  // Run each tool in the pipeline
  for (size_t i = 0; i < pipeline.mToolPasses.Size(); ++i)
  {
    ZilchShaderIRTranslationPass* translationPass = pipeline.mToolPasses[i];

    ShaderTranslationPassResult* prevPassData = pipelineResults.Back();
    ErrorIf(prevPassData->mByteStream.ByteCount() == 0, "No shader bytecode input");
    ShaderTranslationPassResult* toolData = new ShaderTranslationPassResult();
    pipelineResults.PushBack(toolData);

    translationPass->RunTranslationPass(*prevPassData, *toolData);
    ErrorIf(toolData->mByteStream.ByteCount() == 0, "No shader bytecode output");
  }

  ShaderTranslationPassResult* lastPassData = pipelineResults.Back();

  // @Nate: Here's examples for running debug passes like the disassembler.
  // for(size_t i = 0; i < pipeline.mDebugPasses.Size(); ++i)
  //{
  //  ZilchShaderIRTranslationPass* debugBackend = pipeline.mDebugPasses[i];
  //
  //  ShaderTranslationPassResult* prevPassData = pipelineResults.Back();
  //  ShaderTranslationPassResult* resultData = new
  //  ShaderTranslationPassResult(); debugResults.PushBack(resultData);
  //
  //  debugBackend->RunTranslationPass(*prevPassData, *resultData);
  //}

  // Run the final backend
  ShaderTranslationPassResult* backendResult = new ShaderTranslationPassResult();
  pipelineResults.PushBack(backendResult);
  pipeline.mBackend->RunTranslationPass(*lastPassData, *backendResult);

  return true;
}

ShaderInput ZilchShaderGenerator::CreateShaderInput(StringParam fragmentName,
                                                    StringParam inputName,
                                                    ShaderInputType::Enum type,
                                                    AnyParam value)
{
  ShaderInput shaderInput;

  if (type == ShaderInputType::Invalid)
    return shaderInput;

  ZilchShaderIRType* shaderType = GetCurrentInternalProjectLibrary()->FindType(fragmentName);
  if (shaderType == nullptr)
    return shaderInput;

  ShaderIRTypeMeta* shaderTypeMeta = shaderType->mMeta;
  if (shaderTypeMeta == nullptr)
    return shaderInput;

  ShaderIRFieldMeta* fieldMeta = shaderTypeMeta->FindField(inputName);
  if (fieldMeta == nullptr)
    return shaderInput;

  Any valueCopy = value;

  // Ideally this should be using shader reflection.
  // For now, this is just getting names exactly as they will appear in glsl so
  // long as the ZilchShaderGenerator does not change its setup and naming
  // schemes.
  if (type == ShaderInputType::Texture)
  {
    shaderInput.mTranslatedInputName = GenerateSpirVPropertyName(inputName, fragmentName);

    // Check for sampler settings overrides.
    forRange (ShaderIRAttribute& attribute, fieldMeta->mAttributes.All())
    {
      if (mSamplerAttributeValues.ContainsKey(attribute.mAttributeName))
        SamplerSettings::AddValue(shaderInput.mSamplerSettings, mSamplerAttributeValues[attribute.mAttributeName]);
    }
  }
  else
  {
    shaderInput.mTranslatedInputName = BuildString("Material.", GenerateSpirVPropertyName(inputName, fragmentName));
    // SPIR-V doesn't allow boolean uniforms so we convert boolean inputs from
    // Zilch to integers.
    if (type == ShaderInputType::Bool)
    {
      shaderInput.mTranslatedInputName = BuildString(shaderInput.mTranslatedInputName, "_Boolean");
      type = ShaderInputType::Int;
      valueCopy = Any((int)value.Get<bool>());
    }
  }

  // If unsuccessful returned ShaderInput's type will be Invalid, otherwise it
  // will be the passed in type
  shaderInput.mShaderInputType = type;
  ShaderInputSetValue(shaderInput, valueCopy);

  return shaderInput;
}

void ZilchShaderGenerator::OnZilchFragmentCompilationError(Zilch::ErrorEvent* event)
{
  String shortMessage = event->ExactError;
  String fullMessage = event->GetFormattedMessage(Zilch::MessageFormat::Python);
  ZilchFragmentManager::GetInstance()->DispatchScriptError(
      Events::SyntaxError, shortMessage, fullMessage, event->Location);
}

void ZilchShaderGenerator::OnZilchFragmentTypeParsed(Zilch::ParseEvent* event)
{
  // There are a lot of attributes in zilch fragments that aren't valid for
  // zilch script. Because of this, we want to ignore invalid attributes here
  // and let the fragment compilation catch them
  BoundType* boundType = event->Type;
  AttributeStatus status;
  AttributeExtensions::GetInstance()->ProcessType(status, boundType, true);
  if (status.Failed())
    event->BuildingProject->Raise(status.mLocation, ErrorCode::GenericError, status.Message.c_str());

  Status nameStatus;
  ZilchFragmentManager::GetInstance()->ValidateRawName(nameStatus, boundType->TemplateBaseName, boundType);

  if (nameStatus.Failed())
    event->BuildingProject->Raise(boundType->NameLocation, ErrorCode::GenericError, nameStatus.Message.c_str());
}

void ZilchShaderGenerator::OnZilchFragmentTranslationError(TranslationErrorEvent* event)
{
  String fullMessage = event->GetFormattedMessage(Zilch::MessageFormat::Python);
  ZilchFragmentManager::GetInstance()->DispatchScriptError(
      Events::SyntaxError, event->mShortMessage, fullMessage, event->mLocation);
}

void ZilchShaderGenerator::OnZilchFragmentValidationError(ValidationErrorEvent* event)
{
  String fullMessage = event->GetFormattedMessage(Zilch::MessageFormat::Python);
  ZilchFragmentManager::GetInstance()->DispatchScriptError(
      Events::SyntaxError, event->mShortMessage, fullMessage, event->mLocation);
}

ZilchShaderIRLibraryRef ZilchShaderGenerator::GetInternalLibrary(LibraryRef library)
{
  if (library == nullptr)
    return nullptr;

  // First look in pending
  ZilchShaderIRLibraryRef internalLibrary = GetPendingInternalLibrary(library);

  // Then look in current (if it wasn't recompiled)
  if (internalLibrary == nullptr)
    internalLibrary = GetCurrentInternalLibrary(library);

  ErrorIf(internalLibrary == nullptr, "Could not find internal library.");

  return internalLibrary;
}

ZilchShaderIRLibraryRef ZilchShaderGenerator::GetCurrentInternalLibrary(LibraryRef library)
{
  return mCurrentToInternal.FindValue(library, nullptr);
}

ZilchShaderIRLibraryRef ZilchShaderGenerator::GetPendingInternalLibrary(LibraryRef library)
{
  return mPendingToPendingInternal.FindValue(library, nullptr);
}

ZilchShaderGenerator::InternalLibraryRange ZilchShaderGenerator::GetCurrentInternalLibraries()
{
  return mCurrentToInternal.Values();
}

ZilchShaderIRLibraryRef ZilchShaderGenerator::GetCurrentInternalProjectLibrary()
{
  return GetInternalLibrary(ZilchManager::GetInstance()->mCurrentFragmentProjectLibrary);
}

ZilchShaderIRLibraryRef ZilchShaderGenerator::GetPendingInternalProjectLibrary()
{
  return GetInternalLibrary(ZilchManager::GetInstance()->mPendingFragmentProjectLibrary);
}

} // namespace Zero
