/**************************************************************\
* Author: Joshua Davis
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  StubCode::StubCode() :
    SetNativeLocations(false)
  {
  }
  
  //***************************************************************************
  String StubCode::Finalize()
  {
    // Stringify the builder
    String finalCode = this->Builder.ToString();

    // Walk through all native locations and set the code portion
    ZilchForEach(CodeLocation* location, this->NativeLocations)
      location->Code = finalCode;

    return finalCode;
  }
  
  //***************************************************************************
  bool SendsEventSorter(SendsEvent* lhs, SendsEvent* rhs)
  {
    // Sort from A to Z
    int compareName = lhs->Name.CompareTo(rhs->Name);
    if (compareName == -1)
      return true;
    if (compareName == 1)
      return false;

    // The names are equal, so now we need to compare the type names
    return lhs->SentType->Name < rhs->SentType->Name;
  }

  //***************************************************************************
  bool PropertySorter(Property* lhs, Property* rhs)
  {
    // Sort from A to Z (properties should be unique, so no need to sort them by type)
    return lhs->Name < rhs->Name;
  }

  //***************************************************************************
  bool FunctionSorter(Function* lhs, Function* rhs)
  {
    // Sort the entire stringified function (this sorts the signature and the parameters)
    return lhs->ToString() < rhs->ToString();
  }
  
  //***************************************************************************
  void StubCode::Generate(BoundType* type)
  {
    ZilchCodeBuilder& codeBuilder = this->Builder;
    CodeFormat& format = codeBuilder.Format;
    
    this->StartNativeLocation(type->Location);
    this->GenerateHeader(type);

    ZilchTodo("We may want to handle attributes for GetEventHandlerFunction being non-null (may be an interface) / CreatableInScript / Native");


    if (type->CopyMode == TypeCopyMode::ReferenceType)
    {
      codeBuilder.WriteKeywordOrSymbol(Grammar::Class);
    }
    else
    {
      switch (type->SpecialType)
      {
        case SpecialType::Enumeration:
          codeBuilder.WriteKeywordOrSymbol(Grammar::Enumeration);
          break;
        case SpecialType::Flags:
          codeBuilder.WriteKeywordOrSymbol(Grammar::Flags);
          break;
        case SpecialType::Standard:
          codeBuilder.WriteKeywordOrSymbol(Grammar::Struct);
          break;
      }
    }

    codeBuilder.WriteSpace();

    this->StartNativeLocation(type->NameLocation);
    codeBuilder.Write(type->Name);
    this->EndNativeLocation(type->NameLocation);

    if (type->BaseType != nullptr)
    {
      codeBuilder.WriteKeywordOrSymbolSpaceStyle(Grammar::Inheritance, format.SpaceStyleInheritanceColon, format.SpaceStyleGlobalDefaultColon);
      codeBuilder.Write(type->BaseType->Name);

      ZilchForEach(BoundType* interfaceType, type->InterfaceTypes)
      {
        codeBuilder.WriteKeywordOrSymbolSpaceStyle(Grammar::ArgumentSeparator, format.SpaceStyleInheritanceComma, format.SpaceStyleGlobalDefaultComma);
        codeBuilder.Write(interfaceType->Name);
      }
    }

    codeBuilder.BeginScope(ScopeType::Class);
    codeBuilder.WriteLineIndented();

    SendsEventArray sendsEventsSorted = type->SendsEvents;
    Sort(sendsEventsSorted.All(), SendsEventSorter);
    ZilchForEach(SendsEvent* sendsEvent, sendsEventsSorted)
    {
      this->Generate(sendsEvent);
    }
  
    PropertyArray allPropertiesSorted = type->AllProperties;
    Sort(allPropertiesSorted.All(), PropertySorter);
    ZilchForEach(Property* property, allPropertiesSorted)
    {
      this->Generate(property);
    }

    this->Generate(type->Constructors);
    if (type->Destructor != nullptr)
      this->Generate(type->Destructor);
    this->Generate(type->AllFunctions);

    codeBuilder.EndScope();
    this->EndNativeLocation(type->Location);
  }
  
  //***************************************************************************
  void StubCode::Generate(FunctionArray& functions)
  {
    ZilchCodeBuilder& codeBuilder = this->Builder;
    CodeFormat& format = codeBuilder.Format;

    // Loop through all the provided functions
    FunctionArray functionsSorted = functions;
    Sort(functionsSorted.All(), FunctionSorter);
    ZilchForEach(Function* function, functionsSorted)
    {
      this->Generate(function);
    }
  }
  
  //***************************************************************************
  void StubCode::Generate(Function* function)
  {
    ZilchCodeBuilder& codeBuilder = this->Builder;
    CodeFormat& format = codeBuilder.Format;

    // If this is a property get or set, then skip it
    if (function->OwningProperty != nullptr)
      return;
    
    this->StartNativeLocation(function->Location);
    this->GenerateHeader(function);

    // If this is a constructor...
    if (function->Name == ConstructorName)
    {
      this->StartNativeLocation(function->NameLocation);
      codeBuilder.WriteKeywordOrSymbol(Grammar::Constructor);
      this->EndNativeLocation(function->NameLocation);
    }
    else if (function->Name == DestructorName)
    {
      this->StartNativeLocation(function->NameLocation);
      codeBuilder.WriteKeywordOrSymbol(Grammar::Destructor);
      this->EndNativeLocation(function->NameLocation);
    }
    // Otherwise we assume its a normal named function
    else
    {
      codeBuilder.WriteKeywordOrSymbol(Grammar::Function);
      codeBuilder.WriteSpace();
      this->StartNativeLocation(function->NameLocation);
      codeBuilder.Write(function->Name);
      this->EndNativeLocation(function->NameLocation);
    }

    codeBuilder.WriteKeywordOrSymbolSpaceStyle(Grammar::BeginFunctionParameters, format.SpaceStyleFunctionDefinitionBeginParenthesis, format.SpaceStyleGlobalDefaultParenthesis);

    int parameterIndex = 0;
    ZilchForRange(DelegateParameter& parameter, parameterRange, function->FunctionType->Parameters)
    {
      String paramName = parameter.Name;
      if(paramName.Empty())
      {
        if(function->FunctionType->Parameters.Size() == 1)
          paramName = "value";
        else
          paramName = String::Format("p%d", parameterIndex);
      }
        

      codeBuilder.Write(paramName);
      codeBuilder.WriteKeywordOrSymbolSpaceStyle(Grammar::TypeSpecifier, format.SpaceStyleTypeColon, format.SpaceStyleGlobalDefaultColon);
      codeBuilder.Write(parameter.ParameterType->ToString());

      if (parameterRange.Empty() == false)
        codeBuilder.WriteKeywordOrSymbolSpaceStyle(Grammar::ArgumentSeparator, format.SpaceStyleFunctionCallParameterComma, format.SpaceStyleGlobalDefaultComma);
      ++parameterIndex;
    }

    codeBuilder.WriteKeywordOrSymbolSpaceStyle(Grammar::EndFunctionParameters, format.SpaceStyleFunctionDefinitionEndParenthesis, format.SpaceStyleGlobalDefaultParenthesis);

    // Write out the return type if it's not void
    if(function->FunctionType->Return != ZilchTypeId(void))
    {
      codeBuilder.WriteKeywordOrSymbolSpaceStyle(Grammar::TypeSpecifier, format.SpaceStyleTypeColon, format.SpaceStyleGlobalDefaultColon);
      codeBuilder.Write(function->FunctionType->Return->ToString());
    }

    codeBuilder.WriteKeywordOrSymbol(Grammar::StatementSeparator);

    this->EndNativeLocation(function->Location);

    codeBuilder.WriteLineIndented();
    codeBuilder.WriteLineIndented();
  }

  //***************************************************************************
  void StubCode::Generate(SendsEvent* sends)
  {
    ZilchCodeBuilder& codeBuilder = this->Builder;
    CodeFormat& format = codeBuilder.Format;

    this->StartNativeLocation(sends->Location);

    codeBuilder.WriteKeywordOrSymbol(Grammar::Sends);
    codeBuilder.WriteSpace();

    this->StartNativeLocation(sends->NameLocation);
    codeBuilder.Write(sends->Name);
    this->EndNativeLocation(sends->NameLocation);

    codeBuilder.WriteKeywordOrSymbolSpaceStyle(Grammar::TypeSpecifier, format.SpaceStyleTypeColon, format.SpaceStyleGlobalDefaultColon);
    codeBuilder.Write(sends->SentType->ToString());
    codeBuilder.WriteKeywordOrSymbol(Grammar::StatementSeparator);
    this->EndNativeLocation(sends->Location);

    codeBuilder.WriteLineIndented();
    codeBuilder.WriteLineIndented();
  }
  
  //***************************************************************************
  void StubCode::Generate(Property* property)
  {
    ZilchCodeBuilder& codeBuilder = this->Builder;
    CodeFormat& format = codeBuilder.Format;
    
    this->StartNativeLocation(property->Location);

    this->GenerateHeader(property);
    codeBuilder.WriteKeywordOrSymbol(Grammar::Variable);
    codeBuilder.WriteSpace();
    
    this->StartNativeLocation(property->NameLocation);
    codeBuilder.Write(property->Name);
    this->EndNativeLocation(property->NameLocation);

    codeBuilder.WriteKeywordOrSymbolSpaceStyle(Grammar::TypeSpecifier, format.SpaceStyleTypeColon, format.SpaceStyleGlobalDefaultColon);
    codeBuilder.Write(property->PropertyType->ToString());

    // If type is a field (no get/set, just a raw data member)
    Field* field = Type::DynamicCast<Field*>(property);
    if (field != nullptr)
    {
      codeBuilder.WriteKeywordOrSymbol(Grammar::StatementSeparator);

      // We do this again below (but that's alright!)
      // We need to do this here so the get/set locations will be correct
      this->EndNativeLocation(property->Location);

      if (this->SetNativeLocations)
      {
        if (field->Get != nullptr)
        {
          field->Get->Location = property->Location;
          field->Get->NameLocation = property->NameLocation;
        }
        if (field->Set != nullptr)
        {
          field->Set->Location = property->Location;
          field->Set->NameLocation = property->NameLocation;
        }
      }
    }
    // Otherwise type is a property
    else
    {
      codeBuilder.WriteSpace();
      codeBuilder.WriteKeywordOrSymbol(Grammar::BeginScope);
      codeBuilder.WriteSpace();

      if (property->Get != nullptr)
      {
        Function* get = property->Get;
        this->StartNativeLocation(get->Location);
        this->StartNativeLocation(get->NameLocation);
        codeBuilder.WriteKeywordOrSymbol(Grammar::Get);
        this->EndNativeLocation(get->NameLocation);
        this->EndNativeLocation(get->Location);

        codeBuilder.WriteKeywordOrSymbol(Grammar::StatementSeparator);
        codeBuilder.WriteSpace();
      }

      if (property->Set != nullptr)
      {
        Function* set = property->Set;
        this->StartNativeLocation(set->Location);
        this->StartNativeLocation(set->NameLocation);
        codeBuilder.WriteKeywordOrSymbol(Grammar::Set);
        this->EndNativeLocation(set->NameLocation);
        this->EndNativeLocation(set->Location);

        codeBuilder.WriteKeywordOrSymbol(Grammar::StatementSeparator);
        codeBuilder.WriteSpace();
      }

      codeBuilder.WriteKeywordOrSymbol(Grammar::EndScope);
    }

    this->EndNativeLocation(property->Location);

    codeBuilder.WriteLineIndented();
    codeBuilder.WriteLineIndented();
  }

  //***************************************************************************
  void StubCode::GenerateHeader(Function* function)
  {
    // These should be moved to actual attributes in code generation (JoshD)
    Array<Attribute> attributes;
    if (function->IsStatic)
    {
      Attribute staticAttribute;
      staticAttribute.Name = StaticAttribute;
      attributes.PushBack(staticAttribute);
    }
    attributes.Insert(attributes.End(), function->Attributes.All());
    GenerateHeader(function, attributes);
  }

  //***************************************************************************
  void StubCode::GenerateHeader(Property* property)
  {
    // These should be moved to actual attributes in code generation (JoshD)
    Array<Attribute> attributes;
    if (property->IsStatic)
    {
      Attribute staticAttribute;
      staticAttribute.Name = StaticAttribute;
      attributes.PushBack(staticAttribute);
    }
    if (property->IsHiddenWhenNull)
    {
      Attribute staticAttribute;
      staticAttribute.Name = "HiddenWhenNull";
      attributes.PushBack(staticAttribute);
    }

    attributes.Insert(attributes.End(), property->Attributes.All());
    GenerateHeader(property, attributes);
  }

  //***************************************************************************
  void StubCode::GenerateHeader(ReflectionObject* object)
  {
    GenerateHeader(object, object->Attributes);
  }

  //***************************************************************************
  void StubCode::GenerateHeader(ReflectionObject* object, Array<Attribute>& attributes)
  {
    ZilchCodeBuilder& codeBuilder = this->Builder;
    CodeFormat& format = codeBuilder.Format;

    // We don't actually have a grammar symbol for single line comment (we should probably make it one...)
    String wrappedDescription = Zero::WordWrap(object->Description, format.CommentWordWrapLength);
    ZilchForEach(StringRange line, wrappedDescription.Split("\n"))
    {
      codeBuilder.WriteSingleLineComment(line);
      codeBuilder.WriteLineIndented();
    }

    ZilchTodo("We may want to handle the 'IsHidden' attribute specially here because it may not exist inside 'Attributes'");

    // Write out all attributes
    ZilchForEach(Attribute& attribute, attributes)
    {
      codeBuilder.WriteKeywordOrSymbol(Grammar::BeginAttribute);
      codeBuilder.Write(attribute.Name);
      if (attribute.Parameters.Empty() == false)
      {
        codeBuilder.WriteKeywordOrSymbol(Grammar::BeginFunctionCall);

        size_t parameterCount = attribute.Parameters.Size();
        size_t lastParameter = parameterCount - 1;
        for (size_t i = 0; i < parameterCount; ++i)
        {
          AttributeParameter& parameter = attribute.Parameters[i];
          if (parameter.Name.Empty() == false)
          {
            codeBuilder.Write(parameter.Name);
            codeBuilder.WriteKeywordOrSymbolSpaceStyle(Grammar::NameSpecifier, format.SpaceStyleNamedArgumentColon, format.SpaceStyleGlobalDefaultColon);
          }

          codeBuilder.Write(parameter.ToString());

          if (i != lastParameter)
            codeBuilder.WriteKeywordOrSymbolSpaceStyle(Grammar::ArgumentSeparator, format.SpaceStyleFunctionCallParameterComma, format.SpaceStyleGlobalDefaultComma);
        }

        codeBuilder.WriteKeywordOrSymbol(Grammar::EndFunctionCall);
      }
      codeBuilder.WriteKeywordOrSymbol(Grammar::EndAttribute);
    }

    if (attributes.Empty() == false)
      codeBuilder.WriteLineIndented();
  }
  
  //***************************************************************************
  void StubCode::StartNativeLocation(CodeLocation& location)
  {
    // If we're not tracking native locations, or this location isn't native then early out
    // Note: Non-native locations should already be set by the tokenizer/parser/syntaxer
    if (this->SetNativeLocations == false || location.IsNative == false)
      return;

    // Track this location so that we can set the 'Code' portion when we finalize this stub code
    this->NativeLocations.PushBack(&location);
    
    ZilchCodeBuilder& codeBuilder = this->Builder;
    location.Origin = this->GeneratedOriginOrName;
    location.StartLine = codeBuilder.GetLine();
    location.PrimaryLine = codeBuilder.GetLine();
    location.EndLine = codeBuilder.GetLine();
    //location.StartCharacter;
    //location.PrimaryCharacter;
    //location.EndCharacter;
    location.StartPosition = codeBuilder.GetSize();
    location.PrimaryPosition = codeBuilder.GetSize();
    location.EndPosition = codeBuilder.GetSize();
  }
  
  //***************************************************************************
  void StubCode::EndNativeLocation(CodeLocation& location)
  {
    // If we're not tracking native locations, or this location isn't native then early out
    // Note: Non-native locations should already be set by the tokenizer/parser/syntaxer
    if (this->SetNativeLocations == false || location.IsNative == false)
      return;

    ZilchCodeBuilder& codeBuilder = this->Builder;
    location.EndLine = codeBuilder.GetLine();
    //location.EndCharacter;
    location.EndPosition = codeBuilder.GetSize();
  }
}
