///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

void WriteGlslGeometryMain(ZilchShaderTranslator* translator, Zilch::SyntaxNode* node, ShaderFunction* function, ZilchShaderTranslatorContext* context)
{
  ShaderType* currentType = context->mCurrentType;
  // This should never happen!
  if(currentType == nullptr || function == nullptr)
    return;

  ShaderType* inputStreamShaderType = currentType->mInputType;
  ShaderType* outputStreamShaderType = currentType->mOutputType;
  // This should only happen if the user writes an invalid main function for a geometry shader
  if(inputStreamShaderType == nullptr || outputStreamShaderType == nullptr)
    return;
  
  ShaderFunction* classConstructor = currentType->FindFunction(ZilchShaderSettings::GetDefaultConstructorKey());
  ShaderFunction* outputStreamConstructor = outputStreamShaderType->FindFunction(ZilchShaderSettings::GetDefaultConstructorKey());
  // This should only happen if the main function for the fragment takes invalid parameters (should be handled above)
  // or possibly if the user creates extra constructors and hides the default
  if(classConstructor == nullptr || outputStreamConstructor == nullptr)
    return;

  // Mark that we've encountered a "main" function
  currentType->SetHasMain(true);

  // Collect a bunch of required names/strings
  String className = currentType->mShaderName;
  String classVarName = className.ToLower();
  String inTypeName = inputStreamShaderType->mShaderName;
  String inParamName = "inputs";
  String outTypeName = outputStreamShaderType->mShaderName;
  String outParamName = outTypeName.ToLower();
  String outputStreamConstructorName = outputStreamConstructor->mShaderName;
  String classConstructorName = classConstructor->mShaderName;

  ShaderFunction* mainFunction = currentType->CreateFinalShaderFunction("main");
  mainFunction->mComments.PushBack("----- Main -----");
  mainFunction->mShaderReturnType = "void";
  mainFunction->mShaderSignature = "(void)";

  ScopedShaderCodeBuilder mainBuilder(context);
  mainBuilder.BeginScope();

  // Construct input/output types
  mainBuilder << mainBuilder.EmitIndent() << "// Construct input/output types" << mainBuilder.EmitLineReturn();
  mainBuilder << mainBuilder.EmitIndent() << inTypeName << " " << inParamName << "; " << mainBuilder.EmitLineReturn();
  mainBuilder << mainBuilder.EmitIndent() << outTypeName << " " << outParamName << " = " << outputStreamConstructorName << "()" << "; " << mainBuilder.EmitLineReturn();

  // Construct the shader type class
  mainBuilder << mainBuilder.EmitIndent() << "// Construct the shader type" << mainBuilder.EmitLineReturn();
  mainBuilder << mainBuilder.EmitIndent() << className << " " << classVarName << " = " << classConstructorName << "()" << "; " << mainBuilder.EmitLineReturn();
  mainBuilder << mainBuilder.EmitLineReturn();
    
  // Copy the global inputs to the class' member variables
  mainBuilder << mainBuilder.EmitIndent() << "CopyInputs(" << inParamName << ", " << classVarName << ");" << mainBuilder.EmitLineReturn();
  // Call the class' function that had the [Main] attribute
  mainBuilder << mainBuilder.EmitIndent() << function->mShaderName << "(" << inParamName << ", " << outParamName << ", " << classVarName << ");" << mainBuilder.EmitLineReturn();

  mainBuilder.EndScope();

  mainFunction->mShaderBodyCode = mainBuilder.ToString();
  
}

