///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------MemberAccessResolverWrapper
FunctionCallResolverWrapper::FunctionCallResolverWrapper()
  : mFnCall(nullptr), mConstructorCallResolver(nullptr)
{
}

FunctionCallResolverWrapper::FunctionCallResolverWrapper(FunctionCallResolverFn fnCall)
  : mFnCall(fnCall), mConstructorCallResolver(nullptr)
{
}

FunctionCallResolverWrapper::FunctionCallResolverWrapper(ConstructorCallResolverFn constructorCall)
  : mFnCall(nullptr), mConstructorCallResolver(constructorCall)
{
}

FunctionCallResolverWrapper::FunctionCallResolverWrapper(StringParam str)
  : mFnCall(nullptr), mConstructorCallResolver(nullptr)
{ 
  mBackupString = str;
}

void FunctionCallResolverWrapper::Translate(ZilchShaderTranslator* translator, Zilch::FunctionCallNode* fnCallNode, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context)
{
  // Call the function pointer if we have it, otherwise use the backup string
  if(mFnCall != nullptr)
    mFnCall(translator, fnCallNode, memberAccessNode, context);
  else
    context->GetBuilder() << mBackupString;
}

void FunctionCallResolverWrapper::Translate(ZilchShaderTranslator* translator, Zilch::FunctionCallNode* fnCallNode, Zilch::StaticTypeNode* staticTypeNode, ZilchShaderTranslatorContext* context)
{
  // Call the function pointer if we have it, otherwise use the backup string
  if(mConstructorCallResolver != nullptr)
    mConstructorCallResolver(translator, fnCallNode, staticTypeNode, context);
  else
    context->GetBuilder() << mBackupString;
}

void FunctionCallResolverWrapper::Translate(ZilchShaderTranslator* translator, ZilchShaderTranslatorContext* context)
{
  context->GetBuilder() << mBackupString;
}

//-------------------------------------------------------------------MemberAccessResolverWrapper
MemberAccessResolverWrapper::MemberAccessResolverWrapper()
  : mResolverFn(nullptr)
{
}

MemberAccessResolverWrapper::MemberAccessResolverWrapper(MemberAccessResolverFn resolverFn)
  : mResolverFn(resolverFn)
{
}

MemberAccessResolverWrapper::MemberAccessResolverWrapper(StringParam replacementStr)
  : mResolverFn(nullptr), mNameReplacementString(replacementStr)
{
}

void MemberAccessResolverWrapper::Translate(ZilchShaderTranslator* translator, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context)
{
  // Call the function pointer if we have it, otherwise use the backup string
  if(mResolverFn != nullptr)
    mResolverFn(translator, memberAccessNode, context);
  else
    context->GetBuilder() << mNameReplacementString;
}

//-------------------------------------------------------------------BinaryOpResolverWrapper
BinaryOpResolverWrapper::BinaryOpResolverWrapper()
  : mResolverFn(nullptr)
{
}

BinaryOpResolverWrapper::BinaryOpResolverWrapper(BinaryOpResolver resolverFn)
  : mResolverFn(resolverFn)
{
}

BinaryOpResolverWrapper::BinaryOpResolverWrapper(StringParam replacementFnCallStr)
  : mResolverFn(nullptr), mReplacementFnCallStr(replacementFnCallStr)
{
}

void BinaryOpResolverWrapper::Translate(ZilchShaderTranslator* translator, Zilch::BinaryOperatorNode* node, ZilchShaderTranslatorContext* context)
{
  // Call the function pointer if we have it, otherwise use the backup string
  if(mResolverFn != nullptr)
    mResolverFn(translator, node, context);
  else
  {
    ScopedShaderCodeBuilder subBuilder(context);
    subBuilder << mReplacementFnCallStr << "(";
    context->Walker->Walk(translator, node->LeftOperand, context);
    subBuilder.Write(", ");
    context->Walker->Walk(translator, node->RightOperand, context);
    subBuilder.Write(")");

    subBuilder.PopFromStack();

    context->GetBuilder().Write(subBuilder.ToString());
  }
}

//-------------------------------------------------------------------LibraryTranslator
LibraryTranslator::LibraryTranslator()
{
  Reset();
}

