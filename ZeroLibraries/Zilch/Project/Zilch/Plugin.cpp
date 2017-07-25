/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

// Set this to true this to get asserts for plugin types and functions not linking
static const bool ZilchDebugPluginLinking = false;

namespace Zilch
{
  //***************************************************************************
  namespace Events
  {
    ZilchDefineEvent(PreBuild);
  }

  //***************************************************************************
  ZilchDefineType(BuildEvent, builder, type)
  {
  }

  //***************************************************************************
  ZilchDefineType(PluginEvent, builder, type)
  {
  }
  
  //***************************************************************************
  BuildEvent::BuildEvent() :
    BuildingProject(nullptr),
    Dependencies(nullptr),
    Builder(nullptr),
    Reason(BuildReason::FullCompilation)
  {
  }
  
  //***************************************************************************
  LibraryRef BuildEvent::FindLibrary(StringParam name)
  {
    ZilchForEach(LibraryRef library, *this->Dependencies)
    {
      if (library->Name == name)
        return library;
    }

    return nullptr;
  }
  
  //***************************************************************************
  void BuildEvent::AddPluginDependency(LibraryRef library)
  {
    library->CreatedByPlugin = true;
    this->Dependencies->PushBack(library);
  }
  
  //***************************************************************************
  ZeroShared EventHandler PluginEvent::GlobalEvents;
  
  //***************************************************************************
  PluginEvent::PluginEvent() :
    mCString(nullptr),
    mInteger(0),
    mFloat(0),
    mDouble(0),
    mBool(false),
    mPointer(nullptr),
    mHandled(false)
  {
  }

  //***************************************************************************
  Plugin::Plugin() :
    FullCompilationInitialized(false),
    UserData(nullptr)
  {
  }

  //***************************************************************************
  Plugin::~Plugin()
  {
  }
  
  //***************************************************************************
  void Plugin::PreBuild(BuildEvent* event)
  {
  }
  
  //***************************************************************************
  void Plugin::Initialize(BuildEvent* event)
  {
  }
  
  //***************************************************************************
  void Plugin::Uninitialize()
  {
  }
  
  //***************************************************************************
  void CreateAllocatingHandle(Handle& handle, void* pointer)
  {
    ExecutableState* state = ExecutableState::GetCallingState();
    handle.StoredType = state->AllocatingType;
    handle.Manager = state->PointerObjects;
    state->PointerObjects->ObjectToHandle((const byte*)pointer, handle.StoredType, handle);
  }
  
  //***************************************************************************
  PluginStubLibrary& PluginStubLibrary::GetInstance()
  {
    static PluginStubLibrary instance;
    return instance;
  }
  
  //***************************************************************************
  PluginStubLibrary::PluginStubLibrary() :
    StaticLibrary("PluginStubLibrary")
  {
  }
  
  //***************************************************************************
  PluginStubLibrary::~PluginStubLibrary()
  {
  }
  
  //***************************************************************************
  bool PluginStubLibrary::CanBuildTypes()
  {
    return true;
  }
  
  //***************************************************************************
  void NameMangler::MangleLibrary(LibraryRef library)
  {
    // In order to do lookup functions from the side of the plugin
    // We need to mangle the function names so they can be looked up, akin to a linker
    // However, rather than using strings, we use 64bit ids to keep the size of the library small
    for (size_t j = 0; j < library->OwnedFunctions.Size(); ++j)
    {
      Function* currentFunction = library->OwnedFunctions[j];
      Function*& function = this->HashToFunction[currentFunction->Hash];
      ErrorIf(function != nullptr, "Two functions hashed to the same value (for mangling and linking, hashes need to be unique)");
      function = currentFunction;
    }
  }
  
  //***************************************************************************
  Function* NameMangler::FindFunction(GuidType functionHash, const char* functionName, StringParam typeName)
  {
    Function* function = this->HashToFunction.FindValue(functionHash, nullptr);
    ErrorIf
    (
      ZilchDebugPluginLinking && function == nullptr,
      "Unable to find function/overload %s on type %s (with id %llu)",
      functionName,
      typeName.c_str(),
      functionHash
    );
    return function;
  }

  //***************************************************************************
  GuidType NameMangler::GetDelegateTypeId(DelegateType* type)
  {
    // To keep code generation sizes small, we generate one binding function
    // for all function calls of the same signature
    // To reference that function, we generate an id
    // (typically the function will be named _id, eg _61345)
    GuidType& id = this->DelegateTypeToId[type];

    if (id == 0)
      id = (int)this->DelegateTypeToId.Size();
    return id;
  }