void GenerateCopyGeometryInputs(ZilchShaderTranslator* translator, ShaderType* compositeShaderType, ZilchShaderTranslatorContext* context)
{
  NameSettings& settings = translator->mSettings->mNameSettings;
  ShaderSystemValueSettings& systemValueSettings = translator->mSettings->mSystemValueSettings;

  ShaderType* inputStreamShaderType = compositeShaderType->mInputType;
  ShaderType* inputDataShaderType = inputStreamShaderType->mInputType;
  if(inputStreamShaderType == nullptr || inputDataShaderType == nullptr)
    return;

  String inputStreamVarName = "inputs";
  String shaderName = inputStreamShaderType->mShaderName;

  // Simultaneously write out our global inputs and the function that copies them into the class' member variables
  ShaderCodeBuilder copyInputsBuilder;
  ShaderCodeBuilder globalVarsBuilder;

  ShaderFunction* copyInputsFunction = compositeShaderType->CreateFinalShaderFunction("CopyInputs");
  copyInputsFunction->mShaderReturnType = "void";

  // Write: "void CopyInputs(inout `inputStreamType` inputs, inout `shaderType` self)"
  ShaderCodeBuilder signatureBuilder;
  signatureBuilder << "(inout " << shaderName << " " << inputStreamVarName;
  signatureBuilder << ", inout " << compositeShaderType->mShaderName << " " << settings.mThisKeyword << ")";
  copyInputsFunction->mShaderSignature = signatureBuilder.ToString();

  copyInputsBuilder.BeginScope();

  size_t count = inputStreamShaderType->GetTypeData()->mCount;
  String layoutType = inputStreamShaderType->GetTypeData()->mExtraData;
  // Write: "layout(`layoutType`) in;"
  globalVarsBuilder << "layout(" << layoutType << ") in;" << globalVarsBuilder.EmitLineReturn();

  // Iterate over all inputs on the composite type to declare uniforms.
  // Varyings are not declared here as they're part of the input data type.
  for(size_t i = 0; i < compositeShaderType->mFieldList.Size(); ++i)
  {
    ShaderField* field = compositeShaderType->mFieldList[i];
    ShaderFieldKey fieldKey(field);

    String uniformName = field->mShaderName;
    String varName = BuildString(settings.mThisKeyword, ".", field->mShaderName);
    // Translate static uniforms specially. Static uniforms currently only show up on built-in types
    // (since they're from a composite) so they need to be translated so that they don't get mangled
    // (so they can be set from C++). This uniform is later copied into the actual static variable
    // that is still declared. @JoshD: Cleanup and make more uniform through all shaders later.
    if(field->ContainsAttribute(settings.mStaticAttribute))
    {
      varName = field->mShaderName;
      uniformName = field->mZilchName;
    }

    // If this is marked as a system value then try write out the copy line for the system value
    if(field->ContainsAttribute(settings.mSystemValueInputAttributeName))
    {
      LanguageSystemValue* systemValue = systemValueSettings.FindSystemValue(ShaderStageType::GeometryPrimitive, fieldKey, translator);
      if(systemValue != nullptr && systemValue->IsInput())
      {
        copyInputsBuilder << copyInputsBuilder.EmitIndent() << varName << " = " << systemValue->mInputName << ";" << copyInputsBuilder.EmitLineReturn();
        continue;
      }
      // This should never happen
      else
        Error("Expected system value '%s : %s' either didn't exist or wasn't an input.", field->mZilchName.c_str(), field->mZilchType.c_str());
    }

    // If this is marked as a uniform then write the copy line and declare the uniform
    if(field->ContainsAttribute(settings.mUniformName))
    {
      // Don't write out the copying of the input if we can't copy it (samplers)
      ShaderType* fieldShaderType = translator->mCurrentLibrary->FindType(field->mZilchType);
      if(!fieldShaderType->ContainsAttribute(NameSettings::mNonCopyableAttribute))
      {
        // Write: "self.`varName` = `uniformName`;"
        copyInputsBuilder << copyInputsBuilder.EmitIndent() << varName << " = " << uniformName << ";" << copyInputsBuilder.EmitLineReturn();
      }
      // Write: "uniform `varType` `uniformName`;"
      globalVarsBuilder << "uniform " << field->mShaderType << " " << uniformName << ";" << copyInputsBuilder.EmitLineReturn();
    }
  }

  // Now declare all varyings for the input data struct and write all
  // of the lines to copy the varyings into the data struct
  copyInputsBuilder.WriteIndentation();
  // Write: "for(int i = 0; i < `count`; ++i)"
  copyInputsBuilder << "for(int i = 0; i < " << ToString(count) << "; ++i)" << copyInputsBuilder.EmitLineReturn();
  copyInputsBuilder.BeginScope();
  // Walk over all fields on the input type and simultaneously declare a shader stage input (if we can)
  // while writing out the line that copies from that input to the class' data member.
  for(size_t i = 0; i < inputDataShaderType->mFieldList.Size(); ++i)
  {
    ShaderField* field = inputDataShaderType->mFieldList[i];
    String varyingInputNameDeclaration = BuildString(settings.mGeometryShaderInputPrefix, field->mShaderName);
    String varyingInputIndexAccess = BuildString(varyingInputNameDeclaration, "[i]");

    // If this is a system value then copy from the system value
    if(field->ContainsAttribute(settings.mSystemValueInputAttributeName))
    {
      LanguageSystemValue* systemValue = systemValueSettings.FindSystemValue(ShaderStageType::GeometryVertex, field, translator);
      if(systemValue != nullptr && systemValue->IsInput())
      {
        copyInputsBuilder << copyInputsBuilder.EmitIndent() << inputStreamVarName << "[i]." << field->mZilchName << " = " << systemValue->mInputName << ";" << copyInputsBuilder.EmitLineReturn();
        continue;
      }
      // This should never happen
      else
        Error("Expected system value '%s : %s' either didn't exist or wasn't an input.", field->mZilchName.c_str(), field->mZilchType.c_str());
    }
    // If this is a stage input then copy from the stage input and declare the varying
    if(field->ContainsAttribute(settings.mStageInputAttributeName))
    {
      // Hack: to allow outputting varying integers. Should eventually be changed to
      // be an attribute (as even float can choose to not be interpolated).
      if(field->mZilchType == "Integer")
        globalVarsBuilder << "flat ";
      
      // Write out the input varying declaration: "in `varType`[`count`] `varName`;"
      globalVarsBuilder << "in " << field->mShaderType << "[" << ToString(count) << "] " << varyingInputNameDeclaration << ";" << globalVarsBuilder.EmitLineReturn();

      // Write the copying of the input varying: "inputs[i].`varName` = `inputName`;"
      copyInputsBuilder << copyInputsBuilder.EmitIndent() << inputStreamVarName << "[i]." << field->mZilchName << " = " << varyingInputIndexAccess << ";" << copyInputsBuilder.EmitLineReturn();
    }
  }
  copyInputsBuilder.EndScope();

  copyInputsBuilder.EndScope();

  // Write out the variable declarations
  compositeShaderType->mInputAndOutputVariableDeclarations = BuildString(compositeShaderType->mInputAndOutputVariableDeclarations, globalVarsBuilder.ToString());
  copyInputsFunction->mShaderBodyCode = copyInputsBuilder.ToString();
}

