// Authors: Nathan Carlson
// Copyright 2015, DigiPen Institute of Technology

#include "Precompiled.hpp"

namespace Zero
{

//**************************************************************************************************
void CustomEmitVertexCallback(ShaderCodeBuilder& builder, ZilchShaderSettingsRef& settings, EmitVertexCallbackData& data)
{
  ShaderDefinitionSettings& shaderSettings = settings->mShaderDefinitionSettings;
  NameSettings& nameSettings = settings->mNameSettings;

  String fragmentPerspectivePositionName = "PerspectivePosition";

  // If the api perspective transform exists and the fragment contains the fragment perspective position (not the composite name)
  ShaderFieldKey apiPerspectiveTransformKey("ZeroPerspectiveToApiPerspective", "Real4x4");
  if(shaderSettings.mBuiltIns.ContainsKey(apiPerspectiveTransformKey) &&
     data.mFragmentOutputStructShaderType->mFieldMap.ContainsKey(fragmentPerspectivePositionName))
  {
    // Write: "compositeOut.ApiPerspectivePosition = Math.Multiply(`compositeName`.ZeroPerspectiveToApiPerspective, fragmentOut.PerspectivePosition)"
    builder << builder.EmitIndent() << data.mCompositeOutVarName << "." << nameSettings.mPerspectivePositionName << " = Math.Multiply(";
    builder << data.mCompositedShaderTypeName << ".ZeroPerspectiveToApiPerspective, ";
    builder << data.mFragmentOutVarName << "." << fragmentPerspectivePositionName << ");" << builder.EmitLineReturn();
  }
}

//**************************************************************************************************
ZilchShaderGenerator* CreateZilchShaderGenerator()
{
  ZilchShaderGenerator* shaderGenerator = new ZilchShaderGenerator();
  shaderGenerator->mTranslator = new Glsl150Translator();
  shaderGenerator->Initialize();
  return shaderGenerator;
}

//**************************************************************************************************
ZilchShaderGenerator::ZilchShaderGenerator()
  : mFragmentsProject("Fragments")
{
  mSettings = new ZilchShaderSettings();
}

//**************************************************************************************************
ZilchShaderGenerator::~ZilchShaderGenerator()
{
}

//**************************************************************************************************
void ZilchShaderGenerator::Initialize()
{
  if(mTranslator == nullptr)
    mTranslator = new Glsl150Translator();

  ShaderSettingsLibrary::GetInstance().BuildLibrary();
  EventConnect(&mFragmentsProject, Zilch::Events::CompilationError, &ZilchShaderGenerator::OnZilchFragmentCompilationError, this);
  EventConnect(&mFragmentsProject, Zilch::Events::TypeParsed, &ZilchShaderGenerator::OnZilchFragmentTypeParsed, this);
  EventConnect(&mFragmentsProject, Events::TranslationError, &ZilchShaderGenerator::OnZilchFragmentTranslationError, this);

  MainConfig* mainConfig = Z::gEngine->GetConfigCog()->has(MainConfig);

  // Set the default render target names
  // max target count needs to be queried by the renderer
  ZilchShaderSettings* settings = mSettings;

  settings->mNameSettings.mAllowedClassAttributes.Insert("Protected", AttributeInfo());
  settings->mNameSettings.mAllowedClassAttributes.Insert("CoreVertex", AttributeInfo());
  settings->mNameSettings.mAllowedClassAttributes.Insert("RenderPass", AttributeInfo());
  settings->mNameSettings.mAllowedClassAttributes.Insert("PostProcess", AttributeInfo());
  settings->mNameSettings.mAllowedFieldAttributes.Insert("Hidden", AttributeInfo());
  settings->mNameSettings.mAllowedFieldAttributes.Insert(PropertyAttributes::cGroup, AttributeInfo());
  settings->mNameSettings.mAllowedFieldAttributes.Insert(PropertyAttributes::cRange, AttributeInfo());
  settings->mNameSettings.mAllowedFieldAttributes.Insert(PropertyAttributes::cSlider, AttributeInfo());

  ZilchShaderSettingsLoader settingsLoader;
  String settingsDir = FilePath::Combine(mainConfig->DataDirectory, "ZilchFragmentSettings");
  settingsLoader.LoadSettings(settings, settingsDir);

  settings->mShaderDefinitionSettings.SetMaxSimultaneousRenderTargets(8);
  settings->mShaderDefinitionSettings.SetRenderTargetName("Target0", 0);
  settings->mShaderDefinitionSettings.SetRenderTargetName("Target1", 1);
  settings->mShaderDefinitionSettings.SetRenderTargetName("Target2", 2);
  settings->mShaderDefinitionSettings.SetRenderTargetName("Target3", 3);
  settings->mShaderDefinitionSettings.SetRenderTargetName("Target4", 4);
  settings->mShaderDefinitionSettings.SetRenderTargetName("Target5", 5);
  settings->mShaderDefinitionSettings.SetRenderTargetName("Target6", 6);
  settings->mShaderDefinitionSettings.SetRenderTargetName("Target7", 7);
  
  // Built-in inputs

  // Vertex Attributes
  settings->mShaderDefinitionSettings.AddVertexDefinition("LocalPosition", "Real3");
  settings->mShaderDefinitionSettings.AddVertexDefinition("LocalNormal", "Real3");
  settings->mShaderDefinitionSettings.AddVertexDefinition("LocalTangent", "Real3");
  settings->mShaderDefinitionSettings.AddVertexDefinition("LocalBitangent", "Real3");
  settings->mShaderDefinitionSettings.AddVertexDefinition("Uv", "Real2");
  settings->mShaderDefinitionSettings.AddVertexDefinition("UvAux", "Real2");
  settings->mShaderDefinitionSettings.AddVertexDefinition("Color", "Real4");
  settings->mShaderDefinitionSettings.AddVertexDefinition("ColorAux", "Real4");
  settings->mShaderDefinitionSettings.AddVertexDefinition("BoneIndices", "Integer4");
  settings->mShaderDefinitionSettings.AddVertexDefinition("BoneWeights", "Real4");
  settings->mShaderDefinitionSettings.AddVertexDefinition("Aux0", "Real4");
  settings->mShaderDefinitionSettings.AddVertexDefinition("Aux1", "Real4");
  settings->mShaderDefinitionSettings.AddVertexDefinition("Aux2", "Real4");
  settings->mShaderDefinitionSettings.AddVertexDefinition("Aux3", "Real4");
  settings->mShaderDefinitionSettings.AddVertexDefinition("Aux4", "Real4");
  settings->mShaderDefinitionSettings.AddVertexDefinition("Aux5", "Real4");

  // Transformations
  settings->mShaderDefinitionSettings.AddBuiltIn("LocalToWorld", "Real4x4");
  settings->mShaderDefinitionSettings.AddBuiltIn("WorldToLocal", "Real4x4");
  settings->mShaderDefinitionSettings.AddBuiltIn("WorldToView", "Real4x4");
  settings->mShaderDefinitionSettings.AddBuiltIn("ViewToWorld", "Real4x4");
  settings->mShaderDefinitionSettings.AddBuiltIn("LocalToView", "Real4x4");
  settings->mShaderDefinitionSettings.AddBuiltIn("ViewToLocal", "Real4x4");
  settings->mShaderDefinitionSettings.AddBuiltIn("LocalToViewNormal", "Real3x3");
  settings->mShaderDefinitionSettings.AddBuiltIn("ViewToLocalNormal", "Real3x3");
  settings->mShaderDefinitionSettings.AddBuiltIn("LocalToWorldNormal", "Real3x3");
  settings->mShaderDefinitionSettings.AddBuiltIn("WorldToLocalNormal", "Real3x3");
  settings->mShaderDefinitionSettings.AddBuiltIn("LocalToPerspective", "Real4x4");
  settings->mShaderDefinitionSettings.AddBuiltIn("ViewToPerspective", "Real4x4");
  settings->mShaderDefinitionSettings.AddBuiltIn("PerspectiveToView", "Real4x4");
  settings->mShaderDefinitionSettings.AddBuiltIn("ZeroPerspectiveToApiPerspective", "Real4x4");
  // @JoshD: Cleanup later
  ShaderField* field = settings->mShaderDefinitionSettings.mBuiltIns[ShaderFieldKey("ZeroPerspectiveToApiPerspective", "Real4x4")];
  field->mAttributes.AddAttribute(settings->mNameSettings.mStaticAttribute, nullptr);

  settings->mShaderDefinitionSettings.AddBuiltIn("BoneTransforms", "FixedArray[Real4x4, 80]");

  // Utility
  settings->mShaderDefinitionSettings.AddBuiltIn("FrameTime", "Real");
  settings->mShaderDefinitionSettings.AddBuiltIn("LogicTime", "Real");
  settings->mShaderDefinitionSettings.AddBuiltIn("NearPlane", "Real");
  settings->mShaderDefinitionSettings.AddBuiltIn("FarPlane", "Real");
  settings->mShaderDefinitionSettings.AddBuiltIn("ViewportSize", "Real2");
  settings->mShaderDefinitionSettings.AddBuiltIn("InverseViewportSize", "Real2");
  settings->mShaderDefinitionSettings.AddBuiltIn("ObjectWorldPosition", "Real3");

  // SpriteSource input
  settings->mShaderDefinitionSettings.AddBuiltIn("SpriteSource", "Sampler2d");

  // HeightMap input
  settings->mShaderDefinitionSettings.AddBuiltIn("HeightMapWeights", "Sampler2d");

  mTranslator->SetSettings(mSettings);
  mTranslator->Setup();

  mZilchCoreLibrary = new ZilchShaderLibrary();
  mZilchCoreLibrary->mZilchLibrary = Zilch::Core::GetInstance().GetLibrary();
  mTranslator->ParseNativeLibrary(mZilchCoreLibrary);

  // Build intrinsics library
  Zilch::ShaderIntrinsicsLibrary::GetInstance().BuildLibrary();

  mIntrinsicsLibrary = new ZilchShaderLibrary();
  mIntrinsicsLibrary->mZilchLibrary = Zilch::ShaderIntrinsicsLibrary::GetInstance().GetLibrary();
  mTranslator->ParseNativeLibrary(mIntrinsicsLibrary);

  //// Build extensions library
  //ZilchShaderProject extensionsProject("Extensions");
  //EventConnect(&extensionsProject, Zilch::Events::CompilationError, &ZilchShaderGenerator::OnZilchFragmentCompilationError, this);
  //EventConnect(&extensionsProject, Events::TranslationError, &ZilchShaderGenerator::OnZilchFragmentTranslationError, this);
  //
  //String extensionsDir = FilePath::Combine(mainConfig->DataDirectory, "ZilchFragmentExtensions");
  //FileRange fileRange(extensionsDir);
  //
  //for (; !fileRange.Empty(); fileRange.PopFront())
  //{
  //  FileEntry entry = fileRange.frontEntry();
  //  String filePath = entry.GetFullPath();
  //  String fileExt = FilePath::GetExtension(filePath);
  //  if (fileExt == "zilchFrag")
  //    extensionsProject.AddCodeFromFile(filePath, nullptr);
  //}

  //ZilchShaderModuleRef dependencies = new ZilchShaderModule();
  //dependencies->PushBack(mZilchCoreLibrary);
  //dependencies->PushBack(mIntrinsicsLibrary);

  //mExtensionsLibrary = extensionsProject.CompileAndTranslate(dependencies, mTranslator, mSettings);

  // Pre map every attribute value for quick processing of sampler shader inputs
  mSamplerAttributeValues["TextureAddressingXClamp"]        = SamplerSettings::AddressingX(TextureAddressing::Clamp);
  mSamplerAttributeValues["TextureAddressingXRepeat"]       = SamplerSettings::AddressingX(TextureAddressing::Repeat);
  mSamplerAttributeValues["TextureAddressingXMirror"]       = SamplerSettings::AddressingX(TextureAddressing::Mirror);
  mSamplerAttributeValues["TextureAddressingYClamp"]        = SamplerSettings::AddressingY(TextureAddressing::Clamp);
  mSamplerAttributeValues["TextureAddressingYRepeat"]       = SamplerSettings::AddressingY(TextureAddressing::Repeat);
  mSamplerAttributeValues["TextureAddressingYMirror"]       = SamplerSettings::AddressingY(TextureAddressing::Mirror);
  mSamplerAttributeValues["TextureFilteringNearest"]        = SamplerSettings::Filtering(TextureFiltering::Nearest);
  mSamplerAttributeValues["TextureFilteringBilinear"]       = SamplerSettings::Filtering(TextureFiltering::Bilinear);
  mSamplerAttributeValues["TextureFilteringTrilinear"]      = SamplerSettings::Filtering(TextureFiltering::Trilinear);
  mSamplerAttributeValues["TextureCompareModeDisabled"]     = SamplerSettings::CompareMode(TextureCompareMode::Disabled);
  mSamplerAttributeValues["TextureCompareModeEnabled"]      = SamplerSettings::CompareMode(TextureCompareMode::Enabled);
  mSamplerAttributeValues["TextureCompareFuncNever"]        = SamplerSettings::CompareFunc(TextureCompareFunc::Never);
  mSamplerAttributeValues["TextureCompareFuncAlways"]       = SamplerSettings::CompareFunc(TextureCompareFunc::Always);
  mSamplerAttributeValues["TextureCompareFuncLess"]         = SamplerSettings::CompareFunc(TextureCompareFunc::Less);
  mSamplerAttributeValues["TextureCompareFuncLessEqual"]    = SamplerSettings::CompareFunc(TextureCompareFunc::LessEqual);
  mSamplerAttributeValues["TextureCompareFuncGreater"]      = SamplerSettings::CompareFunc(TextureCompareFunc::Greater);
  mSamplerAttributeValues["TextureCompareFuncGreaterEqual"] = SamplerSettings::CompareFunc(TextureCompareFunc::GreaterEqual);
  mSamplerAttributeValues["TextureCompareFuncEqual"]        = SamplerSettings::CompareFunc(TextureCompareFunc::Equal);
  mSamplerAttributeValues["TextureCompareFuncNotEqual"]     = SamplerSettings::CompareFunc(TextureCompareFunc::NotEqual);

  // Add names to list of allowed attributes in zilch fragments
  forRange(String& attribute, mSamplerAttributeValues.Keys())
    settings->mNameSettings.mAllowedFieldAttributes.Insert(attribute, AttributeInfo());
}

//**************************************************************************************************
LibraryRef BuildWrapperLibrary(ZilchShaderLibraryRef fragmentsLibrary)
{
  // Helper structure for collecting property info.
  struct ShaderPropertyInfo
  {
    String mName;
    ShaderField* mShaderField;
    BoundType* mBoundType;
    size_t mMemberOffset;
    bool mIsTexture;
  };

  // The Zero editor attributes to check for valid crossover attributes from the shader types.
  AttributeExtensions* zeroAttributes = AttributeExtensions::GetInstance();

  String wrapperLibraryName = BuildString(fragmentsLibrary->mZilchLibrary->Name, "Wrapper");
  LibraryBuilder builder(wrapperLibraryName);

  forRange (ShaderType* shaderType, fragmentsLibrary->mTypes.Values())
  {
    // Only add fragment types.
    if (shaderType->mFragmentType != FragmentType::None)
    {
      size_t baseSize = sizeof(MaterialBlock);
      size_t fragmentSize = baseSize;

      Array<ShaderPropertyInfo> shaderProperties;

      // Calculate the fragment type's size and get info needed to bind properties.
      forRange (ShaderField* field, shaderType->mFieldList.All())
      {
        if (field->mZilchType.Contains("FixedArray"))
          continue;

        if (field->ContainsAttribute("PropertyInput"))
        {
          ShaderType* fieldType = field->GetShaderType();

          // Property type should be a primitive or a texture.
          BoundType* type = MetaDatabase::GetInstance()->FindType(fieldType->mPropertyType);
          if (type != nullptr)
          {
            ShaderPropertyInfo shaderProperty;
            shaderProperty.mName = field->mZilchName;
            shaderProperty.mShaderField = field;
            shaderProperty.mBoundType = type;

            size_t fieldSize;
            size_t byteAlignment;

            if (type->IsA(ZilchTypeId(Texture)))
            {
              // Textures are stored by their ID's.
              fieldSize = sizeof(u64);
              byteAlignment = fieldSize;
              // Textures need a different getter/setter.
              shaderProperty.mIsTexture = true;
            }
            else
            {
              fieldSize = type->GetCopyableSize();
              byteAlignment = Math::Min(fieldSize, sizeof(float));
              shaderProperty.mIsTexture = false;
            }

            // Compute pad bytes needed for placing this data member on correct alignment.
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
      size_t largestSize = sizeof(u64);
      size_t endPad = (largestSize - fragmentSize % largestSize) % largestSize;
      fragmentSize += endPad;

      // Create fragment type.
      BoundType* boundType = builder.AddBoundType(shaderType->mZilchName, TypeCopyMode::ReferenceType, fragmentSize);
      boundType->BaseType = ZilchTypeId(MaterialBlock);
      // Associate this type to the ZilchFragment resource containing the shader type.
      boundType->Add(new MetaResource((Resource*)shaderType->mLocation.mUserData));

      builder.AddBoundDefaultConstructor(boundType, FragmentConstructor);
      builder.AddBoundDestructor(boundType, FragmentDestructor);

      // Allocate one instance of the derived type and store it on the BoundType
      // for initializing fragments to the default values.
      ByteBufferBlock& defaultMemory = boundType->ComplexUserData.CreateObject<ByteBufferBlock>(0);
      size_t derivedTypeSize = fragmentSize - baseSize;
      defaultMemory.SetData(new byte[derivedTypeSize], derivedTypeSize, true);
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
        GetterSetter* getterSetter = builder.AddBoundGetterSetter(boundType, shaderProperty.mName, shaderProperty.mBoundType, setter, getter, MemberOptions::None);
        // Storing member offset on the property meta for generic getter/setter implementation.
        getterSetter->UserData = (void*)shaderProperty.mMemberOffset;
        getterSetter->Get->UserData = (void*)shaderProperty.mMemberOffset;
        getterSetter->Set->UserData = (void*)shaderProperty.mMemberOffset;
        // Currently just setting the type location, a more specific location for properties still needs to be added.
        getterSetter->NameLocation = shaderProperty.mShaderField->mNameLocation;
        getterSetter->Location = shaderProperty.mShaderField->mSourceLocation;

        // Mark all bound getter/setters as property.
        getterSetter->AddAttribute(PropertyAttributes::cProperty);

        // Add all other valid attributes from shader field.
        forRange (ShaderAttribute& shaderAttribute, shaderProperty.mShaderField->mAttributes.All())
        {
          if (zeroAttributes->IsValidPropertyAttribute(shaderAttribute.mAttributeName))
          {
            Attribute* attribute = getterSetter->AddAttribute(shaderAttribute.mAttributeName);
            attribute->Parameters = shaderAttribute.mParameters;
          }
        }
      }

      // Set file location for this type.
      boundType->NameLocation = shaderType->mNameLocation;
      boundType->Location = shaderType->mSourceLocation;
    }
  }

  // Adds the component getters for the fragment types.
  forRange (BoundType* boundType, builder.BoundTypes.Values())
  {
    AttributeStatus status;
    AttributeExtensions::GetInstance()->ProcessType(status, boundType, nullptr);
    if (status.Failed())
      ZPrint(status.Message.c_str());
    MetaLibraryExtensions::ProcessComponent(builder, boundType);
  }

  return builder.CreateLibrary();
}

//**************************************************************************************************
LibraryRef ZilchShaderGenerator::BuildFragmentsLibrary(Module& dependencies, Array<ZilchDocumentResource*>& fragments, StringParam libraryName)
{
  mFragmentsProject.Clear();
  mFragmentsProject.mProjectName = libraryName;

  // Add all fragments
  forRange (Resource* resource, fragments.All())
  {
    // Templates shouldn't be compiled. They contain potentially invalid code and identifiers
    // such as RESOURCE_NAME_ that are replaced when a new resource is created from the template
    if (resource->GetResourceTemplate())
      continue;

    ZilchFragment* fragment = (ZilchFragment*)resource;
    mFragmentsProject.AddCodeFromString(fragment->mText, fragment->LoadPath, resource);
  }

  // Internal dependencies used to build the internal library
  ZilchShaderModuleRef internalDependencies = new ZilchShaderModule();
  internalDependencies->PushBack(mZilchCoreLibrary);
  internalDependencies->PushBack(mIntrinsicsLibrary);

  // We gave the engine our "wrapped" library, and it's giving them back as dependencies.
  // For fragment compilation, it expects our internal libraries, so we need to look them up
  // and use those instead
  forRange(Library* dependentLibrary, dependencies.All())
  {
    ZilchShaderLibraryRef internalDependency = GetInternalLibrary(dependentLibrary);
    internalDependencies->Append(internalDependency);
  }

  ZilchShaderLibraryRef fragmentsLibrary = mFragmentsProject.CompileAndTranslate(internalDependencies, mTranslator, mSettings);
  if (fragmentsLibrary == nullptr)
    return false;

  // Write to the complex user data of each shader type the name of the resource they came from.
  // This has to be done as a second pass because complex user currently can't be written per file (we also need per type).
  forRange(ShaderType* shaderType, fragmentsLibrary->mTypes.Values())
  {
    Resource* resource = (Resource*)shaderType->mSourceLocation.CodeUserData;
    // If we have a valid user data (some types won't, like arrays that are generated in your library)
    if(resource == nullptr)
      continue;
    FragmentUserData complexUserData(resource->Name);
    shaderType->mComplexUserData.WriteObject(complexUserData);
  }

  // Build the wrapper library around the internal library
  LibraryRef library = BuildWrapperLibrary(fragmentsLibrary);

  if (library == nullptr)
    return nullptr;

  // If pending changes cause scripts to not compile, and then fragments are changed again to fix it,
  // will have duplicate pending libraries that should be replaced.
  // Libraries aren't mapped by name so find it manually.
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
    ShaderType* shaderType = fragmentsLibrary->FindType(boundType->Name);
    
    if (shaderType->ContainsAttribute("CoreVertex") && shaderType->ContainsAttribute("Vertex"))
      fragmentTypes.Insert(boundType->Name, ZilchFragmentType::CoreVertex);
    else if (shaderType->ContainsAttribute("RenderPass") && shaderType->ContainsAttribute("Pixel"))
      fragmentTypes.Insert(boundType->Name, ZilchFragmentType::RenderPass);
    else if (shaderType->ContainsAttribute("PostProcess") && shaderType->ContainsAttribute("Pixel"))
      fragmentTypes.Insert(boundType->Name, ZilchFragmentType::PostProcess);
    else if (shaderType->ContainsAttribute("Protected"))
      fragmentTypes.Insert(boundType->Name, ZilchFragmentType::Protected);
    else
      fragmentTypes.Insert(boundType->Name, ZilchFragmentType::Fragment);
  }

  return library;
}

//**************************************************************************************************
bool ZilchShaderGenerator::Commit(ZilchCompileEvent* e)
{
  // Don't need to do anything if there are no pending libraries
  if(mPendingToPendingInternal.Empty())
    return false;

  // Remove all old libraries
  forRange(ResourceLibrary* library, e->mModifiedLibraries.All())
  {
    Library* pendingLibrary = library->mSwapFragment.mPendingLibrary;
    if(pendingLibrary)
    {
      ZilchShaderLibraryRef internalPendingLibrary = mPendingToPendingInternal.FindValue(pendingLibrary, nullptr);
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

//**************************************************************************************************
void ZilchShaderGenerator::MapFragmentTypes()
{
  mPendingFragmentTypes.Clear();

  mCoreVertexFragments.Clear();
  mRenderPassFragments.Clear();
  mFragmentTypes.Clear();

  // Find fragments needed for shader permutations
  forRange (LibraryRef wrapperLibrary, mCurrentToInternal.Keys())
  {
    ZilchShaderLibraryRef fragmentLibrary = GetInternalLibrary(wrapperLibrary);
    forRange (BoundType* boundType, wrapperLibrary->BoundTypes.Values())
    {
      ShaderType* shaderType = fragmentLibrary->FindType(boundType->Name);

      if (shaderType->ContainsAttribute("CoreVertex") && shaderType->ContainsAttribute("Vertex"))
      {
        mCoreVertexFragments.PushBack(boundType->Name);
        mFragmentTypes.Insert(boundType->Name, ZilchFragmentType::CoreVertex);
      }
      else if (shaderType->ContainsAttribute("RenderPass") && shaderType->ContainsAttribute("Pixel"))
      {
        mRenderPassFragments.PushBack(boundType->Name);
        mFragmentTypes.Insert(boundType->Name, ZilchFragmentType::RenderPass);
      }
      else if (shaderType->ContainsAttribute("PostProcess") && shaderType->ContainsAttribute("Pixel"))
      {
        mFragmentTypes.Insert(boundType->Name, ZilchFragmentType::PostProcess);
      }
      else if (shaderType->ContainsAttribute("Protected"))
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

//**************************************************************************************************
bool ZilchShaderGenerator::BuildShaders(ShaderSet& shaders, HashMap<String, UniqueComposite>& composites, Array<ShaderEntry>& shaderEntries, Array<ZilchShaderDefinition>* compositeShaderDefs)
{
  ZilchCompositor compositor;
  compositor.mEmitVertexCallback = CustomEmitVertexCallback;

  ZilchShaderLibraryRef fragmentsLibrary = GetCurrentInternalProjectLibrary();

  Array<Shader*> shaderArray;
  shaderArray.Append(shaders.All());

  // Value should not be very large to prevent unnecessary memory consumption to compile.
  const size_t compositeBatchCount = 20;

  size_t totalShaderCount = shaderArray.Size();
  for (size_t startIndex = 0; startIndex < totalShaderCount; startIndex += compositeBatchCount)
  {
    ZilchShaderProject shaderProject("ShaderProject");

    // All batches are added to this array, get start index for this batch.
    size_t entryStartIndex = shaderEntries.Size();

    size_t endIndex = Math::Min(startIndex + compositeBatchCount, totalShaderCount);
    for (size_t i = startIndex; i < endIndex; ++i)
    {
      Shader* shader = shaderArray[i];

      // Make shader def.
      ZilchShaderDefinition shaderDef;
      shaderDef.mShaderName = shader->mName;

      ZilchShaderLibrary* lib = fragmentsLibrary;

      // CoreVertex
      ErrorIf(fragmentsLibrary->FindType(shader->mCoreVertex) == nullptr, "No fragment.");
      shaderDef.mFragmentTypes.PushBack(fragmentsLibrary->FindType(shader->mCoreVertex));

      // Composition
      if (composites.ContainsKey(shader->mComposite))
      {
        forRange (String fragmentName, composites[shader->mComposite].mFragmentNames.All())
        {
          ErrorIf(fragmentsLibrary->FindType(fragmentName) == nullptr, "No fragment.");
          shaderDef.mFragmentTypes.PushBack(fragmentsLibrary->FindType(fragmentName));
        }
      }
      else
      {
        String fragmentName = shader->mComposite;
        ErrorIf(fragmentsLibrary->FindType(fragmentName) == nullptr, "No fragment.");
        shaderDef.mFragmentTypes.PushBack(fragmentsLibrary->FindType(fragmentName));
      }

      // ApiPerspective
      ShaderType* apiPerspectiveOutput = fragmentsLibrary->FindType("ApiPerspectiveOutput");
      ErrorIf(apiPerspectiveOutput == nullptr, "Missing fragment from core library.");
      shaderDef.mFragmentTypes.PushBack(apiPerspectiveOutput);

      // RenderPass
      if (shader->mRenderPass.Empty() == false)
      {
        ErrorIf(fragmentsLibrary->FindType(shader->mRenderPass) == nullptr, "No fragment.");
        shaderDef.mFragmentTypes.PushBack(fragmentsLibrary->FindType(shader->mRenderPass));
      }

      compositor.BuildCompositedShader(mTranslator, fragmentsLibrary, shaderDef, mSettings);

      // If the user requested it, then add the resulting shader def as an output.
      // Used for debugging purposes to display the zilch composites.
      if(compositeShaderDefs != nullptr)
        compositeShaderDefs->PushBack(shaderDef);

      ZilchFragmentInfo& vertexInfo = shaderDef.mShaderData[FragmentType::Vertex];
      ZilchFragmentInfo& geometryInfo = shaderDef.mShaderData[FragmentType::Geometry];
      ZilchFragmentInfo& pixelInfo = shaderDef.mShaderData[FragmentType::Pixel];

      shaderProject.AddCodeFromString(vertexInfo.mZilchCode, vertexInfo.mZilchClassName, nullptr);
      shaderProject.AddCodeFromString(geometryInfo.mZilchCode, geometryInfo.mZilchClassName, nullptr);
      shaderProject.AddCodeFromString(pixelInfo.mZilchCode, pixelInfo.mZilchClassName, nullptr);

      ShaderEntry entry(shader);
      entry.mVertexShader = vertexInfo.mZilchClassName;
      entry.mGeometryShader = geometryInfo.mZilchClassName;
      entry.mPixelShader = pixelInfo.mZilchClassName;
      shaderEntries.PushBack(entry);
    }

    ZilchShaderModuleRef shaderDependencies = new ZilchShaderModule();
    shaderDependencies->PushBack(fragmentsLibrary);

    EventConnect(&shaderProject, Zilch::Events::CompilationError, &ZilchShaderGenerator::OnZilchFragmentCompilationError, this);
    ZilchShaderLibraryRef shaderLibrary = shaderProject.CompileAndTranslate(shaderDependencies, mTranslator, mSettings);

    if (shaderLibrary == nullptr)
    {
      DoNotifyError("Shader Error", "Failed to build shader library.");
      return false;
    }

    for (size_t i = entryStartIndex; i < shaderEntries.Size(); ++i)
    {
      ShaderEntry& entry = shaderEntries[i];

      ShaderType* vertexShader = shaderLibrary->FindType(entry.mVertexShader);
      ShaderType* geometryShader = shaderLibrary->FindType(entry.mGeometryShader);
      ShaderType* pixelShader = shaderLibrary->FindType(entry.mPixelShader);
      ErrorIf(vertexShader == nullptr || pixelShader == nullptr, "Invalid shader entry");

      ShaderTypeTranslation vertexShaderResults, geometryShaderResults, pixelShaderResults;
      mTranslator->BuildFinalShader(vertexShader, vertexShaderResults);
      mTranslator->BuildFinalShader(geometryShader, geometryShaderResults);
      mTranslator->BuildFinalShader(pixelShader, pixelShaderResults);

      entry.mVertexShader = vertexShaderResults.mTranslation;
      entry.mGeometryShader = geometryShaderResults.mTranslation;
      entry.mPixelShader = pixelShaderResults.mTranslation;

      // Debug
      if (false)
      {
        String path = FilePath::Combine(GetTemporaryDirectory(), "Shaders");
        CreateDirectoryAndParents(path);
        String baseName = BuildString(entry.mComposite, entry.mCoreVertex, entry.mRenderPass);
        WriteStringRangeToFile(FilePath::Combine(path, BuildString(baseName, "Vertex.glsl")), entry.mVertexShader);
        WriteStringRangeToFile(FilePath::Combine(path, BuildString(baseName, "Geometry.glsl")), entry.mGeometryShader);
        WriteStringRangeToFile(FilePath::Combine(path, BuildString(baseName, "Pixel.glsl")), entry.mPixelShader);
      }
    }
  }

  return true;
}

//**************************************************************************************************
ShaderInput ZilchShaderGenerator::CreateShaderInput(StringParam fragmentName, StringParam inputName, ShaderInputType::Enum type, AnyParam value)
{
  ShaderInput shaderInput;

  if (type == ShaderInputType::Invalid)
    return shaderInput;

  ShaderType* shaderType = GetCurrentInternalProjectLibrary()->FindType(fragmentName);
  if (shaderType == nullptr)
    return shaderInput;

  ShaderField* field = shaderType->mFieldMap.FindValue(inputName, nullptr);
  if (field == nullptr)
    return shaderInput;

  if (type == ShaderInputType::Texture)
  {
    shaderInput.mTranslatedInputName = GenerateMangledName(shaderType->mZilchName, inputName);

    // Check for sampler settings overrides
    forRange (ShaderAttribute& attribute, field->mAttributes.All())
    {
      if (mSamplerAttributeValues.ContainsKey(attribute.mAttributeName))
        SamplerSettings::AddValue(shaderInput.mSamplerSettings, mSamplerAttributeValues[attribute.mAttributeName]);
    }
  }
  else
    shaderInput.mTranslatedInputName = GenerateFieldUniformName(field);

  // If unsuccessful returned ShaderInput's type will be Invalid, otherwise it will be the passed in type
  shaderInput.mShaderInputType = type;
  shaderInput.SetValue(value);

  return shaderInput;
}

//**************************************************************************************************
void ZilchShaderGenerator::OnZilchFragmentCompilationError(Zilch::ErrorEvent* event)
{
  String shortMessage = event->ExactError;
  String fullMessage = event->GetFormattedMessage(Zilch::MessageFormat::Python);
  ZilchFragmentManager::GetInstance()->DispatchScriptError(Events::SyntaxError, shortMessage, fullMessage, event->Location);
}

//**************************************************************************************************
void ZilchShaderGenerator::OnZilchFragmentTypeParsed(Zilch::ParseEvent* event)
{
  // There are a lot of attributes in zilch fragments that aren't valid for zilch script.
  // Because of this, we want to ignore invalid attributes here and let the fragment compilation
  // catch them
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

//**************************************************************************************************
void ZilchShaderGenerator::OnZilchFragmentTranslationError(TranslationErrorEvent* event)
{
  String fullMessage = event->GetFormattedMessage(Zilch::MessageFormat::Python);
  ZilchFragmentManager::GetInstance()->DispatchScriptError(Events::SyntaxError, event->mShortMessage, fullMessage, event->mLocation);
}

//**************************************************************************************************
ZilchShaderLibraryRef ZilchShaderGenerator::GetInternalLibrary(LibraryRef library)
{
  if(library == nullptr)
    return nullptr; 

  // First look in pending
  ZilchShaderLibraryRef internalLibrary = GetPendingInternalLibrary(library);

  // Then look in current (if it wasn't recompiled)
  if(internalLibrary == nullptr)
    internalLibrary = GetCurrentInternalLibrary(library);

  ErrorIf(internalLibrary == nullptr, "Could not find internal library.");

  return internalLibrary;
}

//**************************************************************************************************
ZilchShaderLibraryRef ZilchShaderGenerator::GetCurrentInternalLibrary(LibraryRef library)
{
  return mCurrentToInternal.FindValue(library, nullptr);
}

//**************************************************************************************************
ZilchShaderLibraryRef ZilchShaderGenerator::GetPendingInternalLibrary(LibraryRef library)
{
  return mPendingToPendingInternal.FindValue(library, nullptr);
}

//**************************************************************************************************
ZilchShaderGenerator::InternalLibraryRange ZilchShaderGenerator::GetCurrentInternalLibraries()
{
  return mCurrentToInternal.Values();
}

//**************************************************************************************************
ZilchShaderLibraryRef ZilchShaderGenerator::GetCurrentInternalProjectLibrary()
{
  return GetInternalLibrary(ZilchManager::GetInstance()->mCurrentFragmentProjectLibrary);
}

//**************************************************************************************************
ZilchShaderLibraryRef ZilchShaderGenerator::GetPendingInternalProjectLibrary()
{
  return GetInternalLibrary(ZilchManager::GetInstance()->mPendingFragmentProjectLibrary);
}

} // namespace Zero
