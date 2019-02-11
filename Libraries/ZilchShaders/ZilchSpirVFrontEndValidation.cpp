// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#include "ZilchSpirVFrontEndValidation.hpp"

namespace Zero
{

void ValidateEntryPoint(ZilchSpirVFrontEnd* translator,
                        Zilch::GenericFunctionNode* node,
                        ZilchSpirVFrontEndContext* context)
{
  Zilch::DelegateType* functionType = node->DefinedFunction->FunctionType;
  if (functionType->Return != ZilchTypeId(void))
  {
    translator->SendTranslationError(
        node->Location, "Entry points must have a return type of 'void'.");
    return;
  }

  ZilchShaderIRType* currentType = context->mCurrentType;
  FragmentType::Enum fragmentType = currentType->mMeta->mFragmentType;
  // Quick validation on entry points. Entry points can't be on random helper
  // classes.
  if (fragmentType == FragmentType::None)
  {
    translator->SendTranslationError(node->Location,
                                     "Entry point requires fragment type.");
    return;
  }
  else if (fragmentType == FragmentType::Geometry)
    ValidateGeometryEntryPoint(translator, node, context);
  else
    ValidateBasicEntryPoint(translator, node, context);
}

void ValidateBasicEntryPoint(ZilchSpirVFrontEnd* translator,
                             Zilch::GenericFunctionNode* node,
                             ZilchSpirVFrontEndContext* context)
{
  // Vertex/Pixel entry points can't have any arguments.
  if (node->Parameters.Size() != 0)
  {
    translator->SendTranslationError(
        node->Location, "Entry point function cannot have arguments.");
    return;
  }
}

void ValidateGeometryEntryPoint(ZilchSpirVFrontEnd* translator,
                                Zilch::GenericFunctionNode* node,
                                ZilchSpirVFrontEndContext* context)
{
  SpirVNameSettings& nameSettings = translator->mSettings->mNameSettings;

  if (node->Parameters.Size() != 2)
  {
    translator->SendTranslationError(node->Location,
                                     "Geometry shader entry point must have a "
                                     "signature of (inputType, outputType)");
    return;
  }

  ZilchShaderIRType* currentType = context->mCurrentType;
  ZilchShaderIRType* inputType = translator->FindType(node->Parameters[0]);
  ZilchShaderIRType* outputType = translator->FindType(node->Parameters[1]);

  Zilch::GeometryStreamUserData* inputUserData =
      inputType->mZilchType->Has<Zilch::GeometryStreamUserData>();
  Zilch::GeometryStreamUserData* outputUserData =
      outputType->mZilchType->Has<Zilch::GeometryStreamUserData>();

  // Validate that the parameters are the correct input/output types.
  if (inputUserData == nullptr || inputUserData->mInput == false)
  {
    translator->SendTranslationError(
        node->Parameters[0]->Location,
        "Argument 1 must be an input stream type.");
  }
  if (outputUserData == nullptr || outputUserData->mInput == true)
  {
    translator->SendTranslationError(
        node->Parameters[1]->Location,
        "Argument 2 must be an output stream type.");
  }

  // Validate a specified max vertices value
  ShaderIRAttributeParameter* maxVerticesParam = nullptr;
  ShaderIRAttribute* geometryAttribute =
      currentType->FindFirstAttribute(nameSettings.mGeometryAttribute);
  if (geometryAttribute != nullptr)
    maxVerticesParam =
        geometryAttribute->FindFirstParameter(nameSettings.mMaxVerticesParam);

  if (maxVerticesParam == nullptr)
  {
    translator->SendTranslationError(node->Location,
                                     "Geometry fragment expects max vertices");
    return;
  }
}

} // namespace Zero