void FindOutputStreamTypes(ShaderType* type, Array<ShaderType*>& streamTypes, HashSet<String>& visitedTypes)
{
  // Iterate over all dependencies of the current type
  for(size_t i = 0; i < type->mDependencyList.Size(); ++i)
  {
    ShaderType* dependentShaderType = type->mDependencyList[i];
    // If we've already visited this type then there's no point in checking its dependencies
    if(visitedTypes.Contains(dependentShaderType->mZilchName))
      continue;

    visitedTypes.Insert(dependentShaderType->mZilchName);
    // If this dependency is a geometry output then add it to our list
    if(dependentShaderType->GetTypeData()->mType == ShaderVarType::GeometryOutput)
      streamTypes.PushBack(dependentShaderType);

    // Always recurse to find more output stream types
    FindOutputStreamTypes(dependentShaderType, streamTypes, visitedTypes);
  }
}

void GenerateGeometryShaderCloneVertex(ZilchShaderTranslator* translator, NameSettings& nameSettings, ShaderType* compositeShaderType, ShaderType* fragmentOutputDataType)
{
  // The clone vertex function copies every available input to the corresponding field in the final output type.
  // If either the input or output doesn't exist then no copy should happen.
  ShaderSystemValueSettings& systemValueSettings = translator->mSettings->mSystemValueSettings;

  // Get the actual input data type so we know what values are available for copying
  ShaderType* compositeInputDataShaderType = compositeShaderType->mInputType->mInputType;
  
  // Generate the clone vertex function
  ShaderFunction* cloneFunction = compositeShaderType->CreateFinalShaderFunction("CloneVertex");
  cloneFunction->mShaderReturnType = "void";
  cloneFunction->mShaderSignature = BuildString("(int i, inout ", fragmentOutputDataType->mShaderName, " finalOutput)");

  ShaderCodeBuilder builder;
  builder.BeginScope();
  builder << builder.EmitIndent() << "// Copy input to output" << builder.EmitLineReturn();
  for(size_t i = 0; i < fragmentOutputDataType->mFieldList.Size(); ++i)
  {
    ShaderField* outputShaderField = fragmentOutputDataType->mFieldList[i];
    // Only visit fields marked as stage outputs or system value outputs
    if(outputShaderField->ContainsAttribute(nameSettings.mStageOutputAttributeName) == false &&
       outputShaderField->ContainsAttribute(nameSettings.mSystemValueOutputAttributeName) == false)
      continue;

    // To clone an output we need to set the field from whatever input we can find.
    // This could be a system value that is available in this stage. This could also be a stage input of the input vertex struct.
    
    // Check system values
    LanguageSystemValue* systemValue = systemValueSettings.FindSystemValue(ShaderStageType::GeometryVertex, outputShaderField, translator);
    if(systemValue != nullptr && systemValue->IsInput())
    {
      // Write out the copy line: "finalOutput.`ShaderName` = `inputName`;"
      builder << builder.EmitIndent() << "finalOutput." << outputShaderField->mShaderName << " = " << systemValue->mInputName << ";" << builder.EmitLineReturn();
      continue;
    }
    // Check the input vertex struct
    ShaderField* inputShaderField = compositeInputDataShaderType->FindField(outputShaderField->mZilchName);
    if(inputShaderField && inputShaderField->ContainsAttribute(nameSettings.mStageInputAttributeName))
    {
      // Build the input name which will be something like "varName[i]"
      String inputName = BuildString(nameSettings.mGeometryShaderInputPrefix, outputShaderField->mShaderName, "[i]");
      // Write out the copy line: "finalOutput.`ShaderName` = `inputName`;"
      builder << builder.EmitIndent() << "finalOutput." << outputShaderField->mShaderName << " = " << inputName << ";" << builder.EmitLineReturn();
      continue;
    }
  }
  builder.EndScope();
  cloneFunction->mShaderBodyCode = builder.ToString();
}