void LibraryTranslator::Reset()
{
  mNativeLibraryParser = nullptr;
  mTemplateTypeResolver = nullptr;

  mFunctionCallResolvers.Clear();
  mBackupConstructorResolvers.Clear();
  mMemberAccessFieldResolvers.Clear();
  mMemberAccessPropertyResolvers.Clear();
  mMemberAccessFunctionResolvers.Clear();
  mBackupMemberAccessResolvers.Clear();
  mBinaryOpResolvers.Clear();
  mIntializerResolvers.Clear();
  mRegisteredTemplates.Clear();
}

void LibraryTranslator::RegisterNativeLibraryParser(NativeLibraryParser nativeLibraryParser)
{
  mNativeLibraryParser = nativeLibraryParser;
}

void LibraryTranslator::ParseNativeLibrary(ZilchShaderTranslator* translator, ZilchShaderLibrary* shaderLibrary)
{
  ErrorIf(mNativeLibraryParser == nullptr, "No native library parser registered");
  (*mNativeLibraryParser)(translator, shaderLibrary);
}

void LibraryTranslator::RegisterFunctionCallResolver(Zilch::Function* fn, const FunctionCallResolverWrapper& wrapper)
{
  ErrorIf(fn == nullptr, "Cannot register a null function. Many things will break in this case.");
  mFunctionCallResolvers[fn] = wrapper;
}

void LibraryTranslator::RegisterFunctionCallResolver(Zilch::Function* fn, FunctionCallResolverFn resolver)
{
  ErrorIf(fn == nullptr, "Cannot register a null function. Many things will break in this case.");
  RegisterFunctionCallResolver(fn, FunctionCallResolverWrapper(resolver));
}

void LibraryTranslator::RegisterFunctionCallResolver(Zilch::Function* fn, ConstructorCallResolverFn resolver)
{
  ErrorIf(fn == nullptr, "Cannot register a null function. Many things will break in this case.");
  RegisterFunctionCallResolver(fn, FunctionCallResolverWrapper(resolver));
}

void LibraryTranslator::RegisterFunctionCallResolver(Zilch::Function* fn, StringParam replacementStr)
{
  ErrorIf(fn == nullptr, "Cannot register a null function. Many things will break in this case.");
  RegisterFunctionCallResolver(fn, FunctionCallResolverWrapper(replacementStr));
}

void LibraryTranslator::RegisterBackupConstructorResolver(Zilch::Type* type, ConstructorCallResolverFn resolver)
{
  mBackupConstructorResolvers[type] = FunctionCallResolverWrapper(resolver);
}

bool LibraryTranslator::ResolveFunctionCall(ZilchShaderTranslator* translator, Zilch::FunctionCallNode* fnCallNode, ZilchShaderTranslatorContext* context)
{
  // Check and see if the left node is a static type node. This happens when constructing a type, such as Real2();
  Zilch::StaticTypeNode* staticTypeNode = Zilch::Type::DynamicCast<Zilch::StaticTypeNode*>(fnCallNode->LeftOperand);
  if(staticTypeNode != nullptr)
  {
    FunctionCallResolverWrapper* wrapper = mFunctionCallResolvers.FindPointer(staticTypeNode->ConstructorFunction);
    if(wrapper != nullptr)
    {
      wrapper->Translate(translator, fnCallNode, staticTypeNode, context);
      return true;
    }
    wrapper = mBackupConstructorResolvers.FindPointer(staticTypeNode->ResultType);
    if(wrapper != nullptr)
    {
      wrapper->Translate(translator, fnCallNode, staticTypeNode, context);
      return true;
    }
    
    if(staticTypeNode->ConstructorFunction != nullptr && !translator->IsInOwningLibrary(staticTypeNode->ConstructorFunction->GetOwningLibrary()))
    {
      String msg = String::Format("Constructor of type '%s' is not able to be translated.", staticTypeNode->ResultType->ToString().c_str());
      translator->SendTranslationError(staticTypeNode->Location, msg);
    }
  }

  // Otherwise try a member access node. This happens on both static and instance function calls.
  Zilch::MemberAccessNode* memberAccessNode = Zilch::Type::DynamicCast<Zilch::MemberAccessNode*>(fnCallNode->LeftOperand);
  if(memberAccessNode != nullptr)
  {
    FunctionCallResolverWrapper* wrapper = mFunctionCallResolvers.FindPointer(memberAccessNode->AccessedFunction);
    if(wrapper != nullptr)
    {
      wrapper->Translate(translator, fnCallNode, memberAccessNode, context);
      return true;
    }
    // If we are calling an extension function then mark the class with the extension function as a dependency.
    // There's no need to alter the translation though as the member access node and function translation should be the same as normal.
    ShaderFunction* extensionFunction = mTranslator->mCurrentLibrary->FindExtension(memberAccessNode->AccessedFunction);
    if(extensionFunction != nullptr)
    {
      context->mCurrentType->AddDependency(extensionFunction->mOwner);
      return false;
    }
    // Don't make it an error to not translate a function here. Since this is a member
    // access node we'll have another chance to translate it when we resolve MemberAccessNode.
    // This location allows a simpler replacement of just the function name (eg. Math.Dot -> dot).
    // If we can't translate it by the time we get there then it'll be an error.
  }

  // Otherwise we didn't have a special translation for this function, let the normal translation happen.
  return false;
}