  //***************************************************************************
  BoundType* FindLibraryType(LibraryRef library, const char* name)
  {
    BoundType* type = library->BoundTypes.FindValue(name, nullptr);
    ErrorIf
    (
      ZilchDebugPluginLinking && type == nullptr,
      "Unable to find the %s type",
      name
    );
    return type;
  }

  //***************************************************************************
  NativeName::NativeName()
  {
  }

  //***************************************************************************
  NativeName::NativeName(StringParam className, StringParam parameterName, StringParam returnName) :
    Class(className),
    Parameter(parameterName),
    Return(returnName)
  {
  }

  //***************************************************************************
  NativeStubCode::NativeStubCode() :
    Libraries(nullptr)
  {
    // Because some of the types that we have bound already exist within the Zero or Zilch namespace
    // then when we generate code for them, we want to redirect them to use the special type names specified here
    this->TypeToCppName.Insert(ZilchTypeId(Any), NativeName("Any", "const Zilch::Any&", "Zilch::Any"));

    this->TypeToCppName.Insert(ZilchTypeId(Member), NativeName("Member", "Zilch::Member*", "Zilch::HandleOf<Zilch::Member>"));
    this->TypeToCppName.Insert(ZilchTypeId(Property), NativeName("Property", "Zilch::Property*", "Zilch::HandleOf<Zilch::Property>"));
    this->TypeToCppName.Insert(ZilchTypeId(GetterSetter), NativeName("GetterSetter", "Zilch::GetterSetter*", "Zilch::HandleOf<Zilch::GetterSetter>"));
    this->TypeToCppName.Insert(ZilchTypeId(Field), NativeName("Field", "Zilch::Field*", "Zilch::HandleOf<Zilch::Field>"));
    this->TypeToCppName.Insert(ZilchTypeId(Function), NativeName("Function", "Zilch::Function*", "Zilch::HandleOf<Zilch::Function>"));

    this->TypeToCppName.Insert(ZilchTypeId(Delegate), NativeName("Delegate", "const Zilch::Delegate&", "Zilch::Delegate"));

    this->TypeToCppName.Insert(ZilchTypeId(Void), NativeName("Void", "void", "void"));

    this->TypeToCppName.Insert(ZilchTypeId(Boolean), NativeName("Boolean", "bool", "bool"));
    this->TypeToCppName.Insert(ZilchTypeId(Boolean2), NativeName("Boolean2", "const Zilch::Boolean2&", "Zilch::Boolean2"));
    this->TypeToCppName.Insert(ZilchTypeId(Boolean3), NativeName("Boolean3", "const Zilch::Boolean3&", "Zilch::Boolean3"));
    this->TypeToCppName.Insert(ZilchTypeId(Boolean4), NativeName("Boolean4", "const Zilch::Boolean4&", "Zilch::Boolean4"));

    this->TypeToCppName.Insert(ZilchTypeId(Byte), NativeName("Byte", "unsigned char", "unsigned char"));

    this->TypeToCppName.Insert(ZilchTypeId(Integer), NativeName("Integer", "int", "int"));
    this->TypeToCppName.Insert(ZilchTypeId(Integer2), NativeName("Integer2", "const Zilch::Integer2&", "Zilch::Integer2"));
    this->TypeToCppName.Insert(ZilchTypeId(Integer3), NativeName("Integer3", "const Zilch::Integer3&", "Zilch::Integer3"));
    this->TypeToCppName.Insert(ZilchTypeId(Integer4), NativeName("Integer4", "const Zilch::Integer4&", "Zilch::Integer4"));

    this->TypeToCppName.Insert(ZilchTypeId(Real), NativeName("Real", "float", "float"));
    this->TypeToCppName.Insert(ZilchTypeId(Real2), NativeName("Real2", "const Zilch::Real2&", "Zilch::Real2"));
    this->TypeToCppName.Insert(ZilchTypeId(Real3), NativeName("Real3", "const Zilch::Real3&", "Zilch::Real3"));
    this->TypeToCppName.Insert(ZilchTypeId(Real4), NativeName("Real4", "const Zilch::Real4&", "Zilch::Real4"));

    this->TypeToCppName.Insert(ZilchTypeId(Quaternion), NativeName("Quaternion", "const Zilch::Quaternion&", "Zilch::Quaternion"));
    this->TypeToCppName.Insert(ZilchTypeId(Real3x3), NativeName("Real3x3", "const Zilch::Real3x3&", "Zilch::Real3x3"));
    this->TypeToCppName.Insert(ZilchTypeId(Real4x4), NativeName("Real4x4", "const Zilch::Real4x4&", "Zilch::Real4x4"));

    this->TypeToCppName.Insert(ZilchTypeId(String), NativeName("String", "const Zilch::String&", "Zilch::String"));

    this->TypeToCppName.Insert(ZilchTypeId(DoubleReal), NativeName("DoubleReal", "double", "double"));
    this->TypeToCppName.Insert(ZilchTypeId(DoubleInteger), NativeName("DoubleInteger", "long long", "long long"));

    this->TypeToCppName.Insert(ZilchTypeId(Members::Enum), NativeName("Members::Enum", "Zilch::Members::Enum", "Zilch::Members::Enum"));
    this->TypeToCppName.Insert(ZilchTypeId(FileMode::Enum), NativeName("FileMode::Enum", "Zilch::FileMode::Enum", "Zilch::FileMode::Enum"));
    this->TypeToCppName.Insert(ZilchTypeId(StreamCapabilities::Enum), NativeName("StreamCapabilities::Enum", "Zilch::StreamCapabilities::Enum", "Zilch::StreamCapabilities::Enum"));
    this->TypeToCppName.Insert(ZilchTypeId(StreamOrigin::Enum), NativeName("StreamOrigin::Enum", "Zilch::StreamOrigin::Enum", "Zilch::StreamOrigin::Enum"));

    this->TypeToCppName.Insert(ZilchTypeId(EventsClass), NativeName("EventsClass", "const Zilch::EventsClass&", "Zilch::EventsClass"));
    this->TypeToCppName.Insert(ZilchTypeId(FilePathClass), NativeName("FilePathClass", "const Zilch::FilePathClass&", "Zilch::FilePathClass"));
    this->TypeToCppName.Insert(ZilchTypeId(IStreamClass), NativeName("IStreamClass", "const Zilch::IStreamClass&", "Zilch::IStreamClass"));
    this->TypeToCppName.Insert(ZilchTypeId(StringBuilderExtended), NativeName("StringBuilderExtended", "const Zilch::StringBuilderExtended&", "Zilch::StringBuilderExtended"));
    this->TypeToCppName.Insert(ZilchTypeId(StringRangeExtended), NativeName("StringRangeExtended", "const Zilch::StringRangeExtended&", "Zilch::StringRangeExtended"));
    this->TypeToCppName.Insert(ZilchTypeId(StringSplitRangeExtended), NativeName("StringSplitRangeExtended", "const Zilch::StringSplitRangeExtended&", "Zilch::StringSplitRangeExtended"));

    this->TypeToCppName.Insert(ZilchTypeId(ParameterArray::range), NativeName("ParameterArray::range", "const Zilch::ParameterArray::range&", "Zilch::ParameterArray::range"));
    this->TypeToCppName.Insert(ZilchTypeId(MemberRange<Member>), NativeName("MemberRange<Zilch::Member>", "const Zilch::MemberRange<Zilch::Member>&", "Zilch::MemberRange<Zilch::Member>"));
    this->TypeToCppName.Insert(ZilchTypeId(MemberRange<Property>), NativeName("MemberRange<Zilch::Property>", "const Zilch::MemberRange<Zilch::Property>&", "Zilch::MemberRange<Zilch::Property>"));
    this->TypeToCppName.Insert(ZilchTypeId(MemberRange<GetterSetter>), NativeName("MemberRange<Zilch::GetterSetter>", "const Zilch::MemberRange<Zilch::GetterSetter>&", "Zilch::MemberRange<Zilch::GetterSetter>"));
    this->TypeToCppName.Insert(ZilchTypeId(MemberRange<Field>), NativeName("MemberRange<Zilch::Field>", "const Zilch::MemberRange<Zilch::Field>&", "Zilch::MemberRange<Zilch::Field>"));
    this->TypeToCppName.Insert(ZilchTypeId(MemberRange<Function>), NativeName("MemberRange<Zilch::Function>", "const Zilch::MemberRange<Zilch::Function>&", "Zilch::MemberRange<Zilch::Function>"));
  }