void GenerateGeometryShaderWriteVertex(ZilchShaderTranslator* translator, NameSettings& nameSettings, ShaderType* compositeShaderType, ShaderType* fragmentOutputDataType)
{
  // The write vertex function copies the output type to the final glsl global varying.

  // Create the write vertex function
  ShaderFunction* writeVertexFunction = compositeShaderType->CreateFinalShaderFunction("WriteVertex");
  writeVertexFunction->mShaderReturnType = "void";
  writeVertexFunction->mShaderSignature = BuildString("(", fragmentOutputDataType->mShaderName, " finalOutput)");

  // Get the actual input data type so we know what values are available for copying
  ShaderType* compositeOutputDataShaderType = compositeShaderType->mOutputType->mOutputType;

  ShaderCodeBuilder builder;
  builder.BeginScope();
  // Copy all outputs
  for(size_t i = 0; i < fragmentOutputDataType->mFieldList.Size(); ++i)
  {
    ShaderField* shaderField = fragmentOutputDataType->mFieldList[i];

    // If this is a system value then write to it
    if(shaderField->ContainsAttribute(nameSettings.mSystemValueOutputAttributeName))
    {
      LanguageSystemValue* systemValue = translator->mSettings->mSystemValueSettings.FindSystemValue(ShaderStageType::GeometryVertex, shaderField, translator);
      if(systemValue != nullptr && systemValue->IsOutput())
        builder << builder.EmitIndent() << systemValue->mOutputName << " = finalOutput." << shaderField->mShaderName << ";" << builder.EmitLineReturn();
      else
        Error("Expected system value '%s : %s' either didn't exist or wasn't an input.", shaderField->mZilchName.c_str(), shaderField->mZilchType.c_str());
    }
    
    // If this is a stage output then attempt to write it out
    if(shaderField->ContainsAttribute(nameSettings.mStageOutputAttributeName))
    {
      // Check to make sure that this field is an output of the composite
      // @JoshD: this might not be necessary
      ShaderField* compositeShaderField = compositeOutputDataShaderType->FindField(shaderField->mZilchName);
      if(compositeShaderField == nullptr || !compositeShaderField->ContainsAttribute(nameSettings.mStageOutputAttributeName))
        continue;

      // Build the input name (all outputs start with the pixel shader prefix to avoid name duplicates)
      String psInputName = BuildString(nameSettings.mPixelShaderInputPrefix, shaderField->mShaderName);
      // Write out the copy: "'inputName' = finalOutput.`ShaderName`"
      builder << builder.EmitIndent() << psInputName << " = finalOutput." << shaderField->mShaderName << ";" << builder.EmitLineReturn();
    }
  }
  builder.EndScope();
  writeVertexFunction->mShaderBodyCode = builder.ToString();
}

void GenerateGeometryShaderWriteToVertex(ZilchShaderTranslator* translator, NameSettings& nameSettings, ShaderType* compositeShaderType, ShaderType* fragmentOutputDataType)
{
  // WriteToVertex is a simple function that just calls CloneVertex, EmitVertexHelper, and WriteVertex.
  // This function exists because the OutputStream type doesn't know about the composite output data type (and can't know)
  // so it can't generate a full translation in isolation. Instead it can call this function which can be uniquely generated for the final shader.
  ShaderType* compositeOutputDataShaderType = compositeShaderType->mOutputType->mOutputType;

  ShaderFunction* writeToVertexFunction = compositeShaderType->CreateFinalShaderFunction("WriteToVertex");
  writeToVertexFunction->mShaderReturnType = "void";
  writeToVertexFunction->mShaderSignature = BuildString("(int index, ", fragmentOutputDataType->mShaderName, " fragmentOutput)");

  ShaderCodeBuilder builder;
  builder.BeginScope();
  builder << builder.EmitIndent() << compositeOutputDataShaderType->mShaderName << " finalOutput;" << builder.EmitLineReturn();
  builder << builder.EmitIndent() << "CloneVertex(index, finalOutput);" << builder.EmitLineReturn();
  builder << builder.EmitIndent() << nameSettings.mEmitVertexHelperFunctionName << "(fragmentOutput, finalOutput);" << builder.EmitLineReturn();
  builder << builder.EmitIndent() << "WriteVertex(finalOutput);" << builder.EmitLineReturn();
  builder.EndScope();
  writeToVertexFunction->mShaderBodyCode = builder.ToString();
}

