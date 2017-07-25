/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  void TypingContext::Clear(bool tolerantMode)
  {
    // If we're not in tolerant mode, we better not have anything in our context
    ErrorIf(tolerantMode == false && this->ClassTypeStack.Empty() == false,
      "Classes still leftover in the typing context after it was done being used");
    ErrorIf(tolerantMode == false && this->FunctionStack.Empty() == false,
      "Functions still leftover in the typing context after it was done being used");

    // Always clear everything from the context, regardless of the mode (just for safety)
    this->ClassTypeStack.Clear();
    this->FunctionStack.Clear();
  }

  //***************************************************************************
  Syntaxer::Syntaxer(CompilationErrors& errors) :
    Errors(errors),
    Builder(nullptr),
    Tree(nullptr),
    ParentProject(nullptr),
    Dependencies(nullptr),
    ClassWalker(&errors),
    MemberWalker(&errors),
    FunctionWalker(&errors),
    TypingWalker(&errors),
    ExpressionWalker(&errors)
  {
    ZilchErrorIfNotStarted(Syntaxer);

    // Collect all the classes first
    // We need to do a pass over all classes in the entire code base because if
    // anything references a type name later on, we need to be able to resolve it
    // or know if it's an error on the spot!
    this->ClassWalker.Register(&Syntaxer::CollectClass);
    this->ClassWalker.Register(&Syntaxer::CollectEnum);

    // We need to walk every expression, every node, every place where we could find types
    // We basically need to collect all uses of templates
    this->TemplateWalker.RegisterNonLeafBase(&Syntaxer::CollectTemplateInstantiations);

    // The next pass we do, we dive into each class definition and pull out member variables
    // and property declarations. This is so that if we reference anything using the member
    // access operator '.' in the next pass, then we know if it's an error or not
    this->MemberWalker.Register(&Syntaxer::CollectClassInheritance);
    this->MemberWalker.Register(&Syntaxer::CollectSendsEvents);
    this->MemberWalker.Register(&Syntaxer::CollectEnumInheritance);
    this->MemberWalker.Register(&Syntaxer::CollectMemberVariableAndProperty);

    this->IndexerWalker.Register(&Syntaxer::IndexerBinaryOperator);
    this->IndexerWalker.Register(&Syntaxer::IndexerUnaryOperator);
    this->IndexerWalker.Register(&Syntaxer::IndexerIndexerCall);

    // We have to do functions as a separate pass since we must know
    // the sizes of all objects before doing function signatures
    // We also do parameters here because function delegate types must be done by this point
    this->FunctionWalker.Register(&Syntaxer::PushClass);
    this->FunctionWalker.Register(&Syntaxer::CollectConstructor);
    this->FunctionWalker.Register(&Syntaxer::CollectDestructor);
    this->FunctionWalker.Register(&Syntaxer::CollectFunction);
    this->FunctionWalker.Register(&Syntaxer::CollectPropertyGetSet);
    this->FunctionWalker.RegisterDerived<ParameterNode>(&Syntaxer::CheckLocalVariable);

    // Walk through the tree and attempt to Assign locations to every node (class, function, etc)
    this->LocationWalker.RegisterNonLeafBase(&Syntaxer::DecorateCodeLocations);

    // Walk all any type of expression (often, expressions are nested within each other)
    this->TypingWalker.Register(&Syntaxer::PushClass);
    this->TypingWalker.RegisterDerived<ConstructorNode>(&Syntaxer::PushFunction);
    this->TypingWalker.RegisterDerived<DestructorNode>(&Syntaxer::PushFunction);
    this->TypingWalker.Register(&Syntaxer::CheckAndPushFunction);
    this->TypingWalker.Register(&Syntaxer::DecorateInitializer);
    this->TypingWalker.Register(&Syntaxer::DecorateValue);
    this->TypingWalker.Register(&Syntaxer::DecorateStringInterpolant);
    this->TypingWalker.Register(&Syntaxer::DecorateStaticTypeOrCreationCall);
    this->TypingWalker.Register(&Syntaxer::DecorateExpressionInitializer);
    this->TypingWalker.Register(&Syntaxer::DecorateMultiExpression);
    this->TypingWalker.Register(&Syntaxer::DecorateTypeId);
    this->TypingWalker.Register(&Syntaxer::DecorateMemberId);
    this->TypingWalker.Register(&Syntaxer::DecorateCheckTypeCast);
    this->TypingWalker.Register(&Syntaxer::DecorateCheckBinaryOperator);
    this->TypingWalker.Register(&Syntaxer::DecorateCheckUnaryOperator);
    this->TypingWalker.Register(&Syntaxer::DecorateCheckPropertyDelegateOperator);
    this->TypingWalker.Register(&Syntaxer::DecorateCheckFunctionCall);
    this->TypingWalker.Register(&Syntaxer::CheckReturn);
    this->TypingWalker.Register(&Syntaxer::CheckDelete);
    this->TypingWalker.Register(&Syntaxer::CheckThrow);
    this->TypingWalker.Register(&Syntaxer::CheckMemberVariable);
    this->TypingWalker.Register(&Syntaxer::CheckLocalVariable);
    this->TypingWalker.Register(&Syntaxer::CheckWhile);
    this->TypingWalker.Register(&Syntaxer::CheckDoWhile);
    this->TypingWalker.Register(&Syntaxer::CheckFor);
    this->TypingWalker.RegisterDerived<ForEachNode>(&Syntaxer::CheckFor);
    this->TypingWalker.Register(&Syntaxer::CheckLoop);
    this->TypingWalker.Register(&Syntaxer::CheckScope);
    this->TypingWalker.Register(&Syntaxer::CheckTimeout);
    this->TypingWalker.Register(&Syntaxer::CheckIfRoot);
    this->TypingWalker.Register(&Syntaxer::CheckIf);
    this->TypingWalker.Register(&Syntaxer::CheckBreak);
    this->TypingWalker.Register(&Syntaxer::CheckContinue);
    this->TypingWalker.Register(&Syntaxer::ResolveLocalVariableReference);
    this->TypingWalker.Register(&Syntaxer::ResolveMember);

    // The last thing we do is walk all expressions and verify that r-values and l-values get treated properly
    this->ExpressionWalker.RegisterNonLeafBase(&Syntaxer::CheckExpressionIoModes);
  }

  //***************************************************************************
  Syntaxer::~Syntaxer()
  {
    // Get a list of all template root nodes
    HashMap<String, ClassNode*>::valuerange templateRootNodes = this->InternalBoundTemplates.Values();

    // Loop through all the template root nodes...
    while (templateRootNodes.Empty() == false)
    {
      // Get the current template and move to the next
      ClassNode* templateClassNode = templateRootNodes.Front();
      templateRootNodes.PopFront();

      // Delete the node (frees all the memory even of children)
      delete templateClassNode;
    }
  }

  //***************************************************************************
  void Syntaxer::ErrorAt(SyntaxNode* node, ErrorCode::Enum errorCode, ...)
  {
    // Start a variadic argument list
    va_list argList;
    va_start(argList, errorCode);

    // Call the other error function
    ErrorAtArgs(node, errorCode, argList);

    // End the argument list
    va_end(argList);
  }

  //***************************************************************************
  void Syntaxer::ErrorAtArgs(SyntaxNode* node, ErrorCode::Enum errorCode, va_list argList)
  {
    // Now call the error function
    this->Errors.RaiseArgs(node->Location, errorCode, argList);
  }

  //***************************************************************************
  Type* Syntaxer::RetrieveType(SyntaxType* syntaxType, const CodeLocation& location, const Module& dependencies)
  {
    // Store the dependencies and all their types into a map
    this->Dependencies = &dependencies;
    this->PopulateDependencies();

    // Now attempt to turn the syntax type into an actual type (or return null)
    return this->RetrieveType(syntaxType, location);
  }

  //***************************************************************************
  void Syntaxer::PopulateDependencies()
  {
    // Clear any previous stored bound types, in case we're reusing the syntaxer
    this->ExternalBoundTypes.Clear();

    // Loop through all the passed in libraries
    for (size_t i = 0; i < this->Dependencies->Size(); ++i)
    {
      // Get the current library
      LibraryRef library = (*this->Dependencies)[i];
      ErrorIf(library == nullptr, "The user passed in a null library to the Module or dependencies");

      // Get a range of all bound types in the library
      BoundTypeRange libraryBoundTypes = library->BoundTypes.All();

      // Loop through all named types in the library
      while (libraryBoundTypes.Empty() == false)
      {
        // Grab the current named type in the library and iterate forward
        BoundTypePair& pair = libraryBoundTypes.Front();
        libraryBoundTypes.PopFront();

        // If we didn't Insert the type because something was in its place...
        if (this->ExternalBoundTypes.InsertNoOverwrite(pair) == false)
        {
          // Add all the named true types from this library into to the hash map
          if (!this->Errors.TolerantMode)
          {
            BoundType* alreadyRegisteredType = this->ExternalBoundTypes.FindValue(pair.first, nullptr);
            return this->Errors.Raise(CodeLocation(), ErrorCode::ExternalTypeNamesCollide,
              pair.first.c_str(),
              library->Name.c_str(),
              alreadyRegisteredType->SourceLibrary->Name.c_str());
          }
        }
      }
    }
  }

  //***************************************************************************
  void Syntaxer::FindDependencyCycles(BoundType* type, HashMap<BoundType*, DependencyState::Enum>& dependencies, const CodeLocation& location)
  {
    // Based on the current dependency state of the type
    switch (dependencies[type])
    {
      case DependencyState::BeingDetermined:
      {
        // This is an error, since we should never try to determine the size of an object again
        // whilst its already being determined from a parent call (it indicates a cycle of aggregation)
        // This should actually be a compiler error
        this->Errors.Raise(location, ErrorCode::CompositionCycleDetected);
        return;
      }
      
      case DependencyState::Undetermined:
      {
        // We're in the middle of determining this type's dependencies now
        // If we ever attempt to walk this class again, (see the above case) then we know
        // that there was a cycle of dependencies!
        dependencies[type] = DependencyState::BeingDetermined;

        // If we have a base class type and its in the same library...
        BoundType* baseType = type->BaseType;
        if (baseType != nullptr && baseType->SourceLibrary == type->SourceLibrary)
          this->FindDependencyCycles(baseType, dependencies, baseType->Location);

        // Loop through all the members
        FieldMapRange allFields = type->InstanceFields.All();
        while (allFields.Empty() == false)
        {
          // Get the member
          Field& field = *allFields.Front().second;
          allFields.PopFront();

          // We only care about computing the size of the type if it's a value type
          if (Type::IsValueType(field.PropertyType))
          {
            BoundType* propertyType = nullptr;
            
            if (field.PropertyType->SourceLibrary == type->SourceLibrary)
              propertyType = Type::DynamicCast<BoundType*>(field.PropertyType);

            if (propertyType)
              this->FindDependencyCycles(propertyType, dependencies, field.Location);
          }
        }

        // We finished walking all the dependencies for this type
        dependencies[type] = DependencyState::Completed;
      }
      return;
    }
  }

  //***************************************************************************
  void Syntaxer::ApplyToTree
  (
    SyntaxTree& syntaxTree,
    LibraryBuilder& builder,
    Project& project,
    const Module& dependencies
  )
  {
    // Store syntaxer state
    this->Tree = &syntaxTree;
    this->ParentProject = &project;

    // Store the builder so we can add things to it as we run the syntaxer
    this->Builder = &builder;

    // Store the dependencies and all their types into a map
    this->Dependencies = &dependencies;
    this->PopulateDependencies();

    // Add our own builder's library to the mix
    this->AllLibraries.PushBack(builder.BuiltLibrary);

    // Copy over all the dependencies into our library array
    this->AllLibraries.Insert(this->AllLibraries.End(), this->Dependencies->All());

    // Note: Technically with single expression mode, we should only have to evaluate
    // the types of just the expression (no class/member gathering, etc)
    // However, the expression COULD be referring to a template, which may need to be instantiated

    // Collect all the class types first
    ClassContext classContext;
    this->ClassWalker.Walk(this, syntaxTree.Root, &classContext);

    // If an error occurred, exit out
    if (this->Errors.WasError)
      return;

    // Walk the tree and look for any template instantiations
    this->TemplateWalker.Walk(this, syntaxTree.Root, &classContext);

    // If an error occurred, exit out
    if (this->Errors.WasError)
      return;

    // Walk the tree and collect members
    TypingContext typingContext;
    this->MemberWalker.Walk(this, syntaxTree.Root, &typingContext);

    // If an error occurred, exit out
    if (this->Errors.WasError)
      return;

    // Propegate any flags from the base to the derived classes
    this->Builder->BuiltLibrary->ComputeTypesInDependencyOrderOnce();
    ZilchForEach(BoundType* type, this->Builder->BuiltLibrary->TypesInDependencyOrder)
    {
      if (type->BaseType == nullptr)
        continue;
      
      if (type->BaseType->CreatableInScript == false)
        type->CreatableInScript = false;
    }

    // Loop through all class nodes
    for (size_t i = 0; i < syntaxTree.Root->Classes.Size(); ++i)
    {
      // Grab the current node
      ClassNode* node = syntaxTree.Root->Classes[i];
      if (node == nullptr)
        continue;

      // Send out an event letting the user know that we just parsed this type (but its members have yet to be parsed)
      // NOTE: The user may add things to the library such as extension properties, or more...
      ParseEvent toSend;
      toSend.Builder = this->Builder;
      toSend.Type = node->Type;
      toSend.Location = &node->Location;
      toSend.BuildingProject = this->ParentProject;
      EventSend(this->ParentProject, Events::TypeParsed, &toSend);
    }

    // Loop through all enum nodes
    for (size_t i = 0; i < syntaxTree.Root->Enums.Size(); ++i)
    {
      // Grab the current node
      EnumNode* node = syntaxTree.Root->Enums[i];
      if (node == nullptr)
        continue;
      
      // Send out an event letting the user know that we just parsed this type (but its members have yet to be parsed)
      // NOTE: The user may add things to the library such as extension properties, or more...
      ParseEvent toSend;
      toSend.Builder = this->Builder;
      toSend.Type = node->Type;
      toSend.Location = &node->Location;
      toSend.BuildingProject = this->ParentProject;
      EventSend(this->ParentProject, Events::TypeParsed, &toSend);
    }

    // We've collected all the classes we're compiling, as well as the members
    // Loop through all the classes and look for cycles of dependencies
    HashMap<BoundType*, DependencyState::Enum> dependencyStates;
    for (size_t i = 0; i < classContext.AllClasses.Size(); ++i)
    {
      // Grab the current class type
      BoundType* type = classContext.AllClasses[i];

      this->FindDependencyCycles(type, dependencyStates, type->Location);
    }

    // Clear the typing context after every use (not necessary, except in tolerant mode)
    typingContext.Clear(this->Errors.TolerantMode);

    // Walk the tree and re-arrange the indexer node
    this->IndexerWalker.Walk(this, syntaxTree.Root, &typingContext);

    // If an error occurred, exit out
    if (this->Errors.WasError)
      return;

    SyntaxNode::FixParentPointers(syntaxTree.Root, nullptr);

    // Clear the typing context after every use (not necessary, except in tolerant mode)
    typingContext.Clear(this->Errors.TolerantMode);

    // Walk the tree and give functions types
    this->FunctionWalker.Walk(this, syntaxTree.Root, &typingContext);

    // If an error occurred, exit out
    if (this->Errors.WasError)
      return;

    // Clear the typing context after every use (not necessary, except in tolerant mode)
    typingContext.Clear(this->Errors.TolerantMode);

    // Now actually walk the code and do all the checking
    this->LocationWalker.Walk(this, syntaxTree.Root, &typingContext);

    // If an error occurred, exit out
    if (this->Errors.WasError)
      return;

    // Generate field properties here (after the sizing has been computed)
    builder.GenerateGetSetFields();

    // Clear the typing context after every use (not necessary, except in tolerant mode)
    typingContext.Clear(this->Errors.TolerantMode);

    // Now actually walk the code and do all the checking
    this->TypingWalker.Walk(this, syntaxTree.Root, &typingContext);

    // If an error occurred, exit out
    if (this->Errors.WasError)
      return;

    // Clear the typing context after every use (not necessary, except in tolerant mode)
    typingContext.Clear(this->Errors.TolerantMode);

    // Lastly, we walk expressions to verify that all reads/writes were properly handled
    this->ExpressionWalker.Walk(this, syntaxTree.Root, &typingContext);

    // If an error occurred, exit out
    if (this->Errors.WasError)
      return;

    // Clear the typing context after every use (not necessary, except in tolerant mode)
    typingContext.Clear(this->Errors.TolerantMode);
  }

  //***************************************************************************
  void Syntaxer::ReplaceTypes(SyntaxTypes& typesToReplace, Array<const UserToken*>& names, const BoundSyntaxType* instanceType, const CodeLocation& location)
  {
    // Loop through all the node's types
    for (size_t i = 0; i < typesToReplace.Size(); ++i)
    {
      // Get the current type
      SyntaxType*& typeToReplace = *typesToReplace[i];

      // If the type we're looking at is a bound syntax type...
      if (BoundSyntaxType* dataTypeToReplace = Type::DynamicCast<BoundSyntaxType*>(typeToReplace))
      {
        // Loop through all the template names
        for (size_t j = 0; j < names.Size(); ++j)
        {
          // Get the current template argument name
          const String& name = names[j]->Token;
          
          // Compare the data type name with each of the template argument type names
          if (dataTypeToReplace->TypeName == name)
          {
            // If the name matches, simply replace the name with what was passed in
            SyntaxNode* replaceWithType = instanceType->TemplateArguments[j];

            // If the type we're currently replacing is a template, we have special logic...
            if (dataTypeToReplace->IsTemplateInstantiation())
            {
              // If the type we're replacing with is a data type
              if (BoundSyntaxType* replaceWithDataType = Type::DynamicCast<BoundSyntaxType*>(replaceWithType))
              {
                // We cannot replace a template type with another templated type
                if (replaceWithDataType->IsTemplateInstantiation() == false)
                {
                  dataTypeToReplace->TypeName = replaceWithDataType->TypeName;
                }
                else
                {
                  // We cannot perform the replacement of a templated argument list with a non templatable type
                  return this->Errors.Raise(location, ErrorCode::CannotReplaceTemplateInstanceWithTemplateArguments);
                }
              }
              else
              {
                // We cannot perform the replacement of a templated argument list with a non templatable type
                return this->Errors.Raise(location, ErrorCode::CannotReplaceTemplateInstanceWithTemplateArguments);
              }
            }
            else
            {
              // Otherwise, we just do a full replacement :)
              // We MUST do a clone here or else we'll get a double delete situation
              delete typeToReplace;
              typeToReplace = (SyntaxType*)replaceWithType->Clone();
              dataTypeToReplace = nullptr;
            }
            break;
          }
        }
      }

      // Traverse the syntax type recursively and apply the replacement
      this->PerformTemplateReplacement(typeToReplace, names, instanceType);

      // If we had an error, return out early
      if (this->Errors.WasError)
        return;
    }
  }

  //***************************************************************************
  void Syntaxer::PerformTemplateReplacement(SyntaxType* type, Array<const UserToken*>& names, const BoundSyntaxType* instanceType)
  {
    // Get all the syntax types from this node
    SyntaxTypes types;
    type->PopulateSyntaxTypes(types);

    // Perform recursive replacements of types
    this->ReplaceTypes(types, names, instanceType, CodeLocation());
  }

  //***************************************************************************
  void Syntaxer::PerformTemplateReplacement(SyntaxNode* node, Array<const UserToken*>& names, const BoundSyntaxType* instanceType)
  {
    // Get the children for the current node
    NodeChildren children;
    node->PopulateChildren(children);

    // Loop through all the children...
    for (size_t i = 0; i < children.Size(); ++i)
    {
      // Get the current child
      SyntaxNode*& child = *children[i];

      // Traverse to that child recursively and apply the replacement
      this->PerformTemplateReplacement(child, names, instanceType);

      // If we had an error, return out early
      if (this->Errors.WasError)
        return;
    }

    // Get all the syntax types from this node
    SyntaxTypes types;
    node->PopulateSyntaxTypes(types);

    // Perform recursive replacements of types
    this->ReplaceTypes(types, names, instanceType, node->Location);
  }

  //***************************************************************************
  BoundType* Syntaxer::RetrieveBoundType(BoundSyntaxType* type, const CodeLocation& location, BoundType* classWeAreInsideOf)
  {
    // Get the instance of the type database
    Core& core = Core::GetInstance();

    // In tolerant mode, it is possible to get here and not have the template instantiation replaced
    // because templates are currently not exported in the library (which is reused by code completion)
    // and only the current class we're writing is actually parsed/seen by code completion
    if (this->Errors.TolerantMode == false)
    {
      // Error checking
      ErrorIf(type->IsTemplateInstantiation(),
        "All template instantiations should have been resolved by the TemplateWalker");

      // We ignore the builder not being set in tolerant mode
      ErrorIf(this->Builder == nullptr, "Attempted to retrieve a type without a builder being set");
    }

    // Make sure we have a valid builder, since this function can get called without one
    if (this->Builder != nullptr)
    {
      // Check to see if we found the type
      if (BoundType* namedType = this->Builder->FindBoundType(type->TypeName))
      {
        // Return the found type...
        type->ResolvedType = namedType;
        return namedType;
      }
    }

    // Check to see if we found the type
    if (BoundType* namedType = this->ExternalBoundTypes.FindValue(type->TypeName, nullptr))
    {
      // Return the found type...
      type->ResolvedType = namedType;
      return namedType;
    }

    // If we stopped here, it means the user referenced a type that wasn't defined
    String typeName = type->ToString();
    cstr typeNameCstr = typeName.c_str();
    
    // If we have an extra context of what class we're inside of, then its very possible
    // that the user was trying to implicitly access a member without 'this.', which is not legal in Zilch (but exists in C++)
    // Try to provide better error information for these cases
    bool hasInstanceMember = false;
    if (classWeAreInsideOf != nullptr)
    {
      hasInstanceMember =
        classWeAreInsideOf->InstanceFields.ContainsKey(typeName)        ||
        classWeAreInsideOf->InstanceFunctions.ContainsKey(typeName)     ||
        classWeAreInsideOf->InstanceGetterSetters.ContainsKey(typeName);
    }
    
    // Can we provide better error messages?
    if (hasInstanceMember)
    {
      this->Errors.Raise
      (
        location,
        ErrorCode::ReferenceToUndefinedTypeWithSimilarMember,
        typeNameCstr,
        typeNameCstr,
        typeNameCstr
      );
    }
    else
    {
      this->Errors.Raise(location, ErrorCode::ReferenceToUndefinedType, typeNameCstr);
    }

    // Return null since we don't have a valid type!
    return core.ErrorType;
  }

  //***************************************************************************
  Type* Syntaxer::RetrieveType(SyntaxType* syntaxType, const CodeLocation& location)
  {
    // Get the instance of the type database
    Core& core = Core::GetInstance();
    
    // If the syntax type is an 'any' syntax type...
    if (Type::DynamicCast<AnySyntaxType*>(syntaxType) != nullptr)
    {
      // We only ever have one instance of this type
      syntaxType->ResolvedType = core.AnythingType;
      return core.AnythingType;
    }
    // If the syntax type is an indirection syntax type...
    else if (IndirectionSyntaxType* indirectionSyntaxType = Type::DynamicCast<IndirectionSyntaxType*>(syntaxType))
    {
      // Get our type as a named syntax type
      SyntaxType* referencedType = indirectionSyntaxType->ReferencedType;
      BoundSyntaxType* boundSyntaxType = Type::DynamicCast<BoundSyntaxType*>(referencedType);

      // We're only allowed to refer to a named type
      if (boundSyntaxType == nullptr)
      {
        // The type must not be a named type...
        this->Errors.Raise(location, ErrorCode::ReferencesOnlyToNamedValueTypes, referencedType->ToString().c_str());
        return core.ErrorType;
      }

      // Grab the type it is referring to...
      BoundType* type = this->RetrieveBoundType(boundSyntaxType, location);

      // If the type is not a value type, then we cannot form a reference to it
      if (type->CopyMode == TypeCopyMode::ReferenceType)
      {
        // The ref keyword can only be used to refer to value types
        this->Errors.Raise(location, ErrorCode::ReferencesOnlyToNamedValueTypes, referencedType->ToString().c_str());
        return core.ErrorType;
      }

      // Return the result of attempting to find or create the type from a true type and its qualifiers
      IndirectionType* indirectType = this->Builder->ReferenceOf(type);
      syntaxType->ResolvedType = indirectType;
      return indirectType;
    }
    // If the syntax type is a named syntax type...
    else if (BoundSyntaxType* boundSyntaxType = Type::DynamicCast<BoundSyntaxType*>(syntaxType))
    {
      // Find types by name since it's a data type
      BoundType* boundType = this->RetrieveBoundType(boundSyntaxType, location);
      syntaxType->ResolvedType = boundType;
      return boundType;
    }
    // If the syntax type is a delegate syntax type...
    else if (DelegateSyntaxType* delegateSyntaxType = Type::DynamicCast<DelegateSyntaxType*>(syntaxType))
    {
      // Store the return type
      Type* returnType = nullptr;
      
      // Store all the delegate parameters
      ParameterArray parameters;

      // Loop through all the parameters
      for (size_t i = 0; i < delegateSyntaxType->Parameters.Size(); ++i)
      {
        // Get the syntax parameter
        const DelegateSyntaxParameter& syntaxParameter = delegateSyntaxType->Parameters[i];

        // Add the next parameter to the list
        DelegateParameter& parameter = parameters.PushBack();
        parameter.Name = syntaxParameter.Name->Token;
        parameter.ParameterType = this->RetrieveType(syntaxParameter.Type, location);
      }

      // Loop through all the returns
      if (delegateSyntaxType->Return != nullptr)
      {
        // Set the return type of the delegate (resolve the proper type)
        returnType = this->RetrieveType(delegateSyntaxType->Return, location);
      }
      else
      {
        // Otherwise, there is no return type (use void)
        returnType = core.VoidType;
      }

      // Return the delegate type after adding or merging into the set
      DelegateType* delegateType = this->Builder->GetDelegateType(parameters, returnType);
      syntaxType->ResolvedType = delegateType;
      return delegateType;
    }

    // If we stopped here, it means the user referenced a type that wasn't defined
    this->Errors.Raise(location, ErrorCode::ReferenceToUndefinedType, syntaxType->ToString().c_str());

    // We return a special error type because it makes it so we crash less on null pointers
    return core.ErrorType;
  }
  
  //***************************************************************************
  Constant Syntaxer::ValueNodeToConstant(ValueNode* node)
  {
    // Grab the token for ease of use
    String& tokenString = node->Value.Token;
    const char* token = tokenString.c_str();

    // Check to see what type of literal we have here
    switch (node->Value.TokenId)
    {
      case Grammar::IntegerLiteral:
        return atoi(token);

      case Grammar::DoubleIntegerLiteral:
        return ZilchStringToDoubleInteger(token, 10);

      case Grammar::RealLiteral:
        return (Real)atof(token);

      case Grammar::DoubleRealLiteral:
        return atof(token);

      case Grammar::StringLiteral:
        return ReplaceStringEscapesAndStripQuotes(tokenString);

      case Grammar::True:
      case Grammar::False:
        return (node->Value.TokenId == Grammar::True);

      // The value is a null (which means that the type is unknown)
      case Grammar::Null:
        return Constant();

      default:
      {
        // This is an invalid constant type
        this->ErrorAt(node, ErrorCode::GenericError, "The value must be a constant.");
        return Constant();
      }
    }
  }
  
  //***************************************************************************
  void Syntaxer::ReadAttributes(SyntaxNode* parentNode, NodeList<AttributeNode>& nodes, Array<Attribute>& attributesOut)
  {
    // Go through all attributes and attach them to the class type
    for (size_t nodeIndex = 0; nodeIndex < nodes.Size(); ++nodeIndex)
    {
      // Add each attribute string to the class attributes
      AttributeNode* attributeNode = nodes[nodeIndex];
      Attribute& attribute = attributesOut.PushBack();
      attribute.Name = attributeNode->TypeName->Token;
      
      // If the attribute node has a function call...
      FunctionCallNode* attributeCall = attributeNode->AttributeCall;
      if (attributeCall != nullptr)
      {
        // To avoid later errors, set the io modes of the function call
        attributeCall->Io = IoMode::ReadRValue;
        attributeCall->IoUsage = IoMode::Ignore;

        // Loop through all the arguments to the function call
        for (size_t i = 0; i < attributeCall->Arguments.Size(); ++i)
        {
          // Grab the current argument value
          ExpressionNode* argument = attributeCall->Arguments[i];

          // We only support literals in the attribute arguments
          ValueNode* literalArgument = Type::DynamicCast<ValueNode*>(argument);
          TypeIdNode* typeId = Type::DynamicCast<TypeIdNode*>(argument);
          
          // If this isn't a literal argument, give an error
          if (literalArgument == nullptr && typeId == nullptr)
            return this->ErrorAt(argument, ErrorCode::AttributeArgumentMustBeLiteral);

          // We also only accept type ids that directly take a type (not an expression)
          if (typeId != nullptr && typeId->CompileTimeSyntaxType == nullptr)
            return this->ErrorAt(argument, ErrorCode::AttributeArgumentMustBeLiteral);
          
          // Just so we don't get any errors complaining later, give this things an io mode
          argument->Io = IoMode::ReadRValue;
          argument->IoUsage = IoMode::Ignore;

          // Add the parameter to the attribute
          AttributeParameter& parameter = attribute.Parameters.PushBack();

          // If this argument has a name...
          if (i < attributeCall->ArgumentNames.Size())
            parameter.Name = attributeCall->ArgumentNames[i];

          // If this was a literal value...
          if (literalArgument != nullptr)
          {
            // Store the original token text, just in case the user wants it
            String& tokenString = literalArgument->Value.Token;
            const char* token = tokenString.c_str();
            
            // Check to see what type of literal we have here
            Constant& constant = parameter;
            constant = this->ValueNodeToConstant(literalArgument);
            if (this->Errors.WasError)
              return;
          }
          // It must be a typeid...
          else
          {
            ErrorIf(typeId == nullptr, "The attribute argument wasn't a literal value or a typeid, but we should have validated that above");

            // Store the original token text, just in case the user wants it
            parameter.Type = ConstantType::Type;
            parameter.TypeValue = this->RetrieveType(typeId->CompileTimeSyntaxType, typeId->CompileTimeSyntaxType->Location);
          }
        }
      }
    }
  }

  //***************************************************************************
  void Syntaxer::SetupFunctionLocation(Function* function, const CodeLocation& location, const CodeLocation& nameLocation)
  {
    function->Location = location;
    function->NameLocation = nameLocation;

    // We also want to set the location of 'this' to just point at the function name
    Variable* thisVariable = function->This;
    if (thisVariable != nullptr)
    {
      thisVariable->Location = function->NameLocation;
      thisVariable->NameLocation = function->NameLocation;
    }
  }

  //***************************************************************************
  void Syntaxer::SetupClassInstance(ClassNode* node, ClassContext* context)
  {
    // Get the instance of the type database
    Core& core = Core::GetInstance();

    // Create the class type
    BoundType* type = this->Builder->AddBoundType(node->Name.Token, node->CopyMode, UndeterminedSize, 0);
    type->Sealed = false;
    node->Type = type;
    
    // Mark this type as not being native
    type->Native = false;

    // Use the comments as the class description
    String description = node->GetMergedComments();
    type->Description = description;

    // Set the class type's originating location
    type->Location = node->Location;
    type->NameLocation = node->Name.Location;

    // Push this class onto the list of all classes in the context
    context->AllClasses.PushBack(type);

    // Go through all attributes and attach them to the class type
    this->ReadAttributes(node, node->Attributes, type->Attributes);

    // Set whether we're hidden or not (hides us from documentation, auto-complete, etc)
    type->IsHidden =  type->HasAttribute(HiddenAttribute);

    // Create a new function for the pre-constructor
    String preConstructorName = String::Format("%s%s", PreConstructorName.c_str(), type->Name.c_str());
    Function* preConstructor = this->Builder->CreateRawFunction(type, preConstructorName, VirtualMachine::ExecuteNext, ParameterArray(), core.VoidType, FunctionOptions::None);
    node->PreConstructor = preConstructor;
    this->SetupFunctionLocation(preConstructor, node->Location, node->Name.Location);

    // Only generate a post-destructor if our class is a reference type
    if (type->CopyMode == TypeCopyMode::ReferenceType)
    {
      // Create the post-destructor for this class
      type->PostDestructor = VirtualMachine::PostDestructor;
    }

    // Add the function to the type's list of functions
    type->PreConstructor = node->PreConstructor;
  }

  //***************************************************************************
  void Syntaxer::CollectTemplateInstantiations(SyntaxNode*& node, ClassContext* context)
  {
    // Get all the types for this node
    SyntaxTypes types;
    node->PopulateSyntaxTypes(types);

    // Instantiate any template types
    this->InstantiateTemplatesFromSyntaxTypes(types, context, node->Location);

    // Continue walking all of this node's children
    context->Walker->GenericWalkChildren(this, node, context);
  }

  //***************************************************************************
  void Syntaxer::InstantiateTemplatesFromSyntaxTypes(SyntaxTypes& types, ClassContext* context, const CodeLocation& location)
  {
    // Loop through all the types
    for (size_t i = 0; i < types.Size(); ++i)
    {
      // Get the current type
      SyntaxType* type = *types[i];

      // CHECK THIS LOGIC
      // The first thing we need to do is traverse any sub-types
      // I believe this MUST happen before we try to instantiate templates (makes sense...)
      // otherwise we would try and instantiate a template with incorrect types

      // Get all child types of this type
      SyntaxTypes childTypes;
      type->PopulateSyntaxTypes(childTypes);

      // Traverse those child types and instantiate any templates
      // NOTE: It is very important this happens first
      // If a template type uses templates in it's instantiation (eg Array<Dictionary<Foo, Bar>>)
      // Then we must have instantiated the Dictionary before the Array can complete
      this->InstantiateTemplatesFromSyntaxTypes(childTypes, context, location);

      // If the type is a template instance
      if (type->IsTemplateInstantiation())
      {
        // If the type is a data type
        if (BoundSyntaxType* dataType = Type::DynamicCast<BoundSyntaxType*>(type))
        {
          // Get the fully qualified template name (unique for this instantiation)
          String fullyQualifiedTemplateName = dataType->ToString();

          // Get the base name of the template
          String baseName = dataType->TypeName;

          // First, look to see if we already instantiated this type...
          if (this->Builder->FindBoundType(fullyQualifiedTemplateName) == nullptr &&
              this->ExternalBoundTypes.FindValue(fullyQualifiedTemplateName, nullptr) == nullptr)
          {
            // Check to see if we found the type as a named template
            if (ClassNode* classNode = this->InternalBoundTemplates.FindValue(baseName, nullptr))
            {
              // Make sure the exact number of template arguments were given
              if (classNode->TemplateArguments.Size() != dataType->TemplateArguments.Size())
              {
                // The number of arguments -must- be the same
                return this->ErrorAt
                (
                  classNode,
                  ErrorCode::InvalidNumberOfTemplateArguments,
                  classNode->TemplateArguments.Size(),
                  dataType->TemplateArguments.Size()
                );
              }

              // Clone the class node and everything beneath it
              ClassNode* cloneTree = classNode->Clone();

              // Perform the advanced type replacement for the template
              this->PerformTemplateReplacement(cloneTree, classNode->TemplateArguments, dataType);

              // Tell the class node what type it is an instantiation of (normally null)
              cloneTree->TemplateInstantiation = dataType;

              // Change the name of the class to reflect its template type
              // The data type that we're instantiating it with should be able to represent the new name
              cloneTree->Name.Token = fullyQualifiedTemplateName;

              // Setup the class as an instantiation
              this->SetupClassInstance(cloneTree, context);

              // Add ourselves to the roots. Note this will not be traversed
              // as the walker already grabbed the child list from the root
              ZilchTodo("Make sure we traverse the added class");
              this->Tree->Root->Classes.Add(cloneTree);

              // Now we need to walk the class node we just generated,
              // in case it has other template instantiations/references
              // This isn't exactly 'safe' for the walker, but I know
              // internally how walking works and it will be fine
              context->Walker->Walk(this, cloneTree, context);
            }
            else
            {
              // The resolved type arguments
              Array<Constant> templateInstanceArguments;
              templateInstanceArguments.Resize(dataType->TemplateArguments.Size());

              // Loop through all the given template instance arguments (the actual types)
              for (size_t i = 0; i < dataType->TemplateArguments.Size(); ++i)
              {
                // Get the template argument
                SyntaxNode* templateArgument = dataType->TemplateArguments[i];
                
                // This node should either be a syntax type or a constant
                SyntaxType* templateTypeArgument = Type::DynamicCast<SyntaxType*>(templateArgument);
                ValueNode* constantArgument = Type::DynamicCast<ValueNode*>(templateArgument);

                // Attempt to retrieve it as a valid type (this could be a constant instead...)
                if (templateTypeArgument != nullptr)
                {
                  // If we got no type back, we failed to instantiate this template
                  Type* resolvedType = this->RetrieveType(templateTypeArgument, location);
                  if (resolvedType == ZilchTypeId(void))
                    return;

                  // Store the type in our array of resolved types...
                  templateInstanceArguments[i] = resolvedType;
                }
                // If this is a constant (we still have to be sure the user didn't pass a name reference in)
                else if (constantArgument != nullptr)
                {
                  // Arguments cannot be identifiers (we should probably make a separate LiteralNode)
                  if (constantArgument->Value.TokenId == Grammar::LowerIdentifier)
                    return this->ErrorAt(templateArgument, ErrorCode::TemplateArgumentsMustBeConstantsOrTypes);

                  // Resolve the node into a constant and pass that to the template
                  Constant constant = this->ValueNodeToConstant(constantArgument);
                  templateInstanceArguments[i] = constant;
                }
                // Otherwise, we have no idea what this is (maybe the user is trying to use folding?)
                else
                {
                  return this->ErrorAt(templateArgument, ErrorCode::TemplateArgumentsMustBeConstantsOrTypes);
                }
              }

              // Attempt to instantiate the template
              InstantiatedTemplate instantiatedTemplate = this->Builder->InstantiateTemplate(baseName, templateInstanceArguments, *this->Dependencies);

              // If we failed to instantiate the 
              if (instantiatedTemplate.Result == TemplateResult::FailedNameNotFound)
              {
                // If we got here, it means the user referenced a type that wasn't defined
                // Note that this is the same as in RetrieveNamedType
                return this->Errors.Raise(location, ErrorCode::ReferenceToUndefinedType, fullyQualifiedTemplateName.c_str());
              }
              // If we failed to instantiate the template due to an improper number of arguments...
              else if (instantiatedTemplate.Result == TemplateResult::FailedInvalidArgumentCount)
              {
                // The number of arguments -must- be the same
                //TODO: ExpectedArguments is not currently filled out
                return this->Errors.Raise
                (
                  location,
                  ErrorCode::InvalidNumberOfTemplateArguments,
                  instantiatedTemplate.ExpectedArguments,
                  dataType->TemplateArguments.Size()
                );
              }
              // If we failed to instantiate the template due to an argument mismatch
              else if (instantiatedTemplate.Result == TemplateResult::FailedArgumentTypeMismatch)
              {
                ZilchTodo("This error message can be made a lot better with context");
                return this->Errors.Raise(location, ErrorCode::TemplateParameterTypeMismatch);
              }
              else if (instantiatedTemplate.Result == TemplateResult::FailedInstantiatorDidNotReturnType)
              {
                // Show an error message
                return this->ErrorAt(classNode, ErrorCode::InternalError,
                  "Unable to instantiate template via callback.");
              }

              // Make sure that we got a template type back
              ErrorIf(instantiatedTemplate.Type == nullptr,
                "The template instantiator did not return a failure, but the type was not created");
            }
          }

          // We need to clear and free the nodes that are template arguments since
          // we're replacing this type with an instantiated template type
          for (size_t i = 0; i < dataType->TemplateArguments.Size(); ++i)
          {
            // Free all the template argument nodes
            // (what we're instantiating this template with)
            delete dataType->TemplateArguments[i];
          }

          // Clear out the template arguments and point it to the new class name
          dataType->TemplateArguments.Clear();
          dataType->TypeName = fullyQualifiedTemplateName;
        }
        else
        {
          // Error handling
          Error("Unhandled case: A template instance was made for a type we didn't handle");
        }
      }
    }
  }

  //***************************************************************************
  void Syntaxer::PreventDuplicateTypeNames(StringParam name, const CodeLocation& location)
  {
    // Get our own library name (for convenience)
    cstr ourLibraryName = this->Builder->GetName().c_str();

    // Prevent duplicate template type names with the internal project
    ClassNode* foundTemplate = this->InternalBoundTemplates.FindValue(name, nullptr);
    if (foundTemplate != nullptr)
      return this->Errors.Raise(location, String(), foundTemplate->Location, ErrorCode::DuplicateTypeName, name.c_str(), ourLibraryName);
    
    // Prevent duplicate type names with the internal project
    BoundType* foundInternalType = this->Builder->BoundTypes.FindValue(name, nullptr);
    if (foundInternalType != nullptr)
    {
      // Give a better error message that tells us where the class was defined
      return this->Errors.Raise(location, String(), foundInternalType->Location, ErrorCode::DuplicateTypeName, name.c_str(), ourLibraryName);
    }

    // Prevent duplicate type names that come from external libraries
    BoundType* foundExternalType = this->ExternalBoundTypes.FindValue(name, nullptr);
    if (foundExternalType != nullptr)
    {
      // Give a better error message that tells us where the class was defined
      return this->Errors.Raise(location, String(), foundExternalType->Location, ErrorCode::DuplicateTypeName, name.c_str(), foundExternalType->SourceLibrary->Name.c_str());
    }
  }

  //***************************************************************************
  void Syntaxer::PreventDuplicateMemberNames(BoundType* type, StringParam memberName, const CodeLocation& location, bool isStatic, bool isFunction)
  {
    // Make sure we don't have a property of the same name as this member
    GetterSetterMap& properties = type->GetGetterSetterMap(isStatic);
    Property* foundProperty = properties.FindValue(memberName, nullptr);
    if (foundProperty != nullptr)
      return this->Errors.Raise(location, String(), foundProperty->Location, ErrorCode::DuplicateMemberName, memberName.c_str());
    
    // Make sure we don't have a field of the same name as this member
    FieldMap& fields = type->GetFieldMap(isStatic);
    Field* foundField = fields.FindValue(memberName, nullptr);
    if (fields.ContainsKey(memberName))
      return this->Errors.Raise(location, String(), foundField->Location, ErrorCode::DuplicateMemberName, memberName.c_str());
    
    if (isFunction == false)
    {
      // Make sure we don't have a function of the same name as this member
      FunctionMultiMap& functionMap = type->GetFunctionMap(isStatic);
      FunctionArray* foundFunctions = functionMap.FindPointer(memberName);
      if (foundFunctions != nullptr && foundFunctions->Empty() == false)
      {
        // We only really want to show the location of the first function we find (it's really not necessary to show all of them...)
        return this->Errors.Raise(location, ErrorCode::DuplicateMemberName, memberName.c_str());
      }
    }
  }

  //***************************************************************************
  void Syntaxer::PreventNameHiddenBaseMembers(Member* member)
  {
    // Check if the member we're looking at is a function
    bool isFunction = (ZilchVirtualTypeId(member) == ZilchTypeId(Function));

    // Walk up base classes and make sure we have no name collisions
    bool isVirtual = member->HasAttribute(VirtualAttribute);
    bool isOverride = member->HasAttribute(OverrideAttribute);
    bool isStatic = member->IsStatic;
    String& memberName = member->Name;
    CodeLocation& location = member->Location;
    BoundType* type = member->Owner;
    BoundType* it = type->BaseType;
    while (it != nullptr)
    {
      // We only care about non-virtual properties and fields hiding other members
      if (isVirtual == false && isOverride == false)
      {
        // Make sure we don't have a property of the same name as this member
        GetterSetterMap& properties = it->GetGetterSetterMap(isStatic);
        Property* foundProperty = properties.FindValue(memberName, nullptr);
        if (foundProperty != nullptr)
          return this->Errors.Raise(location, String(), foundProperty->Location, ErrorCode::BaseClassMemberSameName, it->Name.c_str(), memberName.c_str());

        // Make sure we don't have a field of the same name as this member
        FieldMap& fields = it->GetFieldMap(isStatic);
        Field* foundField = fields.FindValue(memberName, nullptr);
        if (foundField != nullptr)
          return this->Errors.Raise(location, String(), foundField->Location, ErrorCode::BaseClassMemberSameName, it->Name.c_str(), memberName.c_str());
      }

      // We only care about functions hiding another member, but not functions hiding a function
      if (isFunction == false)
      {
        // Make sure we don't have a function of the same name as this member
        FunctionMultiMap& functionMap = it->GetFunctionMap(isStatic);
        FunctionArray* foundFunctions = functionMap.FindPointer(memberName);
        if (foundFunctions != nullptr && foundFunctions->Empty() == false)
        {
          // We only really want to show the location of the first function we find (it's really not necessary to show all of them...)
          Function* foundFunction = (*foundFunctions)[0];
          return this->Errors.Raise(location, String(), foundFunction->Location, ErrorCode::BaseClassMemberSameName, it->Name.c_str(), memberName.c_str());
        }
      }

      it = it->BaseType;
    }
  }

  //***************************************************************************
  void Syntaxer::CollectClass(ClassNode*& node, ClassContext* context)
  {
    // Error checking
    ErrorIf(node->TemplateInstantiation != nullptr,
      "We should never find any template instantiations");

    // Make sure another class or type of the same name doesn't conflict with ours
    this->PreventDuplicateTypeNames(node->Name.Token, node->Location);
    if (this->Errors.WasError)
      return;

    // If the class type is a template
    if (node->IsTemplate())
    {
      // Add this node to the named templates list
      this->InternalBoundTemplates.InsertOrError(node->Name.Token, node);

      // Unlink the parent
      node->Parent = nullptr;
      node = nullptr;
    }
    else
    {
      // Setup the class as an instance
      this->SetupClassInstance(node, context);
    }
  }

  //***************************************************************************
  void Syntaxer::CollectEnum(EnumNode*& node, ClassContext* context)
  {
    // Make sure another class or type of the same name doesn't conflict with ours
    this->PreventDuplicateTypeNames(node->Name.Token, node->Location);
    if (this->Errors.WasError)
      return;

    // Note: Most everything we do in major phases due to type references, member references, or template resolving
    // Enums are simple, and therefore can mostly be handled here (apart from inheritance and code generation)
    BoundType* type = this->Builder->AddBoundType(node->Name.Token, TypeCopyMode::ValueType, sizeof(Integer), 0);
    node->Type = type;

    // Use the comments as the class description
    String description = node->GetMergedComments();
    type->Description = description;

    // Set the class type's originating location
    type->Location = node->Location;
    type->NameLocation = node->Name.Location;

    // Push this class onto the list of all classes in the context
    context->AllClasses.PushBack(type);

    // Go through all attributes and attach them to the enum type
    this->ReadAttributes(node, node->Attributes, type->Attributes);

    // Assume this type is just a regular enum (if it's a 'flags', we will set that below)
    type->SpecialType = SpecialType::Enumeration;
    type->ToStringFunction = VirtualMachine::EnumerationToString;
    type->BaseType = ZilchTypeId(Enum);

    // The values we Assign to any unassigned enum value
    // We automatically count up in value giving each one a
    // unique value, unless the user intervenes
    Integer valueCounter = 0;

    // If this is a flags enum...
    if (node->IsFlags)
    {
      // Flags should always start with 1 (the first bit set)
      valueCounter = 1;
      type->SpecialType = SpecialType::Flags;
      type->ToStringFunction = VirtualMachine::FlagsToString;
    }

    // Make sure we don't allow two names that are the same
    HashSet<String> uniqueNames;

    // Walk through all the enum values
    for (size_t i = 0; i < node->Values.Size(); ++i)
    {
      EnumValueNode* enumValueNode = node->Values[i];

      // First, check to see if the name is already taken
      if (uniqueNames.Contains(enumValueNode->Name.Token))
      {
        // We cannot have two names are the same
        return this->ErrorAt
        (
          enumValueNode,
          ErrorCode::EnumDuplicateValue,
          enumValueNode->Name.c_str(),
          node->Name.c_str()
        );
      }

      // Since the name wasn't taken, add it to the list
      uniqueNames.Insert(enumValueNode->Name.Token);
      
      // If the user actually assigned a value to this entry
      if (enumValueNode->Value != nullptr)
      {
        // Convert the string to an integer value
        valueCounter = atoi(enumValueNode->Value->Token.c_str());
      }

      // Let the enum entry know what it's integral value is
      enumValueNode->IntegralValue = valueCounter;

      // If this is a flags enum...
      if (node->IsFlags)
      {
        // There's a special case where we have a bit flag of zero
        if (valueCounter == 0)
        {
          // Start at a value of 1
          valueCounter = 1;
        }
        else
        {
          // Flags enums jump by powers of 2 for the next enum value
          valueCounter *= 2;
        }
      }
      else
      {
        // Standard enums just increment for the next enum value
        ++valueCounter;
      }

      // Create the property using the builder (auto added to our type)
      Property* property = this->Builder->AddEnumValue
      (
        type,
        enumValueNode->Name.Token,
        enumValueNode->IntegralValue
      );
      enumValueNode->IntegralProperty = property;

      // Set the documentation on the property to be any comments on the value, as well as the locations
      property->Description = enumValueNode->GetMergedComments();
      property->Location = enumValueNode->Location;
      property->NameLocation = enumValueNode->Name.Location;
    }
  }

  //***************************************************************************
  void Syntaxer::CollectSendsEvents(SendsEventNode*& node, TypingContext* context)
  {
    // Let the type know which events are sent
    BoundType* classType = context->ClassTypeStack.Back();

    // Use the comments as the event description
    String comments = node->GetMergedComments();
    BoundType* sentType = this->RetrieveBoundType(node->EventType, node->Location);
    SendsEvent& sendsEvent = *this->Builder->AddSendsEvent(classType, node->Name->Token, sentType, comments);

    // Set the documentation on the property to be any comments on the value, as well as the locations
    sendsEvent.Description = node->GetMergedComments();
    sendsEvent.Location = node->Location;
    sendsEvent.NameLocation = node->Name->Location;

    // Go through all attributes and attach them to the sends
    this->ReadAttributes(node, node->Attributes, sendsEvent.Attributes);

    // If we encountered an error with getting a type back, early out
    if (this->Errors.WasError)
      return;
    
    // Set the location where this property originates from
    sendsEvent.EventProperty->Location = node->Location;
    sendsEvent.EventProperty->NameLocation = node->Name->Location;
  }

  //***************************************************************************
  void Syntaxer::CollectClassInheritance(ClassNode*& classNode, TypingContext* context)
  {
    // Walk the class and it's children
    this->PushClass(classNode, context);
    
    // Iterate through all inherited tokens
    for (SyntaxTypeList::range i = classNode->Inheritance.All(); !i.Empty(); i.PopFront())
    {
      // Grab the current token
      SyntaxType* syntaxType = i.Front();

      // Get the actual inherited type
      Type* inheritedType = this->RetrieveType(syntaxType, classNode->Location);

      // If we had an error, return out early
      if (this->Errors.WasError)
        return;

      // Error checking!
      ErrorIf(inheritedType == nullptr, "Type should be valid");

      // If the type is a class type...
      if (BoundType* boundType = Type::DynamicCast<BoundType*>(inheritedType))
      {
        // If we have no base class...
        if (classNode->Type->BaseType == nullptr)
        {
          // Store the base type
          classNode->Type->BaseType = boundType;
        }
        else
        {
          // We don't support multiple inheritance
          return this->ErrorAt(classNode, ErrorCode::MultipleInheritanceNotSupported);
        }
      }
      else
      {
        // We only support inheriting from classes (bound types) and interfaces (which we don't have yet!)
        return this->ErrorAt(classNode, ErrorCode::GenericError,
          "We don't yet support inheriting from types other than class/struct/bound types.");
      }
    }
  }

  //***************************************************************************
  void Syntaxer::CollectEnumInheritance(EnumNode*& node, TypingContext* context)
  {
    // Note: We don't really care about walking children here (our only children are enum values)

    // If we have inheritance...
    if (node->Inheritance)
    {
      // Right now we don't support enum inheritance, but we will in the future
        return this->ErrorAt(node, ErrorCode::GenericError,
          "Enum inheritance is not yet supported.");

      //// Get the actual inherited type
      //Type* inheritedType = this->RetrieveType(node->Inheritance, node->Location);

      //// If we had an error, return out early
      //if (this->Errors.WasError)
      //  return;

      //// Error checking!
      //ErrorIf(inheritedType == nullptr, "Type should be valid");
    }
  }

  //***************************************************************************
  void Syntaxer::SetupGenericFunction(GenericFunctionNode* node, TypingContext* context, const UserToken& name, FunctionOptions::Enum options, Type* returnType, BoundType* owner)
  {
    // If an owner was not passed in, then get a pointer to the current class type that this function is being implemented in
    // Owner can be passed in when we're binding an extension function
    if (owner == nullptr)
      owner = context->ClassTypeStack.Back();

    // Store all the delegate parameters
    ParameterArray delegateParameters;

    // If an error occurred, exit out
    if (this->Errors.WasError)
      return;

    // Walk through parameters
    NodeList<ParameterNode>::range parameters = node->Parameters.All();
    for (; parameters.Empty() == false; parameters.PopFront())
    {
      // Get the current parameter
      ParameterNode* parameter = parameters.Front();

      // Add the parameter type to the signature (get the type from the parameter node)
      DelegateParameter& delegateParameter = delegateParameters.PushBack();
      delegateParameter.Name = parameter->Name.Token;
      
      // Get the variable's type from the syntax type
      delegateParameter.ParameterType = this->RetrieveType(parameter->ResultSyntaxType, node->Location);
    }

    // Set the function on the node
    Function* function = this->Builder->CreateRawFunction(owner, name.Token, VirtualMachine::ExecuteNext, delegateParameters, returnType, options);
    node->DefinedFunction = function;
    this->SetupFunctionLocation(function, node->Location, name.Location);

    // Push the function onto the stack so that children can access it
    // (the top of the stack will be the most relevant function to them)
    context->FunctionStack.PushBack(function);
    
    // Walk all the parameters (which should give them types and generate variables)
    context->Walker->Walk(this, node->Parameters, context);

    // Mark the node as virtual if it is
    function->IsVirtual = (options == FunctionOptions::Virtual);

    // Use the comments as the class description
    String description = node->GetMergedComments();
    function->Description = description;

    // Go through all attributes and attach them to the function
    this->ReadAttributes(node, node->Attributes, function->Attributes);

    // Set whether we're hidden or not (hides us from documentation, auto-complete, etc)
    function->IsHidden =  function->HasAttribute(HiddenAttribute);

     // If the function is a member function (not a static function)...
    if (options != FunctionOptions::Static)
    {
      // Get the scoped variable from the map
      // Note that it should probably be null (we're just getting a reference to the pointer)
      Variable*& thisVariable = node->ScopedVariables[ThisKeyword];

      // If the variable is not null, that means we somehow ended up with a 'this' variable already defined
      if (thisVariable != nullptr)
        return ErrorAt(node, ErrorCode::InternalError,
        "The variable 'this' was already defined.");

      // Adding the function should have created a this variable (since we're not static)
      thisVariable = function->This;
    }

    // Pop the function from the stack
    context->FunctionStack.PopBack();

    // Finally, set the node type so that we'll know it later
    node->Type = node->DefinedFunction->FunctionType;
  }

  //***************************************************************************
  void Syntaxer::CollectConstructor(ConstructorNode*& node, TypingContext* context)
  {
    // Get the instance of the type database
    Core& core = Core::GetInstance();
    
    // Setup the function generically (this actually creates the compiled function object)
    this->SetupGenericFunction(node, context, node->Name, FunctionOptions::None, core.VoidType);

    // Grab the class type
    BoundType* classType = context->ClassTypeStack.Back();

    Function* constructor = node->DefinedFunction;
    AddMemberResult::Enum addResult = classType->AddRawConstructor(constructor);

    // If an overload of the function already existed with the exact same signature (and name)
    if (addResult == AddMemberResult::AlreadyExists)
      return ErrorAt(node, ErrorCode::OverloadsCannotBeTheSame, constructor->Name.c_str());

    // If we have a base class type but we aren't initializing it...
    BoundType* baseType = classType->BaseType;
    if (baseType != nullptr && node->BaseInitializer == nullptr)
    {
      // It is an error to not initialize our base class
      return ErrorAt(node, ErrorCode::BaseInitializerRequired, classType->Name.c_str(), baseType->Name.c_str());
    }
  }

  //***************************************************************************
  void Syntaxer::CollectDestructor(DestructorNode*& node, TypingContext* context)
  {
    // Get the instance of the type database
    Core& core = Core::GetInstance();

    // Setup the function generically (this actually creates the compiled function object)
    this->SetupGenericFunction(node, context, node->Name, FunctionOptions::None, core.VoidType);

    // Add the destructor function to the class
    context->ClassTypeStack.Back()->Destructor = node->DefinedFunction;
  }

  //***************************************************************************
  void Syntaxer::CollectFunction(FunctionNode*& node, TypingContext* context)
  {
    // Store the return type
    Type* delegateReturn = nullptr;

    // Loop through all the return values
    if (node->ReturnType != nullptr)
    {
      // Add the return type to the signature
      //HACK (needs to be actually at the return node, maybe syntax types should get locations?)
      delegateReturn = this->RetrieveType(node->ReturnType, node->Location);

      // If we had an error, return out early
      if (this->Errors.WasError)
        return;
    }
    else
    {
      // Get the instance of the type database
      Core& core = Core::GetInstance();

      // There is no return type
      delegateReturn = core.VoidType;
    }

    // Assume no options are on this function
    FunctionOptions::Enum options = FunctionOptions::None;
    ErrorIf(node->IsStatic && node->Virtualized != VirtualMode::NonVirtual,
      "A function cannot be both static and virtual");

    // Static and virtual are mutually exclusive
    if (node->IsStatic)
      options = FunctionOptions::Static;
    else if (node->Virtualized != VirtualMode::NonVirtual)
      options = FunctionOptions::Virtual;

    // Assume the owner of this function is always the class itself
    BoundType* owner = context->ClassTypeStack.Back();

    // If this is an extension method...
    if (node->ExtensionOwner != nullptr)
      owner = this->RetrieveBoundType(node->ExtensionOwner, node->Location);

    // If we had an error getting the extension type, return out early
    if (this->Errors.WasError)
      return;

    // Make sure we don't have any other members of the same name
    this->PreventDuplicateMemberNames(owner, node->Name.Token, node->Location, node->IsStatic, true);
    if (this->Errors.WasError)
      return;

    // Setup the function generically (this actually creates the compiled function object)
    this->SetupGenericFunction(node, context, node->Name, options, delegateReturn, owner);

    // If we had an error, return out early
    if (this->Errors.WasError)
      return;

    // Grab the function that was created above in 'SetupGenericFunction'
    Function* function = node->DefinedFunction;

    // If this is a normal function (not an extension function...)
    if (node->ExtensionOwner == nullptr)
    {
      // Add the function to the type's list of functions (checks for static or instance methods)
      AddMemberResult::Enum addResult = owner->AddRawFunction(function);

      // If an overload of the function already existed with the exact same signature (and name)
      if (addResult == AddMemberResult::AlreadyExists)
        return ErrorAt(node, ErrorCode::OverloadsCannotBeTheSame, function->Name.c_str());
    }
    // Otherwise, this is an extension method, so add it specially via the library
    else
    {
      ZilchTodo("Handle extension function overload collisions");
      this->Builder->AddRawExtensionFunction(function);
    }
  }

  //***************************************************************************
  void Syntaxer::CollectMemberVariableAndProperty(MemberVariableNode*& node, TypingContext* context)
  {
    // As long as the type is not inferred...
    ErrorIf(node->IsInferred(),
      "Member variables should never have an inferred type (should be checked by the parser)");

    // Store the type on the node (this helps us with byte-code generation later)
    // Essentially, we are decorating the tree :)
    node->ResultType = this->RetrieveType(node->ResultSyntaxType, node->Location);

    // If we had an error, return out early
    if (this->Errors.WasError)
      return;

    // Set the member node's parent class type
    BoundType* classType = context->ClassTypeStack.Back();
    node->ParentClassType = classType;

    // Make sure we don't have any other members of the same name
    this->PreventDuplicateMemberNames(classType, node->Name.Token, node->Location, node->IsStatic, false);
    if (this->Errors.WasError)
      return;

    // Any options we want to apply to the creation of a member
    MemberOptions::Flags options = MemberOptions::None;
      
    // If the variable is a static variable we need to set the option
    if (node->IsStatic)
    {
      options |= MemberOptions::Static;
    }
    
    // The property we create below (members are properties...)
    Property* property = nullptr;

    // If this node is a property...
    if (node->IsGetterSetter)
    {
      GetterSetter* getset = nullptr;

      // The function we use to tell the library builder to ignore it (we will build it later)
      BoundFn doNotGenerate = LibraryBuilder::DoNotGenerate;

      // If this is a normal class property...
      if (node->ExtensionOwner == nullptr)
      {
        // Create the property using the builder (this adds it directly to the class)
        getset = this->Builder->AddBoundGetterSetter(classType, node->Name.Token, node->ResultType, doNotGenerate, doNotGenerate, options);
      }
      else
      {
        // We have to lookup the extension type that the user specified (this could fail)
        BoundType* extensionOwner = this->RetrieveBoundType(node->ExtensionOwner, node->Location);

        // If we had a problem resolving the type, then early out
        if (this->Errors.WasError)
          return;
        
        // Create the property using the builder (this adds it to the library but maps it to the owner)
        getset = this->Builder->AddExtensionGetterSetter(extensionOwner, node->Name.Token, node->ResultType, doNotGenerate, doNotGenerate, options);
      }

      // Store the property variable on the node
      node->CreatedGetterSetter = getset;
      property = getset;
    }
    else
    {
      // If this is a value type (struct) and the member is non-static...
      if (classType->CopyMode == TypeCopyMode::ValueType && node->IsStatic == false)
      {
        // If this member has a complex copy
        if (node->ResultType->IsCopyComplex())
        {
          // A struct is only allowed to contain value types
          return ErrorAt
          (
            node,
            ErrorCode::StructsCanOnlyContainValueTypes,
            classType->Name.c_str(),
            node->Name.c_str()
          );
        }
      }

      // Create the member using the builder
      // The given offset is zero for now (we'll initialize it later when collecting sizes of objects)
      Field* field = this->Builder->AddBoundField(classType, node->Name.Token, node->ResultType, 0, options);
      property = field;

      // Store the member variable on the node
      node->CreatedField = field;
    }

    node->CreatedProperty = property;

    // Use the comments as the class description
    String description = node->GetMergedComments();
    property->Description = description;
    
    // Go through all attributes and attach them to the property
    this->ReadAttributes(node, node->Attributes, property->Attributes);

    // Set whether we're hidden or not (hides us from documentation, auto-complete, etc)
    property->IsHidden = property->HasAttribute(HiddenAttribute);

    // Set the location of the property
    property->Location = node->Location;
    property->NameLocation = node->Name.Location;
  }

  //***************************************************************************
  void Syntaxer::CollectPropertyGetSet(MemberVariableNode*& node, TypingContext* context)
  {
    // Note: The first thing we must do is walk the Get/Set functions since whatever member
    //       we create, we want to give it the proper get and set functions

    // If the member has a get
    if (node->Get != nullptr)
    {
      // Walk the get function
      context->Walker->Walk(this, node->Get, context);

      // If an error occurred, exit out
      if (this->Errors.WasError)
        return;

      // Mark this function as special
      node->Get->DefinedFunction->OwningProperty = node->CreatedGetterSetter;
      node->Get->DefinedFunction->IsHidden = true;
    }

    // If the member has a set
    if (node->Set != nullptr)
    {
      // Walk the get function
      context->Walker->Walk(this, node->Set, context);

      // If an error occurred, exit out
      if (this->Errors.WasError)
        return;

      // Mark this function as special
      node->Set->DefinedFunction->OwningProperty = node->CreatedGetterSetter;
      node->Set->DefinedFunction->IsHidden = true;
    }
    
    // If this node is a property...
    if (node->IsGetterSetter)
    {
      // Store the get function on the property
      if (node->Get != nullptr)
      {
        // Error checking
        ErrorIf(node->CreatedGetterSetter->Get != nullptr,
          "The property getter should not be set yet (we're generating it)");

        // Point the property's getter at the compiled function
        node->CreatedGetterSetter->Get = node->Get->DefinedFunction;
      }
      
      // Store the set function on the property
      if (node->Set != nullptr)
      {
        // Error checking
        ErrorIf(node->CreatedGetterSetter->Set != nullptr,
          "The property setter should not be set yet (we're generating it)");

        // Point the property's setter at the compiled function
        node->CreatedGetterSetter->Set = node->Set->DefinedFunction;
      }
    }
  }

  //***************************************************************************
  void Syntaxer::PushClass(ClassNode*& node, TypingContext* context)
  {
    // Push the class onto the stack so that children can access it
    // (the top of the stack will be the most relevant class to them)
    context->ClassTypeStack.PushBack(node->Type);

    // Let any member initialization generation know what function it belongs to
    context->FunctionStack.PushBack(node->PreConstructor);

    // Generically walk the children
    context->Walker->GenericWalkChildren(this, node, context);

    // Pop the pre-constructor
    context->FunctionStack.PopBack();

    // We are exiting this class, so pop it off
    context->ClassTypeStack.PopBack();
  }

  //***************************************************************************
  template <typename FunctionNodeType>
  void Syntaxer::PushFunctionHelper
  (
    FunctionNodeType* node,
    TypingContext* context,
    void (Syntaxer::*postArgs)(FunctionNodeType* node)
  )
  {
    // Push the function onto the stack so that children can access it
    // (the top of the stack will be the most relevant function to them)
    context->FunctionStack.PushBack(node->DefinedFunction);

    // Loop through all the parameters
    for (size_t i = 0; i < node->Parameters.Size(); ++i)
    {
      // Walk the statements
      ParameterNode* parameterNode = node->Parameters[i];
      context->Walker->Walk(this, parameterNode, context);

      // If an error occurred, exit out
      if (this->Errors.WasError)
        return;
    }

    // If we have a post arguments function...
    if (postArgs != nullptr)
    {
      // Call the function...
      (this->*postArgs)(node);

      // If an error occurred, exit out
      if (this->Errors.WasError)
        return;
    }

    // Process the statements inside the function
    this->ProcessScopeStatements(node, context);

    // If an error occurred, exit out
    if (this->Errors.WasError)
      return;

    // Get a reference to the core library
    Core& core = Core::GetInstance();

    // If not all paths return and we have a return type...
    if (node->AllPathsReturn == false && Type::IsSame(node->Type->Return, core.VoidType) == false)
    {
      // No child statement reported to us that it returned properly
      return ErrorAt(node, ErrorCode::NotAllPathsReturn);
    }

    // We are exiting this function, so pop it off
    context->FunctionStack.PopBack();
  }

  //***************************************************************************
  void Syntaxer::PushFunction(GenericFunctionNode*& node, TypingContext* context)
  {
    return PushFunctionHelper<GenericFunctionNode>(node, context, nullptr);
  }

  //***************************************************************************
  void Syntaxer::CheckAndPushFunction(FunctionNode*& node, TypingContext* context)
  {
    this->PreventNameHiddenBaseMembers(node->DefinedFunction);
    if (this->Errors.WasError)
      return;
    return PushFunctionHelper<GenericFunctionNode>(node, context, nullptr);
  }

  //***************************************************************************
  void Syntaxer::PushConstructor(ConstructorNode*& node, TypingContext* context)
  {
    return PushFunctionHelper<ConstructorNode>(node, context, &Syntaxer::CheckInitializerList);
  }

  //***************************************************************************
  void Syntaxer::CheckInitializerList(ConstructorNode* node)
  {
    ZilchTodo("Finish up initializer lists (at least checking if they pass type validation)");
    //DecorateCheckFunctionCall(node, node->BaseInitializer,
    //node->BaseInitializer
  }

  //***************************************************************************
  void Syntaxer::DecorateInitializer(InitializerNode*& node, TypingContext* /*context*/)
  {
    // Mark the node as being read only (we should not be able to change it)
    node->Io = IoMode::ReadRValue;
  }

  //***************************************************************************
  void Syntaxer::DecorateStringInterpolant(StringInterpolantNode*& node, TypingContext* context)
  {
    // Mark the node as being read only
    node->Io = IoMode::ReadRValue;

    // String interpolants always result in a string type
    node->ResultType = ZilchTypeId(String);

    // Walk through all the children we want to stringify
    for (size_t i = 0; i < node->Elements.Size(); ++i)
    {
      // Get the current element (it may be a string itself...)
      ExpressionNode* elementNode = node->Elements[i];

      // Make sure we walk the element expression
      context->Walker->Walk(this, elementNode, context);

      // If we had an error, return out early
      if (this->Errors.WasError)
        return;
      
      // The element expression only needs to be readable
      elementNode->IoUsage = IoMode::ReadRValue;
    }
  }

  //***************************************************************************
  void Syntaxer::DecorateValue(ValueNode*& node, TypingContext* /*context*/)
  {
    // Mark the node as being read only
    node->Io = IoMode::ReadRValue;

    // Get the instance of the type database
    Core& core = Core::GetInstance();

    // Check to see what type of literal we have here
    switch (node->Value.TokenId)
    {
      // The value is an Integer
      case Grammar::IntegerLiteral:
      {
        node->ResultType = core.IntegerType;
        break;
      }

      // The value is a DoubleInteger
      case Grammar::DoubleIntegerLiteral:
      {
        node->ResultType = core.DoubleIntegerType;
        break;
      }

      // The value is a Real
      case Grammar::RealLiteral:
      {
        node->ResultType = core.RealType;
        break;
      }

      // The value is a DoubleReal
      case Grammar::DoubleRealLiteral:
      {
        node->ResultType = core.DoubleRealType;
        break;
      }

      // The value is a String
      case Grammar::StringLiteral:
      {
        node->ResultType = core.StringType;
        break;
      }

      // The value is a Bool
      case Grammar::True:
      case Grammar::False:
      {
        node->ResultType = core.BooleanType;
        break;
      }

      // The value is a null (which means that the type is unknown)
      case Grammar::Null:
      {
        // Always assume we're going to be a null handle (implicit cast may change us!)
        node->ResultType = core.NullType;
        break;
      }

      default:
      {
        // We don't know what type it is???
        // This especially should not be an identifier, since identifiers are caught as VariableReferences
        return ErrorAt(node, ErrorCode::InternalError,
          "We reached what should be a literal value and we have no idea what type it is.");
      }
    }
  }

  //***************************************************************************
  void SetAllIoToReadAndIgnore(SyntaxNode* node)
  {
    // If the current node is an expression...
    ExpressionNode* expression = Type::DynamicCast<ExpressionNode*>(node);
    if (expression != nullptr)
    {
      // Set the io and usage (technically usage should only be set by the parent, but this is a special case)
      expression->Io = IoMode::ReadRValue;
      expression->IoUsage = IoMode::Ignore;
    }

    // Get all this node's children
    NodeChildren children;
    node->PopulateChildren(children);

    // Loop through all the children and recursively set all their nodes to read only
    for (size_t i = 0; i < children.Size(); ++i)
      SetAllIoToReadAndIgnore(*children[i]);
  }

  //***************************************************************************
  void Syntaxer::DecorateExpressionInitializer(ExpressionInitializerNode*& node, TypingContext* context)
  {
    // Mark the node as being read only (we don't ever write directly to this value on the stack)
    // Note that doesn't mean we can't access a value on the type and write to it
    node->Io = IoMode::ReadRValue;

    // Mark all child nodes as using ignoring io access (we know this is generated by us, so it's ok/valid)
    // This should really come before we do anything else, just because member initialization does actually need to write
    SetAllIoToReadAndIgnore(node);
    
    // Walk the creation initializer (create the node, invoke the constructor...)
    context->Walker->Walk(this, node->LeftOperand, context);

    // If we had an error, return out early
    if (this->Errors.WasError)
      return;

    // Our type is the left operands type
    node->ResultType = node->LeftOperand->ResultType;

    // The creation expression only needs to be readable (always should be!)
    node->LeftOperand->IoUsage = IoMode::ReadRValue;
    
    // Walk the statements that generate the 'Add' and member initializations
    context->Walker->Walk(this, node->InitializerStatements, context);
  }

  //***************************************************************************
  void Syntaxer::DecorateStaticTypeOrCreationCall(StaticTypeNode*& node, TypingContext* context)
  {
    // Mark the node as being read only
    // Note that doesn't mean we can't access a value on the type and write to it
    node->Io = IoMode::ReadRValue;

    // We only grab this so we can provide the user with better error feedback
    BoundType* classWereInsideOf = context->ClassTypeStack.Back();

    // Get the type that is referenced by the syntax type (the type we're possibly creating, or accessing statics upon)
    node->ReferencedType = this->RetrieveBoundType(node->ReferencedSyntaxType, node->Location, classWereInsideOf);
    
    if (this->Errors.WasError)
      return;

    // For the member access case and inferred creation call, this is
    // always correct (the result of the expression is the same as the named type
    // However: In the case where we are using 'new' on a value type, the result is a reference type
    // Warning: If this is a static member access, the type is NOT an instance, but is static (we don't actually return a value)
    node->ResultType = node->ReferencedType;
    
    // Get the copy mode of the type we're possibly creating (reference or value type)
    TypeCopyMode::Enum copyMode = node->ReferencedType->CopyMode;

    // This could be a creation call or a member access
    FunctionCallNode* constructorCall = Type::DynamicCast<FunctionCallNode*>(node->Parent);
    MemberAccessNode* memberAccess = Type::DynamicCast<MemberAccessNode*>(node->Parent);

    // Determine right here if this is a creation call... (we MUST be the left operand of our parent in this case)
    // For example, we could be passed as a parameter in code that should NOT compile, eg SomeCall(String)
    if (constructorCall != nullptr && node == constructorCall->LeftOperand)
    {
      // Check if this is an inferred node
      if (node->Mode == CreationMode::Invalid)
      {
        // Because it's a reference type, we infer the 'new'
        if (copyMode == TypeCopyMode::ReferenceType)
          node->Mode = CreationMode::New;
        // Because it's a value type, we infer the 'local'
        else
          node->Mode = CreationMode::Local;
      }
    }
    // Otherwise, this should be a member access...
    else if (memberAccess != nullptr)
    {
      // If we're trying to do a member access on a new/local node, this would never make sense
      if (node->Mode != CreationMode::Invalid)
        return this->ErrorAt(node, ErrorCode::ConstructorCannotAccessStaticMembers);
    }
    // Otherwise the user is trying to do something with the node that we don't understand
    else if (this->Errors.TolerantMode == false)
    {
      if (node->Mode != CreationMode::Invalid)
        return this->ErrorAt(node, ErrorCode::ConstructorCallNotFound);
      else
        return this->ErrorAt(node, ErrorCode::StaticTypeConstructorOrAccessNotFound);
    }

    // Note: We do this regardless of this being a construction node because auto-complete uses it
    // Walk up the base class chain until we find any constructors (we inherit constructors)
    // We start with the current class we're trying to create
    // Note: We can safely look up to our base classes because if we don't have a constructor
    // then we at least have been pre-constructed, and the user opted to not initialize anything with a constructor
    node->OverloadedConstructors = node->ReferencedType->GetOverloadedInheritedConstructors();

    // If this is being used as a creation node (may have been determined above, or by an explicit new/local)
    if (node->Mode != CreationMode::Invalid)
    {
      // If the type has a flag that stops itself from being created in script...
      // Let the user know that this type can't be created (even if it has a constructor!)
      if (node->ReferencedType->CreatableInScript == false)
        return this->ErrorAt(node, ErrorCode::CannotCreateType, node->ReferencedType->ToString().c_str());

      // If we're doing a call to 'new', meaning we're making a heap object and its a value type then we know it must be a ref...
      if (node->Mode == CreationMode::New && copyMode == TypeCopyMode::ValueType)
      {
        // The result is a reference (indirection) to the created value type
        node->ResultType = this->Builder->ReferenceOf(node->ReferencedType);
      }
      // Otherwise, we're making a stack local or member
      else if (node->Mode == CreationMode::Local)
      {
        // If this type is a reference type...
        if (copyMode == TypeCopyMode::ReferenceType)
        {
          // We can only use local to create value types
          return this->ErrorAt(node, ErrorCode::LocalCreateMustBeValueType, node->ReferencedType->ToString().c_str());
        }
      }
    }
  }
  
  //***************************************************************************
  void Syntaxer::DecorateMultiExpression(MultiExpressionNode*& node, TypingContext* context)
  {
    // If the yield index was never set, this is an internal error!
    // (also handles invalid, because invalid is greater than size!)
    if (node->YieldChildExpressionIndex > node->Expressions.Size())
    {
      return this->ErrorAt
      (
        node,
        ErrorCode::InternalError,
        "YieldChildExpressionIndex was not properly set on MultiExpressionNode."
      );
    }

    // Loop through all the child expressions
    for (size_t i = 0; i < node->Expressions.Size(); ++i)
    {
      // Process all child expressions (this should compute their IO and types)
      ExpressionNode* expression = node->Expressions[i];
      context->Walker->Walk(this, expression, context);

      // Unfortunately, we need to actually forward the IoUsage of whoever was using our node
      // however we don't know, because typically it has yet to be set
      // Right now, we set that we require only read access to the node (because we only use this internally)
      // This means if we try to yield something that is write only, then we will get an error
      expression->IoUsage = (IoMode::Enum)(IoMode::ReadRValue);
    }
    
    // Forward all parameters that the syntaxer typically handles to the yielded expression
    ExpressionNode* yieldedExpression = node->Expressions[node->YieldChildExpressionIndex];
    node->ResultType = yieldedExpression->ResultType;
    node->Io = yieldedExpression->Io;
    node->IsUsedAsStatement = yieldedExpression->IsUsedAsStatement;
  }
  
  //***************************************************************************
  void Syntaxer::DecorateTypeId(TypeIdNode*& node, TypingContext* context)
  {
    // Mark the node as being read only
    // Note that does NOT mean we can't access a value on the type and write to it
    node->Io = IoMode::ReadRValue;

    // If we're attempting to get a the type of an expression
    if (node->Value != nullptr)
    {
      // Process the value expression
      context->Walker->Walk(this, node->Value, context);

      // The value expression only needs to be readable
      node->Value->IoUsage = IoMode::ReadRValue;

      // Get the resulting type of the expression
      node->CompileTimeType = node->Value->ResultType;
    }
    // Otherwise, we're getting the typeid of a static type
    else
    {
      // Error checking
      ErrorIf(node->CompileTimeSyntaxType == nullptr, "If the value wasn't parsed, the type should have been!");

      // Just get the type of what they passed in
      node->CompileTimeType = this->RetrieveType(node->CompileTimeSyntaxType, node->Location);
      
      // If we weren't able to determine the type, then we already threw an error
      if (this->Errors.WasError)
        return;
    }

    // By default the typeid will always something that derives from 'Type'
    // If this is an any type, then we could technically return anything (unless this is a static typeid(any))
    if (Type::IsAnyType(node->CompileTimeType) && node->Value != nullptr)
      node->ResultType = ZilchTypeId(Type);
    else
      node->ResultType = ZilchVirtualTypeId(node->CompileTimeType);
  }

  //***************************************************************************
  void Syntaxer::DecorateMemberId(MemberIdNode*& node, TypingContext* context)
  {
    // Mark the node as being read only
    // Note that does NOT mean we can't access a value on the property and write to it
    node->Io = IoMode::ReadRValue;

    // Process the member expression
    if (node->Member != nullptr)
    {
      MemberAccessNode* memberNode = node->Member;
      context->Walker->Walk(this, memberNode, context);

      // We actually don't even care about reading the value because we will not be generating code for it
      // But we set this regardless to avoid asserts later
      node->Member->IoUsage = IoMode::ReadRValue;

      // Get the type of member this represents
      Member* member = memberNode->AccessedMember;
      if (member != nullptr)
      {
        node->ResultType = ZilchVirtualTypeId(member);
      }
      else
      {
        // It must be an overloaded function then
        return this->ErrorAt(node, ErrorCode::MemberIdOverloadedFunctionsMustBeDisambiguated);
      }
    }
  }

  //***************************************************************************
  void Syntaxer::CheckMemberVariable(MemberVariableNode*& node, TypingContext* context)
  {
    // Get the initial value expression
    ExpressionNode* initialValue = node->InitialValue;

    // Error checking
    ErrorIf(node->IsGetterSetter && initialValue,
      "Properties should never have initial values");

    // Make sure we don't have any base class members of the same name
    Member* createdMember = node->CreatedProperty;
    if(createdMember != nullptr)
      this->PreventNameHiddenBaseMembers(createdMember);

    if (this->Errors.WasError)
      return;

    // If we have an initial value...
    if (initialValue != nullptr)
    {
      // Process the initial value expression
      context->Walker->Walk(this, initialValue, context);

      // If an error occurred, exit out
      if (this->Errors.WasError)
        return;

      // The initial value only needs to be readable
      initialValue->IoUsage = IoMode::ReadRValue;

      // Check if the types match, or if we can implicitly convert one to our resulting type
      if (this->ImplicitConvertAfterWalkAndIo(node->InitialValue, node->ResultType) == false)
      {
        // The expression assigned to the variable was not of the same type as the variable
        return ErrorAt
        (
          node, ErrorCode::VariableTypeMismatch,
          node->Name.c_str(),
          node->ResultType->ToString().c_str(),
          initialValue->ResultType->ToString().c_str()
        );
      }
    }
    
    // Process the get function if we have it
    if (node->Get != nullptr)
    {
      context->Walker->Walk(this, node->Get, context);
    }
    
    // Process the set function if we have it
    if (node->Set != nullptr)
    {
      context->Walker->Walk(this, node->Set, context);
    }
  }

  //***************************************************************************
  void Syntaxer::CheckLocalVariable(LocalVariableNode*& node, TypingContext* context)
  {
    // Note: This extends to both local variables AND parameters
    //       (hence checking for a possibly missing initial value)

    // Get the initial value expression
    ExpressionNode* initialValue = node->InitialValue;

    // If we have an initial value...
    if (initialValue != nullptr)
    {
      // Process the initial value expression
      context->Walker->Walk(this, initialValue, context);

      // If an error occurred, exit out
      if (this->Errors.WasError)
        return;

      // The initial value only needs to be readable
      initialValue->IoUsage = IoMode::ReadRValue;
    }

    // Find the first scope that we're within (where we add our variable to)
    ScopeNode* immediateScope = nullptr;
    SyntaxNode* currentParent = node->Parent;

    // Loop through our scope parents until we hit the root
    do
    {
      // Grab the current scope
      ScopeNode* currentScope = Type::DynamicCast<ScopeNode*>(currentParent);

      // If the parent was a scope...
      if (currentScope != nullptr)
      {
        // If we have no immediate scope yet, then set it (this will always be set to the first one we hit)
        if (immediateScope == nullptr)
          immediateScope = currentScope;

        // Make sure we have no variables defined in our parent scope
        // (or any parent of that) with the same exact name
        if (currentScope->ScopedVariables.ContainsKey(node->Name.Token))
        {
          // We encountered a variable of the same name
          return ErrorAt(node, ErrorCode::DuplicateLocalVariableName, node->Name.c_str());
        }
      }

      // Get our parent's parent as a scope node
      currentParent = currentParent->Parent;
    }
    while (currentParent != nullptr);

    // Add the variable to the current function that we're working on
    Function* function = context->FunctionStack.Back();
    Variable* variable = this->Builder->CreateRawVariable(function, node->Name.Token);
    variable->NameLocation = node->Name.Location;
    variable->Location = node->Location;

    // Use the comments as the variable description
    String description = node->GetMergedComments();
    variable->Description = description;

    // Go through all attributes and attach them to the variable
    this->ReadAttributes(node, node->Attributes, variable->Attributes);
    
    // Try to Insert the variable into the scope, but if we fail...
    // There should NEVER be an error here since we checked above (or something really bad happened)
    immediateScope->ScopedVariables.InsertOrError(variable->Name, variable);

    // Store the variable information
    node->CreatedVariable = variable;

    // Get the instance of the type database
    Core& core = Core::GetInstance();

    // If the variable's type is inferred...
    if (node->IsInferred() == true)
    {
      // Error checking
      ErrorIf(initialValue == nullptr, "The initial value of a local variable was null");
      ErrorIf(initialValue->ResultType == nullptr, "The initial value's type was null");
      
      // Set the variable's type to the initial value's type
      variable->ResultType = initialValue->ResultType;
    }
    else
    {
      // Get the variables type from the syntax type
      variable->ResultType = this->RetrieveType(node->ResultSyntaxType, node->Location);

      // If we weren't able to determine the type, then we already threw an error
      if (this->Errors.WasError)
        return;

      // If we have an initial value...
      if (initialValue != nullptr)
      {
        // We need to check for cases where the result types could be null
        if (this->Errors.TolerantMode)
        {
          // If the variable was not able to resolve its type...
          if (variable->ResultType == nullptr)
          {
            // Lets just try and assume the type will be our initial value type, if it works?
            if (initialValue->ResultType != nullptr)
            {
              // This is actually similar to how we infer local variable types
              variable->ResultType = initialValue->ResultType;
            }
            else
            {
              // Otherwise, we have nothing better to assume this type is
              // Just make it void so it won't crash later
              variable->ResultType = core.ErrorType;
            }
          }

          // If for some reason the initial value has no type...
          if (initialValue->ResultType == nullptr)
          {
            // Lets assume the initial value type will be the variable's type!
            initialValue->ResultType = variable->ResultType;
          }
        }

        // Check if the types match, or if we can implicitly convert one to our resulting type
        if (this->ImplicitConvertAfterWalkAndIo(node->InitialValue, variable->ResultType) == false)
        {
          // The expression assigned to the variable was not of the same type as the variable
          return ErrorAt
          (
            node, ErrorCode::VariableTypeMismatch,
            variable->Name.c_str(),
            variable->ResultType->ToString().c_str(),
            initialValue->ResultType->ToString().c_str()
          );
        }
      }
    }

    // We treat local variables as expressions, so they must output their resulting type (just the type of the variable)
    node->ResultType = variable->ResultType;
  }

  //***************************************************************************
  void Syntaxer::CheckDelete(DeleteNode*& node, TypingContext* context)
  {
    // Process the deleted object expression
    context->Walker->Walk(this, node->DeletedObject, context);

    // If an error occurred, exit out
    if (this->Errors.WasError)
      return;

    // The handle we're deleting needs to be readable (so we can read the handle's value)
    // For example, it cannot be a 'set' only property that we are deleting
    node->DeletedObject->IoUsage = IoMode::ReadRValue;

    // The type of the deleted object has to at least be a reference
    if (Type::IsHandleType(node->DeletedObject->ResultType) == false)
    {
      return this->ErrorAt(node->DeletedObject, ErrorCode::DeletingNonReferenceType);
    }

    // If the type has a flag that stops itself from being deleted in script...
    // Let the user know that this type can't be created (even if it has a constructor!)
    BoundType* type = Type::GetBoundType(node->DeletedObject->ResultType);
    if (type != nullptr && type->CreatableInScript == false)
      return this->ErrorAt(node, ErrorCode::CannotDeleteType, type->Name.c_str());
  }

  //***************************************************************************
  void Syntaxer::CheckThrow(ThrowNode*& node, TypingContext* context)
  {
    // Process the exception expression
    context->Walker->Walk(this, node->Exception, context);

    // If an error occurred, exit out
    if (this->Errors.WasError)
      return;

    // The value we're throwing needs to be readable
    node->Exception->IoUsage = IoMode::ReadRValue;

    // Grab the core library so we can use the exception type
    Core& core = Core::GetInstance();

    // Get the exception type as a bound type (it must be!)
    BoundType* exceptionType = Type::DynamicCast<BoundType*>(node->Exception->ResultType);

    // First, we need to check if the type is even a bound type (if not, it can't be thrown!)
    if (exceptionType == nullptr)
    {
      return this->ErrorAt(node->Exception, ErrorCode::ThrowTypeMustDeriveFromException);
    }

    // The type of the exception expression must derive from the core 'Exception' type
    if (Type::BoundIsA(exceptionType, core.ExceptionType) == false)
    {
      return this->ErrorAt(node->Exception, ErrorCode::ThrowTypeMustDeriveFromException);
    }

    // Mark the parent scope as being a full return (nothing executes after a throw, much like a return)
    this->MarkParentScopeAsAllPathsReturn(node->Parent, false);
  }

  //***************************************************************************
  void Syntaxer::ProcessScopeStatements(ScopeNode* node, TypingContext* context)
  {
    // Loop through all the statements
    // Note: If any of the statements are a return value, it will set 'AllPathsReturn' for this node
    for (size_t i = 0; i < node->Statements.Size(); ++i)
    {
      // If we hit a point where all code paths return,
      // yet we still have a statement left to parse...
      if (node->AllPathsReturn && node->IsDebugReturn == false)
      {
        // Statements after this point will never be reached!
        return ErrorAt(node->Statements[i], ErrorCode::StatementsWillNotBeExecutedEarlyReturn);
      }

      // Get the current statement
      StatementNode* statement = node->Statements[i];

      // Walk the statements
      context->Walker->Walk(this, statement, context);

      // If an error occurred, exit out
      if (this->Errors.WasError)
        return;

      // Check if the statement is an expression
      ExpressionNode* expression = Type::DynamicCast<ExpressionNode*>(statement);

      // If the node was an expression node...
      if (expression != nullptr)
      {
        // We mark every expression statement as having an
        // ignored IO usage, we don't care if it's read/write!
        expression->IoUsage = IoMode::Ignore;
      }
    }
  }

  //***************************************************************************
  // Check the condition and statements in a conditional loop
  void Syntaxer::CheckConditionalLoop(ConditionalLoopNode* node, TypingContext* context)
  {
    // Process the initial value expression
    context->Walker->Walk(this, node->Condition, context);

    // If an error occurred, exit out
    if (this->Errors.WasError)
      return;

    // We need to be able to read the conditional value
    node->Condition->IoUsage = IoMode::ReadRValue;

    // Get the instance of the type database
    Core& core = Core::GetInstance();

    // Now that we've walked the condition, check to make sure it's a bool type
    // Check if the types match, or if we can implicitly convert one to our resulting type
    if (this->ImplicitConvertAfterWalkAndIo(node->Condition, core.BooleanType) == false)
    {
      // The condition was not a bool
      return ErrorAt
      (
        node->Condition,
        ErrorCode::ConditionMustBeABooleanType,
        node->Condition->ResultType->ToString().c_str()
      );
    }

    // Process all the statements inside the conditional loop
    this->ProcessScopeStatements(node, context);
  }

  //***************************************************************************
  void Syntaxer::CheckWhile(WhileNode*& node, TypingContext* context)
  {
    return CheckConditionalLoop(node, context);
  }

  //***************************************************************************
  void Syntaxer::CheckDoWhile(DoWhileNode*& node, TypingContext* context)
  {
    return CheckConditionalLoop(node, context);
  }

  //***************************************************************************
  void Syntaxer::CheckFor(ForNode*& node, TypingContext* context)
  {
    // If we have a range variable (only used in foreach)
    if (node->RangeVariable != nullptr)
    {
      // Process the variable
      context->Walker->Walk(this, node->RangeVariable, context);
    }

    // If an error occurred, exit out
    if (this->Errors.WasError)
      return;

    // If we have a variable...
    if (node->ValueVariable != nullptr)
    {
      // Process the variable
      context->Walker->Walk(this, node->ValueVariable, context);
    }
    // Otherwise, we assume that we have an initialization expression
    else if (node->Initialization != nullptr)
    {
      // Process the initialization expression
      context->Walker->Walk(this, node->Initialization, context);

      // When using an initialization expression, we never need to either read or write from in
      // In that regard, we completely ignore however the user decides to use it
      node->Initialization->IoUsage = IoMode::Ignore;
    }

    // If an error occurred, exit out
    if (this->Errors.WasError)
      return;

    // Process the iterator expression
    context->Walker->Walk(this, node->Iterator, context);

    // If an error occurred, exit out
    if (this->Errors.WasError)
      return;

    // The iterator expression must be a readable one
    node->Iterator->IoUsage = IoMode::ReadRValue;

    // Now we want to check the condition
    return CheckConditionalLoop(node, context);
  }

  //***************************************************************************
  void Syntaxer::CheckLoop(LoopNode*& node, TypingContext* context)
  {
    // Process all the statements inside the loop
    this->ProcessScopeStatements(node, context);
  }

  //***************************************************************************
  void Syntaxer::CheckScope(ScopeNode*& node, TypingContext* context)
  {
    // Process all the statements inside the scope
    this->ProcessScopeStatements(node, context);
  }

  //***************************************************************************
  void Syntaxer::CheckTimeout(TimeoutNode*& node, TypingContext* context)
  {
    // Process all the statements inside the scope
    this->ProcessScopeStatements(node, context);
  }

  //***************************************************************************
  void Syntaxer::CheckIfRoot(IfRootNode*& node, TypingContext* context)
  {
    // Walk all the parts of the if statement
    context->Walker->GenericWalkChildren(this, node, context);

    // If an error occurred, exit out
    if (this->Errors.WasError)
      return;

    // We got a completely empty if statement (probably an if with no body), just early out
    if (this->Errors.TolerantMode && node->IfParts.Empty())
      return;

    // Check if we have a standalone else statement (only logical when we have at least 2 parts of the if)
    bool hasNonConditionalElse = (node->IfParts.Back()->Condition == nullptr);
    
    // If we have a non-conditional else, then it's possible that all paths of the if statement return from the function (or throw, etc)
    if (hasNonConditionalElse)
    {
      // We want to know if all the if parts of our if statement return, and if any of them are a debug return
      bool allIfPartsReturn = true;
      bool anyDebugReturns = false;

      // If any of the children are 'debug returns' then we mark all returns as being debug returns
      for (size_t i = 0; i < node->IfParts.Size(); ++i)
      {
        // Grab the current part of the if statement
        IfNode* part = node->IfParts[i];

        // If we encounter any part of the if statement that doesn't return, then not all paths return!
        if (part->AllPathsReturn == false)
        {
          // Early out, no need to check anything else
          allIfPartsReturn = false;
          break;
        }

        // We just want to know if any if statement 
        anyDebugReturns |= part->IsDebugReturn;
      }

      // If we found that all of our if parts return...
      if (allIfPartsReturn)
      {
        // Inform our parent that all the paths return
        this->MarkParentScopeAsAllPathsReturn(node->Parent, anyDebugReturns);
      }
    }
  }

  //***************************************************************************
  void Syntaxer::CheckIf(IfNode*& node, TypingContext* context)
  {
    // If we have a condition...
    if (node->Condition != nullptr)
    {
      // Process the initial value expression
      context->Walker->Walk(this, node->Condition, context);

      // If an error occurred, exit out
      if (this->Errors.WasError)
        return;

      // We need to be able to read the conditional value
      node->Condition->IoUsage = IoMode::ReadRValue;

      // Get the instance of the type database
      Core& core = Core::GetInstance();

      // Now that we've walked the condition, check to make sure it's a bool type
      // Check if the types match, or if we can implicitly convert one to our resulting type
      if (this->ImplicitConvertAfterWalkAndIo(node->Condition, core.BooleanType) == false)
      {
        // The condition was not a bool
        return ErrorAt
        (
          node->Condition,
          ErrorCode::ConditionMustBeABooleanType,
          node->Condition->ResultType->ToString().c_str()
        );
      }
    }

    // Process all the statements inside the if
    this->ProcessScopeStatements(node, context);
  }

  //***************************************************************************
  LoopScopeNode* Syntaxer::FindLoopScope(size_t scopeCount, SyntaxNode* parent)
  {
    // Loop until we reach the root (we never should actually...)
    while (parent != nullptr)
    {
      // We want to stop if we hit a function node
      if (Type::DynamicCast<FunctionNode*>(parent) != nullptr)
      {
        // We did not find it
        return nullptr;
      }

      // If we hit some kind of breakable scope, then decrement the scope counter
      LoopScopeNode* loopScope = Type::DynamicCast<LoopScopeNode*>(parent);
      if (loopScope != nullptr)
      {
        // Decrement the counter
        --scopeCount;

        // If the counter hits zero, we found the loop we wish to break out of
        if (scopeCount == 0)
        {
          // Return the found loop scope
          return loopScope;
        }
      }

      // Iterate to the parent node
      parent = parent->Parent;
    }

    // We did not find the node
    Error("We actually shouldn't be able to get here, since we should at least hit a class node");
    return nullptr;
  }

  //***************************************************************************
  void Syntaxer::CheckBreak(BreakNode*& node, TypingContext* /*context*/)
  {
    // Attempt to find the scope we're supposed to break out of
    LoopScopeNode* scope = FindLoopScope(node->ScopeCount, node->Parent);

    // If we found that scope
    if (scope != nullptr)
    {
      // Add ourselves to a break
      scope->Breaks.PushBack(node);
    }
    else
    {
      // Error, we didn't find it
      return ErrorAt(node, ErrorCode::BreakLoopNotFound);
    }
  }

  //***************************************************************************
  void Syntaxer::CheckContinue(ContinueNode*& node, TypingContext* /*context*/)
  {
    // Attempt to find the scope we're supposed to break out of
    LoopScopeNode* scope = FindLoopScope(1, node->Parent);

    // If we found that scope
    if (scope != nullptr)
    {
      // Add ourselves to a break
      scope->Continues.PushBack(node);
    }
    else
    {
      // Error, we didn't find it
      return ErrorAt(node, ErrorCode::ContinueLoopNotFound);
    }
  }

  //***************************************************************************
  void Syntaxer::ResolveLocalVariableReference(LocalVariableReferenceNode*& node, TypingContext* /*context*/)
  {
    // We can both read and write to local variables
    node->Io = (IoMode::Enum)(IoMode::ReadRValue | IoMode::WriteLValue);

    // Get the parent node (we'll also use this for iteration up the nodes)
    SyntaxNode* parent = node->Parent;

    // Loop until we reach the root (or something else below breaks us out)
    while (parent != nullptr)
    {
      // Check if our parent is a scope
      ScopeNode* scopedParent = Type::DynamicCast<ScopeNode*>(parent);

      // If it is indeed a scope...
      if (scopedParent != nullptr)
      {
        // Loop through all the variables at that scope
        for (size_t i = 0; i < scopedParent->ScopedVariables.Size(); ++i)
        {
          // Get the current variable
          VariableRange range = scopedParent->ScopedVariables.Find(node->Value.Token);

          // If we have a match with the variable's name...
          if (range.Empty() == false)
          {
            // Get a pointer to the variable info
            Variable* variable = range.Front().second;

            // Copy over the value, the node the identifier is referencing, and the type
            node->Access.Type = OperandType::Local;
            node->AccessedVariable    = variable;
            node->ResultType  = variable->ResultType;
            return;
          }
        }
      }

      // Iterate to the next parent
      parent = parent->Parent;
    }

    // An identifier was used, but we couldn't find the variable it was referencing!
    return ErrorAt
    (
      node,
      ErrorCode::LocalVariableReferenceNotFound,
      node->Value.Token.c_str()
    );
  }

  //***************************************************************************
  void Syntaxer::MarkParentScopeAsAllPathsReturn(SyntaxNode* parent, bool isDebugReturn)
  {
    // After we've actually checked the return is valid
    // Loop up the parents until we reach a scope node or the root
    while (parent != nullptr)
    {
      // Check if our parent is a scope
      ScopeNode* scopedParent = Type::DynamicCast<ScopeNode*>(parent);

      // If it is indeed a scope...
      if (scopedParent != nullptr)
      {
        // We hit a return statement, so this scope must return
        scopedParent->AllPathsReturn = true;

        // Debug return bubbles up
        scopedParent->IsDebugReturn |= isDebugReturn;
        return;
      }

      // Iterate to the next parent
      parent = parent->Parent;
    }

    // Error checking
    Error("We should never reach the root node!");
  }

  //***************************************************************************
  bool Syntaxer::ImplicitConvertAfterWalkAndIo(ExpressionNode*& nodeToReparent, Type* toType)
  {
    // We're casting from the original type of the node that we're re-parenting
    Type* fromType = nodeToReparent->ResultType;

    // First we need to see what kind of cast operation we're dealing with
    Shared& shared = Shared::GetInstance();
    CastOperator cast = shared.GetCastOperator(fromType, toType);

    // If there is no cast operator or it's not implicit, return that we can't do it!
    if (cast.IsValid == false || cast.CanBeImplicit == false)
      return false;

    // If the cast is a 'raw' cast, then there's nothing to do!
    // This means it is valid cast, but we don't actually need to generate a type cast node since no code-gen is required
    if (cast.RequiresCodeGeneration == false)
      return true;

    // Create the type cast node (our main operand is the given node, we are re-parenting the cast)
    TypeCastNode* typeCast = new TypeCastNode();
    typeCast->Operand = nodeToReparent;
    typeCast->OperatorInfo = cast;
    typeCast->Location = nodeToReparent->Location;
    
    // The result type is the type we're casting to in the type cast operation
    typeCast->ResultType = toType;

    // A type cast only requires read ability from the operand
    typeCast->Io = IoMode::ReadRValue;

    // We inherit whatever type of Io we applied to the node itself (typically just Read)
    typeCast->IoUsage = nodeToReparent->IoUsage;
    typeCast->IsUsedAsStatement = false;

    // Finally, perform the re-parenting
    nodeToReparent = typeCast;

    // We performed the implicit cast
    return true;
  }

  //***************************************************************************
  void Syntaxer::CheckReturn(ReturnNode*& node, TypingContext* context)
  {
    // Get the function on the top of the stack
    Function* function = context->FunctionStack.Back();

    // Get the return type for convenience
    Type* returnType = function->FunctionType->Return;

    // Get a reference to the core library
    Core& core = Core::GetInstance();

    // If we have a return type for this function...
    if (Type::IsSame(returnType, core.VoidType) == false)
    {
      // As long as we have an actual return value
      if (node->ReturnValue != nullptr)
      {
        // Process the return value expression
        context->Walker->Walk(this, node->ReturnValue, context);

        // If an error occurred, exit out
        if (this->Errors.WasError)
          return;

        // We need to be able to read the value we're returning
        node->ReturnValue->IoUsage = IoMode::ReadRValue;

        // Check if the types match, or if we can implicitly convert one to our resulting type
        if (this->ImplicitConvertAfterWalkAndIo(node->ReturnValue, returnType) == false)
        {
          // The return values given did not match the function signature
          return ErrorAt
          (
            node,
            ErrorCode::ReturnTypeMismatch,
            node->ReturnValue->ResultType->ToString().c_str(),
            returnType->ToString().c_str()
          );
        }
      }
      else
      {
        // The return value was not found
        return ErrorAt(node, ErrorCode::ReturnValueNotFound, returnType->ToString().c_str());
      }
    }
    else
    {
      // As long as we have an actual return value
      if (node->ReturnValue != nullptr)
      {
        // A return value was given, but was totally not expected
        return ErrorAt
        (
          node,
          ErrorCode::ReturnValueUnexpected,
          node->ReturnValue->ResultType->ToString().c_str());
      }
    }

    // Mark the parent scope as being a full return (since this is a return!)
    this->MarkParentScopeAsAllPathsReturn(node->Parent, node->IsDebugReturn);
  }

  //***************************************************************************
  void Syntaxer::DecorateCheckTypeCast(TypeCastNode*& node, TypingContext* context)
  {
    // The result of a type cast will always be read only
    node->Io = IoMode::ReadRValue;

    // Get the actual type definition associated with the type-cast
    node->ResultType = this->RetrieveType(node->Type, node->Location);

    // If for some reason the type was not resolved, exit out
    if (this->Errors.WasError)
      return;

    // Now we need to traverse the expression!
    context->Walker->Walk(this, node->Operand, context);

    // If an error occurred, exit out
    if (this->Errors.WasError)
      return;

    // The operand only needs to be readable
    node->Operand->IoUsage = IoMode::ReadRValue;

    // Rename the types for convenience
    Type* fromType = node->Operand->ResultType;
    Type* toType = node->ResultType;

    // Figure out what kind of cast this is
    Shared& shared = Shared::GetInstance();
    node->OperatorInfo = shared.GetCastOperator(fromType, toType);

    // If the cast was not valid any way we tried it...
    if (node->OperatorInfo.IsValid == false)
    {
      // Inform the user that the types they were attempting to cast between are not valid
      return ErrorAt
      (
        node,
        ErrorCode::InvalidTypeCast,
        fromType->ToString().c_str(),
        toType->ToString().c_str()
      );
    }
  }

  //***************************************************************************
  void Syntaxer::DecorateCheckFunctionCall(FunctionCallNode*& node, TypingContext* context)
  {
    // For right now, all function calls will result in a read only
    // (note that when we get references properly working, this won't be the case)
    node->Io = IoMode::ReadRValue;

    // Process the left expression first
    if (node->LeftOperand != nullptr)
    {
      // Walk the left expression and compute its types
      context->Walker->Walk(this, node->LeftOperand, context);

      // The left hand node only needs to be readable
      node->LeftOperand->IoUsage = IoMode::ReadRValue;
    }

    // If an error occurred, exit out
    if (this->Errors.WasError)
      return;

    // Loop through all the arguments
    for (size_t i = 0; i < node->Arguments.Size(); ++i)
    {
      // Get the current argument
      ExpressionNode* argument = node->Arguments[i];

      // Walk the arguments so they get properly typed
      context->Walker->Walk(this, argument, context);

      // The arguments only need to be readable
      argument->IoUsage = IoMode::ReadRValue;

      // If an error occurred, exit out
      if (this->Errors.WasError)
        return;
    }

    // Tells us if we already resolved the function and therefore checked parameter types
    bool resolvedAndChecked = false;

    // If this function call node is being used to invoke a constructor, we'll know because our left operand will actually
    // be a local variable whose initial value is a CreationCallNode
    // This is a bit complicated and maybe should be refactored, but this works for now
    StaticTypeNode* creationNode = node->FindCreationCall();
    if (creationNode != nullptr)
    {
      // Our resulting type should be the same as the creation call's type
      node->ResultType = creationNode->ResultType;

      // Attempt to find the overloaded functions with the matching name...
      BoundType* createdType = creationNode->ReferencedType;

      // Look at the creation node's overloaded/inherited constructors (see DecorateCreationCall)
      const FunctionArray* constructors = creationNode->OverloadedConstructors;

      // If we have one or more constructor, then resolve which one it is
      if (constructors != nullptr && constructors->Empty() == false)
      {
        // Resolve the overload
        bool result = Overload::ResolveAndImplicitConvert(constructors, creationNode->ConstructorFunction, *node);
        
        // If the overload failed to be resolved... we need to throw an error
        if (result == false)
        {
          // Give a details report of the error
          return Overload::ReportError(this->Errors, node->Location, constructors, *node);
        }

        // If we got here, it means we successfully resolved the overload and checked it
        resolvedAndChecked = true;
      }
      // If the type we're creating is a native reference type and it has no constructors...
      else if (createdType->Native && createdType->CopyMode == TypeCopyMode::ReferenceType)
      {
        // It is always an error if we have no constructors
        return this->ErrorAt(node, ErrorCode::NativeTypesRequireConstructors, createdType->ToString().c_str());
      }
      // It is legal to have no constructors so long as no arguments were provided
      else if (node->Arguments.Empty() == false)
      {
        // It is always an error if we have no constructors
        return this->ErrorAt(node, ErrorCode::NoArgumentConstructorsProvided, createdType->ToString().c_str());
      }
    }
    // If the left hand node is a base class initializer
    else if (InitializerNode* initializerNode = Type::DynamicCast<InitializerNode*>(node->LeftOperand))
    {
      // As long as this is a base class initializer...
      if (initializerNode->InitializerType->TokenId == Grammar::Base)
      {
        // Get a reference to the core library
        Core& core = Core::GetInstance();

        // We have no resulting type for this call
        node->ResultType = core.VoidType;

        // We should always be able to assume that an initializer is ONLY being called from the class it is within
        BoundType* classType = context->ClassTypeStack.Front();
        BoundType* baseType = classType->BaseType;

        // Make sure the class type has a base class
        if (baseType == nullptr)
        {
          // It is an error to use a base class initializer without a base class
          return ErrorAt
          (
            node,
            ErrorCode::BaseClassInitializerRequiresBaseClassInheritance,
            classType->Name.c_str()
          );
        }

        // Walk up the base class chain until we find any constructors (we inherit constructors)
        // We start with the current class we're trying to create
        // Note: We can safely look up to our base classes because if we don't have a constructor
        // then we at least have been pre-constructed, and the user opted to not initialize anything with a constructor
        const FunctionArray* constructors = baseType->GetOverloadedInheritedConstructors();

        // If there is only one overload of the function, then we know its type!
        if (constructors != nullptr && constructors->Empty() == false)
        {
          // Resolve the overload
          bool result = Overload::ResolveAndImplicitConvert(constructors, initializerNode->InitializerFunction, *node);
      
          // If the overload failed to be resolved... we need to throw an error
          if (result == false)
          {
            // Give a details report of the error
            return Overload::ReportError(this->Errors, node->Location, constructors, *node);
          }

          // Set the resulting type (the function on the node should now be valid)
          initializerNode->ResultType = initializerNode->InitializerFunction->FunctionType;

          // If we got here, it means we successfully resolved the overload and checked it
          resolvedAndChecked = true;
        }
        // If the type we're creating is a native reference type and it has no constructors...
        else if (baseType->Native && baseType->CopyMode == TypeCopyMode::ReferenceType)
        {
          // It is always an error if we have no constructors
          return this->ErrorAt(node, ErrorCode::NativeTypesRequireConstructors, baseType->ToString().c_str());
        }
        else
        {
          // It is always an error if we have no constructors
          return ErrorAt(node, ErrorCode::NoArgumentConstructorsProvided, baseType->ToString().c_str());
        }
      }
      else
      {
        // We need to handle 'this' constructor calling
        ZilchTodo("Handle constructor calling");
        Error("We don't currently handle constructor calling");
      }
    }
    // Otherwise, it's something else (like a direct function call, a delegate call, or an error)
    else if (node->LeftOperand != nullptr)
    {
      // If the left hand node is a member access node...
      // Otherwise, it could just be a delegate type!
      if (MemberAccessNode* functionMember = Type::DynamicCast<MemberAccessNode*>(node->LeftOperand))
      {
        // As long as the member access type is a function
        if (functionMember->MemberType == MemberAccessType::Function)
        {
          // Resolve the overload
          bool result = Overload::ResolveAndImplicitConvert(functionMember->OverloadedFunctions, functionMember->AccessedFunction, *node);
      
          // If the overload failed to be resolved... we need to throw an error
          if (result == false)
          {
            // Give a details report of the error
            return Overload::ReportError(this->Errors, node->Location, functionMember->OverloadedFunctions, *node);
          }

          // Set the resulting type
          functionMember->AccessedMember = functionMember->AccessedFunction;
          functionMember->ResultType = functionMember->AccessedFunction->FunctionType;

          // If we got here, it means we successfully resolved the overload and checked it
          resolvedAndChecked = true;
        }
      }

      // Attempt to get the left operand's type as a function type (signature)
      DelegateType* delegateType = Type::DynamicCast<DelegateType*>(node->LeftOperand->ResultType);

      // If the left hand side was not a delegate type...
      if (delegateType == nullptr)
      {
        // We're trying to invoke a function on something that isn't a function, like "5()"
        return ErrorAt(node, ErrorCode::FunctionCallOnNonCallableType);
      }

      // Note: I believe this got removed in error but we STILL need to check against
      // the delegate type IF we did not need to do overload resolution, as an example, invoking a stored delegate
      // We never want to run this twice due to double implicit conversion issues
      if (resolvedAndChecked == false && Overload::TestCallAndImplicitConvert(delegateType, *node) == false)
      {
        // Report and error that we failed to resolve against the delegate
        return Overload::ReportSingleError(this->Errors, node->Location, delegateType, *node);
      }

      // The "type" of the function call will be the type that it resolves to,
      // basically the return type of the function it's evaluating
      node->ResultType = delegateType->Return;
    } // Not a creation call
  }

  //***************************************************************************
  void Syntaxer::DecorateCheckBinaryOperator(BinaryOperatorNode*& node, TypingContext* context)
  {
    // Process all the left expression
    context->Walker->Walk(this, node->LeftOperand, context);

    // If an error occurred, exit out
    if (this->Errors.WasError)
      return;

    // Process all the right expression
    context->Walker->Walk(this, node->RightOperand, context);

    // The right hand node only needs to be readable (in all cases, assignment, addition, etc)
    node->RightOperand->IoUsage = IoMode::ReadRValue;

    // If an error occurred, exit out
    if (this->Errors.WasError)
      return;

    // Get the instance of the type database
    Core& core = Core::GetInstance();
    Type* lhs = node->LeftOperand->ResultType;
    Type* rhs = node->RightOperand->ResultType;

    // Get the shared database and lookup the binary operator
    Shared& shared = Shared::GetInstance();
    node->OperatorInfo = shared.GetBinaryOperator(lhs, rhs, node->Operator->TokenId);

    // If we actually found a valid operator...
    if (node->OperatorInfo.IsValid)
    {
      // Note: Since an implicit cast of the left operand would change the node type, we must set the io-usage
      // BEFORE we perform the implicit cast. Also note that an implicit cast of the left argument should never
      // occur if the io usage is an l-value (that wouldn't make sense, GetBinaryOperator blocks these)
      // Output the result type and outmode
      node->ResultType = node->OperatorInfo.Result;
      node->LeftOperand->IoUsage = node->OperatorInfo.Io;

      // We already visited the left/right operands, which means its valid for us to generate any implicit type casts here
      // Check to see if we need to cast the left side
      if (node->OperatorInfo.CastLhsTo != nullptr)
      {
        // Apply the implicit conversion
        bool implicitCastResult = ImplicitConvertAfterWalkAndIo(node->LeftOperand, node->OperatorInfo.CastLhsTo);
        ErrorIf(implicitCastResult == false, "The operator told us that we could implicit cast, why did this fail?");
      }

      // Check to see if we need to cast the right side
      if (node->OperatorInfo.CastRhsTo != nullptr)
      {
        // Apply the implicit conversion
        bool implicitCastResult = ImplicitConvertAfterWalkAndIo(node->RightOperand, node->OperatorInfo.CastRhsTo);
        ErrorIf(implicitCastResult == false, "The operator told us that we could implicit cast, why did this fail?");
      }

      // Our usage is always the most restrictive of how the node designates
      // it can be used and how our operator designates it can be used
      node->Io = (IoMode::Enum)(node->LeftOperand->Io & node->LeftOperand->IoUsage);
    }
    else
    {
      // Report an error since we used two types that weren't of the same type!
      return ErrorAt(node, ErrorCode::InvalidBinaryOperation,
        node->Operator->Token.c_str(),
        Grammar::GetName(node->Operator->TokenId).c_str(),
        lhs->ToString().c_str(),
        rhs->ToString().c_str());
    }
  }

  //***************************************************************************
  void Syntaxer::DecorateCheckPropertyDelegateOperator(PropertyDelegateOperatorNode*& node, TypingContext* context)
  {
    // Process all the operand
    context->Walker->Walk(this, node->Operand, context);

    // If an error occurred, exit out
    if (this->Errors.WasError)
      return;

    // Attempt to get the operand as a member access node...
    // This operator only works on properties and members
    MemberAccessNode* memberNode = Type::DynamicCast<MemberAccessNode*>(node->Operand);

    // If the operand is a member
    if (memberNode != nullptr)
    {
      // We only care if we accessed a property
      Property* property = memberNode->AccessedProperty;

      // If we didn't find a property...
      if (property == nullptr)
      {
        // Report an error since we attempted to use the property delegate operator on a non-property
        return ErrorAt(node, ErrorCode::PropertyDelegateOperatorRequiresProperty);
      }

      // Make sure the property has the get and set functions
      if (property->Get == nullptr && property->Set == nullptr)
      {
        // Report an error since we can't form a property delegate to an object that has neither a get or set
        return ErrorAt(node, ErrorCode::PropertyDelegateRequiresGetOrSet, property->Name.c_str());
      }

      // The operand should either have a get or set, but we don't require one or the other
      node->Operand->IoUsage = IoMode::Ignore;

      // We're generating a temporary (just a read value)
      node->Io = IoMode::ReadRValue;

      // We need to instantiate the 'Property' template (an object that Contains the get/set delegates)
      Array<Constant> templateArguments;
      templateArguments.PushBack(property->PropertyType);

      // Instantiate the property object
      InstantiatedTemplate propertyTemplate = this->Builder->InstantiateTemplate(PropertyDelegateName, templateArguments, *this->Dependencies);

      // Make sure we instantiated the template
      ErrorIf(propertyTemplate.Result != TemplateResult::Success,
        "We should always be able to instantiate the property template!");

      // Our node's type is that property template type
      node->ResultType = propertyTemplate.Type;
      node->AccessedProperty = property;
    }
    else
    {
      // Report an error since we did some sort of an invalid binary operation between two types
      return ErrorAt(node, ErrorCode::PropertyDelegateOperatorRequiresProperty);
    }
  }

  //***************************************************************************
  void Syntaxer::DecorateCheckUnaryOperator(UnaryOperatorNode*& node, TypingContext* context)
  {
    // Process all the operand
    context->Walker->Walk(this, node->Operand, context);

    // If an error occurred, exit out
    if (this->Errors.WasError)
      return;

    // Get the operand type
    Type* operandType = node->Operand->ResultType;

    // Get the shared database and lookup the binary operator
    Grammar::Enum oper = node->Operator->TokenId;
    Shared& shared = Shared::GetInstance();
    UnaryOperator& info = node->OperatorInfo;
    info = shared.GetUnaryOperator(operandType, node->Operator->TokenId);

    // If the user wants to get the address of a struct (safe ref handle)
    if (oper == Grammar::AddressOf)
    {
      BoundType* type = Type::DynamicCast<BoundType*>(operandType);
      if (type != nullptr && type->CopyMode == TypeCopyMode::ValueType)
      {
        info.IsValid = true;
        info.Operand = operandType;
        info.Result = this->Builder->ReferenceOf(type);
        info.Operator = oper;
        info.Io = IoMode::ReadRValue;
      }
    }
    
    // If the user wants to turn a ref struct back into a value
    if (oper == Grammar::Dereference && Type::IsIndirectionType(operandType))
    {
      IndirectionType* type = Type::DynamicCast<IndirectionType*>(operandType);
      if (type != nullptr)
      {
        info.IsValid = true;
        info.Operand = operandType;
        info.Result = this->Builder->Dereference(type);
        info.Operator = oper;
        info.Io = IoMode::ReadRValue;
      }
    }

    if (node->OperatorInfo.IsValid)
    {
      // Output the result type and outmode
      node->ResultType = info.Result;
      node->Operand->IoUsage = info.Io;

      // Our usage is always the most restrictive of how the node designates
      // it can be used and how our operator designates it can be used
      node->Io = (IoMode::Enum)(node->Operand->Io & node->Operand->IoUsage);
    }
    else
    {
      // Report an error since we did some sort of an invalid binary operation between two types
      return ErrorAt(node, ErrorCode::InvalidUnaryOperation, operandType->ToString().c_str());
    }
  }

  //***************************************************************************
  void Syntaxer::DecorateCodeLocations(SyntaxNode*& node, TypingContext* context)
  {
    // If this node is a class node...
    ClassNode* classNode = Type::DynamicCast<ClassNode*>(node);
    if (classNode != nullptr)
    {
      // Push the class onto the stack so that children can access it
      // (the top of the stack will be the most relevant class to them)
      context->ClassTypeStack.PushBack(classNode->Type);
    }
    
    // If this node is a function node...
    GenericFunctionNode* functionNode = Type::DynamicCast<GenericFunctionNode*>(node);
    if (functionNode != nullptr)
    {
      // Push the function onto the stack so that children can access it
      // (the top of the stack will be the most relevant function to them)
      context->FunctionStack.PushBack(functionNode->DefinedFunction);
    }

    // The library is the name of our library builder
    node->Location.Library = this->Builder->BuiltLibrary->Name;

    // If we have a class in our context (we are inside a class)
    if (context->ClassTypeStack.Empty() == false)
    {
      // Set the class name on the code location
      node->Location.Class = context->ClassTypeStack.Back()->Name;
    }

    // If we have a function in our context (we are inside a function)
    if (context->FunctionStack.Empty() == false)
    {
      // Why would we ever be inside a function but not inside a class?
      ErrorIf(context->ClassTypeStack.Empty(), "Found a function that was not inside a class");

      // Set the function name on the code location
      node->Location.Function = context->FunctionStack.Back()->Name;
    }

    // We need to traverse the children!
    // Normally we would return if any error occurred, but this is all useful information even with errors
    context->Walker->GenericWalkChildren(this, node, context);

    // If this node is a class node...
    if (classNode != nullptr)
    {
      // We are exiting this class, so pop it off
      context->ClassTypeStack.PopBack();
    }
    
    // If this node is a function node...
    if (functionNode != nullptr)
    {
      // We are exiting this class, so pop it off
      context->FunctionStack.PopBack();
    }
  }

  //***************************************************************************
  void Syntaxer::CheckExpressionIoModes(ExpressionNode*& node, TypingContext* context)
  {
    // We need to traverse the children!
    context->Walker->GenericWalkChildren(this, node, context);

    // If we had an error, do not continue
    if (this->Errors.WasError)
      return;

    // Skip any error checking if we're in tolerant mode...
    if (this->Errors.TolerantMode == false)
    {
      // Error checking
      ErrorIf(node->Io == IoMode::NotSet || node->Io == IoMode::Ignore,
        "A node's usage case was not set by its handler (or somehow it was set to ignore)");
      ErrorIf(node->IoUsage == IoMode::NotSet,
        "The parent node of an expression did not set the child's io usage");
      ErrorIf(node->ResultType == nullptr,
        "All expression node types must be valid (or void)");
    }

    // If we're trying to read from the node, but it's not readable...
    if ((node->IoUsage & IoMode::ReadRValue) != 0 && (node->Io & IoMode::ReadRValue) == 0)
    {
      // Report an error
      return ErrorAt(node, ErrorCode::ReadingFromAWriteOnlyValue);
    }

    // If we're trying to write to the node, but it's not writable...
    if ((node->IoUsage & IoMode::WriteLValue) != 0 && (node->Io & IoMode::WriteLValue) == 0)
    {
      // Report an error
      return ErrorAt(node, ErrorCode::WritingToAReadOnlyValue);
    }
  }

  //***************************************************************************
  void Syntaxer::ResolveMemberAccess(MemberAccessNode* node, const Resolver& resolver)
  {
    // Get access to the core library
    Core& core = Core::GetInstance();

    // Get the type instance for the type we're attempting to resolve on (the left operand generally)
    BoundType* type = resolver.TypeInstance;

    // Attempt to find a field with the matching name...
    Field* field = (type->*resolver.GetField)(node->Name);

    // If we found the member...
    if (field != nullptr)
    {
      // Set the member type (and resulting type)
      // We also want to set the access type to be a member, so that way anyone
      // who tries to modify it knows they can write directly to the member
      node->MemberType            = MemberAccessType::Field;
      node->Access.Type           = OperandType::Field;
      node->ResultType            = field->PropertyType;
      node->AccessedField         = field;
      node->AccessedProperty      = field;
      node->AccessedMember        = field;

      // We can both read and write to a data member
      node->Io = (IoMode::Enum)(IoMode::ReadRValue | IoMode::WriteLValue);

      // Exit out early
      return;
    }

    // Attempt to find a property with the matching name...
    GetterSetter* property = (type->*resolver.GetGetterSetter)(node->Name);

    // If we found no property... attempt to look in dependency extension methods
    if (property == nullptr)
    {
      // Loop through all the libraries
      for (size_t i = 0; i < this->AllLibraries.Size() && property == nullptr; ++i)
      {
        // Grab the current library
        LibraryRef& library = this->AllLibraries[i];

        // We need to look up the entire hierarchy (the property could be on any base classes)
        Type* baseIterator = type;
        do
        {
          // Get the guid of the type (this should be legal here since we've collected all members)
          GuidType guid = baseIterator->Hash();

          // If we're resolving a static member
          GetterSetterMap* properties = nullptr;
          if (resolver.IsStatic)
            properties = library->StaticExtensionGetterSetters.FindPointer(guid);
          else
            properties = library->InstanceExtensionGetterSetters.FindPointer(guid);
        
          // If we got a valid array of properties...
          if (properties != nullptr)
          {
            // Attempt to find the property by name
            // If we find it, the loop will terminate because we don't need to keep looking
            property = properties->FindValue(node->Name, nullptr);
            if (property != nullptr)
              break;
          }
          
          // Iterate to the next type
          baseIterator = Type::GetBaseType(baseIterator);
        }
        while (baseIterator != nullptr);
      }
    }

    // If we found the property...
    if (property != nullptr)
    {
      // Set the property type (and resulting type)
      // We also want to set the access type to be a property, so that way anyone
      // who tries to modify it knows they must use a getter/setter
      node->MemberType            = MemberAccessType::Property;
      node->Access.Type           = OperandType::Property;
      node->AccessedGetterSetter  = property;
      node->AccessedProperty      = property;
      node->AccessedMember        = property;

      // Clear the io so we can set the flags below
      node->Io = IoMode::Ignore;

      // The resulting type is always the type of the property
      // Note this does NOT mean we produce this type (a set only property does not return a value)
      // But the result type must be set in order to Assign to this property
      node->ResultType = property->PropertyType;

      // If we have a valid getter
      if (property->Get != nullptr)
        node->Io = (IoMode::Enum)(node->Io | IoMode::ReadRValue);
      
      // If we have a valid setter
      if (property->Set != nullptr)
        node->Io = (IoMode::Enum)(node->Io | IoMode::WriteLValue);

      // Exit out early
      return;
    }

    // Attempt to find the overloaded functions with the matching name...
    const FunctionArray* functions = (type->*resolver.GetOverloadedFunctions)(node->Name);

    // If we found no functions... attempt to look in dependency extension methods
    if (functions == nullptr || functions->Empty())
    {
      // Loop through all the libraries
      for (size_t i = 0; i < this->AllLibraries.Size(); ++i)
      {
        // Grab the current library
        LibraryRef& library = this->AllLibraries[i];

        // We need to look up the entire hierarchy (the property could be on any base classes)
        Type* baseIterator = type;
        do
        {
          // Get the guid of the type (this should be legal here since we've collected all members)
          GuidType guid = baseIterator->Hash();

          // If we're resolving a static member
          FunctionMultiMap* functionsByName = nullptr;
          if (resolver.IsStatic)
            functionsByName = library->StaticExtensionFunctions.FindPointer(guid);
          else
            functionsByName = library->InstanceExtensionFunctions.FindPointer(guid);
        
          // If we got a valid array of properties...
          if (functionsByName != nullptr)
          {
            // Attempt to find the property by name
            functions = functionsByName->FindPointer(node->Name);

            // If we found a valid function array...
            if (functions != nullptr)
            {
              // We found it!

              // HACK: Quick hack to break out of the outer loop otherwise any other libraries that also extend this type will wipe 
              // out the result we found. Trevor you said this should be switched to your range later!
              i = this->AllLibraries.Size();
              break;
            }
          }

          // Iterate to the next type
          baseIterator = Type::GetBaseType(baseIterator);
        }
        while (baseIterator != nullptr);
      }
    }

    // If we found the functions...
    if (functions != nullptr && functions->Empty() == false)
    {
      // The node represents a function member access
      node->MemberType = MemberAccessType::Function;

      // The function will be referenced as a local (delegates are always locally generated)
      node->Access.Type = OperandType::Local;
    
      // A function can only be read (and in that same note, it really means create a delegate)
      node->Io = IoMode::ReadRValue;

      // If we have only one function
      if (functions->Size() == 1)
      {
        // For now, we assume that the function we result in is the only one it can be
        Function* accessedFunction = functions->Front();
        node->AccessedFunction = accessedFunction;
        node->AccessedMember   = accessedFunction;
        node->ResultType = node->AccessedFunction->FunctionType;
      }
      else
      {
        // We don't yet know the function we'll resolve
        node->AccessedFunction = nullptr;
        node->ResultType = core.OverloadedMethodsType;
      }

      // Set the functions on the node
      node->OverloadedFunctions = functions;

      // Exit out early
      return;
    }

    // A member access/identifier was used, but we couldn't find the member it was referencing!
    return ErrorAt(node, ErrorCode::MemberNotFound, node->Name.c_str(), type->ToString().c_str());
  }
  
  //***************************************************************************
  template <typename NodeType>
  void Syntaxer::BuildGetSetSideEffectIndexerNodes(NodeType*& node, IndexerCallNode* indexer, ExpressionNode* NodeType::* operandMemberThatWasIndexer, TypingContext* context)
  {
    // No node directly points at a BinaryOperatorNode/UnaryOperatorNode (typically just expression children)
    // This would otherwise be an unsafe assumption
    NodeType* operatorNode = node;
    ExpressionNode*& parentsChildPointer = (ExpressionNode*&)node;

    // Example:
    // this.GameBoard.Grid[this.ComputeIndexX(), yValue] += Real3(1, 1, 5);
    // var [source] = this.GameBoard.Grid;
    // var [index0] = this.ComputeIndex();
    // var [index1] = yValue;
    // var [value] = [source].Get([index0], [index1]);
    // [value] += Real3(1, 1, 5);
    // yield [source].Set([index0], [index1], [value]);

    // The entire binary/unary operator gets replaced with a multi-expression that does get/op/set
    MultiExpressionNode* multiGetSet = new MultiExpressionNode();
    multiGetSet->Location = indexer->Location;
    parentsChildPointer = multiGetSet;

    // We explicitly use this as an expression (the indexer no longer owns the left hand side, typically member access)
    // var [source] = this.GameBoard.Grid;
    LocalVariableNode* sourceVar = new LocalVariableNode("source", this->ParentProject, indexer->LeftOperand);
    indexer->LeftOperand = nullptr;
    sourceVar->Location = indexer->Location;
    multiGetSet->Expressions.Add(sourceVar);

    // [source].Get([index0], [index1]...);
    LocalVariableReferenceNode* getSourceRef = new LocalVariableReferenceNode();
    getSourceRef->Location = indexer->Location;
    getSourceRef->Value = sourceVar->Name;
    MemberAccessNode* getMember = new MemberAccessNode();
    getMember->Location = indexer->Location;
    getMember->LeftOperand = getSourceRef;
    getMember->Name = OperatorGet;
    getMember->Operator = Grammar::Access;
    FunctionCallNode* getCall = new FunctionCallNode();
    getCall->Location = indexer->Location;
    getCall->LeftOperand = getMember;

    // [source].Set([index0], [index1]..., [value]);
    LocalVariableReferenceNode* setSourceRef = new LocalVariableReferenceNode();
    setSourceRef->Location = indexer->Location;
    setSourceRef->Value = sourceVar->Name;
    MemberAccessNode* setMember = new MemberAccessNode();
    setMember->Location = indexer->Location;
    setMember->LeftOperand = setSourceRef;
    setMember->Name = OperatorSet;
    setMember->Operator = Grammar::Access;
    FunctionCallNode* setCall = new FunctionCallNode();
    setCall->Location = indexer->Location;
    setCall->LeftOperand = setMember;

    for (size_t i = 0; i < indexer->Arguments.Size(); ++i)
    {
      // var [index#] = this.ComputeIndex();
      ExpressionNode* computeIndex = indexer->Arguments[i];
      LocalVariableNode* indexVar = new LocalVariableNode(String::Format("index%d_", i), this->ParentProject, computeIndex);
      indexVar->Location = indexer->Location;
      multiGetSet->Expressions.Add(indexVar);

      // Make a local variable reference to the index variable (for invoking the getter)
      LocalVariableReferenceNode* indexRefGet = new LocalVariableReferenceNode();
      indexRefGet->Location = indexer->Location;
      indexRefGet->Value = indexVar->Name;
      getCall->Arguments.Add(indexRefGet);

      // Make a local variable reference to the index variable (for invoking the setter)
      LocalVariableReferenceNode* indexRefSet = new LocalVariableReferenceNode();
      indexRefSet->Location = indexer->Location;
      indexRefSet->Value = indexVar->Name;
      setCall->Arguments.Add(indexRefSet);
    }

    // The indexer no longer owns its arguments
    indexer->Arguments.Clear();
        
    // var [value] = [source].Get([index0], [index1]...);
    // Because our multi-expression yields this value, we need to get the index that we pushed it into the expressions list as
    LocalVariableNode* valueVar = new LocalVariableNode("value", this->ParentProject, getCall);
    valueVar->Location = indexer->Location;
    multiGetSet->Expressions.Add(valueVar);
    
    // [value] += Real3(1, 1, 5); // Binary
    // ++[value];                 // Unary
    // Here, we actually re-Assign the left operand of the binary/unary node to be a reference to the above value variable
    LocalVariableReferenceNode* valueRefOperation = new LocalVariableReferenceNode();
    valueRefOperation->Location = indexer->Location;
    valueRefOperation->Value = valueVar->Name;
    operatorNode->*operandMemberThatWasIndexer = valueRefOperation;
    multiGetSet->Expressions.Add(operatorNode);
    
    // Now we actually add in the full set call (constructed above)
    // yield [source].Set([index0], [index1]..., [value]);
    LocalVariableReferenceNode* valueRefSet = new LocalVariableReferenceNode();
    valueRefSet->Location = indexer->Location;
    valueRefSet->Value = valueVar->Name;
    setCall->Arguments.Add(valueRefSet);
    multiGetSet->YieldChildExpressionIndex = multiGetSet->Expressions.Size();
    multiGetSet->Expressions.Add(setCall);

    // We're done with the indexer (may want to store this later for formatting walkers)
    delete indexer;

    // Make sure all statements in the multi-expression node know they're being used as a statement
    for (size_t i = 0; i < multiGetSet->Expressions.Size(); ++i)
      multiGetSet->Expressions[i]->IsUsedAsStatement = node->IsUsedAsStatement;
    multiGetSet->IsUsedAsStatement = node->IsUsedAsStatement;

    // Walk the children of the multi-node (because they could have binary/unary operators that also need to be transformed)
    context->Walker->GenericWalkChildren(this, multiGetSet, context);
  }
  
  //***************************************************************************
  void Syntaxer::IndexerBinaryOperator(BinaryOperatorNode*& node, TypingContext* context)
  {
    // No node directly points at a BinaryOperatorNode (typically just expression children)
    // This would otherwise be an unsafe assumption
    BinaryOperatorNode* binaryOperator = node;
    ExpressionNode*& parentsChildPointer = (ExpressionNode*&)node;

    // We only need to do the transformation if the left operand 
    IndexerCallNode* indexer = Type::DynamicCast<IndexerCallNode*>(binaryOperator->LeftOperand);
    if (indexer == nullptr)
    {
      // Walk the children (because they could have binary operators that also need to be transformed)
      context->Walker->GenericWalkChildren(this, binaryOperator, context);
      return;
    }

    // Based on the operator (if its a side effect operator...)
    switch (binaryOperator->Operator->TokenId)
    {
      // The ultimate side effect operator! Not a compound operator though so we only need to run Set
      case Grammar::Assignment:
      {
        // Example:
        // this.GameBoard.Grid[this.ComputeIndexX(), yValue] = Real3(1, 1, 5);
        // this.GameBoard.Grid.Set(this.ComputeIndexX(), yValue, Real3(1, 1, 5));
        MemberAccessNode* setMember = new MemberAccessNode();
        setMember->Location = indexer->Location;
        setMember->LeftOperand = indexer->LeftOperand;
        indexer->LeftOperand = nullptr;
        setMember->Name = OperatorSet;
        setMember->Operator = Grammar::Access;
        FunctionCallNode* setCall = new FunctionCallNode();
        setCall->Location = indexer->Location;
        setCall->LeftOperand = setMember;
        setCall->IsUsedAsStatement = node->IsUsedAsStatement;
        parentsChildPointer = setCall;

        // Our set call has all the same arguments as the indexer (plus the value, which we handle below)
        // The indexer no longer owns these arguments
        setCall->Arguments = indexer->Arguments;
        indexer->Arguments.Clear();
        
        // Finally, add the 'set' as the last argument to our call to 'Set'
        setCall->Arguments.Add(binaryOperator->RightOperand);
        binaryOperator->RightOperand = nullptr;

        // We're done with the binary operator and indexer (may want to store this later for formatting walkers)
        // Note: The indexer gets deleted by the binary operator because its a child!
        delete binaryOperator;

        // Walk the children of the multi-node (because they could have binary operators that also need to be transformed)
        context->Walker->GenericWalkChildren(this, setCall, context);
        return;
      }

      // If the operator is a compound side-effect operator, then we need to invoke both Get and Set on the indexer
      case Grammar::AssignmentSubtract:
      case Grammar::AssignmentAdd:
      case Grammar::AssignmentDivide:
      case Grammar::AssignmentMultiply:
      case Grammar::AssignmentModulo:
      case Grammar::AssignmentExponent:
      case Grammar::AssignmentLeftShift:
      case Grammar::AssignmentRightShift:
      case Grammar::AssignmentBitwiseXor:
      case Grammar::AssignmentBitwiseOr:
      case Grammar::AssignmentBitwiseAnd:
      {
        // Build the nodes that perform the Get/Operator/Set
        // Binary and unary are similar, so this was refactored into a single function
        this->BuildGetSetSideEffectIndexerNodes(node, indexer, &BinaryOperatorNode::LeftOperand, context);
        return;
      }

      // We hit another operator, but it wasn't a side effect operator (just continue visiting)
      default:
      {

        // Walk the children (because they could have binary operators that also need to be transformed)
        context->Walker->GenericWalkChildren(this, binaryOperator, context);
        return;
      }
    }
  }
  
  //***************************************************************************
  void Syntaxer::IndexerUnaryOperator(UnaryOperatorNode*& node, TypingContext* context)
  {
    // No node directly points at a UnaryOperatorNode (typically just expression children)
    // This would otherwise be an unsafe assumption
    UnaryOperatorNode* unaryOperator = node;
    ExpressionNode*& parentsChildPointer = (ExpressionNode*&)node;

    // We only need to do the transformation if the left operand 
    IndexerCallNode* indexer = Type::DynamicCast<IndexerCallNode*>(unaryOperator->Operand);
    if (indexer == nullptr)
    {
      // Walk the children (because they could have binary operators that also need to be transformed)
      context->Walker->GenericWalkChildren(this, unaryOperator, context);
      return;
    }

    // Based on the operator (if its a side effect operator...)
    switch (unaryOperator->Operator->TokenId)
    {
      // The ultimate side effect operator! Not a compound operator though so we only need to run Set
      case Grammar::Increment:
      case Grammar::Decrement:
      {
        // Build the nodes that perform the Get/Operator/Set
        // Binary and unary are similar, so this was refactored into a single function
        this->BuildGetSetSideEffectIndexerNodes(node, indexer, &UnaryOperatorNode::Operand, context);
        return;
      }

      // We hit another operator, but it wasn't a side effect operator (just continue visiting)
      default:
      {
        // Walk the children (because they could have binary operators that also need to be transformed)
        context->Walker->GenericWalkChildren(this, unaryOperator, context);
        return;
      }
    }
  }
  
  //***************************************************************************
  void Syntaxer::IndexerIndexerCall(IndexerCallNode*& node, TypingContext* context)
  {
    // No node directly points at a IndexerCallNode (typically just expression children)
    // This would otherwise be an unsafe assumption
    IndexerCallNode* indexer = node;
    ExpressionNode*& parentsChildPointer = (ExpressionNode*&)node;

    // Walk the children (because they could have binary operators that also need to be transformed)
    context->Walker->GenericWalkChildren(this, indexer, context);
    
    // this.GameBoard.Grid[this.ComputeIndexX(), yValue];
    // this.GameBoard.Grid.Get(this.ComputeIndexX(), yValue);
    // We remove the indexer call and replace it with the get call
    MemberAccessNode* getMember = new MemberAccessNode();
    getMember->Location = indexer->Location;
    getMember->LeftOperand = indexer->LeftOperand;
    indexer->LeftOperand = nullptr;
    getMember->Name = OperatorGet;
    getMember->Operator = Grammar::Access;
    FunctionCallNode* getCall = new FunctionCallNode();
    getCall->Location = indexer->Location;
    getCall->LeftOperand = getMember;
    parentsChildPointer = getCall;
    getCall->IsUsedAsStatement = node->IsUsedAsStatement;

    // Our get call has all the exact same arguments as the indexer
    // The indexer no longer owns these arguments
    getCall->Arguments = indexer->Arguments;
    indexer->Arguments.Clear();

    // We're done with the indexer (may want to store this later for formatting walkers)
    delete indexer;
    return;
  }

  //***************************************************************************
  void Syntaxer::ResolveMember(MemberAccessNode*& node, TypingContext* context)
  {
    // Process the left expression first
    context->Walker->Walk(this, node->LeftOperand, context);

    // If an error occurred, exit out
    if (this->Errors.WasError)
      return;

    // The left operand only needs to be readable
    node->LeftOperand->IoUsage = IoMode::ReadRValue;
    
    // Get a reference to the left expression's type
    Type* leftType = node->LeftOperand->ResultType;

    // If the left hand side is the 'any' type, it means we're accessing a dynamic property
    // A dynamic property is one that is resolved by string at runtime
    AnyType* anyType = Type::DynamicCast<AnyType*>(leftType);
    if (anyType != nullptr)
    {
      // Set the member type (and resulting type)
      // We also want to set the access type to be a member, so that way anyone
      // who tries to modify it knows they can write directly to the member
      node->MemberType = MemberAccessType::Dynamic;
      //node->Access.Type     = OperandType::;????

      // Right now, we're only allowing reading from 'any' values (no writing to them)
      node->Io = IoMode::ReadRValue;
      //node->Io = (IoMode::Enum)(IoMode::ReadRValue | IoMode::WriteLValue);

      // The resulting type will always end up being the 'any' type (which allows for chaining)
      node->ResultType = ZilchTypeId(Any);
      this->ErrorAt(node, ErrorCode::GenericError,
        "Accessing members on the 'Any' type will result in dynamically looking up the value, however this is not yet supported");
      // Exit out early
      return;
    }

    // Check if the left type is a bound type because we can directly look up members on a bound type
    BoundType* boundType = Type::GetBoundType(leftType);
    if (boundType != nullptr)
    {
      // If this is accessing statics then use a static resolver, otherwise we're accessing instance members
      StaticTypeNode* staticType = Type::DynamicCast<StaticTypeNode*>(node->LeftOperand);
      if (staticType != nullptr)
      {
        // We're accessing a static member
        node->IsStatic = true;
        this->ResolveMemberAccess(node, Resolver::Static(boundType));
      }
      else
      {
        // We're accessing an instance member
        node->IsStatic = false;
        this->ResolveMemberAccess(node, Resolver::Instance(boundType));
      }
    }
    else
    {
      // Only bound types have actual members that can be looked up
      // Any types are handled above and use a special dynamic access
      this->ErrorAt(node, ErrorCode::MemberNotFound, node->Name.c_str(), leftType->ToString().c_str());
    }
  }
}
