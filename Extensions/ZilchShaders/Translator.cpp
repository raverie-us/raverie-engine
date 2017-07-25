///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------ZilchShaderTranslator
ZilchShaderTranslator::ZilchShaderTranslator()
{
  mErrors = nullptr;
  mCurrentProject = nullptr;
  mCurrentLibrary = nullptr;
  mErrorTriggered = false;
  mInitialized = false;
  mLibraryTranslator.mTranslator = this;
}

ZilchShaderTranslator::~ZilchShaderTranslator()
{

}

void ZilchShaderTranslator::SetSettings(ZilchShaderSettingsRef& settings)
{
  mSettings = settings;
}

void ZilchShaderTranslator::Setup()
{
  SetupGeneric();
  SetupShaderLanguage();
}

void ZilchShaderTranslator::SetupGeneric()
{
  if(!mInitialized)
  {
    mInitialized = true;
    SetupWalkerRegistration();
    SetupTokenPrecedence();
  }
}

void ZilchShaderTranslator::SetupWalkerRegistration()
{
  mPreWalker.Register(&ZilchShaderTranslator::PreWalkClassDeclaration);
  mPreWalker.Register(&ZilchShaderTranslator::PreWalkClassFunction);
  mPreWalker.Register(&ZilchShaderTranslator::PreWalkClassVariables);

  mWalker.RegisterNonLeafBase(&ZilchShaderTranslator::FormatCommentsAndLines);
  mWalker.Register(&ZilchShaderTranslator::GenerateClassDeclaration);
  mWalker.Register(&ZilchShaderTranslator::WalkClassVariables);
  mWalker.Register(&ZilchShaderTranslator::WalkClassConstructor);
  mWalker.Register(&ZilchShaderTranslator::WalkClassFunction);
  mWalker.Register(&ZilchShaderTranslator::WalkFunctionCallNode);
  mWalker.Register(&ZilchShaderTranslator::WalkLocalVariable);
  mWalker.Register(&ZilchShaderTranslator::WalkStaticTypeOrCreationCallNode);
  mWalker.Register(&ZilchShaderTranslator::WalkExpressionInitializerNode);
  mWalker.Register(&ZilchShaderTranslator::WalkUnaryOperationNode);
  mWalker.Register(&ZilchShaderTranslator::WalkBinaryOperationNode);
  mWalker.Register(&ZilchShaderTranslator::WalkCastNode);
  mWalker.Register(&ZilchShaderTranslator::WalkValueNode);
  mWalker.Register(&ZilchShaderTranslator::WalkLocalRef);
  mWalker.Register(&ZilchShaderTranslator::WalkMemberAccessNode);
  mWalker.Register(&ZilchShaderTranslator::WalkIfRootNode);
  mWalker.Register(&ZilchShaderTranslator::WalkIfNode);
  mWalker.Register(&ZilchShaderTranslator::WalkContinueNode);
  mWalker.Register(&ZilchShaderTranslator::WalkBreakNode);
  mWalker.Register(&ZilchShaderTranslator::WalkReturnNode);
  mWalker.Register(&ZilchShaderTranslator::WalkWhileNode);
  mWalker.Register(&ZilchShaderTranslator::WalkDoWhileNode);
  mWalker.Register(&ZilchShaderTranslator::WalkForNode);
  mWalker.Register(&ZilchShaderTranslator::WalkForEachNode);
  mWalker.Register(&ZilchShaderTranslator::WalkMultiExpressionNode);
  mWalker.RegisterNonLeafBase(&ZilchShaderTranslator::FormatStatement);

  mWalker.RegisterDerived<Zilch::TypeIdNode>(&ZilchShaderTranslator::WalkUnknownNode);
}

void ZilchShaderTranslator::SetupTokenPrecedence()
{
  //lower number is higher precedence (precedence same as C)
  mTokenPrecedence[Zilch::Grammar::Increment] = OperatorInfo(2, OperatorAssociativityType::RightToLeft);
  mTokenPrecedence[Zilch::Grammar::Decrement] = OperatorInfo(2, OperatorAssociativityType::RightToLeft);
  mTokenPrecedence[Zilch::Grammar::LogicalNot] = OperatorInfo(2, OperatorAssociativityType::RightToLeft);
  mTokenPrecedence[Zilch::Grammar::BitwiseNot] = OperatorInfo(2, OperatorAssociativityType::RightToLeft);
  mTokenPrecedence[Zilch::Grammar::As] = OperatorInfo(2, OperatorAssociativityType::RightToLeft);
  //can't set the precedence of these unary operators since they use the same token as their binary counterparts
  //mTokenPriorities[Zilch::Grammar::Plus] = 2;
  //mTokenPriorities[Zilch::Grammar::Minus] = 2;
  mTokenPrecedence[Zilch::Grammar::Multiply] = OperatorInfo(3, OperatorAssociativityType::LeftToRight);
  mTokenPrecedence[Zilch::Grammar::Divide] = OperatorInfo(3, OperatorAssociativityType::LeftToRight);
  mTokenPrecedence[Zilch::Grammar::Modulo] = OperatorInfo(3, OperatorAssociativityType::LeftToRight);
  //mTokenPrecedence[Zilch::Grammar::Exponent] = 3;
  mTokenPrecedence[Zilch::Grammar::Add] = OperatorInfo(4, OperatorAssociativityType::LeftToRight);
  mTokenPrecedence[Zilch::Grammar::Subtract] = OperatorInfo(4, OperatorAssociativityType::LeftToRight);
  mTokenPrecedence[Zilch::Grammar::BitshiftLeft] = OperatorInfo(5, OperatorAssociativityType::LeftToRight);
  mTokenPrecedence[Zilch::Grammar::BitshiftRight] = OperatorInfo(5, OperatorAssociativityType::LeftToRight);
  mTokenPrecedence[Zilch::Grammar::LessThan] = OperatorInfo(6, OperatorAssociativityType::LeftToRight);
  mTokenPrecedence[Zilch::Grammar::LessThanOrEqualTo] = OperatorInfo(6, OperatorAssociativityType::LeftToRight);
  mTokenPrecedence[Zilch::Grammar::GreaterThan] = OperatorInfo(6, OperatorAssociativityType::LeftToRight);
  mTokenPrecedence[Zilch::Grammar::GreaterThanOrEqualTo] = OperatorInfo(6, OperatorAssociativityType::LeftToRight);
  mTokenPrecedence[Zilch::Grammar::Equality] = OperatorInfo(7, OperatorAssociativityType::LeftToRight);
  mTokenPrecedence[Zilch::Grammar::Inequality] = OperatorInfo(7, OperatorAssociativityType::LeftToRight);
  mTokenPrecedence[Zilch::Grammar::BitwiseAnd] = OperatorInfo(8, OperatorAssociativityType::LeftToRight);
  mTokenPrecedence[Zilch::Grammar::BitwiseXor] = OperatorInfo(9, OperatorAssociativityType::LeftToRight);
  mTokenPrecedence[Zilch::Grammar::BitwiseOr] = OperatorInfo(10, OperatorAssociativityType::LeftToRight);
  mTokenPrecedence[Zilch::Grammar::LogicalAnd] = OperatorInfo(11, OperatorAssociativityType::LeftToRight);
  mTokenPrecedence[Zilch::Grammar::LogicalOr] = OperatorInfo(12, OperatorAssociativityType::LeftToRight);
  mTokenPrecedence[Zilch::Grammar::Assignment] = OperatorInfo(14, OperatorAssociativityType::RightToLeft);
  mTokenPrecedence[Zilch::Grammar::AssignmentAdd] = OperatorInfo(14, OperatorAssociativityType::RightToLeft);
  mTokenPrecedence[Zilch::Grammar::AssignmentSubtract] = OperatorInfo(14, OperatorAssociativityType::RightToLeft);
  mTokenPrecedence[Zilch::Grammar::AssignmentMultiply] = OperatorInfo(14, OperatorAssociativityType::RightToLeft);
  mTokenPrecedence[Zilch::Grammar::AssignmentDivide] = OperatorInfo(14, OperatorAssociativityType::RightToLeft);
  mTokenPrecedence[Zilch::Grammar::AssignmentModulo] = OperatorInfo(14, OperatorAssociativityType::RightToLeft);
  //mTokenPrecedence[Zilch::Grammar::AssignmentExponent] = 14;
  mTokenPrecedence[Zilch::Grammar::AssignmentLeftShift] = OperatorInfo(14, OperatorAssociativityType::RightToLeft);
  mTokenPrecedence[Zilch::Grammar::AssignmentRightShift] = OperatorInfo(14, OperatorAssociativityType::RightToLeft);
  mTokenPrecedence[Zilch::Grammar::AssignmentBitwiseAnd] = OperatorInfo(14, OperatorAssociativityType::RightToLeft);
  mTokenPrecedence[Zilch::Grammar::AssignmentBitwiseXor] = OperatorInfo(14, OperatorAssociativityType::RightToLeft);
  mTokenPrecedence[Zilch::Grammar::AssignmentBitwiseOr] = OperatorInfo(14, OperatorAssociativityType::RightToLeft);
}

void ZilchShaderTranslator::ParseNativeLibrary(ZilchShaderLibrary* shaderLibrary)
{
  mLibraryTranslator.ParseNativeLibrary(this, shaderLibrary);
}

bool ZilchShaderTranslator::Translate(Zilch::SyntaxTree& syntaxTree, ZilchShaderProject* project, ZilchShaderLibraryRef& libraryRef)
{
  mErrorTriggered = false;
  mCurrentProject = project;
  mCurrentLibrary = libraryRef;
  mErrors = mCurrentProject;

  SetupShaderLanguage();

  // Walk the tree to generate translations of all types in this library
  ZilchShaderTranslatorContext context;
  // Run the pre-walk code (to avoid order issues)
  mPreWalker.Walk(this, syntaxTree.Root, &context);
  // This must come after pre-walking classes so that we have all required type names already populated
  RegisterLibraryBoundTemplateTypes();
  // Now run actual translation
  mWalker.Walk(this, syntaxTree.Root, &context);

  return !mErrorTriggered;
}

void ZilchShaderTranslator::BuildFinalShader(ShaderType* mainType, ShaderTypeTranslation& result, bool generatedCodeRangeMappings, bool walkDependencies)
{
  // This should normally only happen when someone tries to request shader that didn't exist (ie. geometry)
  if(mainType == nullptr)
    return;

  HashSet<ShaderType*> visitedType;
  Array<ShaderType*> dependencies;
  // Gather all dependencies (or just append the type if debugging)
  if(walkDependencies)
    BuildDependencies(mainType, visitedType, dependencies);
  else
    dependencies.PushBack(mainType);


  ShaderCodeBuilder finalBuilder;
  
  // Emit the version string needed to compile
  finalBuilder << GetVersionString() << "\n";

  // If the user wants range mappings then set the node to fill out
  CodeRangeMapping* rootRange = nullptr;
  if(generatedCodeRangeMappings)
    rootRange = &result.mRoot;

  // First output all shader inputs and outputs. These should be at the
  // top as any fragment could try to use forced static built-ins.
  finalBuilder << "//----- Shader Inputs/Outputs -----" << finalBuilder.EmitLineReturn();
  finalBuilder << mainType->mInputAndOutputVariableDeclarations;
  finalBuilder << finalBuilder.EmitLineReturn();

  // Now write out all the fragment pieces in this order.
  BuildStructsTranslation(finalBuilder, dependencies, rootRange);
  BuildForwardDeclarationTranslation(finalBuilder, dependencies, mainType, rootRange);
  BuildGlobalVarsTranslation(finalBuilder, dependencies, rootRange);
  BuildFunctionCodeTranslation(finalBuilder, dependencies, mainType, rootRange);

  result.mTranslation = finalBuilder.ToString();
  result.mRoot.mDestPositionEnd = finalBuilder.GetSize();
}