void GenerateDummyEmitVertexHelper(ZilchShaderTranslator* translator, NameSettings& nameSettings, ShaderType* compositeShaderType, ShaderType* fragmentOutputDataType)
{
  ShaderType* compositeOutputDataShaderType = compositeShaderType->mOutputType->mOutputType;
  ShaderCodeBuilder emitVertexSignatureBuilder;
  String compositeOutputVarName = "_compositeOutput";
  String fragmentOutputVarName = "_fragmentOutput";

  // Write: "(inout `fragmentOutType` `fragmentOutName`, inout `compositeOutType` `compositeOutName`)"
  emitVertexSignatureBuilder << "(inout " << fragmentOutputDataType->mShaderName << " " << fragmentOutputVarName;
  emitVertexSignatureBuilder << ", inout " << compositeOutputDataShaderType->mShaderName << " " << compositeOutputVarName << ")";
  String emitVertexSignature = emitVertexSignatureBuilder.ToString();

  // @JoshD: Cleanup! This is really bad but there's no better way to do this now.
  // Find if an emit function of the same parameter types already exists. Unfortunately, there's
  // currently not a good way to lookup functions by type alone (the types are baked into the signature).
  // At the moment this requires building the function signature with the correct variable names otherwise the signatures won't match.
  bool foundEmit = false;
  ShaderType::FunctionList* emitFunctions = compositeShaderType->mFunctionNameMultiMap.FindPointer(nameSettings.mEmitVertexHelperFunctionName);
  if(emitFunctions != nullptr)
  {
    for(size_t i = 0; i < emitFunctions->Size(); ++i)
    {
      ShaderFunction* emitFunction = (*emitFunctions)[i];
      if(emitFunction->mShaderSignature == emitVertexSignature)
      {
        foundEmit = true;
        break;
      }
    }
  }

  // If the emit function of this type already exists then it was likely already defined by the composite
  if(foundEmit)
    return;

  // Since there wasn't already an emit function we need to generate one for the current types
  ShaderFunction* emitFunction = compositeShaderType->CreateFinalShaderFunction(nameSettings.mEmitVertexHelperFunctionName);
  emitFunction->mShaderReturnType = "void";
  emitFunction->mShaderSignature = emitVertexSignature;

  // For every output of this shader (defined by the composite's data type),
  // if that output also exists on the current output data type (the fragment's) then
  // write out the line to copy from the fragment to the composite.
  ShaderCodeBuilder emitFunctionBuilder;
  emitFunctionBuilder.BeginScope();
  for(size_t i = 0; i < compositeOutputDataShaderType->mFieldList.Size(); ++i)
  {
    ShaderField* finalField = compositeOutputDataShaderType->mFieldList[i];
    // Only visit outputs
    if(!finalField->ContainsAttribute(nameSettings.mOutputAttributeName))
      continue;

    // Find the corresponding field on the fragment type, if this field doesn't exist or isn't the same type then skip it
    ShaderField* fragField = fragmentOutputDataType->FindField(finalField->mZilchName);
    if(fragField == nullptr || fragField->mZilchType != finalField->mZilchType)
      continue;

    // If this field also isn't an output on the fragment type then skip it
    if(!fragField->ContainsAttribute(nameSettings.mOutputAttributeName))
      continue;

    // Write: "_compositeOutput.`fieldName` = _fragmentOutput.`fieldName`;"
    String fieldName = finalField->mZilchName;
    emitFunctionBuilder << emitFunctionBuilder.EmitIndent() << compositeOutputVarName << "." << fieldName << " = ";
    emitFunctionBuilder << fragmentOutputVarName << "." << fieldName << ";" << emitFunctionBuilder.EmitLineReturn();
  }
  emitFunctionBuilder.EndScope();
  emitFunction->mShaderBodyCode = emitFunctionBuilder.ToString();
}

void GenerateCopyGeometryOutputs(ZilchShaderTranslator* translator, ShaderType* compositeShaderType, ZilchShaderTranslatorContext* context)
{
  NameSettings& nameSettings = translator->mSettings->mNameSettings;
  ShaderDefinitionSettings& shaderDefSettings = translator->mSettings->mShaderDefinitionSettings;
  ShaderType* outputStreamShaderType = compositeShaderType->mOutputType;
  ShaderType* outputDataShaderType = outputStreamShaderType->mOutputType;
  
  if(outputDataShaderType == nullptr)
    return;

  ShaderCodeBuilder globalVarsBuilder;
  
  String layoutType = outputStreamShaderType->GetTypeData()->mExtraData;
  size_t maxVertexCount = compositeShaderType->GetTypeData()->mCount;
  // Write: layout(`layoutType`, max_vertices = `maxVertexCount`) out;
  globalVarsBuilder << "layout(" << layoutType << ", max_vertices = " << ToString(maxVertexCount) << ") out;" << globalVarsBuilder.EmitLineReturn();
  
  // Iterate over all of the fields of the output data type and emit output
  // varyings for each variable with the output attribute
  for(size_t i = 0; i < outputDataShaderType->mFieldList.Size(); ++i)
  {
    ShaderField* field = outputDataShaderType->mFieldList[i];
    if(!field->ContainsAttribute(nameSettings.mStageOutputAttributeName))
      continue;

    // Write: "out `varType` `varName`;"
    String varName = BuildString(nameSettings.mPixelShaderInputPrefix, field->mShaderName);
    // Hack: to allow outputting varying integers. Should eventually be changed to
    // be an attribute (as even float can choose to not be interpolated).
    if(field->mZilchType == "Integer")
      globalVarsBuilder << "flat ";
    globalVarsBuilder << "out " << field->mShaderType << " " << varName << ";" << globalVarsBuilder.EmitLineReturn();
  }

  // Write out the variable declarations
  compositeShaderType->mInputAndOutputVariableDeclarations = BuildString(compositeShaderType->mInputAndOutputVariableDeclarations, globalVarsBuilder.ToString());

  // In order for compilation to work smoothly, we have to generate a bunch of helper classes on
  // all stream types used in the entire composited shader even though only 1 of them should actually
  // be called (the one of fragmentOuput to compositeOutput). This is because a stream output will
  // generate an append function (in the composite) that will never actually exist. For now generating
  // these extra dummy functions is the easy method to resolve compilation errors. Currently this array
  // should never find more than 2 stream types (fragment and composite).
  HashSet<String> visitedTypes;
  Array<ShaderType*> streamTypes;
  FindOutputStreamTypes(compositeShaderType, streamTypes, visitedTypes);
  for(size_t i = 0; i < streamTypes.Size(); ++i)
  {
    ShaderType* fragmentStreamType = streamTypes[i];
    ShaderType* fragmentOutputDataType = fragmentStreamType->mOutputType;

    GenerateGeometryShaderCloneVertex(translator, nameSettings, compositeShaderType, fragmentOutputDataType);
    GenerateGeometryShaderWriteVertex(translator, nameSettings, compositeShaderType, fragmentOutputDataType);
    GenerateGeometryShaderWriteToVertex(translator, nameSettings, compositeShaderType, fragmentOutputDataType);
    GenerateDummyEmitVertexHelper(translator, nameSettings, compositeShaderType, fragmentOutputDataType);
  }
}