  //***************************************************************************
  NativeName NativeStubCode::GetCppTypeName(Type* type)
  {
    // All delegates that we take in just become a Zilch Delegate (not type checked unfortunately)
    if (Type::IsDelegateType(type))
      type = ZilchTypeId(Delegate);

    // If we've already generated a native name, avoid a bunch of extra string allocations
    // This also allows us to handle the built in types to Zero/Zilch, such as 'Any' (see above)
    NativeName* nativeName = this->TypeToCppName.FindPointer(type);
    if (nativeName != nullptr)
      return *nativeName;

    // Remove any invalid characters from the type name (that's what \0 means here)
    String name = LibraryBuilder::FixIdentifier(type->ToString(), TokenCheck::None, '\0');

    // If the generated type is not from our currently library, then qualify it with a 
    if (this->LibrarySet.Contains(type->SourceLibrary) == false)
      name = BuildString(type->SourceLibrary->GetPluginNamespace(), "::", name);

    NativeName nativeNameResult;
    nativeNameResult.Class = name;
    
    if (Type::IsHandleType(type))
    {
      // If this is a handle type, then when we generate the stub
      // we want to accept it as a pointer when taken as a parameter
      nativeNameResult.Parameter = BuildString(name, "*");
      
      // In order to ensure that reference counting works properly, we also need ALL
      // returns to be of Handle type. Imagine allocating a Handle, then returning a pointer...
      // the handle would be the last reference count, and would get destroyed,
      // so the pointer would go invalid immediately
      nativeNameResult.Return = BuildString("Zilch::HandleOf<", name, ">");
    }
    else
    {
      // Otherwise this is a value type, so just always take it by value
      nativeNameResult.Parameter = name;
      nativeNameResult.Return = name;
    }
    
    // We force every type we generate to at least inherit from a base Zilch class
    // This closes some inheritance holes by ensuring every class has a base
    static const String ReferenceBase = "Zilch::ReferenceType";
    static const String ValueBase = "Zilch::ValueType";
    if (Type::IsValueType(type))
      nativeNameResult.Base = ValueBase;
    else
      nativeNameResult.Base = ReferenceBase;
    
    // We also want to know if this type has a base type (easily access and generate its name too)
    if (BoundType* boundType = Type::DynamicCast<BoundType*>(type))
    {
      BoundType* base = boundType->BaseType;
      if (base != nullptr)
        nativeNameResult.Base = this->GetCppTypeName(base).Class;
    }

    this->TypeToCppName.Insert(type, nativeNameResult);
    return nativeNameResult;
  }
  
