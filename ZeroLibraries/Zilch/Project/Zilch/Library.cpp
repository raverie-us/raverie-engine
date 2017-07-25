/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  ZilchDefineType(Library, builder, type)
  {
  }

  //***************************************************************************
  const BoundFn LibraryBuilder::DoNotGenerate = (BoundFn)0x01;
  const BoundFn LibraryBuilder::NoOperation = (BoundFn)0x02;

  //***************************************************************************
  InstantiateTemplateDelegate::InstantiateTemplateDelegate() :
    Callback(nullptr),
    UserData(nullptr)
  {
  }

  //***************************************************************************
  String InstantiateTemplateInfo::GetFullName(const Array<Constant>& templateArguments)
  {
    // Create a string builder to build the template name
    StringBuilder builder;

    // Start with the base name, eg 'Array'
    builder.Append(this->TemplateBaseName);

    // Use the template bracket
    builder.Append('[');

    // Loop through the arguments and Append the argument type names
    for (size_t i = 0; i < templateArguments.Size(); ++i)
    {
      // Append the argument type name
      builder.Append(templateArguments[i].ToString());

      // Figure out whether we're at the end of the list or not
      bool isNotAtEnd = (i != templateArguments.Size() - 1);

      // As long as we aren't at the end of the list...
      if (isNotAtEnd)
      {
        // Add an argument separator between the template types
        builder.Append(", ");
      }
    }

    // Close the template bracket
    builder.Append(']');

    // Return the name
    return builder.ToString();
  }

  //***************************************************************************
  void VisitBoundTypeLibraryArray(const HashSet<LibraryRef>& librarySet, HashSet<BoundType*>& visitedTypes, BoundType* type, Array<BoundType*>& typesInOrder)
  {
    // If we already visited this type, then skip it
    if (visitedTypes.Contains(type))
      return;
    visitedTypes.Insert(type);

    // Check if we have a base type, if so visit it first (pretty much the entire point of this!)
    BoundType* base = type->BaseType;
    if (base != nullptr && librarySet.Contains(base->SourceLibrary))
      VisitBoundTypeLibraryArray(librarySet, visitedTypes, base, typesInOrder);

    // Push back our type (note that base types would have been pushed before ours)
    typesInOrder.PushBack(type);
  }

  //***************************************************************************
  void ComputeTypesInDependencyOrder
  (
    const Array<LibraryRef>& libraries,
    HashSet<LibraryRef>& outLibrarySet,
    Array<BoundType*>& outOrderedTypes
  )
  {
    HashSet<BoundType*> visitedTypes;

    outLibrarySet.Append(libraries.All());

    ZilchForEach(const LibraryRef& library, libraries)
    {
      // Loop through all types created in this library
      ZilchForEach(Type* type, library->OwnedTypes)
      {
        // Only consider bound types...
        BoundType* boundType = Type::DynamicCast<BoundType*>(type);
        if (boundType == nullptr)
          continue;

        // Visiting a type walks its base types first, and then adds them to the 'typesOut' array
        VisitBoundTypeLibraryArray(outLibrarySet, visitedTypes, boundType, outOrderedTypes);
      }
    }
  }

  //***************************************************************************
  size_t DelegateTypePolicy::operator() (DelegateType* type) const
  {
    return (size_t)type->Hash();
  }

  //***************************************************************************
  bool DelegateTypePolicy::Equal(DelegateType* a, DelegateType* b) const
  {
    // Make sure we have the same number of parameters
    if (a->Parameters.Size() != b->Parameters.Size())
      return false;

    // Make sure the returns are the same type (or non existant type for none)
    if (a->Return != b->Return)
      return false;

    // Loop through and compare each of the parameters
    for (size_t i = 0; i < a->Parameters.Size(); ++i)
    {
      // Grab the two parameters
      const DelegateParameter& aParameter = a->Parameters[i];
      const DelegateParameter& bParameter = b->Parameters[i];

      // Compare the types
      if (aParameter.ParameterType != bParameter.ParameterType)
        return false;
    }

    // If we got here, then it must match!
    return true;
  }

  //***************************************************************************
  LibraryBuilder::LibraryBuilder(StringParam name, StringParam namespaceForPlugins) :
    UserData(nullptr),
    ComputedDelegateAndFunctionSizes(false),
    CreatableInScriptDefault(true)
  {
    // Start out by creating a new library that we'll populate
    this->BuiltLibrary = LibraryRef(new Library());
    this->BuiltLibrary->Name = name;
    this->BuiltLibrary->NamespaceForPlugins = namespaceForPlugins;
  }

  //***************************************************************************
  String LibraryBuilder::GetName()
  {
    return this->BuiltLibrary->Name;
  }

  //***************************************************************************
  Function* LibraryBuilder::AddBoundFunction
  (
    BoundType* owner,
    StringParam name,
    BoundFn function,
    const ParameterArray& parameters,
    Type* returnType,
    FunctionOptions::Flags options,
    NativeVirtualInfo nativeVirtual
  )
  {
    // First add a raw function to the library
    Function* func = this->CreateRawFunction(owner, name, function, parameters, returnType, options, nativeVirtual);

    // If the function is valid...
    if (func != nullptr)
    {
      // Add the function to the bound type
      owner->AddRawFunction(func);
    }

    // Return the function that was created
    return func;
  }

  //***************************************************************************
  Function* LibraryBuilder::AddExtensionFunction
  (
    BoundType* forType,
    StringParam name,
    BoundFn function,
    const ParameterArray& parameters,
    Type* returnType,
    FunctionOptions::Flags options
  )
  {
    // First add a raw function to the library
    Function* createdFunction = this->CreateRawFunction(forType, name, function, parameters, returnType, options);

    // If the function is valid...
    if (createdFunction != nullptr)
      this->AddRawExtensionFunction(createdFunction);

    // Return the function that was created
    return createdFunction;
  }

  //***************************************************************************
  void LibraryBuilder::AddRawExtensionFunction(Function* function)
  {
    // Get the guid for the type that we're extending
    BoundType* forType = function->Owner;
    GuidType guid = forType->Hash();

    // Store the extension map (could be static or instance)
    FunctionMultiMap* overloadedFunctionsByName = nullptr;

    // Add the property to the library extension map
    if (function->This != nullptr)
      overloadedFunctionsByName = &this->BuiltLibrary->InstanceExtensionFunctions[guid];
    else
      overloadedFunctionsByName = &this->BuiltLibrary->StaticExtensionFunctions[guid];

    // Get the array of overloaded functions
    FunctionArray& overloads = (*overloadedFunctionsByName)[function->Name];

    // Add the property to the named map
    overloads.PushBack(function);
  }

  //***************************************************************************
  Function* LibraryBuilder::AddBoundPreConstructor(BoundType* owner, BoundFn function)
  {
    // Error checking
    ErrorIf(owner->PreConstructor != nullptr,
      "A type cannot have two pre-constructors! A pre-constructor is a simple "
      "initializer (like memset) that runs before any constructors get run");

    // First add a raw function to the library
    Function* func = this->CreateRawPreConstructor(owner, function);
    
    // If the function is valid...
    if (func != nullptr)
    {
      // Add the function to the bound type
      owner->PreConstructor = func;
    }

    // Return the function that was created
    return func;
  }
  
  //***************************************************************************
  Function* LibraryBuilder::AddBoundConstructor(BoundType* owner, BoundFn function, const ParameterArray& parameters)
  {
    // First add a raw function to the library
    Function* func = this->CreateRawConstructor(owner, function, parameters);
    
    // If the function is valid...
    if (func != nullptr)
    {
      // Add the function to the bound type
      owner->Constructors.PushBack(func);
    }

    // Return the function that was created
    return func;
  }
  
  //***************************************************************************
  Function* LibraryBuilder::AddBoundDefaultConstructor(BoundType* owner, BoundFn function)
  {
    // First add a raw function to the library
    Function* func = this->CreateRawDefaultConstructor(owner, function);

    // If the function is valid...
    if (func != nullptr)
    {
      // Add the function to the bound type
      owner->Constructors.PushBack(func);
    }

    // Return the function that was created
    return func;
  }
  
  //***************************************************************************
  Function* LibraryBuilder::AddBoundDestructor(BoundType* owner, BoundFn function)
  {
    // Error checking
    ErrorIf(owner->Destructor != nullptr,
      "A type cannot have two destructors! A destructor is a function "
      "that is responsible for cleaning up after an object.");

    // First add a raw function to the library
    Function* func = this->CreateRawDestructor(owner, function);

    // If the function is valid...
    if (func != nullptr)
    {
      // Make sure the user doesn't define two destructors
      ErrorIf(owner->Destructor != nullptr,
        "A class may only have one destructor");

      // Add the function to the bound type
      owner->Destructor = func;
    }

    // Return the function that was created
    return func;
  }

  //***************************************************************************
  GetterSetter* LibraryBuilder::AddBoundGetterSetter(BoundType* owner, StringParam name, Type* type, BoundFn set, BoundFn get, MemberOptions::Flags options)
  {
    // First add a raw property to the library
    GetterSetter* property = this->CreateRawGetterSetter(owner, name, type, set, get, options);

    // If the property is valid...
    if (property != nullptr)
    {
      // Add the property to the bound type
      owner->AddRawGetterSetter(property);

      // Make sure to add the getter function to the bound functions list
      if (property->Get != nullptr)
        owner->AddRawFunction(property->Get);

      // Make sure to add the setter function to the bound functions list
      if (property->Set != nullptr)
        owner->AddRawFunction(property->Set);
    }

    // Return the property that was created
    return property;
  }

  //***************************************************************************
  GetterSetter* LibraryBuilder::AddEnumValue(BoundType* owner, StringParam name, Integer value)
  {
    ErrorIf(Type::IsEnumOrFlagsType(owner) == false, "Use ZilchFullBindEnum before calling ZilchFullBindEnumValue");
    ErrorIf(owner->Size != sizeof(Integer), "Must be the same size as an Integer");
    GetterSetter* getset = this->AddBoundGetterSetter
    (
      owner,
      name,
      owner,
      nullptr,
      &ZZ::VirtualMachine::EnumerationProperty,
      ZZ::MemberOptions::Static
    );
    getset->Get->UserData = (void*)(size_t)(value);

    owner->PropertyToEnumValue[getset] = value;
    owner->StringToEnumValue[name] = value;
    owner->EnumValueToProperties[value].PushBack(getset);
    owner->EnumValueToStrings[value].PushBack(name);

    if (owner->DefaultEnumProperty == nullptr)
    {
      owner->DefaultEnumProperty = getset;
      owner->DefaultEnumValue = value;
    }

    return getset;
  }

  //***************************************************************************
  InstantiatedTemplate LibraryBuilder::InstantiateTemplate(StringParam baseName, const Array<Constant>& arguments, const LibraryArray& fromLibraries)
  {
    // What we'll return to the user
    InstantiatedTemplate result;
    
    // Assume we failed because we couldn't find anything of the same name
    result.Result = TemplateResult::FailedNameNotFound;

    // Loop through all of our dependent libraries
    for (size_t i = 0; i < fromLibraries.Size(); ++i)
    {
      // Get the current dependent library
      const LibraryRef& library = fromLibraries[i];

      // Get a pointer (or null) to the callback delegate
      InstantiateTemplateInfo* info = library->TemplateHandlers.FindPointer(baseName);

      // If there is a template by that name...
      if (info != nullptr)
      {
        // If the number of arguments is valid
        if (arguments.Size() != info->TemplateParameters.Size())
        {
          // Fail out because the user provided an invalid number of arguments
          result.Result = TemplateResult::FailedInvalidArgumentCount;
          result.ExpectedArguments = info->TemplateParameters.Size();
          return result;
        }

        // Validate that all the template argument types match
        for (size_t i = 0; i < info->TemplateParameters.Size(); ++i)
        {
          // If the types of the constants do not match with the expected template type...
          if (arguments[i].Type != info->TemplateParameters[i].Type)
          {
            // Fail out because the user provided an invalid number of arguments
            result.Result = TemplateResult::FailedArgumentTypeMismatch;
            return result;
          }
        }

        // Get a reference to the delegate for convenience
        InstantiateTemplateDelegate& delegate = info->Delegate;

        // Get the fully qualified name
        String fullyQualifiedTemplateName = info->GetFullName(arguments);

        // Check to see if we already have this type...
        result.Type = this->BoundTypes.FindValue(fullyQualifiedTemplateName, nullptr);

        // If we didn't find it, look through all of the dependent libraries
        if(result.Type == nullptr)
        {
          ZilchForEach(Library* dependentLibrary, fromLibraries)
          {
            result.Type = dependentLibrary->BoundTypes.FindValue(fullyQualifiedTemplateName, nullptr);
            if(result.Type != nullptr)
              break;
          }
        }

        // If we didn't find the type (it was not yet instantiated)
        if (result.Type == nullptr)
        {
          // Invoke the callback to instantiate the type
          result.Type = delegate.Callback(*this, baseName, fullyQualifiedTemplateName, arguments, delegate.UserData);

          // If we found the created type
          if (result.Type != nullptr)
          {
            // We successfully instantiated a template
            result.Result = TemplateResult::Success;
            result.Type->TemplateBaseName = baseName;

            // If anyone wants to query which template arguments were
            // used to instantiate this object, store them on the type
            result.Type->TemplateArguments = arguments;

            // Error checking
            ErrorIf(result.Type->Name != fullyQualifiedTemplateName,
              "The template instantiator needs to create a type with the given fully qualified name");
          }
          else
          {
            // We performed the callback, but got no type back
            result.Result = TemplateResult::FailedInstantiatorDidNotReturnType;
          }
        }
        else
        {
          // We successfully found a template
          result.Result = TemplateResult::Success;
        }

        // We found the instantiator and attempted to use it, so break out!
        break;
      }
    }

    // Return the result with the type or an error
    return result;
  }

  //***************************************************************************
  GetterSetter* LibraryBuilder::AddExtensionGetterSetter(BoundType* forType, StringParam name, Type* type, BoundFn set, BoundFn get, MemberOptions::Flags options)
  {
    // First add a raw property to the library
    GetterSetter* property = this->CreateRawGetterSetter(forType, name, type, set, get, options);

    // If the property is valid...
    if (property != nullptr)
    {
      // Get the guid for the type that we're extending
      GuidType guid = forType->Hash();

      // Store the extension map (could be static or instance)
      GetterSetterMap* propertiesByName = nullptr;

      // We also need to get the function extension map so we can add the getter and setter functions
      // Technically these functions aren't referencable, except if native code looks them up (just to be proper!)
      FunctionMultiMap* functionsByName = nullptr;
      
      // Add the property to the library extension map
      if (options & FunctionOptions::Static)
      {
        propertiesByName = &this->BuiltLibrary->StaticExtensionGetterSetters[guid];
        functionsByName = &this->BuiltLibrary->StaticExtensionFunctions[guid];
      }
      else
      {
        propertiesByName = &this->BuiltLibrary->InstanceExtensionGetterSetters[guid];
        functionsByName = &this->BuiltLibrary->InstanceExtensionFunctions[guid];
      }

      // Add the property to the named map
      bool inserted = propertiesByName->InsertNoOverwrite(name, property);
      ErrorIf(inserted == false,
        "Another property with the same name (%s) was added to the extension methods map for type (%s), "
        "or two types came out with the same guid (which would be bad)",
        name.c_str(), forType->Name.c_str());

      // Make sure to add the getter function to the bound functions list
      if (property->Get != nullptr)
        (*functionsByName)[property->Get->Name].PushBack(property->Get);

      // Make sure to add the setter function to the bound functions list
      if (property->Set != nullptr)
        (*functionsByName)[property->Set->Name].PushBack(property->Set);
    }

    // Return the property that was created
    return property;
  }

  //***************************************************************************
  SendsEvent* LibraryBuilder::AddSendsEvent(BoundType* forType, StringParam name, BoundType* sentType, StringParam description)
  {
    ErrorIf(sentType == nullptr, "Event type must be provided");

    forType->SendsEvents.PushBack(new SendsEvent(forType, name, sentType));
    SendsEvent* sendsEvent = forType->SendsEvents.Back();

    // Get the extension properties for the events type
    GuidType guid = ZilchTypeId(EventsClass)->Hash();
    GetterSetterMap& properties = this->BuiltLibrary->StaticExtensionGetterSetters[guid];

    // Check to see if we do not yet have a property by this name (if we do, we won't bother to create another)
    Property* existingProperty = properties.FindValue(sendsEvent->Name, nullptr);
    if (existingProperty == nullptr)
    {
      // Create an extension property on the events object so that the event can be accessed by name
      Property* property = this->AddExtensionGetterSetter
      (
        ZilchTypeId(EventsClass),
        name,
        ZilchTypeId(String),
        nullptr,
        VirtualMachine::EventsProperty,
        MemberOptions::Static
      );
      sendsEvent->EventProperty = property;

      // Store the event name in the complex user data (the 'EventsProperty' will pull it out when it needs it)
      property->Get->ComplexUserData.WriteObject(name);

      // Add in the type that the event declares it sends
      property->Description = BuildString(property->Description, "Event Type: ", sentType->ToString(), ". ", description);
    }
    else
    {
      // This isn't quite correct, but since we don't actually care about the event type...
      ZilchTodo("Don't allow two sends events with the same name, unless maybe they are the same type?");
      sendsEvent->EventProperty = existingProperty;
    }

    return sendsEvent;
  }

  //***************************************************************************
  Field* LibraryBuilder::AddBoundField(BoundType* owner, StringParam name, Type* type, size_t offset, MemberOptions::Flags options)
  {
    // First add a raw field to the library
    Field* field = this->CreateRawField(owner, name, type, offset, options);

    // If the field is valid...
    if (field != nullptr)
    {
      // Add the field to the bound type
      owner->AddRawField(field);
    }

    // Return the field that was created
    return field;
  }

  //***************************************************************************
  Function* LibraryBuilder::CreateRawFunction
  (
    BoundType* owner,
    String name,
    BoundFn boundFunction,
    const ParameterArray& parameters,
    Type* returnType,
    FunctionOptions::Flags options,
    NativeVirtualInfo nativeVirtual
  )
  {
    // Verify that the name is correct
    name = FixIdentifier(name, TokenCheck::IsUpper);

    // Error checking for our owner type
    ReturnIf(owner == nullptr, nullptr, "The owner must never be null");

    // Error checking for the native virtual calls
    if (nativeVirtual.Validate() == false)
      return nullptr;

    // Create the actual function we use for the compiler
    // Note that we leave the source library as not set,
    // because the library has yet to be built
    Function* function      = new Function();
    function->Name          = name;
    function->BoundFunction = boundFunction;
    function->Owner         = owner;

    // Setup any native virtual behavior
    function->NativeVirtual = nativeVirtual;

    // If the user is creating a native virtual function...
    if (nativeVirtual.Index != NativeVirtualInfo::NonVirtual)
    {
      // Just make sure the user marked this function as virtual, just for consistency
      ErrorIf((options & FunctionOptions::Virtual) == 0,
        "The FunctionOptions::Virtual flag must be set when creating a native virtual function");
      options |= FunctionOptions::Virtual;
    }

    // If the function is virtual...
    if (options & FunctionOptions::Virtual)
    {
      // The function cannot also be static
      ErrorIf((options & FunctionOptions::Static) != 0,
        "Static functions cannot be marked as virtual");

      // Mark the function as being virtual
      function->IsVirtual = true;
    }

    // Create a delegate type for the function
    function->FunctionType = this->GetDelegateType(parameters, returnType);
    function->IsStatic = (options & FunctionOptions::Static) != 0;

    // If the function is static (we have no 'this' reference)
    if (function->IsStatic)
    {
      // Otherwise, the this pointer is null since it's a static method
      function->This = nullptr;
    }
    else
    {
      // Create a variable for the this pointer
      Variable* thisVariable = this->CreateRawVariable(function, ThisKeyword);
      thisVariable->ResultType = this->ToHandleType(owner);
      function->This = thisVariable;
    }
    
    // Push the function into a list
    this->BuiltLibrary->OwnedFunctions.PushBack(function);

    // Return the created function
    return function;
  }

  //***************************************************************************
  Function* LibraryBuilder::CreateRawPreConstructor(BoundType* owner, BoundFn function)
  {
    // Get a reference to the core library
    Core& core = Core::GetInstance();

    // Create the function
    return this->CreateRawFunction(owner, PreConstructorName, function, ParameterArray(), core.VoidType, FunctionOptions::None);
  }

  //***************************************************************************
  Function* LibraryBuilder::CreateRawConstructor(BoundType* owner, BoundFn function, const ParameterArray& parameters)
  {
    // Get a reference to the core library
    Core& core = Core::GetInstance();

    // If the type is native, then we want this constructor to call SetNativeTypeFullyConstructed on the handle manager
    BoundFn invokedFunction = function;
    if (owner->Native)
      invokedFunction = VirtualMachine::NativeConstructor;

    // Create the constructor and set the 'original' bound-function on it (may not be used)
    Function* constructor = this->CreateRawFunction(owner, ConstructorName, invokedFunction, parameters, core.VoidType, FunctionOptions::None);
    constructor->NativeConstructor = function;
    return constructor;
  }

  //***************************************************************************
  Function* LibraryBuilder::CreateRawDefaultConstructor(BoundType* owner, BoundFn function)
  {
    return this->CreateRawConstructor(owner, function, ParameterArray());
  }

  //***************************************************************************
  Function* LibraryBuilder::CreateRawDestructor(BoundType* owner, BoundFn function)
  {
    // Get a reference to the core library
    Core& core = Core::GetInstance();

    // Create the function
    return this->CreateRawFunction(owner, DestructorName, function, ParameterArray(), core.VoidType, FunctionOptions::None);
  }

  //***************************************************************************
  GetterSetter* LibraryBuilder::CreateRawGetterSetter(BoundType* owner, String name, Type* type, BoundFn set, BoundFn get, MemberOptions::Flags options)
  {
    // Verify that the name is correct
    name = FixIdentifier(name, TokenCheck::IsUpper);

    // Create the property
    GetterSetter* property = new GetterSetter();
    property->Owner = owner;

    // Pass it to the library so it can delete the property
    this->BuiltLibrary->OwnedProperties.PushBack(property);

    // Set the name of the property
    property->Name = name;

    // Set the type of the property
    property->PropertyType = type;

    // The functions are always positional
    FunctionOptions::Flags functionOptions = FunctionOptions::None;

    // If we are adding a static property
    if (options & MemberOptions::Static)
    {
      // Set this property as static (and any functions we generate as static)
      property->IsStatic = true;
      functionOptions |= FunctionOptions::Static;
    }
    
    // We must have at least a get or a set (or both)
    if (get != nullptr || set != nullptr)
    {
      // Check if we were given a get function
      if (get != nullptr && get != DoNotGenerate)
      {
        // Generate the get function
        property->Get = this->CreateRawFunction
        (
          owner,
          BuildGetterName(name),
          get,
          ParameterArray(),
          type,
          functionOptions
        );
        property->Get->IsHidden = true;
        property->Get->OwningProperty = property;
      }

      // Check if we were given a set function
      if (set != nullptr && set != DoNotGenerate)
      {
        // The set takes a single parameter
        ParameterArray parameters;
        DelegateParameter& value = parameters.PushBack();
        value.Name = ValueKeyword;
        value.ParameterType = type;

        // Get a reference to the core so we can get the void type (setters have no return value)
        Core& core = Core::GetInstance();

        // Generate the set function
        property->Set = this->CreateRawFunction
        (
          owner,
          BuildSetterName(name),
          set,
          parameters,
          core.VoidType,
          functionOptions
        );
        property->Set->IsHidden = true;
        property->Set->OwningProperty = property;
      }
    }
    else
    {
      Error("Properties must have at least a getter or a setter (or both).");
    }

    // Return a reference to the property
    return property;
  }

  //***************************************************************************
  const String& LibraryBuilder::AddStringLiteral(cstr text)
  {
    this->BuiltLibrary->StringLiterals.PushBack(text);
    return this->BuiltLibrary->StringLiterals.Back();
  }
  
  //***************************************************************************
  const String& LibraryBuilder::AddStringLiteral(StringParam text)
  {
    this->BuiltLibrary->StringLiterals.PushBack(text);
    return this->BuiltLibrary->StringLiterals.Back();
  }
  
  //***************************************************************************
  const String& LibraryBuilder::AddStringLiteral(StringRange text)
  {
    this->BuiltLibrary->StringLiterals.PushBack(text);
    return this->BuiltLibrary->StringLiterals.Back();
  }

  //***************************************************************************
  void FieldSetter(Call& call, ExceptionReport& report)
  {
    // Get the field and its offset
    Field* field = (Field*)call.GetFunction()->UserData;

    // Grab a handle to the this object and dereference it
    Handle& handle = call.GetHandle(Call::This);
    byte* memory = handle.Dereference();

    // Get the memory offsetted to the field
    byte* fieldMemory = memory + field->Offset;

    // Get a direct pointer to the parameter (could be a primitive, such as a handle)
    byte* parameterMemory = call.GetParameterUnchecked(0);

    // Generically destruct the existing field (release reference, etc) the current parameter's memory
    field->PropertyType->GenericDestruct(fieldMemory);

    // Now copy construct the parameter they are trying to set over the field memory
    field->PropertyType->GenericCopyConstruct(fieldMemory, parameterMemory);
  }

  //***************************************************************************
  void FieldGetter(Call& call, ExceptionReport& report)
  {
    // Get the field and its offset
    Field* field = (Field*)call.GetFunction()->UserData;

    // Grab a handle to the this object and dereference it
    Handle& handle = call.GetHandle(Call::This);
    byte* memory = handle.Dereference();

    // Get the memory offsetted to the field
    byte* fieldMemory = memory + field->Offset;

    // Get a direct pointer to the return (not initialized yet!)
    byte* returnMemory = call.GetReturnUnchecked();
    call.DisableReturnChecks();

    // Now copy construct the parameter they are trying to set over the field memory
    field->PropertyType->GenericCopyConstruct(returnMemory, fieldMemory);
  }

  //***************************************************************************
  Field* LibraryBuilder::CreateRawField(BoundType* owner, String name, Type* type, size_t offset, MemberOptions::Flags options)
  {
    // Verify that the name is correct
    name = FixIdentifier(name, TokenCheck::IsUpper);

    // Create the member
    Field* field = new Field();
    field->Owner = owner;

    // Pass it to the library so it can delete the member
    this->BuiltLibrary->OwnedProperties.PushBack(field);

    // Set the name of the member
    field->Name = name;

    // Set the type of the member
    field->PropertyType = type;

    // Set whether or not the member is static (accessed on a class or instance)
    field->IsStatic = (options & MemberOptions::Static) != 0;

    // Set the member's offset to the given offset
    field->Offset = offset;

    // Return a reference to the member
    return field;
  }

  //***************************************************************************
  Variable* LibraryBuilder::CreateRawVariable(Function* function, String name)
  {
    // Create the variable and set it's name
    Variable* variable = new Variable();
    variable->Owner = function;
    variable->Name = FixIdentifier(name, TokenCheck::IsLower);
    variable->Location = function->Location;

    // Pass it to the library so it can delete the member
    this->BuiltLibrary->OwnedVariables.PushBack(variable);

    // Also add it to the function (so it can be debugged easily)
    function->Variables.PushBack(variable);

    // Return the created variable
    return variable;
  }
  
  //***************************************************************************
  void LibraryBuilder::AddTemplateInstantiator(StringParam baseName, InstantiateTemplateCallback callback, const StringArray& templateTypeParameters, void* userData)
  {
    Array<TemplateParameter> parameters;
    parameters.Reserve(templateTypeParameters.Size());

    // Create actual template parameters from the string names
    for (size_t i = 0; i < templateTypeParameters.Size(); ++i)
    {
      TemplateParameter& parameter = parameters.PushBack();
      parameter.Name = templateTypeParameters[i];
      parameter.Type = ConstantType::Type;
    }
    
    this->AddTemplateInstantiator(baseName, callback, parameters, userData);
  }

  //***************************************************************************
  void LibraryBuilder::AddTemplateInstantiator(StringParam baseName, InstantiateTemplateCallback callback, const Array<TemplateParameter>& templateParameters, void* userData)
  {
    // Create the delegate that we will call
    InstantiateTemplateInfo info;
    info.Delegate.Callback = callback;
    info.Delegate.UserData = userData;
    info.TemplateParameters = templateParameters;
    info.TemplateBaseName = baseName;

    // Perform the callback
    bool inserted = this->BuiltLibrary->TemplateHandlers.InsertNoOverwrite(baseName, info);
    ErrorIf(inserted == false, "Another template instantiator of the same name (%s) was added to the Library Builder.", baseName.c_str());
  }

  //***************************************************************************
  BoundType* LibraryBuilder::AddBoundType(StringParam name, TypeCopyMode::Enum copyMode, size_t size, size_t nativeVirtualCount)
  {
    // Create a new bound type with the given name / size
    BoundType* type = new BoundType(nullptr);
    type->Name = name;
    type->CopyMode = copyMode;
    type->Size = size;
    type->RawNativeVirtualCount = nativeVirtualCount;
    type->CreatableInScript = this->CreatableInScriptDefault;
    
    // This maps the type by name within the library
    this->AddNativeBoundType(type);

    // Return the created type
    return type;
  }

  //***************************************************************************
  void LibraryBuilder::AddNativeBoundType(BoundType* type)
  {
    // Every value type also gets a corresponding ref (indirection) type
    if (type->CopyMode == TypeCopyMode::ValueType)
    {
      // Create a newly qualified type
      IndirectionType* indirectionType = new IndirectionType();

      // Make sure to store a strong reference, since we now own the type
      this->BuiltLibrary->OwnedTypes.PushBack(indirectionType);

      // Setup the qualifiers and the true type that it references
      indirectionType->ReferencedType = type;
      indirectionType->UserData = type->UserData;
      indirectionType->SourceLibrary = this->BuiltLibrary.Object;
      type->IndirectType = indirectionType;
    }

    // Make sure to store a strong reference, since we now own the type
    this->BuiltLibrary->OwnedTypes.PushBack(type);

    // Map the bound type's name to its type object
    bool inserted = this->BoundTypes.InsertNoOverwrite(type->Name, type);
    ErrorIf(inserted == false, "Another type with the same name (%s) was added to the LibraryBuilder.", type->Name.c_str());
  }

  //***************************************************************************
  void LibraryBuilder::AddNativeBoundType(BoundType* type, BoundType* base, TypeCopyMode::Enum mode)
  {
    type->BaseType = base;
    type->CopyMode = mode;
    if (base)
    {
      if (base->IsInitialized() == false)
      {
        String message = String::Format(
          "The base type must be initialized before we initialize our type %s.\n----------------\n",
          type->Name.c_str());
        base->IsInitializedAssert(message.c_str());
      }
      type->HandleManager = base->HandleManager;
    }

    this->AddNativeBoundType(type);
  }

  //***************************************************************************
  BoundType* LibraryBuilder::FindBoundType(StringParam name)
  {
    return this->BoundTypes.FindValue(name, nullptr);
  }

  //***************************************************************************
  void LibraryBuilder::GenerateGetSetFields()
  {
    // Grab a reference to the library for convenience
    LibraryRef library = this->BuiltLibrary;

    // Loop through all the fields
    for (size_t i = 0; i < library->OwnedProperties.Size(); ++i)
    {
      // If the current property is a field...
      Field* field = Type::DynamicCast<Field*>(library->OwnedProperties[i]);
      if (field == nullptr)
        continue;

      // Function options for the getter and setter
      // In the future, this may be virtual too
      FunctionOptions::Enum functionOptions = FunctionOptions::None;

      // If the member is static, then the getter and setter are static too
      if (field->IsStatic)
        functionOptions = FunctionOptions::Static;

      // If we don't have a get function yet...
      if (field->Get == nullptr)
      {
        // Generate the get function
        field->Get = this->CreateRawFunction
        (
          field->Owner,
          BuildGetterName(field->Name),
          FieldGetter,
          ParameterArray(),
          field->PropertyType,
          functionOptions
        );
        field->Get->IsHidden = true;
        field->Get->OwningProperty = field;

        // The userdata for the getter and setter is just the original field
        // This works out well because in the Syntaxer, we defer the computation of offset
        // This way, we don't need to know the offset (the field will be updated!)
        field->Get->UserData = field;
        field->Owner->AddRawFunction(field->Get);
      }
      
      // If we don't have a set function yet...
      if (field->Set == nullptr)
      {
        // Generate the set function
        field->Set = this->CreateRawFunction
        (
          field->Owner,
          BuildSetterName(field->Name),
          FieldSetter,
          OneParameter(field->PropertyType, ValueKeyword),
          ZilchTypeId(void),
          functionOptions
        );
        field->Set->IsHidden = true;
        field->Set->OwningProperty = field;

        // The userdata for the getter and setter is just the original field
        // This works out well because in the Syntaxer, we defer the computation of offset
        // This way, we don't need to know the offset (the field will be updated!)
        field->Set->UserData = field;
        field->Owner->AddRawFunction(field->Set);
      }
    }
  }

  //***************************************************************************
  bool LibraryBuilder::CheckIdentifier(StringParam identifier, TokenCheck::Flags flags)
  {
    // Just attempt to fix the identifier (this will assert if the flag is set)
    String fixedIdentifier = FixIdentifier(identifier, flags);
    
    // Return if the output is the exact same as the input
    return (fixedIdentifier == identifier);
  }

  //***************************************************************************
  String LibraryBuilder::FixIdentifier(StringParam ident, TokenCheck::Flags flags, char invalidCharacter)
  {
    // We're going to be overwriting this identifier
    String identifier = ident;

    // If the identifier Contains the scope resolution operator from C++, take everything after it
    if ((flags & TokenCheck::IgnoreScopeResolution) == 0)
    {
      // If we found the scope resolution operator...
      static const String ScopeResolution("::");
      StringRange index = ident.FindLastOf(ScopeResolution);
      if (!index.Empty())
      {
        // If the end is simply an enum, ideally use the name before
        if (ident.EndsWith("::Enum"))
          identifier = identifier.SubString(identifier.Begin(), index.Begin());
        // Otherwise trim the identifier to everything after the scope resolution operator
        else
          identifier = identifier.SubString(index.End(), identifier.End());
      }
    }
      
    // Should we be asserting in this function?
    bool asserts = (flags & TokenCheck::Asserts) != 0;
    bool expectUpperIdentifier = (flags & TokenCheck::IsUpper) != 0;
    bool expectLowerIdentifier = (flags & TokenCheck::IsLower) != 0;
    bool removeOuterBrackets = (flags & TokenCheck::RemoveOuterBrackets) != 0;
    bool noMultipleInvalidCharacters = (flags & TokenCheck::NoMultipleInvalidCharacters) != 0;

    // Check if the given identifier was empty
    if (identifier.Empty())
    {
      ErrorIf(asserts, "The identifier must be at least one character in length!");
      if (expectLowerIdentifier)
        return EmptyLowerIdentifier;
      else
        return EmptyUpperIdentifier;
    }

    // Get the first and last character
    {
      Zero::Rune first = identifier.Front();
      Zero::Rune last  = identifier.Back();
      
      // Check if this is a special identifier
      if (first == '[' && last == ']')
      {
        // If the user wishes to remove the outer brackets, then strip them off and continue
        if (removeOuterBrackets)
        {
          StringIterator begin = identifier.Begin();
          StringIterator end = identifier.End();
          ++begin;
          --end;
          identifier = identifier.SubString(begin, end);
        }
        // Otherwise it doesn't matter what comes in between, this is special (hidden effectively
        else
          return identifier;
      }
    }

    // We're going to build the identifier name and return it
    StringBuilder builder;
    bool readFirstLetter = false;

    // Walk through all characters in the identifier
    StringRange identifierRange = identifier.All();
    while (!identifierRange.Empty())
    {
      Rune r = identifierRange.Front();

      // If we have yet to read the first letter...
      if (readFirstLetter == false)
      {
        // The first letter must always be [A-Z]
        if (CharacterUtilities::IsAlpha(r))
        {
          // We read a letter! Check to see if its a mismatch for the upper or lower identifier
          bool isUpper = CharacterUtilities::IsUpper(r);
          bool isLower = !isUpper;

          if (isUpper && expectLowerIdentifier)
          {
            // Append an lowercase version of the first letter
            ErrorIf(asserts, "The first letter '%c' must be lowercase", r);
            builder.Append(Zero::UTF8::ToLower(r.mValue));
            readFirstLetter = true;
          }
          else if (isLower && expectUpperIdentifier)
          {
            // Append an uppercase version of the first letter
            ErrorIf(asserts, "The first letter '%c' must be uppercase", r);
            // As a special case, we ignore the Hungarian notations (starting with a lowercase letter then an uppercase)
            // (below we also ignore the '_' prefix by not appending symbols at the start)
            // We need to peek at the next character (and make sure that's even valid)
            StringRange peekRange = identifierRange;
            peekRange.PopFront();
            bool isNextUpperOrUnderscore = !peekRange.Empty() && (CharacterUtilities::IsUpper(peekRange.Front()) || r.mValue == '_');

            // Automatically make the next character uppercase, unless we detected Hungarian notation
            if (isNextUpperOrUnderscore == false)
            {
              builder.Append(Zero::UTF8::ToUpper(r.mValue));
              readFirstLetter = true;
            }
          }
          else
          {
            // The first letter was fine, Append it as is
            builder.Append(r.mValue);
            readFirstLetter = true;
          }
        }
        else
        {
          // We don't Append anything here because we haven't hit the first character yet...
          ErrorIf(asserts, "The first character '%c' must be a letter", r);
        }
      }
      // If any part of the rest of the identifier is NOT alpha-numeric or underscore
      else if (CharacterUtilities::IsAlphaNumeric(r.mValue) == false && r.mValue != '_')
      {
        ErrorIf(asserts, "Character '%c' in the identifier must be either a letter, number, or underscore (no other symbols)", r);
        // We should only Append invalid characters if the user specified one (not null)
        // and if the user doesn't want multiple of the same character
        bool shouldAppendInvalidCharacter =
          invalidCharacter != '\0' &&
          (noMultipleInvalidCharacters == false ||
          builder.GetSize() == 0 ||
          builder[builder.GetSize() - 1] != invalidCharacter);

        // Append the invalid character
        if (shouldAppendInvalidCharacter)
          builder.Append(invalidCharacter);
      }
      // It was a completely valid character
      else
      {
        builder.Append(r.mValue);
      }

      identifierRange.PopFront();
    }

    // If the builder is empty, then we didn't event hit a valid first letter...
    // Perform the same behavior as above when the original identifier was empty
    if (builder.GetSize() == 0)
    {
      ErrorIf(asserts, "The identifier must be at least one character in length!");
      if (expectLowerIdentifier)
        return EmptyLowerIdentifier;
      else
        return EmptyUpperIdentifier;
    }

    // Get the fixed identifier of the builder and return it
    String fixedIdentifier = builder.ToString();

    // This just helps reduce memory and increase string comparison performance
    // If the 'fixed' version is exactly the same as the original, just return
    // the original and destroy the 'fixed' allocation
    if (identifier == fixedIdentifier)
      return identifier;

    return fixedIdentifier;
  }

  //***************************************************************************
  bool LibraryBuilder::CheckUpperIdentifier(StringParam identifier)
  {
    return CheckIdentifier(identifier, TokenCheck::IsUpper);
  }

  //***************************************************************************
  bool LibraryBuilder::CheckLowerIdentifier(StringParam identifier)
  {
    return CheckIdentifier(identifier, TokenCheck::IsLower);
  }

  //***************************************************************************
  void LibraryBuilder::SetEntries(const Array<CodeEntry>& entries)
  {
    this->BuiltLibrary->Entries = entries;
  }
  
  //***************************************************************************
  Plugin* LibraryBuilder::LoadPlugin(Status& status, StringParam pluginFile, void* userData)
  {
    // In order to not lock the library and support dynamic reloading, we make a copy of any plugin files
    // Ideally we want to load the same libraries and not duplicate code loading,
    // therefore we use the hash of the library to uniquely identify it
    File file;
    file.Open(pluginFile, Zero::FileMode::Read, Zero::FileAccessPattern::Sequential, Zero::FileShare::Read, &status);
    if (status.Failed())
    {
      status.SetFailed("We failed to open the plugin file for Read only access (does it exist or is there permission?)");
      return nullptr;
    }

    // If the file is empty, then skip it
    if (file.CurrentFileSize() == 0)
    {
      status.SetFailed("The plugin file was empty");
      return nullptr;
    }

    // Get the hash of the shared library and then close the file
    String sha1Hash = Sha1Builder::GetHashStringFromFile(file);
    file.Close();

    // Copy the library to a new temporary location
    StringRange pluginName = Zero::FilePath::GetFileNameWithoutExtension(pluginFile);
    String fileName = BuildString(pluginName, ".", sha1Hash, ".zilchPlugin");
    String pluginLocation = Zero::FilePath::Combine(Zero::GetTemporaryDirectory(), fileName);
    
    // Only copy if the file doesn't already exist
    if (Zero::FileExists(pluginLocation) == false)
    {
      // If we fail to copy the file, then just load the plugin directly...
      if (Zero::CopyFile(pluginLocation, pluginFile) == false)
        pluginLocation = pluginFile;
    }
    
    // Attempt to load the plugin file
    ExternalLibrary lib;
    lib.Load(status, pluginLocation.c_str());
    if (status.Failed())
      return nullptr;

    // If we failed to load the library, then early out
    if (lib.IsValid() == false)
    {
      status.SetFailed("The plugin dynamic/shared library was not a valid library and could not be loaded");
      return nullptr;
    }

    // Look for the create plugin functionality, early out if we don't find it
    CreateZilchPluginFn createPlugin = (CreateZilchPluginFn)lib.GetFunctionByName("CreateZilchPlugin");
    if (createPlugin == nullptr)
    {
      status.SetFailed("The 'CreateZilchPlugin' function was not exported within the dll (did you use the ZeroExport macro?)");
      return nullptr;
    }

    // Finally, attempt to create a plugin (the user should return us a plugin at this point)
    Plugin* plugin = createPlugin();
    if (plugin == nullptr)
    {
      status.SetFailed("We found the 'CreateZilchPlugin' function and called it, but it returned null so no plugin was created");
      return nullptr;
    }

    // We successfully loaded the plugin
    plugin->UserData = userData;

    // For now, we'll just keep the code loaded forever until we get a good mechanism for releasing plugins
    lib.mUnloadOnDestruction = false;
    UniquePointer<Plugin> ownedPlugin(plugin);
    this->BuiltLibrary->OwnedPlugins.PushBack(ZeroMove(ownedPlugin));
    return plugin;
  }

  //***************************************************************************
  void LibraryBuilder::ComputeDelegateAndFunctionSizesOnce()
  {
    // If we already computed the sizes/offsets, then early out
    if (this->ComputedDelegateAndFunctionSizes)
      return;

    // We only want to do this once (otherwise required stack offsets for functions can get messed up,
    // eg if we added locals and then went back and recomputed the sizes and overwrite the stack offsets)
    this->ComputedDelegateAndFunctionSizes = true;

    // For how we give parameter positions, check CodeGenerator.cpp around line 233
    // Returns go at the beginning, parameters go after returns, and the
    // this handle (if it exists) goes after the parameters

    // Create a range to iterate through all the delegate types
    Array<DelegateType*>::range delegateTypes = this->DelegateTypes.All();

    // Loop through all the delegate types in the range
    while (delegateTypes.Empty() == false)
    {
      // Get a pointer to the current type and iterate to the next
      DelegateType* delegateType = delegateTypes.Front();
      delegateTypes.PopFront();

      // Set the return stack offset, which is always zero
      delegateType->ReturnStackOffset = 0;

      // The cumulative offset of each parameter on the stack
      OperandIndex parameterStackOffset = 0;

      // Move the first parameter forward by the return value's size
      // (void has a size of zero, so this always works!)
      parameterStackOffset += (OperandIndex)AlignToBusWidth(delegateType->Return->GetCopyableSize());

      // Walk through the parameters and Assign stack offsets
      for (size_t i = 0; i < delegateType->Parameters.Size(); ++i)
      {
        // Grab the current delegate parameter
        DelegateParameter& parameter = delegateType->Parameters[i];

        // Set the parameter's stack offset
        parameter.Index = i;
        parameter.StackOffset = parameterStackOffset;

        // Push forward the next parameter's stack offset by this parameter's size
        parameterStackOffset += (OperandIndex)AlignToBusWidth(parameter.ParameterType->GetCopyableSize());

        // Get a reference to the core library
        Core& core = Core::GetInstance();

        // Make sure all delegates have returns
        ErrorIf(parameter.ParameterType == nullptr || parameter.ParameterType == core.VoidType,
          "Delegate/function parameters cannot be null or void");
      }

      // Finally, the 'this' handle comes right after all the parameters
      delegateType->ThisHandleStackOffset = parameterStackOffset;
      delegateType->TotalStackSizeExcludingThisHandle = parameterStackOffset;
    }

    // Now that we computed all delegate type sizes / stack offsets, we can update any functions to reflect those offsets
    // This is also generally required to be done before code generation so we know required stack size (just for parameters and returns)
    FunctionArray& functions = this->BuiltLibrary->OwnedFunctions;
    for (size_t i = 0; i < functions.Size(); ++i)
    {
      // The base required stack space for any function is the parameters and return sizes totaled
      Function* function = functions[i];
      function->RequiredStackSpace = function->FunctionType->TotalStackSizeExcludingThisHandle;
      function->ComputeHash();

      // If the function has a this handle
      if (function->This != nullptr)
      {
        // The this handle is the last parameter (after the returns and the parameters)
        function->This->Local = (OperandLocal)function->FunctionType->ThisHandleStackOffset;

        // Add the size of the this handle
        function->RequiredStackSpace += function->This->ResultType->GetCopyableSize();
      }
    }
  }
  
  //***************************************************************************
  String GetInheritedDescription(Function* function)
  {
    // FindFunction will walk up the base class chain for me, but we need to make sure we look for the same type of functions
    FindMemberOptions::Enum options = FindMemberOptions::None;
    if (function->IsStatic)
      options = FindMemberOptions::Static;

    // Walk up parent functions starting with our function
    Function* foundFunction = function;
    ZilchLoop
    {
      // If we have a valid description, return it, otherwise look for a
      // base class version of this function that may have a description
      if (foundFunction->Description.Empty() == false)
        return foundFunction->Description;

      // Grab the base type of the owner of this function (could be null)
      BoundType* baseType = foundFunction->Owner->BaseType;
      if (baseType == nullptr)
        break;

      // Find will automatically walk up base classes to look for the function
      // We are puposefully looking on our base class for the same exact function as ourselves
      // This will return null if we have no parent function
      foundFunction = baseType->FindFunction(function->Name, function->FunctionType, options);
      if (foundFunction == nullptr)
        break;
    }

    // We didn't find a valid description
    return String();
  }

  //***************************************************************************
  LibraryRef LibraryBuilder::CreateLibrary()
  {
    // Grab a reference to the library for convenience
    LibraryRef library = this->BuiltLibrary;

    // Compute all types in order from base to derived type
    // (may have already been done by the Syntaxer)
    library->ComputeTypesInDependencyOrderOnce();

    // Generate getters and setters for any field that hasn't already been generated
    this->GenerateGetSetFields();

    // We need to create all the bound functions
    for (size_t i = 0; i < library->OwnedFunctions.Size(); ++i)
    {
      // Get the current bound function
      Function* function = this->BuiltLibrary->OwnedFunctions[i];

      // If the function has no description, then grab it from its base class
      function->Description = GetInheritedDescription(function);

      // Compact the byte code into a single byte buffer (may be no opcode!)
      function->CompactedOpcode.Resize(function->OpcodeBuilder.RelativeSize());
      function->OpcodeBuilder.RelativeCompact(function->CompactedOpcode.Data());

      // Add the function to the library so it can be looked up
      function->SourceLibrary = library.Object;
    }

    // Create a range to iterate through all the named types
    BoundTypeValueRange boundTypes = this->BoundTypes.Values();

    // Loop through all the named types in the range
    while (boundTypes.Empty() == false)
    {
      // Get a pointer to the current type and iterate to the next
      BoundType* boundType = boundTypes.Front();
      boundTypes.PopFront();

      ErrorIf
      (
        boundType->BaseType != nullptr && boundType->BaseType->CopyMode != boundType->CopyMode,
        "The type %s must be bound with the same TypeCopyMode as its base class %s",
        boundType->Name.c_str(),
        boundType->BaseType->Name.c_str()
      );

      // Insert all the named types into the map
      library->BoundTypes.InsertOrError(boundType->Name, boundType);

      // Set the type's source library to the current library
      boundType->SourceLibrary = library.Object;
    }

    // Make sure all delegates know their sizes (may be computed more than once due to code-gen needing the sizes)
    this->ComputeDelegateAndFunctionSizesOnce();

    // Clear out our built library so we don't use the this builder anymore
    this->BuiltLibrary = nullptr;
    return library;
  }
  

  //***************************************************************************
  Type* LibraryBuilder::ToHandleType(BoundType* type)
  {
    // If the type is a reference type...
    if (type->CopyMode == TypeCopyMode::ReferenceType)
    {
      // The reference type itself is effectively a handle
      return type;
    }
    else
    {
      // Otherwise we need to make a reference to it
      return this->ReferenceOf(type);
    }
  }

  //***************************************************************************
  IndirectionType* LibraryBuilder::ReferenceOf(BoundType* type)
  {
    // If the type is a reference type...
    if (type->CopyMode == TypeCopyMode::ReferenceType)
    {
      // Show an error
      Error("It is not legal to get a reference of a non-value type");
      return nullptr;
    }

    ErrorIf(type->IndirectType == nullptr, "We don't have the indirect type for this type");
    return type->IndirectType;
  }

  //***************************************************************************
  BoundType* LibraryBuilder::Dereference(IndirectionType* qualifiedType)
  {
    // Just return the referenced type (there can only be one level of indirection
    return qualifiedType->ReferencedType;
  }

  //***************************************************************************
  DelegateType* LibraryBuilder::GetDelegateType(const ParameterArray& parameters, Type* returnType)
  {
    // Create a delegate type and fill it in
    DelegateType* delegateType = new DelegateType();
    delegateType->Parameters = parameters;
    delegateType->Return = returnType;
    delegateType->SourceLibrary = this->BuiltLibrary.Object;
    
    // Make sure all delegates have returns
    ErrorIf(returnType == nullptr,
      "To mark a delegate/function as having no return, use the void type via ZilchTypeId(void)");

    // Make sure to store a strong reference, since we now own the type
    this->BuiltLibrary->OwnedTypes.PushBack(delegateType);
    this->DelegateTypes.PushBack(delegateType);

    // Return the newly created delegate type
    return delegateType;
  }

  //***************************************************************************
  CodeEntry::CodeEntry() :
    CodeUserData(nullptr)
  {
  }
  
  //***************************************************************************
  size_t CodeEntry::GetHash()
  {
    return this->Code.Hash() ^ this->Origin.Hash() * 5689;
  }

  //***************************************************************************
  Library::Library() :
    UserData(nullptr),
    GeneratedDefinitionStubCode(false),
    TolerantMode(false),
    CreatedByPlugin(false)
  {
  }
  
  //***************************************************************************
  void Library::GenerateDefinitionStubCode()
  {
    // If we already generated the stub code, our library should not have changed so early out
    if (this->GeneratedDefinitionStubCode)
      return;
    this->GeneratedDefinitionStubCode = true;

    // Walk through all bound types and generate stub code for each one
    ZilchForEach(BoundType* type, this->BoundTypes.Values())
    {
      // Generate stub code for this bound type and set all the native
      // locations for the class/struct, properties, functions, etc
      StubCode stubGenerator;
      stubGenerator.SetNativeLocations = true;
      stubGenerator.GeneratedOriginOrName = BuildString("[", type->Name, "]");
      stubGenerator.Generate(type);
      stubGenerator.Finalize();
    }
  }

  //***************************************************************************
  void VisitBoundType(HashSet<BoundType*>& visitedTypes, BoundType* type, Array<BoundType*>& typesInOrder)
  {
    // If we already visited this type, then skip it
    if (visitedTypes.Contains(type))
      return;
    visitedTypes.Insert(type);

    // Check if we have a base type, if so visit it first (pretty much the entire point of this!)
    BoundType* base = type->BaseType;
    if (base != nullptr && base->SourceLibrary == type->SourceLibrary)
      VisitBoundType(visitedTypes, base, typesInOrder);

    // Push back our type (note that base types would have been pushed before ours)
    typesInOrder.PushBack(type);
  }
  
  //***************************************************************************
  void Library::ComputeTypesInDependencyOrderOnce()
  {
    // Ensure that we only compute these once
    if (!this->TypesInDependencyOrder.Empty())
      return;

    HashSet<BoundType*> visitedTypes;

    // Loop through all types created in this library
    for (size_t i = 0; i < this->OwnedTypes.Size(); ++i)
    {
      // Only consider bound types...
      BoundType* boundType = Type::DynamicCast<BoundType*>(this->OwnedTypes[i]);
      if (boundType == nullptr)
        continue;

      // Visiting a type walks its base types first, and then adds them to the 'typesOut' array
      VisitBoundType(visitedTypes, boundType, this->TypesInDependencyOrder);
    }
  }

  //***************************************************************************
  String Library::GetPluginNamespace()
  {
    if (!this->NamespaceForPlugins.Empty())
      return this->NamespaceForPlugins;

    return this->Name;
  }

  //***************************************************************************
  void Library::ClearComponents()
  {
    for (size_t i = 0; i < this->OwnedTypes.Size(); ++i)
    {
      Type* type = this->OwnedTypes[i];
      type->ClearComponents();
    }

    for (size_t i = 0; i < this->OwnedFunctions.Size(); ++i)
    {
      Function* function = this->OwnedFunctions[i];
      function->ClearComponents();
    }

    for (size_t i = 0; i < this->OwnedProperties.Size(); ++i)
    {
      Property* property = this->OwnedProperties[i];
      property->ClearComponents();
    }

    for (size_t i = 0; i < this->OwnedVariables.Size(); ++i)
    {
      Variable* variable = this->OwnedVariables[i];
      variable->ClearComponents();
    }
  }

  //***************************************************************************
  Library::~Library()
  {
    // First, release all components
    this->ClearComponents();

    ZilchForEach(Plugin* plugin, this->OwnedPlugins)
    {
      if (plugin->FullCompilationInitialized)
        plugin->Uninitialize();
    }

    for (size_t i = 0; i < this->OwnedTypes.Size(); ++i)
    {
      Type* type = this->OwnedTypes[i];
      delete type;
    }

    for (size_t i = 0; i < this->OwnedFunctions.Size(); ++i)
    {
      Function* function = this->OwnedFunctions[i];
      delete function;
    }

    for (size_t i = 0; i < this->OwnedProperties.Size(); ++i)
    {
      Property* property = this->OwnedProperties[i];
      delete property;
    }

    for (size_t i = 0; i < this->OwnedVariables.Size(); ++i)
    {
      Variable* variable = this->OwnedVariables[i];
      delete variable;
    }
  }
  
  //***************************************************************************
  Module::Module()
  {
    // Always add the core library (both for the linker, and as a compiler dependence)
    this->PushBack(Core::GetInstance().GetLibrary());
  }

  //***************************************************************************
  BoundType* Module::FindType(StringParam name)
  {
    // Loop through all libraries
    for (size_t i = 0; i < this->Size(); ++i)
    {
      // Grab the current library
      LibraryRef& library = (*this)[i];

      // Attempt to find the type in this library
      BoundType* type = library->BoundTypes.FindValue(name, nullptr);
      
      // If we found the type...
      if (type != nullptr)
        return type;
    }

    // Otherwise we found nothing
    return nullptr;
  }

  //***************************************************************************
  ExecutableState* Module::Link() const
  {
    // Create an executable state to link everything together into
    ExecutableState* state = new ExecutableState();

    // Store references to the dependent libraries we were compiled from
    state->Dependencies = *this;

    // Loop through all dependent libraries
    for (size_t i = 0; i < this->Size(); ++i)
    {
      // Grab the current library
      const LibraryRef& library = (*this)[i];

      // Loop through all code entries
      for (size_t j = 0; j < library->Entries.Size(); ++j)
      {
        // Grab the current code entry
        CodeEntry* entry = &library->Entries[j];

        // Map the code entry id to the entry itself on the state (we should never collide here!)
        state->CodeHashToCodeEntry.InsertNoOverwrite(entry->GetHash(), entry);
      }
    }

    return state;
  }

  //***************************************************************************
  void Module::BuildTypeDocumentation(BoundType* type, DocumentationType* docType)
  {
    // Add the type to the docs
    docType->Name = type->Name;

    // Set the description and remarks
    docType->Description = type->Description;
    docType->Remarks = type->Remarks;

    // If we have a base class, make sure we set it
    if (type->BaseType != nullptr)
    {
      // Set the base type name (we should be able to look it up later)
      docType->BaseName = type->BaseType->Name;
    }

    // If the type is a value type...
    if (type->CopyMode == TypeCopyMode::ValueType)
    {
      docType->IsValueType = true;
    }

    // Get the constructor functions
    this->BuildFunctionDocumentation(docType->Constructors, type->Constructors);

    // Add all instance and static functions to the documentation
    this->BuildFunctionDocumentation(docType->InstanceMethods, type->InstanceFunctions);
    this->BuildFunctionDocumentation(docType->StaticMethods, type->StaticFunctions);

    // Add all the properties and members
    this->BuildPropertyDocumentation(docType->InstanceProperties, type->InstanceFields);
    this->BuildPropertyDocumentation(docType->InstanceProperties, type->InstanceGetterSetters);
    this->BuildPropertyDocumentation(docType->StaticProperties, type->StaticFields);
    this->BuildPropertyDocumentation(docType->StaticProperties, type->StaticGetterSetters);
  }
  
  //***************************************************************************
  bool DocumentationFunctionSorter(DocumentationFunction* left, DocumentationFunction* right)
  {
    // Compare the names with each other
    return left->Name < right->Name;
  }

  //***************************************************************************
  void Module::BuildFunctionDocumentation(Array<DocumentationFunction*>& addTo, const FunctionMultiMap& functions)
  {
    // Get the instance functions
    FunctionMultiValueRange functionArrays = functions.Values();

    // Normally it's a map of strings to overloaded functions, so we need to loop through all arrays
    while (functionArrays.Empty() == false)
    {
      // Grab the current instance function array and move to the next
      FunctionArray& functionArray = functionArrays.Front();
      functionArrays.PopFront();

      // Build documentation for those functions
      this->BuildFunctionDocumentation(addTo, functionArray);
    }

    // Sort the methods by name
    Sort(addTo.All(), DocumentationFunctionSorter);
  }

  //***************************************************************************
  void Module::BuildFunctionDocumentation(Array<DocumentationFunction*>& addTo, const FunctionArray& functions)
  {
    // Loop through all the functions in an array
    for (size_t i = 0; i < functions.Size(); ++i)
    {
      // Grab the current function
      Function* function = functions[i];

      // Skip the special getter and setter functions (those are documented by properties)
      if (function->OwningProperty != nullptr)
        continue;

      // Create the function's documentation
      DocumentationFunction* docFunction = new DocumentationFunction();
      docFunction->Name = function->Name;

      // Set the description and remarks
      docFunction->Description = function->Description;
      docFunction->Remarks = function->Remarks;

      // Add the doc function to the array
      addTo.PushBack(docFunction);

      // Create the signature for the function
      docFunction->Signature = function->FunctionType->GetSignatureString();

      // Get the return type of the function
      docFunction->ReturnType = function->FunctionType->Return->ToString();

      // Get the parameters for the function we're binding
      ParameterArray& parameters = function->FunctionType->Parameters;
      
      // Loop through all the parameters
      for (size_t j = 0; j < parameters.Size(); ++j)
      {
        // Grab the current parameter
        DelegateParameter& parameter = parameters[j];

        // Create a documentation mirrored parameter to describe it
        DocumentationParameter* docParameter = new DocumentationParameter();
        docParameter->Name = parameter.Name;
        docParameter->Type = parameter.ParameterType->ToString();

        // Add it to the documentation function
        docFunction->Parameters.PushBack(docParameter);
      }

      // If this function is a constructor, it has no description, and it takes no arguments (defaulted)...
      if (function->Name == ConstructorName && docFunction->Description.Empty() && docFunction->Parameters.Empty())
      {
        // Set the description to something special
        docFunction->Description = "*Default constructor*";
      }
    }
  }

  //***************************************************************************
  void Module::BuildPropertyDocumentation(Array<DocumentationProperty*>& addTo, const FieldMap& members)
  {
    // Get a range of all the properties
    FieldMapValueRange range = members.Values();

    // Walk through all the properties
    while (range.Empty() == false)
    {
      // Get the current property and move forward
      Field* field = range.Front();
      range.PopFront();

      // Build documentation for the property
      this->BuildPropertyDocumentation(addTo, field);
    }
  }

  //***************************************************************************
  bool DocumentationPropertySorter(DocumentationProperty* left, DocumentationProperty* right)
  {
    // Compare the names with each other
    return left->Name < right->Name;
  }

  //***************************************************************************
  void Module::BuildPropertyDocumentation(Array<DocumentationProperty*>& addTo, const GetterSetterMap& properties)
  {
    // Get a range of all the properties
    PropertyMapValueRange range = properties.Values();

    // Walk through all the properties
    while (range.Empty() == false)
    {
      // Get the current property and move forward
      Property* property = range.Front();
      range.PopFront();

      // Build documentation for the property
      this->BuildPropertyDocumentation(addTo, property);
    }

    // Sort the instance properties by name
    Sort(addTo.All(), DocumentationPropertySorter);
  }

  //***************************************************************************
  void Module::BuildPropertyDocumentation(Array<DocumentationProperty*>& addTo, const Property* property)
  {
    // Create the documentation property to represent the property
    DocumentationProperty* docProperty = new DocumentationProperty();
    docProperty->Name = property->Name;
    docProperty->IsGettable = (property->Get != nullptr);
    docProperty->IsSettable = (property->Set != nullptr);
    docProperty->IsField = (Type::DynamicCast<const Field*>(property) != nullptr);
    docProperty->Type = property->PropertyType->ToString();

    // Set the description and remarks
    docProperty->Description = property->Description;
    docProperty->Remarks = property->Remarks;

    // Add the property to the array
    addTo.PushBack(docProperty);
  }

  //***************************************************************************
  bool DocumentationTypeSorter(DocumentationType* left, DocumentationType* right)
  {
    // Compare the names with each other
    return left->Name < right->Name;
  }

  //***************************************************************************
  DocumentationModule* Module::BuildDocumentation()
  {
    // Create the documentation module object
    DocumentationModule* docs = new DocumentationModule();

    // Loop through all the libraries
    for (size_t i = 0; i < this->Size(); ++i)
    {
      // Grab the current library
      LibraryRef& library = (*this)[i];
      
      // Create a documentation object that represents a library
      DocumentationLibrary* docLibrary = new DocumentationLibrary();
      docLibrary->Name = library->Name;

      // Add the library to the docs
      docs->Libraries.PushBack(docLibrary);
      
      // Loop through all the types in the library
      BoundTypeValueRange typeRange = library->BoundTypes.Values();
      while (typeRange.Empty() == false)
      {
        // Grab the current type
        BoundType* type = typeRange.Front();
        typeRange.PopFront();

        // Add the type to the docs
        DocumentationType* docType = new DocumentationType();

        // Add the type to the documentation for the library
        docLibrary->TypesSorted.PushBack(docType);
        docLibrary->TypesByName.Insert(type->Name, docType);

        // Build the documentation for this type
        this->BuildTypeDocumentation(type, docType);
      }

      // Create a temporary library builder to build templates
      LibraryBuilder builder("Documenation");

      // Loop through all template handlers, instantiate them as templates that take the 'Any' type
      HashMap<String, InstantiateTemplateInfo>::valuerange templates = library->TemplateHandlers.Values();
      while (templates.Empty() == false)
      {
        // Get the current template and move to the next one
        InstantiateTemplateInfo& templateInfo = templates.Front();
        templates.PopFront();

        // We attempt to instantiate the template using fake types
        // TODO: Create full typedefs of the Any type
        Array<Constant> arguments;
        for (size_t i = 0; i < templateInfo.TemplateParameters.Size(); ++i)
        {
          TemplateParameter& parameter = templateInfo.TemplateParameters[i];
          BoundType* fakeType = builder.BoundTypes.FindValue(parameter.Name, nullptr);

          if (fakeType == nullptr)
            fakeType = builder.AddBoundType(parameter.Name, TypeCopyMode::ReferenceType, 0);

          arguments.PushBack(fakeType);
        }

        InstantiatedTemplate finalTemplate = builder.InstantiateTemplate(templateInfo.TemplateBaseName, arguments, *this);

        if (finalTemplate.Result == TemplateResult::Success)
        {
          BoundType* type = finalTemplate.Type;

          // Add the type to the docs
          DocumentationType* docType = new DocumentationType();

          // Add the type to the documentation for the library
          docLibrary->TypesSorted.PushBack(docType);
          docLibrary->TypesByName.Insert(type->Name, docType);

          // Build the documentation for this type
          this->BuildTypeDocumentation(type, docType);
        }
      }

      // Sort the types by name
      Sort(docLibrary->TypesSorted.All(), DocumentationTypeSorter);
    }

    // Return the newly created documentation module
    return docs;
  }
  
  //***************************************************************************
  void Module::BuildJsonConstructors(JsonBuilder& json, const Array<DocumentationFunction*>& constructors, StringParam name)
  {
    // Early out if we have no functions
    if (constructors.Empty())
      return;

    json.Key(name);
    json.Begin(JsonType::Object);
    {
      json.Key("columns");
      json.Begin(JsonType::ArraySingleLine);
      json.Value("Signature");
      json.Value("Description");
      json.End();

      json.Key("rows");
      json.Begin(JsonType::ArrayMultiLine);
      for (size_t i = 0; i < constructors.Size(); ++i)
      {
        DocumentationFunction* constructor = constructors[i];

        json.Begin(JsonType::ArraySingleLine);
        json.Value(BuildString("constructor", constructor->Signature));
        json.Value(constructor->Description);
        json.End();
      }
      json.End();
    }
    json.End();
  }

  //***************************************************************************
  void Module::BuildJsonMethods(JsonBuilder& json, const Array<DocumentationFunction*>& functions, StringParam name)
  {
    // Early out if we have no functions
    if (functions.Empty())
      return;

    json.Key(name);
    json.Begin(JsonType::Object);
    {
      json.Key("columns");
      json.Begin(JsonType::ArraySingleLine);
      json.Value("Name / Signature");
      json.Value("Description");
      json.End();

      json.Key("rows");
      json.Begin(JsonType::ArrayMultiLine);
      for (size_t i = 0; i < functions.Size(); ++i)
      {
        DocumentationFunction* function = functions[i];

        json.Begin(JsonType::ArraySingleLine);
        json.Value(BuildString(function->Name, function->Signature));
        json.Value(function->Description);
        json.End();
      }
      json.End();
    }
    json.End();
  }
  
  //***************************************************************************
  void Module::BuildJsonProperties(JsonBuilder& json, const Array<DocumentationProperty*>& properties, StringParam name)
  {
    // Early out if we have no functions
    if (properties.Empty())
      return;

    json.Key(name);
    json.Begin(JsonType::Object);

    {
      json.Key("columns");
      json.Begin(JsonType::ArraySingleLine);
      json.Value("Name");
      json.Value("Type");
      json.Value("Description");
      json.End();

      json.Key("rows");
      json.Begin(JsonType::ArrayMultiLine);
      for (size_t i = 0; i < properties.Size(); ++i)
      {
        DocumentationProperty* property = properties[i];

        json.Begin(JsonType::ArraySingleLine);
        json.Value(property->Name);
        json.Value(property->Type);
        json.Value(property->Description);
        json.End();
      }
      json.End();
    }
    json.End();
  }
  
  //***************************************************************************
  void BuildBaseChain(StringBuilderExtended& builder, DocumentationLibrary* library, DocumentationType* type)
  {
    if (type->BaseName.Empty())
      return;

    DocumentationType* baseType = library->TypesByName[type->BaseName];

    BuildBaseChain(builder, library, baseType);

    builder.Write(baseType->Name);
    builder.Write(" |rarr| ");
  }

  //***************************************************************************
  void Module::BuildDocumentationRst(StringParam directory)
  {
    DocumentationModule* docs = this->BuildDocumentation();

    for (size_t i = 0; i < docs->Libraries.Size(); ++i)
    {
      DocumentationLibrary* library = docs->Libraries[i];

      for (size_t j = 0; j < library->TypesSorted.Size(); ++j)
      {
        RstBuilder builder;

        builder.WriteLine(".. include:: <isonum.txt>");
        builder.WriteLine();

        DocumentationType* type = library->TypesSorted[j];

        BuildBaseChain(builder, library, type);
        builder.WriteLine();
        builder.WriteLine();
        
        builder.WriteLineHeading(type->Name, RstHeadingType::Section);

        builder.Write("  ");
        builder.Write(type->Description);

        if (type->Description.Empty() == false)
          builder.Write(" ");

        if (type->IsValueType)
          builder.WriteLine("(struct)");
        else
          builder.WriteLine("(class)");

        builder.WriteLine();

        if (type->Remarks.Empty() == false)
        {
          builder.WriteLineHeading("Remarks", RstHeadingType::SubSection);
        
          for (size_t i = 0; i < type->Remarks.Size(); ++i)
          {
            builder.Write("  ");
            builder.WriteLine(type->Remarks[i]);
          }
          builder.WriteLine();
        }
        
        if (type->Constructors.Empty() == false)
        {
          builder.WriteLineHeading("Constructors", RstHeadingType::SubSection);

          RstTable table;
          table.Resize(2, type->Constructors.Size() + 1);
          table.HeaderRows = 1;
          table.SetCell("Signature", 0, 0);
          table.SetCell("Row", 1, 0);

          for (size_t i = 0; i < type->Constructors.Size(); ++i)
          {
            DocumentationFunction* function = type->Constructors[i];
            
            table.SetCell(function->Signature, 0, i + 1);
            table.SetCell(function->Description, 1, i + 1);
          }

          builder.WriteLine(table);
          builder.WriteLine();
        }

        if (type->InstanceProperties.Empty() == false)
        {
          builder.WriteLineHeading("Instance Properties", RstHeadingType::SubSection);

          RstTable table;
          table.Resize(3, type->InstanceProperties.Size() + 1);
          table.HeaderRows = 1;
          table.SetCell("Name", 0, 0);
          table.SetCell("Type", 1, 0);
          table.SetCell("Description", 2, 0);

          for (size_t i = 0; i < type->InstanceProperties.Size(); ++i)
          {
            DocumentationProperty* property = type->InstanceProperties[i];

            String typeLink = BuildString(":doc:`", property->Type, "`");
            
            table.SetCell(property->Name, 0, i + 1);
            table.SetCell(typeLink, 1, i + 1);
            table.SetCell(property->Description, 2, i + 1);
          }

          builder.WriteLine(table);
          builder.WriteLine();
        }

        if (type->StaticProperties.Empty() == false)
        {
          builder.WriteLineHeading("Static Properties", RstHeadingType::SubSection);

          RstTable table;
          table.Resize(3, type->StaticProperties.Size() + 1);
          table.HeaderRows = 1;
          table.SetCell("Name", 0, 0);
          table.SetCell("Type", 1, 0);
          table.SetCell("Description", 2, 0);

          for (size_t i = 0; i < type->StaticProperties.Size(); ++i)
          {
            DocumentationProperty* property = type->StaticProperties[i];

            String typeLink = BuildString(":doc:`", property->Type, "`");
            
            table.SetCell(property->Name, 0, i + 1);
            table.SetCell(typeLink, 1, i + 1);
            table.SetCell(property->Description, 2, i + 1);
          }

          builder.WriteLine(table);
          builder.WriteLine();
        }
        
        
        if (type->InstanceMethods.Empty() == false)
        {
          builder.WriteLineHeading("Instance Methods", RstHeadingType::SubSection);

          RstTable table;
          table.Resize(2, type->InstanceMethods.Size() + 1);
          table.HeaderRows = 1;
          table.SetCell("Name / Signature", 0, 0);
          table.SetCell("Description", 1, 0);

          for (size_t i = 0; i < type->InstanceMethods.Size(); ++i)
          {
            DocumentationFunction* function = type->InstanceMethods[i];

            String nameAndSignature = BuildString(function->Name, function->Signature);

            table.SetCell(nameAndSignature, 0, i + 1);
            table.SetCell(function->Description, 1, i + 1);
          }

          builder.WriteLine(table);
          builder.WriteLine();
        }
        
        if (type->StaticMethods.Empty() == false)
        {
          builder.WriteLineHeading("Static Methods", RstHeadingType::SubSection);

          RstTable table;
          table.Resize(2, type->StaticMethods.Size() + 1);
          table.HeaderRows = 1;
          table.SetCell("Name / Signature", 0, 0);
          table.SetCell("Description", 1, 0);

          for (size_t i = 0; i < type->StaticMethods.Size(); ++i)
          {
            DocumentationFunction* function = type->StaticMethods[i];

            String nameAndSignature = BuildString(function->Name, function->Signature);

            table.SetCell(nameAndSignature, 0, i + 1);
            table.SetCell(function->Description, 1, i + 1);
          }

          builder.WriteLine(table);
          builder.WriteLine();
        }

        String typeRst = builder.ToString();

        //HACK should be using file path stuff (platform agnostic!)
        String rstFileName = BuildString(directory, "\\", type->Name, ".rst");
        
        Zero::WriteToFile(rstFileName.c_str(), (const byte*)typeRst.c_str(), typeRst.SizeInBytes());
      }
    }
  }
  
  //***************************************************************************
  String Module::BuildDocumentationHtml()
  {
    JsonBuilder json;

    DocumentationModule* docs = this->BuildDocumentation();

    json.Begin(JsonType::Object);

    json.Key("start");
    json.Value("GettingStarted");

    {
      json.Key("libraries");
      json.Begin(JsonType::Object);

      for (size_t i = 0; i < docs->Libraries.Size(); ++i)
      {
        DocumentationLibrary* library = docs->Libraries[i];

        json.Key(library->Name);
        json.Begin(JsonType::Object);

        {
          json.Key("types");
          json.Begin(JsonType::Object);

          for (size_t j = 0; j < library->TypesSorted.Size(); ++j)
          {
            DocumentationType* type = library->TypesSorted[j];
            
            json.Key(type->Name);
            json.Begin(JsonType::Object);

            {
              json.Key("base");
              json.Value(type->BaseName);
              json.Key("description");
              json.Value(type->Description);
              json.Key("isValueType");
              json.Value(type->IsValueType);
              json.Key("remarks");
              json.Begin(JsonType::ArrayMultiLine);

              {
                for (size_t k = 0; k < type->Remarks.Size(); ++k)
                {
                  String& remark = type->Remarks[k];

                  json.Value(remark);
                }
              }

              json.End();

              json.Key("tables");
              json.Begin(JsonType::Object);
              {
                this->BuildJsonConstructors(json, type->Constructors, "Constructors");

                this->BuildJsonMethods(json, type->InstanceMethods, "Instance Methods");
                this->BuildJsonMethods(json, type->StaticMethods, "Static Methods");
                
                this->BuildJsonProperties(json, type->InstanceProperties, "Instance Properties");
                this->BuildJsonProperties(json, type->StaticProperties, "Static Properties");
              }
              json.End();
            }


            json.End();
          }

          json.End();
        }

        json.End();
      }

      // End the libraries
      json.End();
    }

    // End the module
    json.End();

    return json.ToString();
  }

  //***************************************************************************
  InstantiatedTemplate::InstantiatedTemplate() :
    Type(nullptr),
    Result(TemplateResult::FailedInstantiatorDidNotReturnType),
    ExpectedArguments(0)
  {
  }

  //***************************************************************************
  ParameterArray OneParameter(Type* type)
  {
    ParameterArray parameters;
    parameters.PushBack(type);
    return parameters;
  }

  //***************************************************************************
  ParameterArray OneParameter(Type* type, StringParam name)
  {
    ParameterArray parameters;

    DelegateParameter& a = parameters.PushBack();
    a.Name = name;
    a.ParameterType = type;

    return parameters;
  }

  //***************************************************************************
  ParameterArray TwoParameters(Type* type)
  {
    ParameterArray parameters;
    parameters.PushBack(type);
    parameters.PushBack(type);
    return parameters;
  }

  //***************************************************************************
  ParameterArray TwoParameters(Type* type, StringParam name1, StringParam name2)
  {
    ParameterArray parameters;

    DelegateParameter& a = parameters.PushBack();
    a.Name = name1;
    a.ParameterType = type;

    DelegateParameter& b = parameters.PushBack();
    b.Name = name2;
    b.ParameterType = type;

    return parameters;
  }

  //***************************************************************************
  ParameterArray TwoParameters(Type* type1, Type* type2)
  {
    ParameterArray parameters;
    parameters.PushBack(type1);
    parameters.PushBack(type2);
    return parameters;
  }

  //***************************************************************************
  ParameterArray TwoParameters(Type* type1, StringParam name1, Type* type2, StringParam name2)
  {
    ParameterArray parameters;

    DelegateParameter& a = parameters.PushBack();
    a.Name = name1;
    a.ParameterType = type1;

    DelegateParameter& b = parameters.PushBack();
    b.Name = name2;
    b.ParameterType = type2;

    return parameters;
  }

  //***************************************************************************
  ParameterArray ThreeParameters(Type* type)
  {
    ParameterArray parameters;
    parameters.PushBack(type);
    parameters.PushBack(type);
    parameters.PushBack(type);
    return parameters;
  }

  //***************************************************************************
  ParameterArray ThreeParameters(Type* type1, StringParam name1, Type* type2, StringParam name2, Type* type3, StringParam name3)
  {
    ParameterArray parameters;

    DelegateParameter& a = parameters.PushBack();
    a.Name = name1;
    a.ParameterType = type1;

    DelegateParameter& b = parameters.PushBack();
    b.Name = name2;
    b.ParameterType = type2;

    DelegateParameter& c = parameters.PushBack();
    c.Name = name3;
    c.ParameterType = type3;

    return parameters;
  }

  //***************************************************************************
  ParameterArray ThreeParameters(Type* type, StringParam name1, StringParam name2, StringParam name3)
  {
    ParameterArray parameters;

    DelegateParameter& a = parameters.PushBack();
    a.Name = name1;
    a.ParameterType = type;

    DelegateParameter& b = parameters.PushBack();
    b.Name = name2;
    b.ParameterType = type;

    DelegateParameter& c = parameters.PushBack();
    c.Name = name3;
    c.ParameterType = type;

    return parameters;
  }

  //***************************************************************************
  ParameterArray ThreeParameters(Type* type1, Type* type2, Type* type3)
  {
    ParameterArray parameters;
    parameters.PushBack().ParameterType = type1;
    parameters.PushBack().ParameterType = type2;
    parameters.PushBack().ParameterType = type3;
    return parameters;
  }
  
  //***************************************************************************
  ParameterArray FourParameters(Type* type)
  {
    ParameterArray parameters;
    parameters.PushBack().ParameterType = type;
    parameters.PushBack().ParameterType = type;
    parameters.PushBack().ParameterType = type;
    parameters.PushBack().ParameterType = type;
    return parameters;
  }
  
  //***************************************************************************
  ParameterArray FourParameters(Type* type, StringParam name1, StringParam name2, StringParam name3, StringParam name4)
  {
    ParameterArray parameters;

    DelegateParameter& a = parameters.PushBack();
    a.Name = name1;
    a.ParameterType = type;

    DelegateParameter& b = parameters.PushBack();
    b.Name = name2;
    b.ParameterType = type;

    DelegateParameter& c = parameters.PushBack();
    c.Name = name3;
    c.ParameterType = type;

    DelegateParameter& d = parameters.PushBack();
    d.Name = name4;
    d.ParameterType = type;

    return parameters;
  }
  
  //***************************************************************************
  ParameterArray FiveParameters(Type* type)
  {
    ParameterArray parameters;
    parameters.PushBack().ParameterType = type;
    parameters.PushBack().ParameterType = type;
    parameters.PushBack().ParameterType = type;
    parameters.PushBack().ParameterType = type;
    parameters.PushBack().ParameterType = type;
    return parameters;
  }
  
  //***************************************************************************
  ParameterArray FiveParameters(Type* type, StringParam name1, StringParam name2, StringParam name3, StringParam name4, StringParam name5)
  {
    ParameterArray parameters;

    DelegateParameter& a = parameters.PushBack();
    a.Name = name1;
    a.ParameterType = type;

    DelegateParameter& b = parameters.PushBack();
    b.Name = name2;
    b.ParameterType = type;

    DelegateParameter& c = parameters.PushBack();
    c.Name = name3;
    c.ParameterType = type;

    DelegateParameter& d = parameters.PushBack();
    d.Name = name4;
    d.ParameterType = type;

    DelegateParameter& e = parameters.PushBack();
    e.Name = name5;
    e.ParameterType = type;

    return parameters;
  }

  ////***************************************************************************
  //ExtensionPropertyMapRange::ExtensionPropertyMapRange(LibraryArray& libraries, bool isStatic, Type* type) :
  //  CurrentMap(nullptr),
  //  LibraryIndex(0),
  //  BaseIterator(type),
  //  Libraries(libraries),
  //  IsStatic(isStatic),
  //  ExtensionType(type)
  //{
  //}

  ////***************************************************************************
  //void ExtensionPropertyMapRange::popFront()
  //{

  //}

  ////***************************************************************************
  //void ExtensionPropertyMapRange::UpdateCurrentMap()
  //{
  //  LibraryRef& library = this->Libraries[this->LibraryIndex];

  //  // Get the guid of the type (this should be legal here since we've collected all members)
  //  GuidType guid = this->BaseIterator->Hash();
  //  
  //  // Get the array of properties (may be empty)
  //  PropertyMap* properties = nullptr;
  //      
  //  // If we're resolving a static member
  //  if (this->IsStatic)
  //  {
  //    properties = library->StaticExtensionProperties.FindPointer(guid);
  //  }
  //  else
  //  {
  //    properties = library->InstanceExtensionProperties.FindPointer(guid);
  //  }

  //  // If we got a valid array of properties...
  //  if (properties != nullptr)
  //  {
  //    return properties;
  //  }
  //}

  ////***************************************************************************
  //PropertyMap* ExtensionPropertyMapRange::front()
  //{

  //    // We need to look up the entire heirarchy (the property could be on any base classes)
  //    Type* baseIterator = type;
  //    while (baseIterator != nullptr)
  //    {


  //      
  //        
  //      // Iterate to the next type
  //      baseIterator = Type::GetBaseType(baseIterator);
  //    }
  //}
}