void ZilchShaderTranslator::BuildDependencies(ShaderType* currentType, HashSet<ShaderType*>& visitedTypes, Array<ShaderType*>& dependencies)
{
  // Don't double visit a type
  if(visitedTypes.Contains(currentType))
    return;

  visitedTypes.Insert(currentType);
  
  // Iterate over all dependencies of this class
  for(size_t i = 0; i < currentType->mDependencyList.Size(); ++i)
  {
    ShaderType* dependencyShaderType = currentType->mDependencyList[i];
    if(dependencyShaderType == nullptr)
      continue;

    BuildDependencies(dependencyShaderType, visitedTypes, dependencies);
  }

  // push this type on last so we have the most top level dependency first
  dependencies.PushBack(currentType);
}

void ZilchShaderTranslator::BuildStructsTranslation(ShaderCodeBuilder& finalBuilder, Array<ShaderType*>& dependencies, CodeRangeMapping* rootRange)
{
  // Start a range of all of the structs being declared
  bool generateRanges = rootRange != nullptr;
  ScopedRangeMapping structsRange(finalBuilder, rootRange, nullptr, generateRanges, CodeRangeDebugString("Structs"));

  // Write out all structs
  finalBuilder << "//----- Struct Definitions -----" << finalBuilder.EmitLineReturn();
  for(size_t i = 0; i < dependencies.Size(); ++i)
  {
    ShaderType* type = dependencies[i];
    // Check if this type hides the struct declaration
    if(type->GetHideStruct())
      continue;

    // Map this entire class' struct
    ScopedRangeMapping typeStructRange(finalBuilder, &structsRange, nullptr, &type->mSourceLocation, generateRanges,
                                       CodeRangeDebugString("Class '%s' Struct Declaration", type->mZilchName.c_str()));

    finalBuilder << "struct " << type->mShaderName << finalBuilder.EmitLineReturn();
    finalBuilder.BeginScope();

    // Write out all variable declarations
    for(size_t i = 0; i < type->mFieldList.Size(); ++i)
    {
      ShaderField* field = type->mFieldList[i];
      // Map each variable declaration
      ScopedRangeMapping varDeclarationRange(finalBuilder, &typeStructRange, &field->mPreInitRange, nullptr, generateRanges,
                                             CodeRangeDebugString("'%s.%s' Struct Var Declaration", type->mZilchName.c_str(), field->mZilchName.c_str()));

      // Only add non-static fields into the struct definition
      if(field->IsStatic() == false)
      {
        finalBuilder.WriteIndent();
        finalBuilder << field->mShaderType << " " << field->mShaderName << ";" << finalBuilder.LineReturn;
      }
    }

    // Close the class scope
    finalBuilder.EndClassScope();
    finalBuilder.WriteLine();
  }
}

void ZilchShaderTranslator::BuildForwardDeclarationTranslationHelper(ShaderCodeBuilder& finalBuilder, ShaderType* shaderType, ShaderFunction* shaderFunction, ScopedRangeMapping& typeForwardDeclarationRange, bool generateRanges)
{
  // Build a range for the full function's declaration (independent of the signature)
  ScopedRangeMapping functionForwardDeclarationRange(finalBuilder, &typeForwardDeclarationRange, nullptr, &shaderFunction->mSourceLocation, generateRanges,
    CodeRangeDebugString("'%s.%s' Forward Declaration", shaderType->mZilchName.c_str(), shaderFunction->mZilchName.c_str()));

  // Write out the beginning of the function declaration
  finalBuilder << shaderFunction->mShaderReturnType << " " << shaderFunction->mShaderName;

  // Add a range for the signature (and all of its children) under the total function declaration
  ScopedRangeMapping signatureRange(finalBuilder, &functionForwardDeclarationRange, &shaderFunction->mSignatureRange, nullptr, generateRanges,
    CodeRangeDebugString("'%s.%s' signature", shaderType->mZilchName.c_str(), shaderFunction->mZilchName.c_str()));

  // Now write out the function signature
  finalBuilder << shaderFunction->mShaderSignature << ";" << finalBuilder.LineReturn;
}

void ZilchShaderTranslator::BuildForwardDeclarationTranslation(ShaderCodeBuilder& finalBuilder, Array<ShaderType*>& dependencies, ShaderType* mainType, CodeRangeMapping* rootRange)
{
  bool generateRanges = rootRange != nullptr;
  // Build a range to map all forward declarations
  ScopedRangeMapping forwardDeclarationRanges(finalBuilder, rootRange, nullptr, generateRanges, CodeRangeDebugString("Forward Declarations"));

  // Write out all forward declarations
  finalBuilder << "//----- Forward Declarations -----" << finalBuilder.EmitLineReturn();
  for(size_t i = 0; i < dependencies.Size(); ++i)
  {
    ShaderType* type = dependencies[i];
    // If this type hides functions then skip it. This is typically on native
    // types (such as Real3) that completely map to built-in types.
    if(type->GetHideFunctions())
      continue;

    // Map all forward declarations for this type
    ScopedRangeMapping typeForwardDeclarationRange(finalBuilder, &forwardDeclarationRanges, nullptr, &type->mSourceLocation, generateRanges,
                                                   CodeRangeDebugString("Class '%s' Forward Declarations", type->mZilchName.c_str()));

    // Forward declare functions (do this before functions/global variables in case any variable relies on that)
    for(size_t i = 0; i < type->mFunctionList.Size(); ++i)
    {
      ShaderFunction* function = type->mFunctionList[i];
      BuildForwardDeclarationTranslationHelper(finalBuilder, type, function, typeForwardDeclarationRange, generateRanges);
    }

    // If this is the main type then write out any extra shader functions. This is done inside the loop of 
    // all dependency types instead of afterwards to deal with the code range mapping.
    if(type == mainType)
    {
      for(size_t i = 0; i < type->mFinalShaderFunctions.Size(); ++i)
      {
        ShaderFunction* function = type->mFinalShaderFunctions[i];
        BuildForwardDeclarationTranslationHelper(finalBuilder, type, function, typeForwardDeclarationRange, generateRanges);
      }
    }

    finalBuilder.WriteLine();
  }
}

void ZilchShaderTranslator::BuildGlobalVarsTranslationHelper(ShaderCodeBuilder& finalBuilder, ShaderField* field, ScopedRangeMapping& typeGlobalVarsRange, bool generateRanges)
{
  ShaderType* owningType = field->mOwner;
  // Map all global variable declarations for this type
  ScopedRangeMapping varRange(finalBuilder, &typeGlobalVarsRange, nullptr, &field->mSourceLocation, generateRanges,
    CodeRangeDebugString("'%s.%s' Global Var Declaration", owningType->mZilchName.c_str(), field->mZilchName.c_str()));

  // If the type (only check shader types) has a forced declaration qualifier (uniform, etc...) then add that
  ShaderType* fieldType = mCurrentLibrary->FindType(field->mZilchType);
  if(!fieldType->mForcedDeclarationQualifiers.Empty())
    finalBuilder << fieldType->mForcedDeclarationQualifiers << " ";

  // Write out the field declaration
  finalBuilder << field->mShaderType << " " << field->mShaderName;
  // If the field has a default value then write it out
  if(!field->mShaderDefaultValueString.Empty())
  {
    // Map the variable's default value code
    ScopedRangeMapping preInitRange(finalBuilder, &varRange, &field->mPreInitRange, nullptr, generateRanges,
      CodeRangeDebugString("'%s.%s' Global Var Pre-Init", owningType->mZilchName.c_str(), field->mZilchName.c_str()));

    finalBuilder << " = " << field->mShaderDefaultValueString;
  }
  finalBuilder << ";" << finalBuilder.LineReturn;
}

void ZilchShaderTranslator::BuildGlobalVarsTranslation(ShaderCodeBuilder& finalBuilder, Array<ShaderType*>& dependencies, CodeRangeMapping* rootRange)
{
  bool generate = rootRange != nullptr;
  // Build a range to map all global variables
  ScopedRangeMapping globalVarRanges(finalBuilder, rootRange, nullptr, generate, CodeRangeDebugString("Global Vars"));
  // Store the "ordered map" of all shared fields so that we can emit them only once
  HashSet<String> sharedFieldNames;
  Array<ShaderField*> sharedFieldList;

  // Write out all globals for all types
  finalBuilder << "//----- Global Variable Declarations -----" << finalBuilder.EmitLineReturn();
  for(size_t i = 0; i < dependencies.Size(); ++i)
  {
    ShaderType* type = dependencies[i];
    // If this type hides the struct declaration then it doesn't have any members and hence global
    // variables (typically statics) should also be hidden. Maybe separate into another flag later if this becomes an issue?
    if(type->GetHideStruct())
      continue;

    // Map all global variable declarations for this type
    ScopedRangeMapping typeGlobalVarsRange(finalBuilder, &globalVarRanges, nullptr, &type->mSourceLocation, generate,
                                           CodeRangeDebugString("Class '%s' Global Vars", type->mZilchName.c_str()));

    // To properly deal with basic newline formatting keep track of if this type ever
    // wrote out a static variable so we don't add random newlines
    bool containsStatic = false;

    // Write out all static variable declarations
    for(size_t i = 0; i < type->mFieldList.Size(); ++i)
    {
      ShaderField* field = type->mFieldList[i];
      // If this field is not static then don't write it out
      if(!field->IsStatic())
        continue;

      // If this field is actually a built-in (meaning this value will actually come
      // from the built-in, not that it just shares the same name) then we should not
      // write out the variable declaration because it would be written out by every
      // fragment that uses it. Instead, writing this out is the responsibility of the
      // main shader type in it's CopyInputOutputVariableDeclarations.
      if(IsBuiltInField(field))
        continue;

      // If this is a shared field then it should only be emitted once
      if(field->IsShared())
      {
        if(!sharedFieldNames.Contains(field->mShaderName))
        {
          sharedFieldNames.Insert(field->mShaderName);
          sharedFieldList.PushBack(field);
        }
        continue;
      }

      containsStatic = true;
      // Emit the variable declaration
      BuildGlobalVarsTranslationHelper(finalBuilder, field, globalVarRanges, generate);
    }

    // Only write a newline if we wrote out static variables (to avoid random extra newlines)
    if(containsStatic)
      finalBuilder.WriteLine();
  }

  // Now write out all shared fields (should only be samplers). Note: any line number remappings will
  // point to the first fragment in the composition with the field.
  for(size_t i = 0; i < sharedFieldList.Size(); ++i)
  {
    ShaderField* shaderField = sharedFieldList[i];
    BuildGlobalVarsTranslationHelper(finalBuilder, shaderField, globalVarRanges, generate);
  }
}

void ZilchShaderTranslator::BuildFunctionCodeTranslationHelper(ShaderCodeBuilder& finalBuilder, ShaderType* shaderType, ShaderFunction* shaderFunction, ScopedRangeMapping& typeCodeRange, bool generateRanges)
{
  // Map this entire function's declaration
  ScopedRangeMapping functionDeclarationRange(finalBuilder, &typeCodeRange, nullptr, &shaderFunction->mSourceLocation, generateRanges,
    CodeRangeDebugString("'%s.%s' Function Declaration", shaderType->mZilchName.c_str(), shaderFunction->mZilchName.c_str()));

  // Write all comments out above the function declaration
  for(size_t i = 0; i < shaderFunction->mComments.Size(); ++i)
    finalBuilder << "//" << shaderFunction->mComments[i] << finalBuilder.EmitLineReturn();

  // Write out the beginning of the function declaration
  finalBuilder << shaderFunction->mShaderReturnType << " " << shaderFunction->mShaderName;

  // Map and write out the function's signature
  {
    ScopedRangeMapping signatureRange(finalBuilder, &functionDeclarationRange, &shaderFunction->mSignatureRange, nullptr, generateRanges,
      CodeRangeDebugString("'%s.%s' Function Signature", shaderType->mZilchName.c_str(), shaderFunction->mZilchName.c_str()));

    // Now we can write out the signature
    finalBuilder << shaderFunction->mShaderSignature << finalBuilder.LineReturn;
  }

  // Map and write out the function's code
  {
    ScopedRangeMapping functionBodyRange(finalBuilder, &functionDeclarationRange, &shaderFunction->mBodyRange, nullptr, generateRanges,
      CodeRangeDebugString("'%s.%s' Function Body", shaderType->mZilchName.c_str(), shaderFunction->mZilchName.c_str()));

    // Now write out the function body code
    finalBuilder.Write(shaderFunction->mShaderBodyCode);
  }
  finalBuilder.WriteLine();
}