  //***************************************************************************
  void NativeStubCode::WriteParameters(ZilchCodeBuilder& builder, DelegateType* delegateType)
  {
    Library* library = delegateType->GetOwningLibrary();

    builder.Write("(");

    size_t lastIndex = delegateType->Parameters.Size() - 1;
    for (size_t k = 0; k < delegateType->Parameters.Size(); ++k)
    {
      DelegateParameter& parameter = delegateType->Parameters[k];
      builder.Write(this->GetCppTypeName(parameter.ParameterType).Parameter);
      builder.WriteSpace();
      builder.Write(parameter.GetNameOrGenerate());

      if (k != lastIndex)
        builder.Write(", ");
    }

    builder.Write(")");
  }
  
  //***************************************************************************
  void NativeStubCode::WriteDescription(ZilchCodeBuilder& builder, ReflectionObject* object)
  {
    if (object->Description.Empty() == false)
    {
      builder.WriteSingleLineComment(object->Description);
      builder.WriteLineIndented();
    }
  }

  //***************************************************************************
  String NativeStubCode::GenerateHpp()
  {
    String nameDefine = this->Namespace.ToUpper();

    ZilchCodeBuilder builder;

    if (this->HppHeader.Empty() == false)
    {
      builder.Write(this->HppHeader);
      builder.WriteLineIndented();
      builder.WriteLineIndented();
    }

    builder.WriteLineIndented("#pragma once");
    builder.Write("#ifndef ");
    builder.Write(nameDefine);
    builder.WriteLineIndented("_HPP");
    builder.Write("#define ");
    builder.Write(nameDefine);
    builder.WriteLineIndented("_HPP");
    builder.WriteLineIndented();
    builder.WriteLineIndented("#include \"Zilch.hpp\"");
    builder.WriteLineIndented();

    if (this->HppMiddle.Empty() == false)
    {
      builder.WriteLineIndented();
      builder.Write(this->HppMiddle);
      builder.WriteLineIndented();
      builder.WriteLineIndented();
    }

    // Declare types if this is not the Core library (which has already declared its types within Zilch.hpp)
    static const String CoreNamespace = Core::GetInstance().GetLibrary()->GetPluginNamespace();

    if (this->Namespace != CoreNamespace)
    {
      builder.Write("namespace ");
      builder.Write(this->Namespace);
      builder.BeginScope(ScopeType::Block);
      builder.WriteLineIndented();

      // Forward declarations
      ZilchForEach(BoundType* type, this->TypesInDependencyOrder)
      {
        String typeName = this->GetCppTypeName(type).Class;

        builder.Write("class ");
        builder.Write(typeName);
        builder.WriteLineIndented(";");
      }

      builder.WriteLineIndented();

      // Outputting all the declarations for all the types (in order)
      ZilchForEach(BoundType* type, this->TypesInDependencyOrder)
      {
        this->WriteDescription(builder, type);

        NativeName name = this->GetCppTypeName(type);
        String typeName = name.Class;
        String baseName = name.Base;

        BoundType* base = type->BaseType;
        if (base != nullptr)
          baseName = this->GetCppTypeName(base).Class;

        builder.Write("class ");
        builder.Write(typeName);
        builder.Write(" : public ");
        builder.Write(baseName);

        builder.BeginScope(ScopeType::Class);

        // We generally put 'public' on the same line as the scope
        --builder.Indentation;
        builder.WriteLineIndented();
        builder.Write("public:");
        ++builder.Indentation;
        builder.WriteLineIndented();

        // Add typedefs for ZilchSelf and ZilchBase
        builder.Write("typedef ");
        builder.Write(typeName);
        builder.Write(" ZilchSelf;");
        builder.WriteLineIndented();
        builder.Write("typedef ");
        builder.Write(baseName);
        builder.Write(" ZilchBase;");
        builder.WriteLineIndented();
        builder.WriteLineIndented();

        String* customDefine = this->CustomClassHeaderDefines.FindPointer(type);
        if (customDefine != nullptr && customDefine->Empty() == false)
          builder.WriteLineIndented(*customDefine);

        // All function declarations (static and instance)
        for (size_t j = 0; j < type->AllFunctions.Size(); ++j)
        {
          Function* function = type->AllFunctions[j];

          String functionName = LibraryBuilder::FixIdentifier(function->Name, TokenCheck::RemoveOuterBrackets);

          this->WriteDescription(builder, function);

          if (function->IsStatic)
            builder.Write("static ");

          DelegateType* delegateType = function->FunctionType;
          Type* returnType = delegateType->Return;
          NativeName returnTypeName = this->GetCppTypeName(returnType);
          builder.Write(returnTypeName.Return);

          builder.WriteSpace();
          builder.Write(functionName);
          this->WriteParameters(builder, delegateType);
          builder.Write(';');
          builder.WriteLineIndented();
          builder.WriteLineIndented();
        }

        if (Type::IsEnumOrFlagsType(type))
        {
          builder.WriteLineIndented("int mValue;");
        }
        else
        {
          size_t size = type->GetAllocatedSize();
          if (type->BaseType != nullptr)
            size -= type->BaseType->GetAllocatedSize();

          if (size != 0)
          {
            builder.Write("unsigned char mValue[");
            builder.Write((Integer)size);
            builder.WriteLineIndented("];");
          }
        }

        bool isValueType = Type::IsValueType(type);
        if (isValueType == false)
        {
          // Protected members (such as constructors)
          --builder.Indentation;
          builder.WriteLineIndented();
          builder.Write("protected:");
          ++builder.Indentation;
          builder.WriteLineIndented();
        }

        // Inheritable constructor declarations (not allocation!)
        if (type->Sealed == false || isValueType)
        {
          // If we don't have a default constructor, implicitly add one
          if (isValueType && type->GetDefaultConstructor(false) == nullptr)
          {
            // Make a default constructor that zeroes our type out
            builder.Write(typeName);
            builder.Write("();");
            builder.WriteLineIndented();
            builder.WriteLineIndented();
          }

          for (size_t j = 0; j < type->Constructors.Size(); ++j)
          {
            Function* constructor = type->Constructors[j];
            this->WriteDescription(builder, constructor);

            builder.Write(typeName);
            DelegateType* delegateType = constructor->FunctionType;
            this->WriteParameters(builder, delegateType);
            builder.Write(';');
            builder.WriteLineIndented();
            builder.WriteLineIndented();
          }
        }

        if (Type::IsValueType(type) == false)
        {
          // Make an empty 'base constructor'
          builder.Write(typeName);
          builder.Write("(Zilch::NoType none) : ");
          builder.Write(baseName);
          builder.Write("(none) {}");
          builder.WriteLineIndented();

          // Make this type not copyable
          builder.Write("ZilchNoCopy(");
          builder.Write(typeName);
          builder.Write(");");
          builder.WriteLineIndented();
        }

        builder.EndScope();
        builder.WriteLineIndented(";");
        builder.WriteLineIndented();
      }

      builder.EndScope();
      builder.WriteLineIndented();
      builder.WriteLineIndented();
    }

    // Declare HookUpLibrary function
    builder.Write("bool HookUp");
    builder.Write(this->Namespace);
    builder.WriteLineIndented("(Zilch::BuildEvent* event);");
    builder.WriteLineIndented();

    builder.WriteLineIndented("#endif");
    
    if (this->HppFooter.Empty() == false)
    {
      builder.WriteLineIndented();
      builder.Write(this->HppFooter);
      builder.WriteLineIndented();
    }

    String result = builder.ToString();
    return result;
  }

