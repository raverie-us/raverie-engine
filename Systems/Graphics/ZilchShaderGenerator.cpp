#include "Precompiled.hpp"

namespace Zero
{

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

ZilchShaderGenerator* CreateZilchShaderGenerator()
{
  ZilchShaderGenerator* shaderGenerator = new ZilchShaderGenerator();
  shaderGenerator->mTranslator = new Glsl150Translator();
  shaderGenerator->Initialize();
  return shaderGenerator;
}

ZilchShaderGenerator::ZilchShaderGenerator()
  : mFragmentsProject("Fragments")
{
  mSettings = new ZilchShaderSettings();
}

ZilchShaderGenerator::~ZilchShaderGenerator()
{
  
}

void ZilchShaderGenerator::Initialize()
{
  if(mTranslator == nullptr)
    mTranslator = new Glsl150Translator();

  ShaderSettingsLibrary::GetInstance().BuildLibrary();
  EventConnect(&mFragmentsProject, Zilch::Events::CompilationError, &ZilchShaderGenerator::OnZilchFragmentCompilationError, this);
  EventConnect(&mFragmentsProject, Events::TranslationError, &ZilchShaderGenerator::OnZilchFragmentTranslationError, this);

  MainConfig* mainConfig = Z::gEngine->GetConfigCog()->has(MainConfig);

  // Set the default render target names
  // max target count needs to be queried by the renderer
  ZilchShaderSettings* settings = mSettings;

  settings->mNameSettings.mAllowedClassAttributes.Insert("Protected");
  settings->mNameSettings.mAllowedClassAttributes.Insert("CoreVertex");
  settings->mNameSettings.mAllowedClassAttributes.Insert("RenderPass");
  settings->mNameSettings.mAllowedClassAttributes.Insert("PostProcess");
  settings->mNameSettings.mAllowedFieldAttributes.Insert("Hidden");

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
  settings->mNameSettings.mAllowedFieldAttributes.Append(mSamplerAttributeValues.Keys());
}

// Quick fix function for turning a zilch any into a string (the to-string function isn't always correct
String DefaultValueToString(StringParam propertyType, Zilch::Any& defaultValue)
{
  if(defaultValue.IsNull() || propertyType == "Texture" || propertyType == "TextureCube"|| 
    propertyType == "Real4x4" || propertyType == "Real3x3")
    return String();

  if(propertyType == "Boolean")
    return defaultValue.ToString();
  if(propertyType == "Integer")
    return defaultValue.ToString();
  if(propertyType == "Integer2")
    return BuildString(propertyType, defaultValue.ToString());
  if(propertyType == "Integer3")
    return BuildString(propertyType, defaultValue.ToString());
  if(propertyType == "Integer4")
    return BuildString(propertyType, defaultValue.ToString());
  if(propertyType == "Real")
    return defaultValue.ToString();
  if(propertyType == "Real2")
    return BuildString(propertyType, defaultValue.ToString());
  if(propertyType == "Real3")
    return BuildString(propertyType, defaultValue.ToString());
  if(propertyType == "Real4")
    return BuildString(propertyType, defaultValue.ToString());
  return String();
}

LibraryRef BuildWrapperLibrary(ZilchShaderLibraryRef fragmentsLibrary)
{
  // METAREFACTOR This whole function is temporary. It should build the types in a custom way
  // instead of building a file that has to be parsed. It was done because I was having issues
  // and needed to move on

  Project project;
  forRange(ShaderType* shaderType, fragmentsLibrary->mTypes.Values())
  {
    StringBuilder file;
    // Only add fragment types 
    if(shaderType->mFragmentType != FragmentType::None)
    {
      forRange(ShaderAttribute& attribute, shaderType->mAttributes.All())
      {
        file.Append("[");
        file.Append(attribute.mAttributeName);
        file.Append("]");
      }
      file.Append("\n");

      file.Append("class ");
      file.Append(shaderType->mZilchName);
      file.Append(" : MaterialBlock\n{\n");

      uint offset = 0;
      forRange(ShaderField* field, shaderType->mFieldList.All())
      {
        if(field->mZilchType.Contains("FixedArray"))
          continue;

        if(field->ContainsAttribute("PropertyInput"))
        {
          ShaderType* type = field->GetShaderType();

          file.Append("\t[Property] var ");
          file.Append(field->mZilchName);
          file.Append(" : ");
          file.Append(type->mPropertyType);

          // Append the default value if it exists
          String defaultValueString = DefaultValueToString(type->mPropertyType, field->mDefaultValueVariant);
          if(!defaultValueString.Empty())
          {
            file.Append(" = ");
            file.Append(defaultValueString);
          }

          file.Append(";\n");
        }
      }

      file.Append("}\n\n");
      Resource* resource = (Resource*)shaderType->mLocation.mUserData;
      project.AddCodeFromString(file.ToString(), Zilch::CodeString, (void*)resource);
    }
  }

  Module module;
  module.Append(GraphicsLibrary::GetLibrary());
  
  EventConnect(&project, Zilch::Events::TypeParsed, &EngineLibraryExtensions::TypeParsedCallback);
  EventConnect(&project, Zilch::Events::PostSyntaxer, &ZilchShaderGenerator::OnFragmentProjectPostSyntaxer);

  String wrapperProjectName = BuildString(fragmentsLibrary->mZilchLibrary->Name, "Wrapper");
  return project.Compile(wrapperProjectName, module, EvaluationMode::Project);
}

LibraryRef ZilchShaderGenerator::BuildFragmentsLibrary(Module& dependencies, Array<ZilchDocumentResource*>& fragments, StringParam libraryName)
{
  mFragmentsProject.Clear();
  mFragmentsProject.mProjectName = libraryName;

  // Add all fragments
  forRange (Resource* resource, fragments.All())
  {
    // Templates shouldn't be compiled. They contain invalid strings (%RESOURCENAME%) that are
    // replaced when a new resource is created from the template
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

  mPendingToPendingInternal.Insert(library, fragmentsLibrary);

  ZilchFragmentTypeMap& fragmentTypes = mPendingFragmentTypes[library];
  fragmentTypes.Clear();
  forRange (BoundType* boundType, library->BoundTypes.Values())
  {
    if (boundType->HasAttribute("CoreVertex") != nullptr && boundType->HasAttribute("Vertex") != nullptr)
      fragmentTypes.Insert(boundType->Name, ZilchFragmentType::CoreVertex);
    else if (boundType->HasAttribute("RenderPass") != nullptr && boundType->HasAttribute("Pixel") != nullptr)
      fragmentTypes.Insert(boundType->Name, ZilchFragmentType::RenderPass);
    else if (boundType->HasAttribute("PostProcess") != nullptr && boundType->HasAttribute("Pixel") != nullptr)
      fragmentTypes.Insert(boundType->Name, ZilchFragmentType::PostProcess);
    else if (boundType->HasAttribute("Protected") != nullptr)
      fragmentTypes.Insert(boundType->Name, ZilchFragmentType::Protected);
    else
      fragmentTypes.Insert(boundType->Name, ZilchFragmentType::Fragment);
  }

  return library;
}

bool ZilchShaderGenerator::Commit(ZilchCompileEvent* e)
{
  // Don't need to do anything if there are no pending libraries
  if(mPendingToPendingInternal.Empty())
    return false;

  // Remove all old libraries
  forRange(ResourceLibrary* library, e->mModifiedLibraries.All())
  {
    Library* pendingLibrary = library->mPendingFragmentLibrary;
    if(pendingLibrary)
    {
      ZilchShaderLibraryRef internalPendingLibrary = mPendingToPendingInternal.FindValue(pendingLibrary, nullptr);
      ErrorIf(internalPendingLibrary == nullptr, "Invalid pending library");

      mCurrentToInternal.Erase(library->mCurrentFragmentLibrary);
      mCurrentToInternal.Insert(pendingLibrary, internalPendingLibrary);
      mPendingToPendingInternal.Erase(pendingLibrary);
    }
  }

  ErrorIf(!mPendingToPendingInternal.Empty(), "We created a new library but it was not given to commit");

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
    forRange (BoundType* boundType, wrapperLibrary->BoundTypes.Values())
    {
      if (boundType->HasAttribute("CoreVertex") != nullptr && boundType->HasAttribute("Vertex") != nullptr)
      {
        mCoreVertexFragments.PushBack(boundType->Name);
        mFragmentTypes.Insert(boundType->Name, ZilchFragmentType::CoreVertex);
      }
      else if (boundType->HasAttribute("RenderPass") != nullptr && boundType->HasAttribute("Pixel") != nullptr)
      {
        mRenderPassFragments.PushBack(boundType->Name);
        mFragmentTypes.Insert(boundType->Name, ZilchFragmentType::RenderPass);
      }
      else if (boundType->HasAttribute("PostProcess") != nullptr && boundType->HasAttribute("Pixel") != nullptr)
      {
        mFragmentTypes.Insert(boundType->Name, ZilchFragmentType::PostProcess);
      }
      else if (boundType->HasAttribute("Protected") != nullptr)
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

bool ZilchShaderGenerator::BuildShaders(ShaderSet& shaders, HashMap<String, UniqueComposite>& composites, Array<ShaderEntry>& shaderEntries, Array<ZilchShaderDefinition>* compositeShaderDefs)
{
  ZilchCompositor compositor;
  compositor.mEmitVertexCallback = CustomEmitVertexCallback;
  ZilchShaderProject shaderProject("ShaderProject");

  ZilchShaderLibraryRef fragmentsLibrary = GetCurrentInternalProjectLibrary();

  forRange (Shader* shader, shaders.All())
  {
    ZilchShaderDefinition shaderDef;

    // make shader def
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

  forRange (Shader* shader, shaders.All())
    shader->mLibrary = shaderLibrary;

  forRange (ShaderEntry& entry, shaderEntries.All())
  {
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

  return true;
}

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

void ZilchShaderGenerator::OnZilchFragmentCompilationError(Zilch::ErrorEvent* event)
{
  String shortMessage = event->ExactError;
  String fullMessage = event->GetFormattedMessage(Zilch::MessageFormat::Python);
  ZilchFragmentManager::GetInstance()->DispatchScriptError(Events::SyntaxError, shortMessage, fullMessage, event->Location);
}

void ZilchShaderGenerator::OnZilchFragmentTranslationError(TranslationErrorEvent* event)
{
  String fullMessage = event->GetFormattedMessage(Zilch::MessageFormat::Python);
  ZilchFragmentManager::GetInstance()->DispatchScriptError(Events::SyntaxError, event->mShortMessage, fullMessage, event->mLocation);
}

void ZilchShaderGenerator::OnFragmentProjectPostSyntaxer(ParseEvent* e)
{
  MetaLibraryExtensions::AddExtensionsPostCompilation(*e->Builder);
}

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

ZilchShaderLibraryRef ZilchShaderGenerator::GetCurrentInternalLibrary(LibraryRef library)
{
  return mCurrentToInternal.FindValue(library, nullptr);
}

ZilchShaderLibraryRef ZilchShaderGenerator::GetPendingInternalLibrary(LibraryRef library)
{
  return mPendingToPendingInternal.FindValue(library, nullptr);
}

ZilchShaderGenerator::InternalLibraryRange ZilchShaderGenerator::GetCurrentInternalLibraries()
{
  return mCurrentToInternal.Values();
}

ZilchShaderLibraryRef ZilchShaderGenerator::GetCurrentInternalProjectLibrary()
{
  return GetInternalLibrary(ZilchManager::GetInstance()->mCurrentFragmentProjectLibrary);
}

ZilchShaderLibraryRef ZilchShaderGenerator::GetPendingInternalProjectLibrary()
{
  return GetInternalLibrary(ZilchManager::GetInstance()->mPendingFragmentProjectLibrary);
}

} // namespace Zero