void ZilchShaderTranslator::BuildFunctionCodeTranslation(ShaderCodeBuilder& finalBuilder, Array<ShaderType*>& dependencies, ShaderType* mainType, CodeRangeMapping* rootRange)
{
  bool generate = rootRange != nullptr;
  // Build a range to map all code functions declarations
  ScopedRangeMapping codeRanges(finalBuilder, rootRange, nullptr, generate, CodeRangeDebugString("Global Vars"));


  // Write out all class functions
  for(size_t i = 0; i < dependencies.Size(); ++i)
  {
    ShaderType* type = dependencies[i];
    if(type->GetHideFunctions())
      continue;

    // Map the all of the code for the current type
    ScopedRangeMapping typeCodeRange(finalBuilder, &codeRanges, nullptr, &type->mSourceLocation, generate,
                                     CodeRangeDebugString("Class '%s' Code", type->mZilchName.c_str()));

    // Add a small comment for starting this class' functions
    finalBuilder << "//----- " << type->mZilchName << " Functions -----" << finalBuilder.EmitLineReturn();
    for(size_t i = 0; i < type->mFunctionList.Size(); ++i)
    {
      ShaderFunction* function = type->mFunctionList[i];
      BuildFunctionCodeTranslationHelper(finalBuilder, type, function, typeCodeRange, generate);
    }
    // If this is the main type then write out the final shader functions
    if(type == mainType)
    {
      // Add a small comment for starting this class' shader only functions
      if(!type->mFinalShaderFunctions.Empty())
        finalBuilder << "//----- " << type->mZilchName << " Final Shader Functions -----" << finalBuilder.EmitLineReturn();

      for(size_t i = 0; i < type->mFinalShaderFunctions.Size(); ++i)
      {
        ShaderFunction* function = type->mFinalShaderFunctions[i];
        BuildFunctionCodeTranslationHelper(finalBuilder, type, function, typeCodeRange, generate);
      }
    }
  }
}

void ZilchShaderTranslator::FormatCommentsAndLines(Zilch::SyntaxNode*& node, ZilchShaderTranslatorContext* context)
{
  context->Flags = Zilch::WalkerFlags::ChildrenNotHandled;

  //if we don't perform the generic handling then don't indent the statement
  if(context->mPerformAutoFormatting == false)
    return;

  ShaderCodeBuilder& builder = context->GetBuilder();

  // If the node is a statement then we want to do formatting of newlines
  if(Zilch::StatementNode::IsNodeUsedAsStatement(node))
  {
    // This is basically a copy-paste from the zilch formatter, not entirely sure how it's working other than
    // keeping track of the last scope to offset (so dealing with the '{}' operators) and then subtracting of comment lines.
    bool scopeFound = false;

    for(int i = (int)(context->FormatScopes.Size() - 1); i >= 0; --i)
    {
      ScopeLastNode& scope = context->FormatScopes[i];
      if(scope.AssociatedScope == node->Parent)
      {
        scopeFound = true;
        break;
      }
    }

    ScopeLastNode* foundScope = nullptr;
    if(scopeFound)
    {
      while(context->FormatScopes.Back().AssociatedScope != node->Parent)
      {
        context->FormatScopes.PopBack();
      }
      foundScope = &context->FormatScopes.Back();
    }
    else
    {
      foundScope = &context->FormatScopes.PushBack();
      foundScope->AssociatedScope = node->Parent;
    }

    int lineDifference = 0;
    if(foundScope->LastNode != nullptr)
      lineDifference = (int)(node->Location.StartLine - foundScope->LastNode->Location.EndLine);

    lineDifference -= (int)node->Comments.Size();

    // Since we will always emit one line, we skip one
    for(int i = 1; i < lineDifference; ++i)
      builder << builder.EmitIndent() << builder.EmitLineReturn();

    foundScope->LastNode = node;
  }
  

  // Add all comments for this node
  for(uint i = 0; i < node->Comments.Size(); ++i)
  {
    builder.WriteIndentation();
    builder.Write("//");
    builder.WriteLine(node->Comments[i]);
  }

  // If this node is a statement (and not the root if not) then add indentation for the line
  if(Zilch::StatementNode::IsNodeUsedAsStatement(node) &&
     Zilch::Type::DynamicCast<Zilch::IfRootNode*>(node) == nullptr)
     context->GetBuilder().WriteIndentation();
}

void ZilchShaderTranslator::FormatStatement(Zilch::StatementNode*& node, ZilchShaderTranslatorContext* context)
{
  context->Flags = Zilch::WalkerFlags::ChildrenNotHandled;

  //if we don't perform the generic handling then add ';'
  if(context->mPerformAutoFormatting == false)
    return;

  // This will always run, even if other handlers will handle it

  // If this node is not being used as a direct statement
  // For example, an expression is a statement, but isn't always used as a statement
  if(Zilch::StatementNode::IsNodeUsedAsStatement(node) == false)
    return;

  // As long as the statement isn't a scoped based node (if, for, while, etc)
  // then we know it requires delimiting
  if(Zilch::Type::DynamicCast<Zilch::ScopeNode*>(node) == nullptr &&
     Zilch::Type::DynamicCast<Zilch::IfRootNode*>(node) == nullptr)
  {
    context->GetBuilder().WriteLine(";");
  }
}

bool ZilchShaderTranslator::ValidateParamType(Zilch::AttributeParameter& attributeParam, Zilch::ConstantType::Enum expectedType, Zilch::CodeLocation& location)
{
  // Make sure the parameter type is correct
  if(attributeParam.Type != expectedType)
  {
    String msg = String::Format("Parameter '%s' must be of type %s", attributeParam.Name.c_str(), Zilch::ConstantType::Names[expectedType]);
    SendTranslationError(location, msg);
    return false;
  }
  return true;
}

bool ZilchShaderTranslator::ValidateParamType(Zilch::ValueNode* valueNode, StringParam paramName, Zilch::Grammar::Enum expectedType, StringParam expectedTypeStr, Zilch::CodeLocation& location)
{
  // Make sure the parameter type is correct
  if(valueNode == nullptr || valueNode->Value.TokenId != expectedType)
  {
    String msg = String::Format("Parameter '%s' must be of type %s", paramName.c_str(), expectedTypeStr.c_str());
    SendTranslationError(location, msg);
    return false;
  }
  return true;
}

void ZilchShaderTranslator::ParseIntrinsicAttribute(Zilch::ClassNode*& node, ShaderType* currentType)
{
  NameSettings& nameSettings = mSettings->mNameSettings;

  Array<String> attributesToAdd;
  // Find the first Intrinsic attribute that we care about
  ShaderAttributeList::NamedRange range = currentType->mAttributes.FindAttributes(nameSettings.mIntrinsicAttribute);
  while(!range.Empty())
  {
    ShaderAttribute* shaderAttribute = range.Front();
    // Find what language this attribute is for, if it's not one we care about then skip it (for now mark as an error)
    Zilch::AttributeParameter* languageParam = shaderAttribute->FindFirstParameter(nameSettings.mLanguageParamName);
    if(languageParam->StringValue != "glsl")
    {
      String msg = String::Format("Language '%s' is unsupported. Currently only 'glsl' is supported", languageParam->StringValue.c_str());
      Zilch::CodeLocation* location = currentType->mAttributes.GetLocation(shaderAttribute, languageParam);
      SendTranslationError(*location, msg);
      break;
    }

    // Parse all of the parameters we care about now that we know this
    // attribute is for the language we are currently translating to
    for(size_t paramIndex = 0; paramIndex < shaderAttribute->mParameters.Size(); ++paramIndex)
    {
      Zilch::AttributeParameter& attributeParam = shaderAttribute->mParameters[paramIndex];
      if(attributeParam.Name == nameSettings.mTypeNameParamName)
        currentType->mShaderName = attributeParam.StringValue;
      else if(attributeParam.Name == nameSettings.mStorageQualifierParamName)
        currentType->mForcedDeclarationQualifiers = BuildString(currentType->mForcedDeclarationQualifiers, attributeParam.StringValue);
      else if(attributeParam.Name == nameSettings.mRefTypeParamName)
        currentType->mReferenceTypeQualifier = attributeParam.StringValue;
      else if(attributeParam.Name == nameSettings.mPropertyTypeParamName)
        currentType->mPropertyType = attributeParam.StringValue;
      else if(attributeParam.Name == nameSettings.mForcedStaticParamName && attributeParam.BooleanValue == true)
        attributesToAdd.PushBack(nameSettings.mForcedStaticAttribute);
      else if(attributeParam.Name == nameSettings.mNonCopyableParamName && attributeParam.BooleanValue == true)
      {
        currentType->SetNonCopyable(attributeParam.BooleanValue);
        attributesToAdd.PushBack(nameSettings.mNonCopyableAttribute);
      }
    }
    break;
  }

  // Add any attributes to this type that were implied by the intrinsic attribute (such as forced static)
  for(size_t i = 0; i < attributesToAdd.Size(); ++i)
    currentType->mAttributes.AddAttribute(attributesToAdd[i], nullptr);
}

void ZilchShaderTranslator::ParseSharedAttribute(Zilch::MemberVariableNode*& node, ShaderAttribute* attribute, ShaderField* currentField)
{
  NameSettings& nameSettings = mSettings->mNameSettings;

  currentField->mIsShared = true;
  String varName = currentField->mZilchName;

  // If there is a parameter to override the shared name then use that name
  if(attribute->mParameters.Size() == 1)
    varName = attribute->mParameters[0].StringValue;

  // Mangle the variable name to include the type so as to avoid naming conflicts
  currentField->mShaderName = BuildString(varName, "_", currentField->mZilchType);
}

void ZilchShaderTranslator::PreWalkClassDeclaration(Zilch::ClassNode*& node, ZilchShaderTranslatorContext* context)
{
  NameSettings& nameSettings = mSettings->mNameSettings;

  ShaderType* currentType = FindShaderType(node->Type, node);
  context->mCurrentType = currentType;
  // Translate the class name (needed in advance often times for implements/extensions)
  currentType->mShaderName = FixClassName(node->Type);
  // By default set a type's reference qualifier to be the shared reference keyword
  currentType->mReferenceTypeQualifier = nameSettings.mReferenceKeyword;

  // Check for and parse the intrinsic attribute
  ParseIntrinsicAttribute(node, currentType);

  context->Walker->Walk(this, node->Functions, context);
  context->Walker->Walk(this, node->Variables, context);

  // If this is a geometry fragment then make sure that we have valid input/output streams and data types.
  // If not then throw an error and replace the stored type with the unknown type to help prevent crashes.
  if(currentType->mFragmentType == FragmentType::Geometry)
  {
    // If we never found the 'Main' function with the correct parameters
    // then we won't have the input or output type so notify the user.
    if(currentType->mInputType == nullptr && currentType->mOutputType == nullptr)
    {
      SendTranslationError(node->Location, "Geometry shaders must have a 'Main' function of signature (inputType, outputType)");
    }

    // Replace the input stream and input data types with dummies if needed
    if(currentType->mInputType == nullptr)
      currentType->mInputType = FindAndReportUnknownType(node->Type, node);
    if(currentType->mInputType->mInputType == nullptr)
      currentType->mInputType->mInputType = FindAndReportUnknownType(node->Type, node);

    // Replace the output stream and output data types with dummies if needed
    if(currentType->mOutputType == nullptr)
      currentType->mOutputType = FindAndReportUnknownType(node->Type, node);
    if(currentType->mOutputType->mOutputType == nullptr)
      currentType->mOutputType->mOutputType = FindAndReportUnknownType(node->Type, node);
  }

  // Verify that if this is a fragment type that it has the main function
  if(currentType->mFragmentType != FragmentType::None)
  {
    ShaderType::FunctionList* functions = currentType->mFunctionNameMultiMap.FindPointer("Main");
    // @JoshD: This should really verify that the correct main function (Main() : Void) exists, 
    // but I don't have this information here right now. Refactor later!
    if(functions == nullptr)
    {
      String msg = String::Format("Type '%s' must have a function of signature 'Main() : Void'.", currentType->mZilchName.c_str());
      SendTranslationError(node->Location, msg);
    }
  }
}