bool LibraryTranslator::ResolveDefaultConstructor(ZilchShaderTranslator* translator, Zilch::Type* type, Zilch::Function* constructorFn, ZilchShaderTranslatorContext* context)
{
  FunctionCallResolverWrapper* wrapper = mFunctionCallResolvers.FindPointer(constructorFn);
  if(wrapper != nullptr)
  {
    wrapper->Translate(translator, context);
    return true;
  }
  if(constructorFn != nullptr && !translator->IsInOwningLibrary(constructorFn->GetOwningLibrary()))
  {
    String msg = String::Format("Default constructor of type '%s' is not able to be translated", type->ToString().c_str());
    translator->SendTranslationError(constructorFn->Location, msg);
  }
  return false;
}


void LibraryTranslator::RegisterMemberAccessResolver(Zilch::Field* field, const MemberAccessResolverWrapper& wrapper)
{
  mMemberAccessFieldResolvers[field] = wrapper;
}

void LibraryTranslator::RegisterMemberAccessResolver(Zilch::Field* field, MemberAccessResolverFn resolverFn)
{
  RegisterMemberAccessResolver(field, MemberAccessResolverWrapper(resolverFn));
}

void LibraryTranslator::RegisterMemberAccessResolver(Zilch::Field* field, StringParam replacementStr)
{
  RegisterMemberAccessResolver(field, MemberAccessResolverWrapper(replacementStr));
}

void LibraryTranslator::RegisterMemberAccessResolver(Zilch::Property* prop, const MemberAccessResolverWrapper& wrapper)
{
  ErrorIf(prop == nullptr, "Cannot register a null property. Many things will break in this case.");
  mMemberAccessPropertyResolvers[prop] = wrapper;
}

void LibraryTranslator::RegisterMemberAccessResolver(Zilch::Property* prop, MemberAccessResolverFn resolverFn)
{
  RegisterMemberAccessResolver(prop, MemberAccessResolverWrapper(resolverFn));
}

void LibraryTranslator::RegisterMemberAccessResolver(Zilch::Property* prop, StringParam replacementStr)
{
  RegisterMemberAccessResolver(prop, MemberAccessResolverWrapper(replacementStr));
}

void LibraryTranslator::RegisterMemberAccessResolver(Zilch::Function* function, const MemberAccessResolverWrapper& wrapper)
{
  mMemberAccessFunctionResolvers[function] = wrapper;
}
void LibraryTranslator::RegisterMemberAccessResolver(Zilch::Function* function, MemberAccessResolverFn resolverFn)
{
  RegisterMemberAccessResolver(function, MemberAccessResolverWrapper(resolverFn));
}

void LibraryTranslator::RegisterMemberAccessResolver(Zilch::Function* function, StringParam replacementStr)
{
  RegisterMemberAccessResolver(function, MemberAccessResolverWrapper(replacementStr));
}

void LibraryTranslator::RegisterMemberAccessBackupResolver(Zilch::Type* type, MemberAccessResolverFn resolverFn)
{
  mBackupMemberAccessResolvers[type] = MemberAccessResolverWrapper(resolverFn);
}