void WriteGlslMain(ZilchShaderTranslator* translator, Zilch::SyntaxNode* node, ShaderFunction* function, ZilchShaderTranslatorContext* context)
{
  ScopedShaderCodeBuilder mainBuilder(context);
  ShaderType* currentType = context->mCurrentType;
  String className = currentType->mShaderName;
  String classVarName = className.ToLower();
  String classConstructorName = translator->MangleName("Constructor", currentType);

  // Mark that we've encountered a "main" function
  currentType->SetHasMain(true);

  ShaderFunction* mainFunction = currentType->CreateFinalShaderFunction("main");
  mainFunction->mComments.PushBack("----- Main -----");
  mainFunction->mShaderReturnType = "void";
  mainFunction->mShaderSignature = "(void)";

  mainBuilder.BeginScope();
  // Construct the class' variable
  mainBuilder << mainBuilder.EmitIndent() << className << " " << classVarName << " = " << classConstructorName << "()" << "; " << mainBuilder.EmitLineReturn();
  // Copy the global inputs to the class' member variables
  mainBuilder << mainBuilder.EmitIndent() << "CopyInputs(" << classVarName << ");" << mainBuilder.EmitLineReturn();
  // Call the class' function that had the [Main] attribute
  mainBuilder << mainBuilder.EmitIndent() << function->mShaderName << "(" << classVarName << ");" << mainBuilder.EmitLineReturn();
  // Copy the class' member variables to the global outputs
  mainBuilder << mainBuilder.EmitIndent() << "CopyOutputs(" << classVarName << ");" << mainBuilder.EmitLineReturn();

  mainBuilder.EndScope();
  mainFunction->mShaderBodyCode = mainBuilder.ToString();
}

void GenerateCopyInputs(ZilchShaderTranslator* translator, ShaderType* type, ZilchShaderTranslatorContext* context, bool legacyMode)
{
  NameSettings& settings = translator->mSettings->mNameSettings;
  ShaderDefinitionSettings& shaderSettings = translator->mSettings->mShaderDefinitionSettings;
  ShaderSystemValueSettings& systemValueSettings = translator->mSettings->mSystemValueSettings;

  String attributeKeyword = "attribute";
  String varyingKeyword = "varying";
  if(!legacyMode)
    attributeKeyword = varyingKeyword = "in";

  ShaderFunction* copyInputsFunction = type->CreateFinalShaderFunction("CopyInputs");
  copyInputsFunction->mShaderReturnType = "void";
  copyInputsFunction->mShaderSignature = BuildString("(inout ", type->mShaderName, " ", settings.mThisKeyword, ")");

  // Simultaneously write out our global inputs and the function that copies them into the class' member variables
  ShaderCodeBuilder copyInputsBuilder;
  ShaderCodeBuilder globalVarsBuilder;

  copyInputsBuilder.BeginScope();
  for(size_t i = 0; i < type->mFieldList.Size(); ++i)
  {
    ShaderField* field = type->mFieldList[i];
    if(field->ContainsAttribute(settings.mUniformName))
    {
      String uniformName = field->mShaderName;
      String varName = BuildString(settings.mThisKeyword, ".", field->mShaderName);

      // Translate static uniforms specially. Static uniforms currently only show up on built-in types
      // (since they're from a composite) so they need to be translated so that they don't get mangled
      // (so they can be set from C++). This uniform is later copied into the actual static variable
      // that is still declared. @JoshD: Cleanup and make more uniform through all shaders later.
      if(field->ContainsAttribute(settings.mStaticAttribute))
      {
        varName = field->mShaderName;
        uniformName = field->mZilchName;
      }

      // Don't write out the copying of the input if we can't copy it (samplers)
      ShaderType* fieldShaderType = translator->mCurrentLibrary->FindType(field->mZilchType);
      if(!fieldShaderType->ContainsAttribute(NameSettings::mNonCopyableAttribute))
      {
        // Write: "self.`varName` = `uniformName`;"
        copyInputsBuilder << copyInputsBuilder.EmitIndent() << varName << " = " << uniformName << ";" << copyInputsBuilder.EmitLineReturn();
      }
      // Write: "uniform `varType` `uniformName`;"
      globalVarsBuilder << "uniform " << field->mShaderType << " " << uniformName << ";" << copyInputsBuilder.EmitLineReturn();
    }
    // If this field is marked as a system input
    else if(field->ContainsAttribute(settings.mSystemValueInputAttributeName))
    {
      // Find the system value that matches this field to copy the value from
      LanguageSystemValue* systemValue = systemValueSettings.FindSystemValue(field, translator);
      if(systemValue != nullptr && systemValue->IsInput())
      {
        String varName = BuildString(settings.mThisKeyword, ".", field->mShaderName);
        copyInputsBuilder << copyInputsBuilder.EmitIndent() << varName << " = " << systemValue->mInputName << ";" << copyInputsBuilder.EmitLineReturn();
        continue;
      }
      else
        Error("Expected system value '%s : %s' either didn't exist or wasn't an input.", field->mZilchName.c_str(), field->mZilchType.c_str());
    }
    else if(field->ContainsAttribute(settings.mStageInputAttributeName) && type->mFragmentType == FragmentType::Vertex)
    {
      // Write: self.VarName = attributeName;
      String attributeName = BuildString(settings.mAttributePrefix, field->mShaderName);
      copyInputsBuilder << copyInputsBuilder.EmitIndent() << settings.mThisKeyword << "." << field->mShaderName << " = " << attributeName << ";" << copyInputsBuilder.EmitLineReturn();
      // Write: attribute type attributeName;
      globalVarsBuilder << attributeKeyword << " " << field->mShaderType << " " << attributeName << ";" << copyInputsBuilder.EmitLineReturn();
    }
    // Write out the varying declaration and copying (only if a pixel shader though, as we could
    // be writing out to a varying in a vertex shader but inputting from a uniform)
    else if(field->ContainsAttribute(settings.mStageInputAttributeName) && type->mFragmentType == FragmentType::Pixel)
    {
      // Write: self.VarName = varyingName;
      String varyingName = BuildString(settings.mPixelShaderInputPrefix, field->mShaderName);
      copyInputsBuilder << copyInputsBuilder.EmitIndent() << settings.mThisKeyword << "." << field->mShaderName << " = " << varyingName << ";" << copyInputsBuilder.EmitLineReturn();
      // Hack: to allow outputting varying integers. Should eventually be changed to
      // be an attribute (as even float can choose to not be interpolated).
      if(field->mZilchType == "Integer")
        globalVarsBuilder << "flat ";
      // Write: varying type varyingName;
      globalVarsBuilder << varyingKeyword << " " << field->mShaderType << " " << varyingName << ";" << copyInputsBuilder.EmitLineReturn();
    }
  }
  copyInputsBuilder.EndScope();
  // Write out the variable declarations
  type->mInputAndOutputVariableDeclarations = BuildString(type->mInputAndOutputVariableDeclarations, globalVarsBuilder.ToString());
  copyInputsFunction->mShaderBodyCode = copyInputsBuilder.ToString();
}