void ZilchShaderTranslator::PreWalkClassFunction(Zilch::FunctionNode*& node, ZilchShaderTranslatorContext* context)
{
  NameSettings& nameSettings = mSettings->mNameSettings;
  ShaderType* currentType = context->mCurrentType;

  ShaderFunction* function = currentType->FindFunction(node->DefinedFunction);
  // Translate the function name
  function->mShaderName = MangleName(function->mZilchName, currentType);
  // If this function says to not mangle then replace the shader name with the zilch name
  if(function->ContainsAttribute(nameSettings.mNoMangleAttributeName))
    function->mShaderName = function->mZilchName;

  // If this is an extension property then change the shader name to be mangled based upon the extension type's name
  ShaderAttribute* extensionAttribute = function->mAttributes.FindFirstAttribute(nameSettings.mExtensionAttribute);
  if(extensionAttribute != nullptr && extensionAttribute->mParameters.Size() == 1)
  {
    Zilch::AttributeParameter& param = extensionAttribute->mParameters[0];
    String extendsTypeStr = param.TypeValue->ToString();
    ShaderType* shaderSelfType = mCurrentLibrary->FindType(extendsTypeStr);
    if(shaderSelfType != nullptr)
      function->mShaderName = MangleName(function->mZilchName, shaderSelfType);
  }
  
  // If this is a geometry fragment's main function with the correct number of arguments then parse the input and output stream types
  if(currentType->mFragmentType == FragmentType::Geometry && node->Name.Token == "Main" && node->Parameters.Size() == 2)
  {
    // Find the shader type for param 0. If it's an input type then save it and its template type
    Zilch::BoundType* param0Type = (Zilch::BoundType*)node->Parameters[0]->ResultType;
    ShaderType* param0ShaderType = FindShaderType(param0Type, node->Parameters[0]);
    if(param0ShaderType->GetTypeData()->mType == ShaderVarType::GeometryInput)
    {
      currentType->mInputType = param0ShaderType;
      param0ShaderType->mInputType = FindShaderType(param0Type->TemplateArguments[0].TypeValue, node->Parameters[0]);
    }

    // Find the shader type for param 1. If it's an output type then save it and its template type
    Zilch::BoundType* param1Type = (Zilch::BoundType*)node->Parameters[1]->ResultType;
    ShaderType* param1ShaderType = FindShaderType(param1Type, node->Parameters[1]);
    if(param1ShaderType->GetTypeData()->mType == ShaderVarType::GeometryOutput)
    {
      currentType->mOutputType = param1ShaderType;
      param1ShaderType->mOutputType = FindShaderType(param1Type->TemplateArguments[0].TypeValue, node->Parameters[1]);
    }
  }
}

void ZilchShaderTranslator::PreWalkClassVariables(Zilch::MemberVariableNode*& node, ZilchShaderTranslatorContext* context)
{
  NameSettings& nameSettings = mSettings->mNameSettings;
  ShaderType* currentType = context->mCurrentType;

  // Check if this is actually a get/set
  Zilch::GetterSetter* getterSetter = Zilch::Type::DynamicCast<Zilch::GetterSetter*>(node->CreatedProperty);
  if(getterSetter)
  {
    if(node->Get)
      PreWalkClassFunction(node->Get, context);
    return;
  }

  ShaderField* field = currentType->FindField(node->Name.Token);
  field->mShaderName = field->mZilchName;

  // Find the shader type that this variable results in
  ShaderType* shaderResultType = FindShaderType(node->ResultType, node);
  field->mZilchType = shaderResultType->mZilchName;
  field->mShaderType = shaderResultType->mShaderName;
  field->mSourceLocation = node->Location;

  field->mIsStatic = false;
  // if this type is static or a type that is forced to be static (such as samplers)
  ShaderType* resultShaderType = FindShaderType(node->ResultType, node);
  bool isForcedStatic = resultShaderType->ContainsAttribute(NameSettings::mForcedStaticAttribute);
  if(field->ContainsAttribute(nameSettings.mStaticAttribute) || isForcedStatic)
  {
    field->mIsStatic = true;
    field->mShaderName = field->mZilchName;

    // We have to mangle the variable's name (so that it can't conflict with anything else in global scope,
    // however if this field comes from a built-in then we can't mangle the name,
    // instead we need to access the global uniform's name (just the zilch name).
    if(!IsBuiltInField(field))
    {
      field->mShaderName = MangleName(field->mZilchName, currentType);
      if(node->CreatedField == nullptr)
        return;

      // Check if this is a shared input
      ShaderAttribute* sharedInputAttribute = field->mAttributes.FindFirstAttribute(nameSettings.mSharedInputAttributeName);
      if(sharedInputAttribute != nullptr)
        ParseSharedAttribute(node, sharedInputAttribute, field);
    }
  }

  // Fragment types are currently illegal as member variables. Validate and throw an error if necessary.
  if(shaderResultType->mFragmentType != FragmentType::None)
  {
    String fragmentTypeName = FragmentType::Names[shaderResultType->mFragmentType];
    String message = String::Format("Variables with the attribute '[%s]' are not allowed as member variables.", fragmentTypeName.c_str());
    SendTranslationError(node->Location, message);
  }
}

void ZilchShaderTranslator::GenerateClassDeclaration(Zilch::ClassNode*& node, ZilchShaderTranslatorContext* context)
{
  NameSettings& nameSettings = mSettings->mNameSettings;

  ShaderType* currentType = FindShaderType(node->Type, node);
  ErrorIf(currentType == nullptr, "Somehow didn't find this type from the pre-walker?");

  context->mCurrentType = currentType;

  
  // If this is an intrinsic type then don't walk constructors or variables
  // since the built-in type doesn't contain those
  if(currentType->GetIntrinsic() == false)
  {
    // Walk all variables on the class
    context->Walker->Walk(this, node->Variables, context);
    // If this class doesn't have any members then it will be an error
    // (glsl structs must have at least 1 member) so generate a dummy variable for now.
    GenerateDummyMemberVariable(currentType);
    // now that we have all variables we can generate the preconstructor
    GeneratePreConstructor(node, context);

    // Now walk all constructors, if we didn't have a default constructor then auto-generate one.
    // We do this because constructors are actually functions that create the type, call the
    // preconstructor, and return it, as opposed to real constructors.
    context->Walker->Walk(this, node->Constructors, context);
    GenerateDefaultConstructor(node, context);
  }

  // Finally we can walk all functions
  context->Walker->Walk(this, node->Functions, context);
}

void ZilchShaderTranslator::GenerateDummyMemberVariable(ShaderType* shaderType)
{
  // Find out if this class is empty (any variables that are not static)
  bool isEmptyClass = true;
  for(size_t i = 0; i < shaderType->mFieldList.Size(); ++i)
  {
    ShaderField* field = shaderType->mFieldList[i];
    if(field->mIsStatic == false)
      isEmptyClass = false;
  }
  // If the class is empty then generate an integer with the value of 0
  if(isEmptyClass)
  {
    Zilch::Core& core = Zilch::Core::GetInstance();
    String dummyName = "Dummy";
    // Use find or create instead of find as this is a glsl requirement to make a dummy var
    ShaderField* field = shaderType->FindOrCreateField(dummyName);
    field->mShaderDefaultValueString = "0";
    field->mIsStatic = false;
    field->mShaderName = dummyName;
    // Find the integer type so we can set that as the dummy's type
    ShaderType* integerType = FindShaderType(core.IntegerType, nullptr);
    field->mZilchType = integerType->mZilchName;
    field->mShaderType = integerType->mShaderName;
  }
}

void ZilchShaderTranslator::GeneratePreConstructor(Zilch::ClassNode*& node, ZilchShaderTranslatorContext* context)
{
  NameSettings& nameSettings = mSettings->mNameSettings;
  Zilch::Core& core = Zilch::Core::GetInstance();

  Zilch::Function* preConstructorFn = node->PreConstructor;
  ShaderType* currentType = context->mCurrentType;
  ShaderFunction* function = currentType->FindFunction(preConstructorFn);
  function->mShaderName = MangleName(function->mZilchName, currentType);
  // Pre-constructors return void. Find the void type to know how to translate it
  ShaderType* voidShaderType = FindShaderType(core.VoidType, nullptr);
  function->mZilchReturnType = voidShaderType->mZilchName;
  function->mShaderReturnType = voidShaderType->mShaderName;
  
  // The pre-constructor doesn't really have any node to map to but its signature range will
  // try to map to something. To get around this just set the source location back to the class node's location
  function->mSignatureRange.Set(node->Location);
  // Also each function needs its total range which is just the node's location (or in this case the class')
  function->mSourceLocation = node->Location;

  // Declare the pre-constructor as taking the class in by reference
  StringBuilder signatureBuilder;
  signatureBuilder << "(" << nameSettings.mReferenceKeyword << " " << currentType->mShaderName << " " << nameSettings.mThisKeyword << ")";
  function->mShaderSignature = signatureBuilder.ToString();

  ScopedShaderCodeBuilder builder(context);
  // Map the body of the pre-constructor
  ScopedCodeRangeMapping preConstructorRange(context, &function->mBodyRange, preConstructorFn->Location, CodeRangeDebugString("PreConstructor"));

  builder.BeginScope();
  // Have the pre-constructor initialize all non-static variables to their default values
  for(size_t i = 0; i < currentType->mFieldList.Size(); ++i)
  {
    ShaderField* field = currentType->mFieldList[i];
    // If the field is static or it has no default value string then don't write out the pre-constructor setting of this argument
    if(field->IsStatic() || field->mShaderDefaultValueString.Empty())
      continue;

    builder.WriteIndent();

    ScopedCodeRangeMapping childRange(context, field->mPreInitRange);
    builder << nameSettings.mThisKeyword << "." << field->mShaderName << " = " << field->mShaderDefaultValueString << ";" << builder.LineReturn;
  }
  builder.EndScope();
  // Save off the function body
  function->mShaderBodyCode = builder.ToString();
}

void ZilchShaderTranslator::GenerateDefaultConstructor(Zilch::ClassNode*& node, ZilchShaderTranslatorContext* context)
{
  NameSettings& nameSettings = mSettings->mNameSettings;
  ShaderType* currentType = context->mCurrentType;
  // Find if we need to create a default constructor (determined in the type collector phase).
  // This happens only if no constructor is defined, hence there is an implicit default constructor.
  ShaderFunction* constructor = currentType->mFunctionOverloadMap.FindValue(ZilchShaderSettings::GetDefaultConstructorKey(), nullptr);
  if(constructor != nullptr)
  {
    // The default constructor is special and didn't actually exist in zilch, for this create a special function
    constructor->mShaderName = MangleName(constructor->mZilchName, currentType);
    constructor->mZilchReturnType = currentType->mZilchName;
    constructor->mShaderReturnType = currentType->mShaderName;
    constructor->mShaderSignature = "()";
    // Map the signature range to the class node (default constructors don't have any other logical mapping)
    constructor->mSignatureRange.Set(node->Location);
    // Also each function needs its total range which is just the node's location (or in this case the class')
    constructor->mSourceLocation = node->Location;

    // Create an instance of the class named 'self' and then call the pre-constructor
    ScopedShaderCodeBuilder subBuilder(context);
    ScopedCodeRangeMapping constructorRange(context, &constructor->mBodyRange, node->Location, CodeRangeDebugString("Constructor"));
    
    subBuilder.BeginScope();
    subBuilder << subBuilder.WriteScopedIndent() << constructor->mShaderReturnType<< " " << nameSettings.mThisKeyword << ";" << subBuilder.LineReturn;
    subBuilder << subBuilder.WriteScopedIndent() << MangleName(nameSettings.mPreConstructorName, currentType) << "(" << nameSettings.mThisKeyword << ");" << subBuilder.LineReturn;
    subBuilder << subBuilder.WriteScopedIndent() << "return " << nameSettings.mThisKeyword << ";" << subBuilder.LineReturn;
    subBuilder.EndScope();
    constructor->mShaderBodyCode = subBuilder.ToString();
  }
}