bool LibraryTranslator::ResolveMemberAccess(ZilchShaderTranslator* translator, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context)
{
  if(memberAccessNode->AccessedField != nullptr)
  {
    Zilch::Field* field = memberAccessNode->AccessedField;
    MemberAccessResolverWrapper* wrapper = mMemberAccessFieldResolvers.FindPointer(field);
    if(wrapper != nullptr)
    {
      wrapper->Translate(translator, memberAccessNode, context);
      return true;
    }
    // Get the bound type (converts indirection types to bound types)
    Zilch::Type* resultType = Zilch::BoundType::GetBoundType(memberAccessNode->LeftOperand->ResultType);
    wrapper = mBackupMemberAccessResolvers.FindPointer(resultType);
    if(wrapper != nullptr)
    {
      wrapper->Translate(translator, memberAccessNode, context);
      return true;
    }
    else if(!mTranslator->IsInOwningLibrary(field->GetOwningLibrary()))
    {
      String msg = String::Format("Member Field '%s' is not able to be translated.", memberAccessNode->Name.c_str());
      translator->SendTranslationError(memberAccessNode->Location, msg);
    }
  }
  if(memberAccessNode->AccessedProperty != nullptr)
  {
    Zilch::Property* prop = memberAccessNode->AccessedProperty;
    MemberAccessResolverWrapper* wrapper = mMemberAccessPropertyResolvers.FindPointer(prop);
    if(wrapper != nullptr)
    {
      wrapper->Translate(translator, memberAccessNode, context);
      return true;
    }
    wrapper = mBackupMemberAccessResolvers.FindPointer(memberAccessNode->LeftOperand->ResultType);
    if(wrapper != nullptr)
    {
      wrapper->Translate(translator, memberAccessNode, context);
      return true;
    }

    // If this is actually a get/set property then mark a dependency on the type that actually implements the extension function
    Zilch::GetterSetter* getSet = memberAccessNode->AccessedGetterSetter;
    if(getSet != nullptr)
    {
      if(memberAccessNode->IoUsage == Zilch::IoMode::ReadRValue)
      {
        ShaderFunction* extensionFunction = mTranslator->mCurrentLibrary->FindExtension(getSet->Get);
        if(extensionFunction != nullptr)
        {
          context->mCurrentType->AddDependency(extensionFunction->mOwner);
          return false;
        }
      }
    }
    else if(!mTranslator->IsInOwningLibrary(prop->GetOwningLibrary()))
    {
      String resultType = memberAccessNode->LeftOperand->ResultType->ToString();
      String propertyResultType = memberAccessNode->ResultType->ToString();
      String msg = String::Format("Member property '%s:%s' on type '%s' is not able to be translated.", memberAccessNode->Name.c_str(), propertyResultType.c_str(), resultType.c_str());
      translator->SendTranslationError(memberAccessNode->Location, msg);
    }
  }
  if(memberAccessNode->AccessedFunction != nullptr)
  {
    Zilch::Function* fn = memberAccessNode->AccessedFunction;
    MemberAccessResolverWrapper* wrapper = mMemberAccessFunctionResolvers.FindPointer(fn);
    if(wrapper != nullptr)
    {
      wrapper->Translate(translator, memberAccessNode, context);
      return true;
    }

    // See if this function has been implemented by another class
    ShaderFunction* implementsFunction = mTranslator->mCurrentLibrary->FindImplements(fn);
    if(implementsFunction != nullptr)
    {
      // Mark the implementing class as a dependency of this one
      context->mCurrentType->AddDependency(implementsFunction->mOwner);
      // Instead of printing the function's name from zilch, replace it with the name of the implementing function
      context->GetBuilder() << implementsFunction->mShaderName;
      // Return that we replaced this function name
      return true;
    }
    // For now just mark that extension functions are not errors (let translation happen as normal)
    // @JoshD: Cleanup and merge somehow with the function call portion!
    ShaderFunction* extensionFunction = mTranslator->mCurrentLibrary->FindExtension(fn);
    if(extensionFunction != nullptr)
    {
      return false;
    }
    else if(!mTranslator->IsInOwningLibrary(fn->GetOwningLibrary()))
    {
      String fnName = GetFunctionDescription(memberAccessNode->Name, memberAccessNode->AccessedFunction);
      String msg = String::Format("Member function '%s' is not able to be translated.", fnName.c_str());
      translator->SendTranslationError(memberAccessNode->Location, msg);
    }
  }

  // Otherwise we didn't have a special translation for this member, let the normal translation happen.
  return false;
}