void GenerateCopyOutputs(ZilchShaderTranslator* translator, ShaderType* type, ZilchShaderTranslatorContext* context, bool legacyMode)
{
  NameSettings& settings = translator->mSettings->mNameSettings;
  ShaderDefinitionSettings& shaderDefSettings = translator->mSettings->mShaderDefinitionSettings;
  ShaderSystemValueSettings& systemValueSettings = translator->mSettings->mSystemValueSettings;

  ShaderCodeBuilder globalVarsBuilder;
  ShaderCodeBuilder copyOutputsBuilder;
  ShaderFunction* copyOutputsFunction = type->CreateFinalShaderFunction("CopyOutputs");
  copyOutputsFunction->mShaderReturnType = "void";
  copyOutputsFunction->mShaderSignature = BuildString("(inout ", type->mShaderName, " ", settings.mThisKeyword, ")");

  String varyingKeyword = "varying";
  String renderTargetKeyword = "gl_FragData";
  if(!legacyMode)
  {
    varyingKeyword = "out";
    renderTargetKeyword = "outputs";
    String renderTargetCount = ToString((int)shaderDefSettings.mRenderTargetNames.Size());
    // Declare the pixel outputs
    if(type->mFragmentType == FragmentType::Pixel)
      globalVarsBuilder << "out vec4 [" << renderTargetCount << "] " << renderTargetKeyword << ";" << globalVarsBuilder.EmitLineReturn();
  }

  HashMap<String, String> remappings;
  if(type->mFragmentType == FragmentType::Pixel)
  {
    for(size_t i = 0; i < shaderDefSettings.mRenderTargetNames.Size(); ++i)
      remappings[shaderDefSettings.mRenderTargetNames[i]] = BuildString(renderTargetKeyword, "[", ToString(i), "]");
  }

  // Simultaneously write out our global outputs and the function that copies the class' member variables into them
  copyOutputsBuilder.BeginScope();
  for(size_t i = 0; i < type->mFieldList.Size(); ++i)
  {
    ShaderField* field = type->mFieldList[i];

    // If this is marked as a system value output then write out to the system value (not mutually exclusive with writing to varyings)
    if(field->ContainsAttribute(settings.mSystemValueOutputAttributeName))
    {
      // If this is not a system variable then write out the uniform (system variables are implicitly declared)
      LanguageSystemValue* systemValue = systemValueSettings.FindSystemValue(field, translator);
      if(systemValue != nullptr && systemValue->IsOutput())
        copyOutputsBuilder << copyOutputsBuilder.EmitIndent() << systemValue->mOutputName << " = " << settings.mThisKeyword << "." << field->mShaderName << ";" << copyOutputsBuilder.EmitLineReturn();
      else
        Error("Expected system value '%s : %s' either didn't exist or wasn't an output.", field->mZilchName.c_str(), field->mZilchType.c_str());
    }

    // Otherwise, if this is a stage output then write out to the varying (only for vertex shaders as pixel shaders can't write to a varying).
    // @JoshD: Check to remove this pixel shader test, the compositor just shouldn't generate this?
    // Maybe keep this in for being able to write individual shaders that don't come from composites?
    if(field->ContainsAttribute(settings.mStageOutputAttributeName) && type->mFragmentType != FragmentType::Pixel)
    {
      // Write: varyingName = self.VarName;
      String prefix = settings.mPixelShaderInputPrefix;
      if(field->ContainsAttribute(settings.mGeometryShaderOutputAttribute))
        prefix = settings.mGeometryShaderInputPrefix;

      String varyingName = BuildString(prefix, field->mShaderName);
      copyOutputsBuilder << copyOutputsBuilder.EmitIndent() << varyingName << " = " << settings.mThisKeyword << "." << field->mShaderName << ";" << copyOutputsBuilder.EmitLineReturn();
      // Hack: to allow outputting varying integers. Should eventually be changed to
      // be an attribute (as even float can choose to not be interpolated).
      if(field->mZilchType == "Integer")
        globalVarsBuilder << "flat ";
      // Write: varying type varyingName;
      globalVarsBuilder << varyingKeyword << " " << field->mShaderType << " " << varyingName << ";" << copyOutputsBuilder.EmitLineReturn();
    }

    // If we have a re-mapped name then also write it out (this can happen on top of varyings)
    String remappedName = remappings.FindValue(field->mShaderName, String());
    if(remappings.ContainsKey(field->mShaderName))
    {
      copyOutputsBuilder << copyOutputsBuilder.EmitIndent() << remappedName << " = " << settings.mThisKeyword << "." << field->mShaderName << "; " << copyOutputsBuilder.EmitLineReturn();
    }
  }
  copyOutputsBuilder.EndScope();

  copyOutputsFunction->mShaderBodyCode = copyOutputsBuilder.ToString();
  // Write out the variable declarations
  type->mInputAndOutputVariableDeclarations = BuildString(type->mInputAndOutputVariableDeclarations, globalVarsBuilder.ToString());
}