void ZilchShaderTranslator::WalkClassVariables(Zilch::MemberVariableNode*& node, ZilchShaderTranslatorContext* context)
{
  ShaderType* currentType = context->mCurrentType;

  // Check if this is actually a get/set
  Zilch::GetterSetter* getterSetter = Zilch::Type::DynamicCast<Zilch::GetterSetter*>(node->CreatedProperty);
  if(getterSetter)
  {
    if(node->Get)
      WalkClassFunction(node->Get, context);
    return;
  }

  ShaderField* field = currentType->FindField(node->Name.Token);

  // Map the pre-init of this variable
  ScopedCodeRangeMapping preInitRange(context, &field->mPreInitRange, node->Location,
                                      CodeRangeDebugString("%s PreInit", field->mZilchName.c_str()));

  // if this type is static or a type that is forced to be static (such as samplers)
  ShaderType* resultShaderType = FindShaderType(node->ResultType, node);
  bool isForcedStatic = resultShaderType->ContainsAttribute(NameSettings::mForcedStaticAttribute);
  // if the variable has an initial value then walk the sub-tree and save the result string
  if(node->InitialValue != nullptr)
  {
    // If this type says that it cannot be copied then that currently also means it can't have a default value assigned to it
    if(resultShaderType->ContainsAttribute(NameSettings::mNonCopyableAttribute))
    {
      String msg = String::Format("Type '%s' cannot be assigned to.", field->mZilchType.c_str());
      SendTranslationError(node->InitialValue->Location, msg);
    }

    ScopedShaderCodeBuilder initialValueBuilder(context);
    context->Walker->Walk(this, node->InitialValue, context);
    field->mShaderDefaultValueString = initialValueBuilder.ToString();
  }
  else
  {
    // @JoshD: This eventually needs to be updated to work with FixedArrays

    // Otherwise we don't have a default value, however we still need to emit zilch's default value.
    // We want to try to find the default constructor for the result type.
    Zilch::BoundType* boundType = Zilch::Type::GetBoundType(node->ResultType);
    if(boundType != nullptr && !isForcedStatic)
    {
      const Zilch::FunctionArray* constructors = boundType->GetOverloadedInheritedConstructors();
      if(constructors != nullptr && constructors->Empty() == false)
      {
        // If we managed to find the default constructor then resolve the default constructor's translation
        Zilch::Function* defaultConstructor = Zilch::BoundType::GetDefaultConstructor(constructors);

        ScopedShaderCodeBuilder initialValueBuilder(context);
        mLibraryTranslator.ResolveDefaultConstructor(this, boundType, defaultConstructor, context);
        field->mShaderDefaultValueString = initialValueBuilder.ToString();
      }
    }
  }
}

void ZilchShaderTranslator::WalkClassConstructor(Zilch::ConstructorNode*& node, ZilchShaderTranslatorContext* context)
{
  // For now, only allow default constructors (could allow this later as long as we force them to also
  // have a default constructor so that the compositor can create fragments)
  if(node->Parameters.Size() != 0)
  {
    SendTranslationError(node->Location, "Can only have default constructors");
    return;
  }

  Zilch::Core& core = Zilch::Core::GetInstance();
  NameSettings& nameSettings = mSettings->mNameSettings;
  ShaderType* currentType = context->mCurrentType;
  // In order to have a constructed class being the same as in zilch, constructors are changed into
  // functions that return a newly created class. This constructor will call the pre-constructor
  // function and then perform any other logic for the constructor.

  ShaderFunction* function = currentType->FindFunction(node->DefinedFunction);
  function->mShaderName = MangleName(function->mZilchName, currentType);
  function->mZilchReturnType = currentType->mZilchName;
  function->mShaderReturnType = currentType->mShaderName;

  // Also each function needs its total range which is just the node's location (or in this case the class')
  function->mSourceLocation = node->Location;

  // Build up the arguments of the constructor
  ScopedShaderCodeBuilder parametersBuilder(context);
  ScopedCodeRangeMapping signatureRange(context, &function->mSignatureRange, node->Location, CodeRangeDebugString("Constructor Signature"));
  parametersBuilder.Write("(");
  context->Walker->Walk(this, node->Parameters, context);
  parametersBuilder.Write(")");
  signatureRange.PopRange();
  // Save the signature of the constructor
  function->mShaderSignature = parametersBuilder.ToString();

  ScopedShaderCodeBuilder subBuilder(context);
  ScopedCodeRangeMapping constructorRange(context, &function->mBodyRange, node->Location, CodeRangeDebugString("Constructor"));

  subBuilder.BeginScope();
  // As the first 2 lines we need to create the class named self "ClassName self;" and then call the pre-constructor
  subBuilder << subBuilder.EmitIndent() << currentType->mShaderName << " " << nameSettings.mThisKeyword << ";" << subBuilder.LineReturn;
  subBuilder << subBuilder.EmitIndent() << MangleName(nameSettings.mPreConstructorName, currentType) << "(" << nameSettings.mThisKeyword << ");" << subBuilder.LineReturn;
  // Now we can iterate through all of the statements of the constructor
  WriteStatements(node->Statements, context);
  // Finally return the struct that we created
  subBuilder << subBuilder.EmitIndent() << "return " << nameSettings.mThisKeyword << ";" << subBuilder.LineReturn;
  subBuilder.EndScope();
  // Save the constructor's body
  function->mShaderBodyCode = subBuilder.mBuilder.ToString();
}

void ZilchShaderTranslator::WalkClassFunction(Zilch::FunctionNode*& node, ZilchShaderTranslatorContext* context)
{
  NameSettings& nameSettings = mSettings->mNameSettings;
  Zilch::Core& core = Zilch::Core::GetInstance();

  ShaderType* currentType = context->mCurrentType;

  ShaderFunction* function = currentType->FindFunction(node->DefinedFunction);
  ShaderType* shaderReturnType = FindShaderType(node->Type->Return, node->ReturnType);
  function->mZilchReturnType = shaderReturnType->mZilchName;
  function->mShaderReturnType = shaderReturnType->mShaderName;
  function->mComments = node->Comments;

  // Store the function's full location
  function->mSourceLocation = node->Location;
  // Unfortunately the function's location includes the statements. To get the range of just
  // the signature we need to start at the function name and go until the last argument or the return type (if it exists)
  int signatureStart = node->Name.Location.StartPosition;
  int signatureEnd = node->Name.Location.EndPosition;
  if(!node->Parameters.Empty())
    signatureEnd = node->Parameters.Back()->Location.EndPosition;
  if(node->ReturnType != nullptr)
    signatureEnd = node->ReturnType->Location.EndPosition;

  // Start a builder to make the signature
  ScopedShaderCodeBuilder signatureBuilder(context);
  ScopedCodeRangeMapping signatureRange(context, &function->mSignatureRange, node->Location, CodeRangeDebugString("Function Signature"));
  // Override the source location for the signature
  function->mSignatureRange.mSourcePositionStart = signatureStart;
  function->mSignatureRange.mSourcePositionEnd = signatureEnd;

  signatureBuilder.Write("(");
  // Write out the regular signature
  WriteFunctionSignature(node, context);
  
  String selfType = currentType->mShaderName;
  Zilch::Type* zilchSelfType = node->DefinedFunction->Owner;
  
  // Check for the extension attribute. If we have it then alter
  // the self type name (on statics) to be the class we are extending.
  ShaderAttribute* extensionAttribute = function->mAttributes.FindFirstAttribute(nameSettings.mExtensionAttribute);
  if(extensionAttribute != nullptr && extensionAttribute->mParameters.Size() > 0)
  {
    Zilch::AttributeParameter& param = extensionAttribute->mParameters[0];
    String extendsTypeStr = param.TypeValue->ToString();
    ShaderType* shaderSelfType = mCurrentLibrary->FindType(extendsTypeStr);

    // If the function is not static then resolve the type name of the self parameter (e.g. convert Real2 to vec2)
    if(shaderSelfType != nullptr && function->IsStatic() == false)
      selfType = shaderSelfType->mShaderName;
  }

  // Add dependencies on all input types
  for(size_t i = 0; i < node->Parameters.Size(); ++i)
  {
    Zilch::ParameterNode* parameter = node->Parameters[i];
    Zilch::Type* resultType = parameter->ResultType;
    ShaderType* paramShaderType = FindShaderType(resultType, parameter);
    context->mCurrentType->AddDependency(paramShaderType);
  }

  // If the function call is not static then we have to add the self parameter
  if(!function->ContainsAttribute(nameSettings.mStaticAttribute))
  {
    // If there were other parameters then we have to add the ',' to separate from the previous arguments
    if(!node->Parameters.Empty())
      signatureBuilder << ", ";
    AddSelfParameter(zilchSelfType, selfType, context);
  }
  signatureBuilder.Write(")");
  signatureRange.PopRange();
  signatureBuilder.PopFromStack();
  function->mShaderSignature = signatureBuilder.mBuilder.ToString();

  // Write out all of statements of the function
  ScopedShaderCodeBuilder functionBodyBuilder(context);
  ScopedCodeRangeMapping functionBodyRange(context, &function->mBodyRange, node->Location, CodeRangeDebugString("Function %s Body", function->mZilchName.c_str()));

  WriteFunctionStatements(node, context);

  function->mShaderBodyCode = functionBodyBuilder.mBuilder.ToString();

  if(function->ContainsAttribute(nameSettings.mMainAttribute))
  {
    // Only allow 1 main function per class
    if(currentType->GetHasMain())
    {
      String msg = String::Format("Only 1 function in a class can have a function with attribute '[%s]'", nameSettings.mMainAttribute.c_str());
      SendTranslationError(node->Location, msg);
    }

    WriteMainForClass(node, context->mCurrentType, function, context);
  }
}