  //***************************************************************************
  String NativeStubCode::GenerateCpp()
  {
    ZilchCodeBuilder builder;

    if (this->CppHeader.Empty() == false)
    {
      builder.Write(this->CppHeader);
      builder.WriteLineIndented();
      builder.WriteLineIndented();
    }

    if (this->PrecompiledHeader.Empty())
    {
      builder.WriteLineIndented("#define ZeroImportDll");
      builder.Write("#include \"");
      builder.Write(this->Namespace);
      builder.WriteLineIndented(".hpp\"");
    }
    else
    {
      builder.Write("#include ");
      builder.WriteLineIndented(this->PrecompiledHeader);
    }
    builder.WriteLineIndented();
    
    if (this->CppMiddle.Empty() == false)
    {
      builder.WriteLineIndented();
      builder.Write(this->CppMiddle);
      builder.WriteLineIndented();
      builder.WriteLineIndented();
    }

    // Define types if this is not the Core library (which has already defined its types)
    static const String CoreNamespace = Core::GetInstance().GetLibrary()->GetPluginNamespace();

    if (this->Namespace != CoreNamespace)
    {
      builder.Write("namespace ");
      builder.Write(this->Namespace);
      builder.BeginScope(ScopeType::Block);
      builder.WriteLineIndented();

      // Unique function calling signatures (independent of actual method names)
      HashSet<DelegateType*, DelegateTypePolicy> delegates;
      ZilchForEach(const LibraryRef& library, *this->Libraries)
      {
        ZilchForEach(Function* function, library->OwnedFunctions)
        {
          DelegateType* delegateType = function->FunctionType;
          if (delegates.Contains(delegateType))
            continue;

          delegates.Insert(delegateType);

          builder.WriteLineIndented("//***************************************************************************");

          Type* returnType = delegateType->Return;
          NativeName returnTypeName = this->GetCppTypeName(returnType);

          bool isReturnVoid = (delegateType->Return == ZilchTypeId(void));
          String defaultReturn;
          if (isReturnVoid == false)
            defaultReturn = String::Format(" %s()", returnTypeName.Return.c_str());

          builder.Write(returnTypeName.Return);
          builder.WriteSpace();

          GuidType cachedDelegateId = this->Mangler.GetDelegateTypeId(delegateType);
          builder.Write('_');
          builder.Write(cachedDelegateId);

          builder.Write("(Zilch::Handle* thisHandle, Zilch::Function* function");

          for (size_t k = 0; k < delegateType->Parameters.Size(); ++k)
          {
            builder.Write(", ");
            DelegateParameter& parameter = delegateType->Parameters[k];
            builder.Write(this->GetCppTypeName(parameter.ParameterType).Parameter);
            builder.WriteSpace();
            builder.Write(parameter.GetNameOrGenerate());
          }

          builder.Write(")");
          builder.BeginScope(ScopeType::Function);
          builder.WriteLineIndented();

          builder.Write("ReturnIf(function == nullptr,");
          builder.Write(defaultReturn);
          builder.WriteLineIndented(", \"The function does not exist (it may have been removed)\");");

          builder.WriteLineIndented("Zilch::ExecutableState* __state = Zilch::ExecutableState::GetCallingState();");
          builder.Write("ReturnIf(__state == nullptr,");
          builder.Write(defaultReturn);
          builder.WriteLineIndented(", \"You can only invoke this function when your code is called from Zilch\");");
          builder.WriteLineIndented();

          builder.WriteLineIndented("Zilch::Call __call(function, __state);");

          builder.WriteLineIndented("if (thisHandle != nullptr)");
          builder.WriteIndent();
          builder.WriteLineIndented("__call.Set<Zilch::Handle>(Zilch::Call::This, *thisHandle);");

          for (size_t k = 0; k < delegateType->Parameters.Size(); ++k)
          {
            DelegateParameter& parameter = delegateType->Parameters[k];

            builder.Write("__call.Set<");
            builder.Write(this->GetCppTypeName(parameter.ParameterType).Parameter);
            builder.Write(">(");
            builder.Write((Integer)k);
            builder.Write(", ");
            builder.Write(parameter.GetNameOrGenerate());
            builder.WriteLineIndented(");");
          }
          builder.WriteLineIndented();

          if (isReturnVoid)
          {
            builder.WriteLineIndented("__call.Invoke();");
            builder.WriteLineIndented();
            builder.WriteLineIndented("return;");
          }
          else
          {
            builder.WriteLineIndented("if (__call.Invoke() == false)");
            builder.WriteIndent();
            builder.Write("return");
            builder.Write(defaultReturn);
            builder.WriteLineIndented(";");
            builder.WriteLineIndented();

            builder.Write("return __call.Get<");
            builder.Write(returnTypeName.Return);
            builder.WriteLineIndented(" >(Zilch::Call::Return);");
          }

          builder.EndScope();
          builder.WriteLineIndented();
          builder.WriteLineIndented();
        }
      }

      // Emit all types and all functions for those types
      ZilchForEach(BoundType* type, this->TypesInDependencyOrder)
      {
        NativeName name = this->GetCppTypeName(type);
        String typeName = name.Class;
        String baseName = name.Base;

        builder.WriteLineIndented("//***************************************************************************");
        builder.Write("static Zilch::BoundType* ");
        String cachedTypeName = String::Format("%s_Type", typeName.c_str());
        builder.Write(cachedTypeName);
        builder.WriteLineIndented(" = nullptr;");

        // Inheritable constructors (not allocation!)
        bool isValueType = Type::IsValueType(type);
        if (type->Sealed == false || isValueType)
        {
          // If we don't have a default constructor, implicitly add one
          if (isValueType && type->GetDefaultConstructor(false) == nullptr)
          {
            // Make a default constructor that zeroes our type out
            builder.Write(typeName);
            builder.Write("::");
            builder.Write(typeName);
            builder.Write("()");

            builder.BeginScope(ScopeType::Function);
            builder.WriteLineIndented();

            builder.WriteLineIndented("memset(this, 0, sizeof(*this));");

            builder.EndScope();
            builder.WriteLineIndented();
            builder.WriteLineIndented();
          }

          for (size_t j = 0; j < type->Constructors.Size(); ++j)
          {
            Function* constructor = type->Constructors[j];
            builder.WriteLineIndented("//***************************************************************************");
            builder.Write("static Zilch::Function* _");
            builder.Write(constructor->Hash);
            builder.WriteLineIndented(" = nullptr;");
          
            builder.Write(typeName);
            builder.Write("::");
            builder.Write(typeName);

            DelegateType* delegateType = constructor->FunctionType;
            this->WriteParameters(builder, delegateType);

            if (isValueType == false)
            {
              builder.Write(" : ");
              builder.Write(baseName);
              builder.Write("(Zilch::NoType())");
            }

            builder.BeginScope(ScopeType::Function);
            builder.WriteLineIndented();
          
            builder.WriteLineIndented("Zilch::Handle thisHandle;");
            builder.WriteLineIndented("Zilch::CreateAllocatingHandle(thisHandle, this);");

            GuidType cachedDelegateId = this->Mangler.GetDelegateTypeId(delegateType);
            builder.Write("_");
            builder.Write(cachedDelegateId);
            builder.Write("(&thisHandle, _");
            builder.Write(constructor->Hash);

            for (size_t k = 0; k < delegateType->Parameters.Size(); ++k)
            {
              builder.Write(", ");
              DelegateParameter& parameter = delegateType->Parameters[k];
              builder.Write(parameter.GetNameOrGenerate());
            }
            builder.WriteLineIndented(");");

            builder.EndScope();
            builder.WriteLineIndented();
            builder.WriteLineIndented();
          }
        }

        // All function definitions (static and instance)
        for (size_t j = 0; j < type->AllFunctions.Size(); ++j)
        {
          Function* function = type->AllFunctions[j];

          String functionName = function->Name;
          if (function->OwningProperty != nullptr)
          {
            StringIterator begin = functionName.Begin();
            StringIterator end = functionName.End();
            ++begin;
            --end;
            functionName = functionName.SubString(begin, end);
          }
          DelegateType* delegateType = function->FunctionType;

          builder.WriteLineIndented("//***************************************************************************");
          builder.Write("static Zilch::Function* _");
          builder.Write(function->Hash);
          builder.WriteLineIndented(" = nullptr;");

          Type* returnType = delegateType->Return;
          NativeName returnTypeName = this->GetCppTypeName(returnType);

          builder.Write(returnTypeName.Return);
          builder.WriteSpace();
          builder.Write(typeName);
          builder.Write("::");
          builder.Write(functionName);
        
          this->WriteParameters(builder, delegateType);

          builder.BeginScope(ScopeType::Function);
          builder.WriteLineIndented();
        
          GuidType cachedDelegateId = this->Mangler.GetDelegateTypeId(delegateType);

          if (function->This != nullptr)
          {
            builder.Write("Zilch::HandleOf<");
            builder.Write(typeName);
            builder.Write("> thisHandle(this);");
            builder.WriteLineIndented();
          }

          builder.Write("return _");
          builder.Write(cachedDelegateId);
          builder.Write("(");

          if (function->This != nullptr)
            builder.Write("&thisHandle, ");
          else
            builder.Write("nullptr, ");

          builder.Write('_');
          builder.Write(function->Hash);

          for (size_t k = 0; k < delegateType->Parameters.Size(); ++k)
          {
            builder.Write(", ");
            DelegateParameter& parameter = delegateType->Parameters[k];
            builder.Write(parameter.GetNameOrGenerate());
          }
          builder.WriteLineIndented(");");

          builder.EndScope();
          builder.WriteLineIndented();
          builder.WriteLineIndented();
        }
      }
      builder.EndScope();
      builder.WriteLineIndented();
      builder.WriteLineIndented();
    }

    builder.WriteLineIndented("#if defined(_MSC_VER)");
    builder.WriteLineIndented("#pragma optimize(\"\", off)");
    builder.WriteLineIndented("#endif");

    // Define HookUpLibrary function
    builder.WriteLineIndented();
    builder.Write("bool HookUp");
    builder.Write(this->Namespace);
    builder.Write("(Zilch::BuildEvent* event)");
    builder.BeginScope(ScopeType::Function);
    builder.WriteLineIndented();

    builder.Write("Zilch::NativeBindingList::SetBuildingLibraryForThisThread(true);");
    builder.WriteLineIndented();

    builder.WriteLineIndented("Zilch::NameMangler mangler;");

    ZilchForEach(const LibraryRef& library, *this->Libraries)
    {
      builder.BeginScope(ScopeType::Block);

      builder.Write("const char* libraryName = \"");
      builder.Write(library->Name);
      builder.WriteLineIndented("\";");

      builder.WriteLineIndented("Zilch::BoundType* type = nullptr;");

      builder.WriteLineIndented("Zilch::LibraryRef library = event->FindLibrary(libraryName);");
      builder.WriteLineIndented("ReturnIf(library == nullptr, false, \"Unable to find the library %s in the list of dependencies\", libraryName);");

      builder.WriteLineIndented("mangler.MangleLibrary(library);");

      builder.BeginScope(ScopeType::Block);
    }

    ZilchForEach(BoundType* type, this->TypesInDependencyOrder)
    {
      if (this->Namespace == CoreNamespace)
      {
        // Ignore non-native bound types in the Core library (these types don't have a corresponding C++ type)
        if (type->Native == false)
          continue;
      }

      bool isValueType = Type::IsValueType(type);
      String typeName = this->GetCppTypeName(type).Class;

      String qualifiedTypeName = String::Format("%s::%s", this->Namespace.c_str(), typeName.c_str());

      builder.Write("type = Zilch::PatchLibraryType");
      builder.Write("< ");
      builder.Write(qualifiedTypeName);
      builder.Write(" >");
      builder.Write("(library, \"");
      builder.Write(type->Name);
      builder.WriteLineIndented("\");");

      // Assign defined functions if this is not the Core library
      if (this->Namespace != CoreNamespace)
      {
        builder.Write("if (type != nullptr)");
        builder.BeginScope(ScopeType::Block);
        builder.WriteLineIndented();

        FunctionArray functions = type->AllFunctions;

        if (type->Sealed == false || isValueType)
          functions.Append(type->Constructors.All());

        for (size_t j = 0; j < functions.Size(); ++j)
        {
          Function* function = functions[j];
        
          builder.Write(this->Namespace);
          builder.Write("::_");
          builder.Write(function->Hash);
          builder.Write(" = mangler.FindFunction(");
          builder.Write(function->Hash);
          builder.Write(", \"");
          builder.Write(function->Name);
          builder.WriteLineIndented("\", type->Name);");
        }

        builder.EndScope();
        builder.WriteLineIndented();
        builder.WriteLineIndented();
      }
    }

    builder.WriteLineIndented();
    builder.Write("Zilch::NativeBindingList::SetBuildingLibraryForThisThread(false);");
    builder.WriteLineIndented();

    builder.WriteLineIndented();
    builder.WriteLineIndented("return true;");
    builder.EndScope();
    builder.WriteLineIndented();
    
    if (this->CppFooter.Empty() == false)
    {
      builder.WriteLineIndented();
      builder.Write(this->CppFooter);
      builder.WriteLineIndented();
    }

    String result = builder.ToString();
    return result;
  }

  //***************************************************************************
  String NativeStubCode::Generate(const LibraryArray& libraries)
  {
    this->Libraries = &libraries;

    ZilchForEach(const LibraryRef& library, libraries)
    {
      if (this->Namespace.Empty())
      {
        this->Namespace = library->GetPluginNamespace();
        continue;
      }

      ErrorIf(library->GetPluginNamespace() != this->Namespace,
        "All the libraries in the array must have the same namespace");
    }

    ComputeTypesInDependencyOrder(libraries, this->LibrarySet, this->TypesInDependencyOrder);

    this->Hpp = this->GenerateHpp();
    this->Cpp = this->GenerateCpp();

    return this->Namespace;
  }

  //***************************************************************************
  String NativeStubCode::Generate(LibraryParam library)
  {
    return this->Generate(LibraryArray(ZeroInit, library));
  }
}