void LibraryTranslator::RegisterBinaryOpResolver(Zilch::Type* typeA, Zilch::Grammar::Enum op, Zilch::Type* typeB, const BinaryOpResolverWrapper wrapper)
{
  String key = BuildString(typeA->ToString(), Zilch::Grammar::GetKeywordOrSymbol(op), typeB->ToString());
  RegisterBinaryOpResolver(key, wrapper);
}

void LibraryTranslator::RegisterBinaryOpResolver(Zilch::Type* typeA, Zilch::Grammar::Enum op, Zilch::Type* typeB, StringParam replacementFnCallStr)
{
  RegisterBinaryOpResolver(typeA, op, typeB, BinaryOpResolverWrapper(replacementFnCallStr));
}

void LibraryTranslator::RegisterBinaryOpResolver(Zilch::Type* typeA, Zilch::Grammar::Enum op, Zilch::Type* typeB, BinaryOpResolver resolverFn)
{
  RegisterBinaryOpResolver(typeA, op, typeB, BinaryOpResolverWrapper(resolverFn));
}

void LibraryTranslator::RegisterBinaryOpResolver(StringParam key, const BinaryOpResolverWrapper wrapper)
{
  mBinaryOpResolvers[key] = wrapper;
}

void LibraryTranslator::RegisterBinaryOpResolver(StringParam key, StringParam replacementFnCallStr)
{
  RegisterBinaryOpResolver(key, BinaryOpResolverWrapper(replacementFnCallStr));
}

void LibraryTranslator::RegisterBinaryOpResolver(StringParam key, BinaryOpResolver resolverFn)
{
  RegisterBinaryOpResolver(key, BinaryOpResolverWrapper(resolverFn));
}

bool LibraryTranslator::ResolveBinaryOp(ZilchShaderTranslator* translator, Zilch::BinaryOperatorNode* node, ZilchShaderTranslatorContext* context)
{
  // Build the key for this operator which is (type)(operator)(type) and use that to lookup a replacement
  String operatorSignature = BuildString(node->LeftOperand->ResultType->ToString(), node->Operator->Token, node->RightOperand->ResultType->ToString());

  BinaryOpResolverWrapper* resolverWrapper = mBinaryOpResolvers.FindPointer(operatorSignature);
  if(resolverWrapper != nullptr)
    resolverWrapper->Translate(translator, node, context);

  return resolverWrapper != nullptr;
}

void LibraryTranslator::RegisterTemplateTypeResolver(TemplateTypeResolver resolver)
{
  mTemplateTypeResolver = resolver;
}

void LibraryTranslator::RegisterInitializerNodeResolver(Zilch::Type* type, InitializerResolver resolver)
{
  mIntializerResolvers[type] = resolver;
}

bool LibraryTranslator::ResolveInitializerNode(ZilchShaderTranslator* translator, Zilch::Type* type, Zilch::ExpressionInitializerNode* initializerNode, ZilchShaderTranslatorContext* context)
{
  InitializerResolver resolver = mIntializerResolvers.FindValue(type, nullptr);
  if(resolver == nullptr)
    return false;

  (*resolver)(translator, type, initializerNode, context);
  return true;
}

String LibraryTranslator::GetFunctionDescription(StringParam fnName, Zilch::Function* fn)
{
  StringBuilder builder;
  builder.Append(fnName);
  fn->FunctionType->BuildSignatureString(builder, false);
  return builder.ToString();
}

bool LibraryTranslator::IsUserLibrary(Zilch::Library* library)
{
  //@JoshD: Fix!
  Array<ZilchShaderLibrary*> libraryStack;
  libraryStack.PushBack(mTranslator->mCurrentLibrary);

  while(!libraryStack.Empty())
  {
    ZilchShaderLibrary* testLibrary = libraryStack.Back();
    libraryStack.PopBack();

    Zilch::Library* testZilchLibrary = testLibrary->mZilchLibrary;
    // If the library is the same and it's user code
    // (as in we compiled zilch scripts, not C++) then this is a user library
    if(testZilchLibrary == library && testLibrary->mIsUserCode)
      return true;

    // Add all dependencies of this library
    ZilchShaderModule* dependencies = testLibrary->mDependencies;
    if(dependencies != nullptr)
    {
      for(size_t i = 0; i < dependencies->Size(); ++i)
      libraryStack.PushBack((*dependencies)[i]);
    }
  }
  return false;
}

}//namespace Zero