void ZilchShaderTranslator::WalkFunctionCallNode(Zilch::FunctionCallNode*& node, ZilchShaderTranslatorContext* context)
{
  // Make sure that this function call isn't in a fragment type that is not allowed (such as Discard in a vertex shader)
  CheckForAllowedFragmentIntrinsicTypes(node->LeftOperand, context);

  // See if there's a library translation for this function call.
  // Note: function call nodes are the "top-most" portion of a function call in the tree.
  // Meaning this node contains the arguments and then the left operand will be (if it exists)
  // the member or static type access. If we only need to replace a function's name (such as Math.Dot -> dot)
  // then the translation should be taking place in WalkMemberAccess. This replacement is needed when the function
  // and the arguments change, for instance changing Shader.Discard() -> discard and changing sampler.Sample(vec2) -> Sample(sampler, vec2).
  if(mLibraryTranslator.ResolveFunctionCall(this, node, context) == true)
    return;

  Zilch::MemberAccessNode* memberAccessNode = Zilch::Type::DynamicCast<Zilch::MemberAccessNode*>(node->LeftOperand);
  // Translate non-member access functions and static member functions as normal
  if(memberAccessNode == nullptr || memberAccessNode->IsStatic == true)
  {
    context->Walker->Walk(this, node->LeftOperand, context);
    WriteFunctionCall(node->Arguments, nullptr, nullptr, context);
    return;
  }
  
  // Otherwise we know this is a function call off a member access node.

  // We must change the member function call to a global function call, so we have to grab the "self" variable.
  ScopedShaderCodeBuilder selfBuilder(context);
  context->Walker->Walk(this, memberAccessNode->LeftOperand, context);
  String selfParam = selfBuilder.ToString();
  // Remove the scoped builder from the stack (so we can use the regular builder below)
  selfBuilder.PopFromStack();

  // To start with, try to resolve the member access if needed
  // (mostly to produces errors on native type functions that haven't been translated)
  // If we fail to resolve the member access then we need to write out the function call's name
  if(mLibraryTranslator.ResolveMemberAccess(this, memberAccessNode, context) == false)
  {
    ShaderCodeBuilder& builder = context->GetBuilder();
    // We need to mangle the function call's name based upon the class' type
    Zilch::Type* thisType = memberAccessNode->AccessedFunction->This->ResultType;
    String fnCallName = MangleName(memberAccessNode->Name, thisType);
    builder << fnCallName;
  }

  // Write out all of the arguments (with the self param we pulled out as the last argument)
  WriteFunctionCall(node->Arguments, nullptr, &selfParam, context);
}

void ZilchShaderTranslator::WalkLocalVariable(Zilch::LocalVariableNode*& node, ZilchShaderTranslatorContext* context)
{
  ShaderCodeBuilder& builder = context->GetBuilder();

  // Resolve the variable's type to the actual translation type
  Zilch::Type* resultType = node->CreatedVariable->ResultType;
  ShaderType* variableShaderType = FindShaderType(resultType, node);

  // Geometry outputs are special and can't always be declared as a new variable.
  // Leave this up to the current language to define how this is translated.
  if(variableShaderType->GetTypeData()->mType == ShaderVarType::GeometryOutput)
  {
    WriteGeometryOutputVariableDeclaration(node, variableShaderType, context);
    return;
  }

  // Write out "Type name"
  builder.Write(variableShaderType->mShaderName);
  builder.WriteSpace();
  String variableName = ApplyVariableReplacement(node->Name.Token);
  builder.Write(variableName);

  // If the variable had an initial value then write it out
  if(node->InitialValue != nullptr)
  {
    // Build up a string that is the default value
    ScopedShaderCodeBuilder defaultValueBuilder(context);
    context->Walker->Walk(this, node->InitialValue, context);
    defaultValueBuilder.PopFromStack();
    String defaultValue = defaultValueBuilder.ToString();

    // If we didn't get an actual default value (such as an array's default constructor) then don't write out a default value
    if(!defaultValue.Empty())
    {
      builder.Write(" = ");
      builder.Write(defaultValue);
    }
  }
}

void ZilchShaderTranslator::WalkStaticTypeOrCreationCallNode(Zilch::StaticTypeNode*& node, ZilchShaderTranslatorContext* context)
{
  // This node is for new Type() or local Type() or even just Type().

  // This walker needs to write out the final type of the thing we're creating. Sometimes this will be replacing the type
  // (if it was from another library) and other times it will be "constructing" the class so we replace that with a "constructor" function call.

  ShaderCodeBuilder& builder = context->GetBuilder();

  Zilch::Type* zilchCreatedType = node->ReferencedType;
  // If the created type is in the fragment library then we need to mangle it's name (but not replace types)
  if(IsInOwningLibrary(zilchCreatedType))
  {
    // Mark the type we're constructing as a type we're dependent on
    // (so we can make sure to include it when composing the final file)
    ShaderType* createdShaderType = FindShaderType(zilchCreatedType, node);
    context->mCurrentType->AddDependency(createdShaderType);
    // Since this type is from their library, call the "constructor" function instead of the type's constructor
    builder << MangleName(mSettings->mNameSettings.mConstructorName, zilchCreatedType);
  }
  else
  {
    // Otherwise this type is not in their library so just resolve it's name
    ShaderType* createdShaderType = FindShaderType(zilchCreatedType, node);
    builder.Write(createdShaderType->mShaderName);
  }
}

void ZilchShaderTranslator::WalkExpressionInitializerNode(Zilch::ExpressionInitializerNode*& node, ZilchShaderTranslatorContext* context)
{
  // If we have a registered initializer node resolver then don't perform the regular walker
  if(mLibraryTranslator.ResolveInitializerNode(this, node->ResultType, node, context))
    return;

  // Otherwise just walk the left operand and all the initializer statements
  // @JoshD: This probably won't work correctly right now. Test initializer lists of non-array types later!
  context->Walker->Walk(this, node->LeftOperand, context);
  context->Walker->Walk(this, node->InitializerStatements, context);
}

void ZilchShaderTranslator::WalkUnaryOperationNode(Zilch::UnaryOperatorNode*& node, ZilchShaderTranslatorContext* context)
{
  // Later update this to allow the replacement of certain operators per type (such as ~ on Integers)
  // @JoshD-Update

  // Want to pass types by Zilch ref but have to not output this character in translation
  if (node->Operator->TokenId == Zilch::Grammar::AddressOf)
  {
    context->Walker->Walk(this, node->Operand, context);
    return;
  }

  // Write out the token and then the operand
  ShaderCodeBuilder& builder = context->GetBuilder();
  builder.Write(node->Operator->Token);
  context->Walker->Walk(this, node->Operand, context);
}

void ZilchShaderTranslator::WalkBinaryOperationNode(Zilch::BinaryOperatorNode*& node, ZilchShaderTranslatorContext* context)
{
  bool needsGrouping = false;

  // If the parent node is a unary op then we need to add parenthesis
  Zilch::UnaryOperatorNode* parentUnaryOp = Zilch::Type::DynamicCast<Zilch::UnaryOperatorNode*>(node->Parent);
  if(parentUnaryOp != nullptr)
    needsGrouping = true;

  // Otherwise, if the parent is a binary op then we need to check if we need parenthesis
  Zilch::BinaryOperatorNode* parentBinaryOp = Zilch::Type::DynamicCast<Zilch::BinaryOperatorNode*>(node->Parent);
  if(parentBinaryOp != nullptr)
  {
    OperatorInfo defaultPrecedence(999, OperatorAssociativityType::LeftToRight);
    OperatorInfo parentPrecedence = mTokenPrecedence.FindValue(parentBinaryOp->Operator->TokenId, defaultPrecedence);
    OperatorInfo nodePrecedence = mTokenPrecedence.FindValue(node->Operator->TokenId, defaultPrecedence);
    // If our parent has a lower precedence than us then that means this parent would attach to whatever is closest to it.
    // Instead parent needs to attach to the result of this node so we need to add parenthesis.
    if(parentPrecedence.mPrecedence < nodePrecedence.mPrecedence)
      needsGrouping = true;
    // If the parent has the same precedence then we might still need parenthesis. This is based upon the associativity of the operator.
    // If an operator is left-to-right then the tree will naturally be left-heavy. If this node is on the right of our parent then that
    // means grouping should be applied to us. One example of this is "a / (b / c) will produce "b / c" on the right of the parent
    // '/' instead of the left hence we need parenthesis. The same holds true for right-to-left operators.
    else if(parentPrecedence.mPrecedence == nodePrecedence.mPrecedence)
    {
      if(parentPrecedence.mAssociativityType == OperatorAssociativityType::LeftToRight && parentBinaryOp->RightOperand == node)
        needsGrouping = true;
      else if(parentPrecedence.mAssociativityType == OperatorAssociativityType::RightToLeft && parentBinaryOp->LeftOperand == node)
        needsGrouping = true;
    }
  }

  ShaderCodeBuilder& builder = context->GetBuilder();
  if(needsGrouping)
    builder << "(";

  // See if there's a replacement for the binary op (such as < in glsl 1.2).
  // This may turn into another op, or even a function call, but either way we still
  // want to apply grouping based upon the parent operator's precedence.
  if(mLibraryTranslator.ResolveBinaryOp(this, node, context) == false)
  {
    context->Walker->Walk(this, node->LeftOperand, context);

    // Write out the operator (with a space in-between)
    builder << builder.EmitSpace() << node->Operator->Token << builder.EmitSpace();

    context->Walker->Walk(this, node->RightOperand, context);
  }

  if(needsGrouping)
    builder << ")";
}

void ZilchShaderTranslator::WalkCastNode(Zilch::TypeCastNode*& node, ZilchShaderTranslatorContext* context)
{
  ShaderCodeBuilder& builder = context->GetBuilder();

  // Do C++ style casting float(0), ignore implicit casting for now
  ShaderType* castType = FindShaderType(node->ResultType, node);
  builder.Write(castType->mShaderName);
  builder.Write("(");
  context->Walker->Walk(this, node->Operand, context);
  builder.Write(")");
}

void ZilchShaderTranslator::WalkValueNode(Zilch::ValueNode*& node, ZilchShaderTranslatorContext* context)
{
  ShaderCodeBuilder& builder = context->GetBuilder();

  // Write out the value type (replace it if needed)
  String value = ApplyValueReplacement(node->Value.Token);
  builder.Write(value);

  // If the value type is a float then append a f to the end (0.0->0.0f)
  Zilch::Core& core = Zilch::Core::GetInstance();
  if(node->ResultType == core.RealType)
    builder.Write("f");
}

void ZilchShaderTranslator::WalkLocalRef(Zilch::LocalVariableReferenceNode*& node, ZilchShaderTranslatorContext* context)
{
  // Build a scoped range to capture the range of this local ref (mostly for debugging ranges...)
  ScopedCodeRangeMapping subRange(context, node->Location, CodeRangeDebugString("LocalRef"));

  // Replace the local variable if needed (such as turn "this" into "self")
  String localVar = ApplyVariableReplacement(node->Value.Token);

  context->GetBuilder().Write(localVar);
}

void ZilchShaderTranslator::WalkGetterSetter(Zilch::MemberAccessNode*& node, Zilch::GetterSetter* getSet, ZilchShaderTranslatorContext* context)
{
  // Iousage can be a mixture or r and l
  if(node->IoUsage == Zilch::IoMode::ReadRValue)
  {
    Zilch::Function* zilchGetter = node->AccessedGetterSetter->Get;

    ShaderFunction* shaderfunction = nullptr;
    // Find the shader function on from the member's type
    ShaderType* accessedShaderType = FindShaderType(node->ResultType, node, false);
    if(accessedShaderType != nullptr)
      shaderfunction = accessedShaderType->FindFunction(zilchGetter);
    // If we couldn't find the function then try finding an extension function
    if(shaderfunction == nullptr)
      shaderfunction = mCurrentLibrary->FindExtension(zilchGetter);
    
    // If we managed to get the shader function then translate it
    if(shaderfunction != nullptr)
    {
      context->GetBuilder() << shaderfunction->mShaderName << "(";
      // If the function is not static then pass the left operand (the self type)
      if(!shaderfunction->ContainsAttribute(mSettings->mNameSettings.mStaticAttribute))
        context->Walker->Walk(this, node->LeftOperand, context);
      context->GetBuilder() << ")";
      return;
    }
  }
}