//-------------------------------------------------------------------BaseGlslTranslator
String BaseGlslTranslator::mLanguageName = "glsl";

void BaseGlslTranslator::SetupShaderLanguage()
{
  mLibraryTranslator.Reset();
  SetupGlsl_1_3(this);
}

String BaseGlslTranslator::GetLanguageName()
{
  return mLanguageName;
}

void BaseGlslTranslator::WriteGeometryOutputVariableDeclaration(Zilch::LocalVariableNode*& node, ShaderType* variableShaderType, ZilchShaderTranslatorContext* context)
{
  // For glsl, simply construct the to-type variable
  Zilch::FunctionCallNode* fnCallNode = Zilch::Type::DynamicCast<Zilch::FunctionCallNode*>(node->InitialValue);
  String toName = ApplyVariableReplacement(node->Name.Token);
  ShaderType* toType = variableShaderType;
  // Find the default constructor so we can call it
  ShaderFunction* defaultConstructor = toType->FindFunction(ZilchShaderSettings::GetDefaultConstructorKey());

  // Write: varType varName = varType();
  ShaderCodeBuilder& builder = context->GetBuilder();
  builder << toType->mShaderName << " " << toName << " = " << defaultConstructor->mShaderName << "()";
}

void BaseGlslTranslator::WriteMainForClass(Zilch::SyntaxNode* node, ShaderType* currentType, ShaderFunction* function, ZilchShaderTranslatorContext* context)
{
  if(context->mCurrentType->mFragmentType == FragmentType::Geometry)
  {
    GenerateCopyGeometryInputs(this, currentType, context);
    GenerateCopyGeometryOutputs(this, currentType, context);
    WriteGlslGeometryMain(this, node, function, context);
  }
  else
  {
    GenerateCopyInputs(this, currentType, context, false);
    GenerateCopyOutputs(this, currentType, context, false);
    WriteGlslMain(this, node, function, context);
  }
}

//-------------------------------------------------------------------Glsl130Translator
String Glsl130Translator::GetFullLanguageString()
{
  return "Glsl130";
}

int Glsl130Translator::GetLanguageVersionNumber()
{
  return 130;
}

String Glsl130Translator::GetVersionString()
{
  return "#version 130";
}

bool Glsl130Translator::SupportsFragmentType(ShaderType* shaderType)
{
  return shaderType->mFragmentType != FragmentType::Geometry;
}

//-------------------------------------------------------------------Glsl130Translator
String Glsl150Translator::GetFullLanguageString()
{
  return "Glsl150";
}

int Glsl150Translator::GetLanguageVersionNumber()
{
  return 150;
}

String Glsl150Translator::GetVersionString()
{
  return "#version 150";
}

bool Glsl150Translator::SupportsFragmentType(ShaderType* shaderType)
{
  return true;
}

}//namespace Zero