void ZilchShaderTranslator::WalkMemberAccessNode(Zilch::MemberAccessNode*& node, ZilchShaderTranslatorContext* context)
{
  // See if there's a library translation for this member access. Note that at this point we are below a
  // function call if this was one. We can only change the member access itself, that is we can change
  // "obj.Name" but not any function calls. If we wanted to do this then we should've added a resolver for a function call node.
  if(mLibraryTranslator.ResolveMemberAccess(this, node, context))
    return;

  // If this member access node is actually a getter/setter then walk it (generate function calls) instead of the member access
  if(node->AccessedGetterSetter != nullptr)
  {
    WalkGetterSetter(node, node->AccessedGetterSetter, context);
    return;
  }

  ShaderCodeBuilder& builder = context->GetBuilder();
  // If this is a static member access or a forced static type then just ignore the object.Member
  // and transform this into the mangled global name, something like Object_Member.
  String resultType = node->ResultType->ToString();

  bool isForcedStatic = false;

  // The "result type" could be a delegate which we can't get a type to (e.g. static extension function).
  // In this case it's safe to translate without validating if it's forced static
  ShaderType* resultShaderType = FindShaderType(node->ResultType, node, false);
  if(resultShaderType != nullptr)
    isForcedStatic = resultShaderType->ContainsAttribute(NameSettings::mForcedStaticAttribute);

  if(node->IsStatic || isForcedStatic)
  {
    String mangledMemberName = MangleName(node->Name, node->LeftOperand->ResultType);
    // We don't actually know if this member access node is on a field of a type
    // we already know about. If it is then we should use whatever the pre-established
    // shader name is for the field (especially when it comes to built-in samplers),
    // otherwise just use the regular mangled name.
    ShaderType* memberShaderType = FindShaderType(node->LeftOperand->ResultType, node, false);
    if(memberShaderType != nullptr)
    {
      ShaderField* field = memberShaderType->mFieldMap.FindValue(node->Name, nullptr);
      if(node->AccessedField != nullptr &&  field != nullptr)
        mangledMemberName = field->mShaderName;
    }
    
    builder << mangledMemberName;

    // We want to make a dependency on the type we are accessing, but this could be a function call or a member access.
    // For now just make a dependency on both the node's result and the left op's result. If one is invalid it will likely be a delegate type.
    ShaderType* resultShaderType = FindShaderType(node->ResultType, node, false);
    if(resultShaderType != nullptr)
      context->mCurrentType->AddDependency(resultShaderType);
    ShaderType* leftOpResultShaderType = FindShaderType(node->LeftOperand->ResultType, node, false);
    if(leftOpResultShaderType != nullptr)
      context->mCurrentType->AddDependency(leftOpResultShaderType);
  }  
  else
  {
    // Otherwise this is a regular member access, so first walk the left operand
    context->Walker->Walk(this, node->LeftOperand, context);
    // Grab the string representing the operator
    String token = Zilch::Grammar::GetKeywordOrSymbol(node->Operator);
    // To prevent future kinds of member access tokens from being implemented (such as ~>)
    // and breaking, only allow the regular member access '.' for now.
    if(node->Operator != Zilch::Grammar::Access)
    {
      String msg = String::Format("Operator '%s' is invalid for use in translation", token.c_str());
      SendTranslationError(node->Location, msg);
    }

    // Then just append the access of the variable (no mangling needed here)
    builder << token << node->Name;
  }
}

void ZilchShaderTranslator::WalkMultiExpressionNode(Zilch::MultiExpressionNode*& node, ZilchShaderTranslatorContext* context)
{
  String msg = "This expression is too complex to be translated. Please simplify the expression. "
    "The most common examples are compound operators such as 'array[i] += value' which should change to 'array[i] = array[i] + value'.";
  SendTranslationError(node->Location, msg);
}

void ZilchShaderTranslator::WalkIfRootNode(Zilch::IfRootNode*& node, ZilchShaderTranslatorContext* context)
{
  // Walk all of the parts of the if statement
  Zilch::NodeList<Zilch::IfNode>::range range = node->IfParts.All();
  for(; !range.Empty(); range.PopFront())
  {
    Zilch::IfNode* node = range.Front();
    context->Walker->Walk(this, node, context);
  }
}

void ZilchShaderTranslator::WalkIfNode(Zilch::IfNode*& node, ZilchShaderTranslatorContext* context)
{
  ShaderCodeBuilder& builder = context->GetBuilder();

  // Write out the correct part and conditions for each kind of if node (such as "if" vs. "if else" vs. "else")
  if(node->IsFirstPart)
  {
    builder.Write("if(");
    context->Walker->Walk(this, node->Condition, context);
    builder.WriteLine(")");
  }
  else if(node->Condition != nullptr)
  {
    builder.Write("else if(");
    context->Walker->Walk(this, node->Condition, context);
    builder.WriteLine(")");
  }
  else
  {
    builder.WriteLine("else");
  }

  // Write all all of the statements in the node
  builder.BeginScope();
  WriteStatements(node->Statements, context);
  builder.EndScope();
}

void ZilchShaderTranslator::WalkBreakNode(Zilch::BreakNode*& node, ZilchShaderTranslatorContext* context)
{
  ShaderCodeBuilder& builder = context->GetBuilder();
  builder.Write("break");
}

void ZilchShaderTranslator::WalkContinueNode(Zilch::ContinueNode*& node, ZilchShaderTranslatorContext* context)
{
  ShaderCodeBuilder& builder = context->GetBuilder();
  builder.Write("continue");
}

void ZilchShaderTranslator::WalkReturnNode(Zilch::ReturnNode*& node, ZilchShaderTranslatorContext* context)
{
  ShaderCodeBuilder& builder = context->GetBuilder();
  builder.Write("return");

  // If there's a return value then write it out
  if(node->ReturnValue != nullptr)
  {
    builder.WriteSpace();
    context->Walker->Walk(this, node->ReturnValue, context);
  }
}

void ZilchShaderTranslator::WalkWhileNode(Zilch::WhileNode*& node, ZilchShaderTranslatorContext* context)
{
  ShaderCodeBuilder& builder = context->GetBuilder();

  builder.Write("while(");
  context->Walker->Walk(this, node->Condition, context);
  builder.WriteLine(")");

  builder.BeginScope();
  WriteStatements(node->Statements, context);
  builder.EndScope();
}

void ZilchShaderTranslator::WalkDoWhileNode(Zilch::DoWhileNode*& node, ZilchShaderTranslatorContext* context)
{
  ShaderCodeBuilder& builder = context->GetBuilder();

  builder.WriteLine("do");

  builder.BeginScope();
  WriteStatements(node->Statements, context);
  builder.EndScope();

  builder.WriteIndentation();
  builder.Write("while(");
  context->Walker->Walk(this, node->Condition, context);
  builder.Write(")");
  builder.WriteLine(";");
}

void ZilchShaderTranslator::WalkForNode(Zilch::ForNode*& node, ZilchShaderTranslatorContext* context)
{
  ShaderCodeBuilder& builder = context->GetBuilder();

  builder.Write("for(");

  // The first portion of a for loop can either be creating a new variable or just setting a variable to a value
  if(node->Initialization)
    context->Walker->Walk(this, node->Initialization, context);
  else if(node->ValueVariable)
  {
    // The generic handler would treat the variable as a statement which we don't want,
    // set the state on the context to temporarily ignore the generic handling
    bool oldState = context->mPerformAutoFormatting;
    context->mPerformAutoFormatting = false;
    context->Walker->Walk(this, node->ValueVariable, context);
    context->mPerformAutoFormatting = oldState;
  }
  builder.Write("; ");

  // Write the condition
  if(node->Condition != nullptr)
    context->Walker->Walk(this, node->Condition, context);
  builder.Write("; ");

  // Write the iteration
  if(node->Iterator != nullptr)
    context->Walker->Walk(this, node->Iterator, context);
  builder.WriteLine(")");

  builder.BeginScope();
  WriteStatements(node->Statements, context);
  builder.EndScope();
}

void ZilchShaderTranslator::WalkForEachNode(Zilch::ForEachNode*& node, ZilchShaderTranslatorContext* context)
{
  // Don't allow for each loops
  SendTranslationError(node->Location, "foreach statements are not allowed in shader translation");
}

void ZilchShaderTranslator::WalkUnknownNode(Zilch::SyntaxNode*& node, ZilchShaderTranslatorContext* context)
{
  SendTranslationError(node->Location, String::Format("Node type '%s' is not allowed in translation", node->ToString().c_str()));
}

void ZilchShaderTranslator::WriteStatements(Zilch::NodeList<Zilch::StatementNode>& statements, ZilchShaderTranslatorContext* context)
{
  ShaderCodeBuilder& builder = context->GetBuilder();

  for(size_t i = 0; i < statements.Size(); ++i)
  {
    Zilch::StatementNode* statement = statements[i];

    // For each statement keep track of the line number mappings
    ScopedCodeRangeMapping statementRange(context, statement->Location, CodeRangeDebugString("Statement"));
    context->Walker->Walk(this, statement, context);
  }
}

void ZilchShaderTranslator::WriteFunctionStatements(Zilch::GenericFunctionNode* node, ZilchShaderTranslatorContext* context)
{
  ShaderCodeBuilder& builder = context->GetBuilder();
  builder.BeginScope();
  WriteStatements(node->Statements, context);
  builder.EndScope(); 
}

void ZilchShaderTranslator::WriteFunctionSignature(Zilch::GenericFunctionNode* node, ZilchShaderTranslatorContext* context)
{
  ShaderCodeBuilder& builder = context->GetBuilder();

  //add all of the parameters
  for(uint i = 0; i < node->Parameters.Size(); ++i)
  {
    Zilch::ParameterNode* parameter = node->Parameters[i];
    Zilch::Type* resultType = parameter->CreatedVariable->ResultType;
    ShaderType* resultShaderType = FindShaderType(resultType, node);

    // Check if this is a reference type, if so we need to properly translate the in/inout/out keywords
    bool isRefType = Zilch::Type::IsHandleType(resultType);

    // If this is a reference type then add the appropriate qualifier (from the shader type if it has one)
    if(isRefType)
    {
      if(!resultShaderType->mReferenceTypeQualifier.Empty())
        builder << resultShaderType->mReferenceTypeQualifier << " ";
      else
        builder << mSettings->mNameSettings.mReferenceKeyword << " ";
    }
    // Otherwise, check and see if we're trying to pass to a function a non-copyable type. If so report an error
    else if(resultShaderType->IsNonCopyable())
    {
      String msg = String::Format("Type '%s' is non-copyable. It can only be passed to a function using the 'ref' keyword.", resultShaderType->mZilchName.c_str());
      SendTranslationError(parameter->Location, msg);
    }

    builder.Write(resultShaderType->mShaderName);
    builder.WriteSpace();
    String parameterName = ApplyVariableReplacement(parameter->Name.Token);
    builder.Write(parameterName);
    if(i != node->Parameters.Size() - 1)
      builder.Write(", ");
  }
}

void ZilchShaderTranslator::WriteFunctionCall(StringParam functionName, Zilch::NodeList<Zilch::ExpressionNode>& arguments, String* firstParam, String* lastParam, ZilchShaderTranslatorContext* context)
{
  ShaderCodeBuilder& builder = context->GetBuilder();
  builder << functionName;
  WriteFunctionCall(arguments, firstParam, lastParam, context);
}

void ZilchShaderTranslator::WriteFunctionCall(Zilch::NodeList<Zilch::ExpressionNode>& arguments, String* firstParam, String* lastParam, ZilchShaderTranslatorContext* context)
{
  ShaderCodeBuilder& builder = context->GetBuilder();
  builder << "(";
  if(firstParam != nullptr && !firstParam->Empty())
  {
    builder << *firstParam;
    if(arguments.Size() != 0)
      builder << ", ";
  }

  for(size_t i = 0; i < arguments.Size(); ++i)
  {
    context->Walker->Walk(this, arguments[i], context);
    if(i != arguments.Size() - 1)
      builder << ", ";
  }

  if(lastParam != nullptr && !lastParam->Empty())
  {
    if(arguments.Size() != 0)
      builder << ", ";
    builder << *lastParam;
  }
  builder << ")";
}

void ZilchShaderTranslator::AddSelfParameter(Zilch::Type* zilchSelfType, ZilchShaderTranslatorContext* context)
{
  AddSelfParameter(zilchSelfType, context->mCurrentType->mShaderName, context);
}

void ZilchShaderTranslator::AddSelfParameter(Zilch::Type* zilchSelfType, StringParam selfType, ZilchShaderTranslatorContext* context)
{
  ShaderType* shaderSelfType = FindShaderType(zilchSelfType, nullptr);
  context->GetBuilder() << shaderSelfType->mReferenceTypeQualifier << " " << selfType << " " << mSettings->mNameSettings.mThisKeyword;
}

String ZilchShaderTranslator::ApplyValueReplacement(StringParam value)
{
  return value;
}

String ZilchShaderTranslator::ApplyVariableReplacement(StringParam varName)
{
  // Currently only replace the 'this' keyword with 'self'
  if(varName == Zilch::ThisKeyword)
    return mSettings->mNameSettings.mThisKeyword;
  // Otherwise, pre-pend the variable with a unique key to prevent clashing with
  // built-in function names (so we get '_dot' instead of the math library function 'dot')
  return BuildString("_", varName);
}

String ZilchShaderTranslator::MangleName(StringParam name, Zilch::Type* classType)
{
  ShaderType* shaderType = FindShaderType(classType, nullptr);
  // If we have a shader type then get the correct mangled name from it (it stores the correct shader type name)
  if(shaderType != nullptr)
    return MangleName(name, shaderType);

  // If we didn't find the type then fix the class' name if necessary
  // (I don't think this should ever happen anymore as this is only really templates)
  String mangledClassName = FixClassName(classType);
  return GenerateMangledName(mangledClassName, name);
}

String ZilchShaderTranslator::MangleName(StringParam name, ShaderType* type)
{
  return GenerateMangledName(type->mShaderName, name);
}

String ZilchShaderTranslator::FixClassName(Zilch::Type* type)
{
  // Get the bound type (converts indirection types to bound types)
  StringRange className = type->ToString();
  Zilch::BoundType* boundType = Zilch::BoundType::GetBoundType(type);
  if(boundType != nullptr)
    className = boundType->ToString();

  // If the type is not template then just return the normal class name
  if(IsTemplateType(boundType) == false)
    return className;

  // Otherwise strip out invalid symbols.
  // Also pre-pend it with "template_" both for readability and to prevent name conflicts.
  StringBuilder builder;
  builder.Append("template_");
  while(!className.Empty())
  {
    Rune r = className.Front();
    className.PopFront();
    // Convert some symbols into underscores (such as the template brackets)
    if(r == '[' || r == ']' || r == ',')
      builder.Append("_");
    // And just strip out some other symbols (otherwise it gets really cumbersome to read)
    else if(r == ' ')
      continue;
    else
      builder.Append(r);
  }

  return builder.ToString();
}

String ZilchShaderTranslator::GenerateDefaultConstructorString(Zilch::Type* type, ZilchShaderTranslatorContext* context)
{
  // If this is not a user defined type then call the default constructor for this type.
  // This could be a more elaborate string depending on the type, such as Real4(1, 1, 1, 1).
  if(!IsInOwningLibrary(type))
  {
    Zilch::BoundType* boundType = Zilch::BoundType::GetBoundType(type);
    const Zilch::FunctionArray* constructors = boundType->GetOverloadedInheritedConstructors();
    Zilch::Function* defaultConstructor = Zilch::BoundType::GetDefaultConstructor(constructors);
    if(constructors != nullptr && constructors->Empty() == false)
    {
      // If we managed to find the default constructor then resolve the default constructor's translation
      Zilch::Function* defaultConstructor = Zilch::BoundType::GetDefaultConstructor(constructors);

      ScopedShaderCodeBuilder initialValueBuilder(context);
      mLibraryTranslator.ResolveDefaultConstructor(this, boundType, defaultConstructor, context);
      return initialValueBuilder.ToString();
    }
  }
  // Otherwise, this is a user defined type so just call the constructor function.
  return BuildString(MangleName(mSettings->mNameSettings.mConstructorName, type), "()");
}

bool ZilchShaderTranslator::IsTemplateType(Zilch::BoundType* type)
{
  // Currently there's no way (from a Type*) to know if a type is a template.
  // So instead I'm just seeing (for now) if the type has the '[' character.
  String className = type->ToString();
  return className.Contains("[");
}

void ZilchShaderTranslator::RegisterLibraryBoundTemplateTypes()
{
  // Iterate over all bound types in this library and if any are
  // a template then try to register them with the library translator
  Zilch::BoundTypeMap::range boundTypes = mCurrentLibrary->mZilchLibrary->BoundTypes.All();
  for(; !boundTypes.Empty(); boundTypes.PopFront())
  {
    Zilch::BoundType* boundType = boundTypes.Front().second;
    if(!boundType->TemplateBaseName.Empty())
    {
      String resultTypeName;
      (mLibraryTranslator.mTemplateTypeResolver)(this, boundType, nullptr, resultTypeName);
    }
  }
}

void ZilchShaderTranslator::CheckForAllowedFragmentIntrinsicTypes(Zilch::ExpressionNode* node, ZilchShaderTranslatorContext* context)
{
  if(node == nullptr)
    return;

  Zilch::MemberAccessNode* memberAccessNode = Zilch::Type::DynamicCast<Zilch::MemberAccessNode*>(node);
  if(memberAccessNode == nullptr)
    return;

  bool isVertexAllowed = false;
  bool isGeometryAllowed = false;
  bool isPixelAllowed = false;
  // Keep track of the access type string for error messages
  String accessType;

  if(memberAccessNode != nullptr)
  {
    Zilch::Function* function = memberAccessNode->AccessedFunction;
    Zilch::Array<Zilch::Attribute>* attributes = nullptr;
    // Get the attribute array from whatever the accessed type was
    if(function != nullptr)
    {
      attributes = &function->Attributes;
      accessType = "Function";
    }
    Zilch::Field* field = memberAccessNode->AccessedField;
    if(field != nullptr)
    {
      attributes = &field->Attributes;
      accessType = "Field";
    }
    Zilch::Property* property = memberAccessNode->AccessedProperty;
    if(property != nullptr)
    {
      attributes = &property->Attributes;
      accessType = "Property";
    }
    Zilch::GetterSetter* getterSetter = memberAccessNode->AccessedGetterSetter;
    if(getterSetter != nullptr)
    {
      attributes = &getterSetter->Attributes;
      accessType = "GetSet";
    }

    // Check for all of the allowed attributes
    isVertexAllowed = ContainsAttribute(*attributes, NameSettings::mVertexIntrinsicAttributeName);
    isGeometryAllowed = ContainsAttribute(*attributes, NameSettings::mGeometryIntrinsicAttributeName);
    isPixelAllowed = ContainsAttribute(*attributes, NameSettings::mPixelIntrinsicAttributeName);
  }

  // If this didn't contain any of the attributes to limit fragment types
  // then there are no restrictions and there is no error
  if(isVertexAllowed == false && isGeometryAllowed == false && isPixelAllowed == false)
    return;

  // This is an error if the current fragment type isn't allowed or this isn't in a fragment type
  FragmentType::Enum fragmentType = context->mCurrentType->mFragmentType;
  bool isError = (fragmentType == FragmentType::None);
  isError |= (fragmentType == FragmentType::Vertex && isVertexAllowed == false);
  isError |= (fragmentType == FragmentType::Geometry && isGeometryAllowed == false);
  isError |= (fragmentType == FragmentType::Pixel && isPixelAllowed == false);

  // Report the error
  if(isError)
  {
    // Build up a string of the allowed fragment types
    Array<String> allowedFragmentTypes;
    if(isVertexAllowed)
      allowedFragmentTypes.PushBack("Vertex");
    if(isGeometryAllowed)
      allowedFragmentTypes.PushBack("Geometry");
    if(isPixelAllowed)
      allowedFragmentTypes.PushBack("Pixel");
    String allowedTypesStr = JoinStrings(allowedFragmentTypes, " or ");

    String msg = String::Format("%s '%s' is only allowed in %s fragments.", accessType.c_str(), memberAccessNode->Name.c_str(), allowedTypesStr.c_str());
    SendTranslationError(node->Location, msg);
  }
}

bool ZilchShaderTranslator::ContainsAttribute(Array<Zilch::Attribute>& attributes, StringParam attributeToSearch)
{
  for(uint i = 0; i < attributes.Size(); ++i)
  {
    Zilch::Attribute& attribute = attributes[i];
    if(attribute.Name == attributeToSearch)
      return true;
  }
  return false;
}

bool ZilchShaderTranslator::IsInOwningLibrary(Zilch::Library* library)
{
  // Check and see if this library is the same as the source library (that we are passed in from the compositor)
  if(mLibraryTranslator.IsUserLibrary(library))
    return true;
  return false;
}

bool ZilchShaderTranslator::IsInOwningLibrary(Zilch::Type* type)
{
  Zilch::Library* owningLibrary = type->GetOwningLibrary();
  return IsInOwningLibrary(owningLibrary);
}

ShaderType* ZilchShaderTranslator::FindShaderType(Zilch::Type* type, Zilch::SyntaxNode* syntaxNode, bool reportErrors)
{
  Zilch::BoundType* boundType = Zilch::BoundType::GetBoundType(type);
  // If we found the bound type successfully then try to find the shader type
  if(boundType != nullptr)
  {
    // If this is a template type then there is a chance we haven't yet created this specific
    // template's translation (such as an array of real3 vs real). There is also a chance that
    // a different library owns the bound type that is needed here and that the template is an
    // intrinsic type (FixedArray). In this case we won't have the pre-existing shader type
    // (because there isn't one) but we still need to translate functions. This should eventually
    // be removed and these shader types should either be created somehow or the translation
    // resolution functions need to stick around in all dependent libraries.
    if(IsTemplateType(boundType))
    {
      String resultTypeName;
      (mLibraryTranslator.mTemplateTypeResolver)(this, type, syntaxNode, resultTypeName);
    }

    // Try and find a shader type to return from the corresponding bound type
    ShaderType* resultShaderType = mCurrentLibrary->FindType(boundType);
    if(resultShaderType != nullptr)
      return resultShaderType;
  }

  ShaderType* resultType = nullptr;
  // Otherwise we couldn't produce a shader type so return a fake type (to prevent crashes) and report an error
  if(reportErrors)
    resultType = FindAndReportUnknownType(type, syntaxNode);
  
  return resultType;
}

ShaderType* ZilchShaderTranslator::FindAndReportUnknownType(Zilch::Type* type, Zilch::SyntaxNode* syntaxNode)
{
  String msg = String::Format("Type '%s' is not a valid type for use in a shader", type->ToString().c_str());
  SendTranslationError(syntaxNode->Location, msg);
  return mCurrentLibrary->FindType("[Unknown]");
}

bool ZilchShaderTranslator::IsBuiltInField(ShaderField* field)
{
  ShaderType* fieldShaderType = field->GetShaderType();
  ShaderFieldKey fieldKey(field);
  
  // A field is only considered to come from a built-in if it matches a declared built-in and
  // it is either declared as an input or a forced static. A forced static (such as a sampler)
  // auto becomes a built-in because they must be an input, otherwise they make no sense right now.
  // Otherwise we could have an variable like 'Time' that could either be [Static] or contain no
  // attribute in which case it is not considered as coming from a built-in.
  bool isBuiltIn = mSettings->mShaderDefinitionSettings.mBuiltIns.ContainsKey(fieldKey);
  bool isInput = field->ContainsAttribute(mSettings->mNameSettings.mInputAttributeName);
  bool isForcedStatic = fieldShaderType->ContainsAttribute(mSettings->mNameSettings.mForcedStaticAttribute);
  if(isBuiltIn && (isInput || isForcedStatic))
    return true;
  return false;
}

void ZilchShaderTranslator::SendTranslationError(Zilch::CodeLocation& codeLocation, StringParam message)
{
  SendTranslationError(codeLocation, message, message);
}

void ZilchShaderTranslator::SendTranslationError(Zilch::CodeLocation& codeLocation, StringParam shortMsg, StringParam fullMsg)
{
  mErrorTriggered = true;
  if(mErrors != nullptr)
    mErrors->SendTranslationError(codeLocation, shortMsg, fullMsg);
}

}//namespace Zero
