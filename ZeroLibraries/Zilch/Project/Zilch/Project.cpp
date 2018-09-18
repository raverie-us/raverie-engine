/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  namespace Events
  {
    ZilchDefineEvent(PreParser);
    ZilchDefineEvent(TypeParsed);
    ZilchDefineEvent(PostSyntaxer);
  }

  //***************************************************************************
  ZilchDefineType(ParseEvent, builder, type)
  {
  }

  //***************************************************************************
  ParseEvent::ParseEvent() :
    Builder(nullptr),
    Type(nullptr),
    Location(nullptr),
    BuildingProject(nullptr)
  {
  }

  //***************************************************************************
  CompletionEntry::CompletionEntry() :
    CodeUserData(nullptr),
    Hidden(false)
  {
  }

  //***************************************************************************
  bool CompletionEntry::operator<(const CompletionEntry& rhs) const
  {
    // We want names to be alphabetical starting with A
    if (this->Name < rhs.Name)
      return true;
    if (this->Name > rhs.Name)
      return false;
    
    // We want types to be alphabetical starting with A
    if (this->Type < rhs.Type)
      return true;
    if (this->Type > rhs.Type)
      return false;

    // We actually use > to compare descriptions because we want the longest first
    if (this->Description.SizeInBytes() > rhs.Description.SizeInBytes())
      return true;
    if (this->Description.SizeInBytes() < rhs.Description.SizeInBytes())
      return false;

    // Finally, the descriptions had the same length, so compare their contents (may be equal!)
    return this->Description < rhs.Description;
  }
  
  //***************************************************************************
  CompletionParameter::CompletionParameter()
  {
  }

  //***************************************************************************
  AutoCompleteInfo::AutoCompleteInfo() :
    IsStatic(false),
    NearestType(nullptr),
    IsLiteral(false),
    RemoveDuplicateNameEntries(true),
    BestCompletionOverload(-1),
    Success(false),
    CallArgumentIndex(-1)
  {
  }
  
  //***************************************************************************
  String AutoCompleteInfo::GetShortTypeName(StringParam fullTypeName)
  {
    // Empty strings should still return an entry when splitting (at least I think that would make sense), but just to be safe...
    if (fullTypeName.Empty())
      return String();

    // Get the first word from the full type name
    String shortName = fullTypeName.Split(" ").Front();

    // If the name is still too long, then trim it down and add an elipsis
    static const String Elipsis("...");
    if (shortName.ComputeRuneCount() > ShortTypeNameMaxLength)
    {
      StringIterator start = shortName.Begin();
      StringIterator end = start + ShortTypeNameMaxLength;
      shortName = BuildString(shortName.SubString(start, end), Elipsis);
    }
    return shortName;
  }
  
  //***************************************************************************
  String AutoCompleteInfo::GetJson()
  {
    JsonBuilder builder;
    builder.Begin(JsonType::Object);
    {
      builder.Key("Success");
      builder.Value(this->Success);

      builder.Key("NearestType");
      if (this->NearestType != nullptr)
        builder.Value(this->NearestType->ToString());
      else
        builder.Value(String());
      
      builder.Key("IsLiteral");
      builder.Value(this->IsLiteral);
      
      builder.Key("IsStatic");
      builder.Value(this->IsStatic);

      builder.Key("CompletionEntries");
      builder.Begin(JsonType::ArrayMultiLine);
      {
        for (size_t i = 0; i < this->CompletionEntries.Size(); ++i)
        {
          CompletionEntry& entry = this->CompletionEntries[i];
          builder.Begin(JsonType::Object);
          {
            builder.Key("Name");
            builder.Value(entry.Name);

            builder.Key("Description");
            builder.Value(entry.Description);

            builder.Key("Type");
            builder.Value(entry.Type);

            builder.Key("ShortType");
            builder.Value(entry.ShortType);
          }
          builder.End();
        }
      }
      builder.End();
      
      builder.Key("FunctionName");
      builder.Value(this->FunctionName);

      builder.Key("BestCompletionOverload");
      builder.Value(this->BestCompletionOverload);

      builder.Key("CompletionOverloads");
      builder.Begin(JsonType::ArrayMultiLine);
      {
        for (size_t i = 0; i < this->CompletionOverloads.Size(); ++i)
        {
          CompletionOverload& overload = this->CompletionOverloads[i];
          builder.Begin(JsonType::Object);
          {
            builder.Key("Description");
            builder.Value(overload.Description);
            
            builder.Key("ReturnType");
            builder.Value(overload.ReturnType);
            
            builder.Key("ReturnShortType");
            builder.Value(overload.ReturnType);
            
            builder.Key("Signature");
            builder.Value(overload.Signature);

            builder.Key("Parameters");
            builder.Begin(JsonType::ArrayMultiLine);
            {
              for (size_t i = 0; i < overload.Parameters.Size(); ++i)
              {
                CompletionParameter& parameter = overload.Parameters[i];
                builder.Begin(JsonType::Object);
                {
                  builder.Key("Name");
                  builder.Value(parameter.Name);
                  
                  builder.Key("Description");
                  builder.Value(parameter.Description);
                  
                  builder.Key("Type");
                  builder.Value(parameter.Type);
                  
                  builder.Key("ShortType");
                  builder.Value(parameter.ShortType);
                }
                builder.End();
              }
            }
            builder.End();
          }
          builder.End();
        }
      }
      builder.End();
    }
    builder.End();

    String result = builder.ToString();
    return result;
  }
  
  //***************************************************************************
  CodeDefinition::CodeDefinition() :
    DefinedVariable(nullptr),
    DefinedFunction(nullptr),
    DefinedGetterSetter(nullptr),
    DefinedField(nullptr),
    DefinedProperty(nullptr),
    DefinedMember(nullptr),
    DefinedObject(nullptr),
    ResolvedType(nullptr)
  {
  }

  //***************************************************************************
  Project::Project() :
    CursorPosition(NoCursor),
    UserData(nullptr),
    VariableUniqueIdCounter(0)
  {
    ZilchErrorIfNotStarted(Project);
  }

  //***************************************************************************
  void Project::AddCodeFromString(StringParam code, StringParam origin, void* codeUserData)
  {
    // Add an entry to the list of all entries
    CodeEntry& entry = this->Entries.PushBack();
    entry.Code = code;
    entry.Origin = origin;
    entry.CodeUserData = codeUserData;
  }

  //***************************************************************************
  String Project::ReadTextFile(Status& status, StringParam fileName)
  {
    // Attempt to open the file for text reading
    File file;
    file.Open(fileName, Zero::FileMode::Read, Zero::FileAccessPattern::Random, Zero::FileShare::Read, &status);

    // Create a string builder to concatenate all read in chunks together
    StringBuilder builder;

    // If the file pointer is valid...
    if (status.Succeeded())
    {
      // Create a temporary buffer to store read in chunks
      const size_t BufferSize = 4096;
      byte buffer[BufferSize];
      
      // While we haven't reached the end of the file and have no errors...
      ZilchLoop
      {
        size_t amountRead = file.Read(status, buffer, BufferSize);

        // Add the amount read into the string range
        builder.Append(StringRange((char*)buffer, (char*)(buffer + amountRead)));

        // If we reached the end or had an error
        if (amountRead == 0 || status.Failed())
          break;
      }
    }

    return builder.ToString();
  }

  //***************************************************************************
  bool Project::AddCodeFromFile(StringParam fileName, void* codeUserData)
  {
    // Read all the file into a string
    Status status;
    String code = ReadTextFile(status, fileName);

    // If we successfully read in the file...
    if (status.Succeeded())
    {
      // Add the code to the project
      this->AddCodeFromString(code, fileName, codeUserData);
    }

    // Return whether or not we successfully read the file
    return status.Succeeded();
  }

  //***************************************************************************
  void Project::Clear()
  {
    this->Entries.Clear();
  }

  //***************************************************************************
  bool Project::Tokenize(Array<UserToken>& tokensOut, Array<UserToken>& commentsOut)
  {
    // Reset whether there was an error or not
    this->WasError = false;

    // The tokenizer that parses the input stream into a list of tokens
    Tokenizer tokenizer(*this);

    // Loop through all the project entries
    for (size_t i = 0; i < this->Entries.Size(); ++i)
    {
      // Grab the current project entry
      CodeEntry& entry = this->Entries[i];

      // Keep parsing all code into the same token stream
      tokenizer.Parse(entry, tokensOut, commentsOut);
    }

    // Finalize the token stream
    tokenizer.Finalize(tokensOut);

    // Return true if it succeeded, or false if there was an error in tokenizing
    return !this->WasError;
  }

  //***************************************************************************
  // This simple struct is used to define information about which syntax nodes came from which lines
  class ZeroShared OriginInfo
  {
  public:
    // Constructor
    OriginInfo() :
      MaxLine(0)
    {
    }

    // The max line reachable in the file (actually it's the last node we saw)
    size_t MaxLine;

    // The lines that map to a syntax node (some lines will be empty)
    HashMap<size_t, SyntaxNode*> LineToNode;
  };

  //***************************************************************************
  // Maps the line that a node occurs on to the node itself
  void MapLinesToNodes(HashMap<String, OriginInfo>& info, SyntaxNode* node)
  {
    // Populate all the children of the current node
    NodeChildren children;
    node->PopulateChildren(children);
    
    // Get the origin information by name
    OriginInfo& origin = info[node->Location.Origin];

    // Map the current node's line to the node itself
    // If another node exists under that same line, keep the first one
    // This is so that comments get attached to the highest parent node occurring on that line
    // (eg for 'var i = 5;' to the 'var' statement instead of the '5' expression);
    origin.LineToNode.InsertNoOverwrite(node->Location.StartLine, node);

    // If this is the furthest node we've encountered in the file...
    if (node->Location.StartLine > origin.MaxLine)
    {
      // Push out the max line so we know how far to look when attaching comments
      origin.MaxLine = node->Location.StartLine;
    }

    // Loop through the children and map the lines again
    for (size_t i = 0; i < children.Size(); ++i)
    {
      // Recursively invoke the map lines function
      MapLinesToNodes(info, (*children[i]));
    }
  }

  //***************************************************************************
  void Project::AttachCommentsToNodes(SyntaxTree& syntaxTree, Array<UserToken>& comments)
  {
    // Setup the map that we use to associate lines with nodes
    HashMap<String, OriginInfo> info;

    // Perform the actual line to node association, starting at the root going down
    MapLinesToNodes(info, syntaxTree.Root);

    // Loop through all the comments we parsed
    for (size_t i = 0; i < comments.Size(); ++i)
    {
      // Get the current comment
      UserToken& comment = comments[i];

      // We need to start by looking in the file/origin where the comment existed
      OriginInfo& origin = info[comment.Location.Origin];

      // Check only if it's on the same line or the next line (don't attach comments if they have one line space in between)
      size_t endLine = comment.Location.EndLine + 1;
      for (size_t line = comment.Location.StartLine; line <= origin.MaxLine && line <= endLine; ++line)
      {
        // Attempt to find a node at the current line
        SyntaxNode* node = origin.LineToNode.FindValue(line, nullptr);

        // If we found a node and that node's primary location is the line we're attaching to...
        if (node != nullptr && node->Location.PrimaryLine == line)
        {
          // If this is an attribute, then instead attach it to the parent of the attribute (class, function, property, etc)
          if (AttributeNode* attributeNode = node->FindParentOrSelf<AttributeNode>())
            node = attributeNode->Parent;

          // Append the comment to the node
          node->Comments.PushBack(comment.Token);

          // Check if we need to attach this comment to any children who are at the exact same position
          // This handles nodes such as IfRootNode and the first IfNode under it
          // LineToNode will always start with the most root node
          NodeChildren children;
          node->PopulateChildren(children);
          ZilchForEach(SyntaxNode** childPtr, children)
          {
            SyntaxNode* child = *childPtr;

            // Append the comment to the child also if the child has the same primary line and start position
            if (child->Location.StartPosition == node->Location.StartPosition && child->Location.PrimaryLine == line)
              child->Comments.PushBack(comment.Token);
          }

          break;
        }

        // If we got here and it's the end line, then we didn't attach the comment
        if (line == endLine)
        {
          // If there's a comment on this line, then extend our end line
          for (size_t k = i; k < comments.Size(); ++k)
          {
            UserToken& nextComment = comments[k];

            // If the line is before the next comment, then we're done searching...
            if (line < nextComment.Location.StartLine)
              break;

            // If the line is within the end line of the next comment,
            // then we should keep searching past the end of that comment!
            if (line <= nextComment.Location.EndLine)
              endLine = nextComment.Location.EndLine + 1;
          }
        }
      }
    }
  }

  //***************************************************************************
  bool Project::CompileUncheckedSyntaxTree(SyntaxTree& syntaxTreeOut, Array<UserToken>& tokensOut, EvaluationMode::Enum evaluation)
  {
    // Reset the unique variable-id counter (ensures deterministic behavior)
    this->VariableUniqueIdCounter = 0;

    // Store all the parsed comment tokens
    Array<UserToken> comments;

    // Start by tokenizing the stream
    if (this->Tokenize(tokensOut, comments) == false)
      return false;

    // The parser parses the list of tokens into a syntax tree
    Parser parser(*this);
    
    // Apply the parser to the token stream, which should output a syntax tree!
    parser.ParseIntoTree(tokensOut, syntaxTreeOut, evaluation);

    // Make sure to attach all the comments we parsed to
    // any nodes, so we can collect them for documentation
    this->AttachCommentsToNodes(syntaxTreeOut, comments);

    // Fix up any parent pointers
    SyntaxNode::FixParentPointers(syntaxTreeOut.Root, nullptr);

    // Return true if it succeeded, or false if there was an error in parsing
    return !this->WasError;
  }

  //***************************************************************************
  bool Project::CompileCheckedSyntaxTree
  (
    SyntaxTree& syntaxTreeOut,
    LibraryBuilder& builder,
    Array<UserToken>& tokensOut,
    const Module& dependencies,
    EvaluationMode::Enum evaluation
  )
  {
    // The syntaxer holds information about all the internal and parsed types
    // It is also responsible for checking syntax for things like scope, etc
    Syntaxer syntaxer(*this);

    // Start by compiling the code into an unchecked tree
    if (this->CompileUncheckedSyntaxTree(syntaxTreeOut, tokensOut, evaluation) == false)
      return false;

    // Collect all the types, Assign types where they are needed, and perform syntax checking
    syntaxer.ApplyToTree(syntaxTreeOut, builder, *this, dependencies);

    // Fix up any parent pointers (in case anything gets moved around)
    // This may be unnecessary... but we'd still like to do it
    SyntaxNode::FixParentPointers(syntaxTreeOut.Root, nullptr);

    // Return true if it succeeded, or false if there was a syntax error
    return !this->WasError;
  }
  
  //***************************************************************************
  LibraryRef Project::Compile(StringParam libraryName, Module& dependencies, EvaluationMode::Enum evaluation, SyntaxTree& treeOut, Array<UserToken>& tokensOut)
  {
    // We're about to generate a library so we need a builder
    LibraryBuilder builder(libraryName);
    builder.BuiltLibrary->TolerantMode = this->TolerantMode;

    // Let the user know the library finished running the syntaxer (the user may add types here)
    ParseEvent preEvent;
    preEvent.Builder = &builder;
    preEvent.BuildingProject = this;
    EventSend(this, Events::PreParser, &preEvent);

    // Let the library know what source was used to build it
    builder.SetEntries(this->Entries);

    // Compile the code into a checked syntax tree
    if (this->CompileCheckedSyntaxTree(treeOut, builder, tokensOut, dependencies, evaluation) == false)
      return nullptr;

    // Let the user know the library finished running the syntaxer (the user may add types here)
    ParseEvent postEvent;
    postEvent.Builder = &builder;
    postEvent.BuildingProject = this;
    EventSend(this, Events::PostSyntaxer, &postEvent);

    // Only generate code if we're not in tolerant mode (otherwise it would probably be seriously messed up...)
    if (this->TolerantMode == false)
    {
      // The code generator uses the syntax tree to generate opcode for each function
      CodeGenerator codeGenerator;
      LibraryRef library = codeGenerator.Generate(treeOut, builder);

      // Check that the library was valid
      ErrorIf(library == nullptr, "Somehow the library returned from code generation was not valid!");
      return library;
    }
    else
    {
      // Create the library without code generation
      return builder.CreateLibrary();
    }
  }

  //***************************************************************************
  LibraryRef Project::Compile(StringParam libraryName, Module& dependencies, EvaluationMode::Enum evaluation)
  {
    // The syntax tree holds a more intuitive representation of the parsed program and is easy to traverse
    SyntaxTree syntaxTree;
    Array<UserToken> tokens;
    return this->Compile(libraryName, dependencies, evaluation, syntaxTree, tokens);
  }
  
  //***************************************************************************
  CompletionOverload& Project::AddAutoCompleteOverload(AutoCompleteInfo& info, DelegateType* delegateType)
  {
    // Create a new overload that we'll return to the user
    CompletionOverload& overload = info.CompletionOverloads.PushBack();

    // First, output the entire signature of the delegate
    overload.Signature = delegateType->ToString();

    // Fill out the return type of the delegate if its not void
    if (Zilch::Type::IsSame(ZilchTypeId(void), delegateType->Return) == false)
    {
      overload.ReturnType = delegateType->Return->ToString();
      overload.ReturnShortType = AutoCompleteInfo::GetShortTypeName(overload.ReturnType);
    }

    // Walk through all the delegate parameters and add them as completion parameters
    for (size_t i = 0; i < delegateType->Parameters.Size(); ++i)
    {
      // Grab the current delegate parameter and make a completion parameter for it
      DelegateParameter& delegateParam = delegateType->Parameters[i];
      CompletionParameter& completionParam = overload.Parameters.PushBack();
      completionParam.Type = delegateParam.ParameterType->ToString();
      completionParam.ShortType = AutoCompleteInfo::GetShortTypeName(completionParam.Type);
      completionParam.Name = delegateParam.Name;
    }

    return overload;
  }

  //***************************************************************************
  // This is a functor entirely used for the below function (should be a local to the function, but some compilers don't support that...)
  class AutoCompletePropertyFunctionQuery
  {
  public:
    AutoCompleteInfo* Info;
    String CursorOrigin;

    // Every time we encounter an extension property...
    bool operator()(Property* property)
    {
      // Fill out the entry with information about the property name, description, and stringified type
      CompletionEntry& entry = this->Info->CompletionEntries.PushBack();
      entry.Name = property->Name;
      entry.Description = property->Description;
      entry.Type = property->PropertyType->ToString();
      entry.ShortType = AutoCompleteInfo::GetShortTypeName(entry.Type);
      entry.CodeUserData = property->NameLocation.CodeUserData;
      entry.CodeUserDataU64 = property->NameLocation.CodeUserDataU64;
      entry.Hidden = IsInternalHidden(property);
      return false;
    }
    
    // Every time we encounter an extension function...
    bool operator()(Function* function)
    {
      // Fill out the entry with information about the property name, description, and stringified type
      CompletionEntry& entry = this->Info->CompletionEntries.PushBack();
      entry.Name = function->Name;
      entry.Description = function->Description;
      entry.Type = function->FunctionType->ToString();
      entry.ShortType = AutoCompleteInfo::GetShortTypeName(entry.Type);
      entry.CodeUserData = function->NameLocation.CodeUserData;
      entry.CodeUserDataU64 = function->NameLocation.CodeUserDataU64;
      entry.Hidden = IsInternalHidden(function);
      return false;
    }

    bool IsInternalHidden(Member* member)
    {
      if (member->HasAttribute(InternalAttribute))
        return (member->Location.Origin != CursorOrigin);
      return false;
    }
  };
  
  //***************************************************************************
  void Project::InitializeDefinitionInfo(CodeDefinition& resultOut, ReflectionObject* object)
  {
    // Get the type of the documented object (could be a property type, function type, class/struct type, etc)
    resultOut.ResolvedType = object->GetTypeOrNull();

    // If this is a native location, we need to generate stub code
    if (object->NameLocation.IsNative)
    {
      // Generate stub code for the library (if its already generated, this will do nothing)
      Library* library = object->GetOwningLibrary();
      library->GenerateDefinitionStubCode();
    }

    // Set the element and the name of the location
    resultOut.ElementLocation = object->Location;
    resultOut.NameLocation = object->NameLocation;

    resultOut.DefinedObject = object;
  }

  //***************************************************************************
  String Project::GetFriendlyTypeName(Type* type)
  {
    String name = type->ToString();

    // If this is a bound type...
    if (BoundType* boundType = Type::DynamicCast<BoundType*>(type))
    {
      if (boundType->CopyMode == TypeCopyMode::ReferenceType)
      {
        name = BuildString(Grammar::GetKeywordOrSymbol(Grammar::Class), " ", name);
      }
      else
      {
        name = BuildString(Grammar::GetKeywordOrSymbol(Grammar::Struct), " ", name);
      }
    }

    return name;
  }

  //***************************************************************************
  void Project::GetDefinitionInfo(Module& dependencies, size_t cursorPosition, StringParam cursorOrigin, CodeDefinition& resultOut)
  {
    GetDefinitionInfoInternal(dependencies, cursorPosition, cursorOrigin, resultOut);

    if (resultOut.ToolTip.Empty())
    {
      // If we have a name such as a member or a variable...
      if (!resultOut.Name.Empty())
      {
        // In almost all cases we should have gotten a valid type, so append it to the end with a ':'
        if (resultOut.ResolvedType != nullptr)
        {
          resultOut.ToolTip = BuildString(resultOut.Name, " : ", GetFriendlyTypeName(resultOut.ResolvedType));
        }
        // If we didn't get a type somehow, our tooltip is just the name
        else
        {
          resultOut.ToolTip = resultOut.Name;
        }
      }
      // Otherwise, we have no name, but we do have a type
      // This occurs when we're pointing at a class/struct definition, or any expression, etc
      else if (resultOut.ResolvedType != nullptr)
      {
        resultOut.ToolTip = GetFriendlyTypeName(resultOut.ResolvedType);
      }

      // Finally, if we're pointing at the definition of anything...
      if (resultOut.DefinedObject != nullptr)
      {
        String name = ZilchVirtualTypeId(resultOut.DefinedObject)->ToString();

        if (GetterSetter* getterSetter = Type::DynamicCast<GetterSetter*>(resultOut.DefinedObject))
        {
          if (getterSetter->Get != nullptr && getterSetter->Set != nullptr)
            name = BuildString(name, " (get/set)");
          else if (getterSetter->Get != nullptr)
            name = BuildString(name, " (get)");
          else if (getterSetter->Set != nullptr)
            name = BuildString(name, " (set)");
        }

        // Put the type of the definition at the beginning to give users more info
        resultOut.ToolTip = BuildString(
          name,
          " - ",
          resultOut.ToolTip);


        // If the definition has a description, append that with a newline to the end
        if (!resultOut.DefinedObject->Description.Empty())
        {
          resultOut.ToolTip = BuildString(
            resultOut.ToolTip,
            "\n",
            resultOut.DefinedObject->Description);
        }
      }
    }
  }

  //***************************************************************************
  void Project::GetDefinitionInfoInternal(Module& dependencies, size_t cursorPosition, StringParam cursorOrigin, CodeDefinition& resultOut)
  {
    // Temporary set tolerant mode to true (recall it on any exiting of this function)
    Zero::SetAndRecallOnDestruction<bool> changeTolerantMode(&this->TolerantMode, true);

    // Compile the entirety of the project and get the syntax tree out of it
    // We MUST store the library or all the resources will be released
    SyntaxTree syntaxTree;
    Array<UserToken> tokens;
    resultOut.IncompleteLibrary = this->Compile(DefaultLibraryName, dependencies, EvaluationMode::Project, syntaxTree, tokens);

    // Get all the syntax nodes under the cursor
    Array<SyntaxNode*> nodes;
    syntaxTree.GetNodesAtCursor(cursorPosition, cursorOrigin, nodes);

    // If we received no nodes, then return nothing (CodeDefinition will remain empty)
    if (nodes.Empty())
      return;

    bool isValidToken = false;

    // Find where we are in the token stream
    for (size_t i = 0; i < tokens.Size(); ++i)
    {
      UserToken& token = tokens[i];
      if (token.Location.Origin != cursorOrigin)
        continue;
      
      // If we found the token that our cursor is over...
      if (cursorPosition >= token.Location.StartPosition && cursorPosition <= token.Location.EndPosition)
      {
        Grammar::Enum id = token.TokenId;

        isValidToken =
          id != Grammar::Invalid      &&
          id != Grammar::End          &&
          id != Grammar::Error        &&
          id != Grammar::Whitespace   &&
          id != Grammar::CommentStart &&
          id != Grammar::CommentLine  &&
          id != Grammar::CommentEnd;
        break;
      }
    }

    if (!isValidToken)
      return;

    // Walk through the nodes backwards (since the more leaf nodes end up at the back)
    for (int i = (int)nodes.Size() - 1; i >= 0; --i)
    {
      // Grab the current node
      SyntaxNode* node = nodes[i];

      // Definitions
      if (ClassNode* typedNode = Type::DynamicCast<ClassNode*>(node))
      {
        BoundType* resolvedType = typedNode->Type;
        if (resolvedType != nullptr)
        {
          this->InitializeDefinitionInfo(resultOut, resolvedType);
          return;
        }
      }
      else if (EnumNode* enumNode = Type::DynamicCast<EnumNode*>(node))
      {
        BoundType* resolvedType = enumNode->Type;
        if (resolvedType != nullptr)
        {
          this->InitializeDefinitionInfo(resultOut, resolvedType);
          return;
        }
      }
      else if (FunctionNode* typedNode = Type::DynamicCast<FunctionNode*>(node))
      {
        Function* function = typedNode->DefinedFunction;
        if (function != nullptr)
        {
          this->InitializeDefinitionInfo(resultOut, function);
          resultOut.DefinedFunction = function;
          resultOut.Name = function->Name;
          return;
        }
      }
      else if (MemberVariableNode* typedNode = Type::DynamicCast<MemberVariableNode*>(node))
      {
        Property* property = typedNode->CreatedProperty;
        if (property != nullptr)
        {
          this->InitializeDefinitionInfo(resultOut, property);
          resultOut.DefinedField = typedNode->CreatedField;
          resultOut.DefinedGetterSetter = typedNode->CreatedGetterSetter;
          resultOut.DefinedMember = property;
          resultOut.Name = resultOut.DefinedMember->Name;
          return;
        }
      }
      else if (SendsEventNode* typedNode = Type::DynamicCast<SendsEventNode*>(node))
      {
        Property* property = typedNode->EventProperty;
        if (property != nullptr)
        {
          this->InitializeDefinitionInfo(resultOut, property);
          resultOut.DefinedMember = property;
          resultOut.Name = resultOut.DefinedMember->Name;
          return;
        }
      }
      else if (LocalVariableNode* typedNode = Type::DynamicCast<LocalVariableNode*>(node))
      {
        Variable* variable = typedNode->CreatedVariable;
        if (variable != nullptr)
        {
          this->InitializeDefinitionInfo(resultOut, variable);
          resultOut.DefinedVariable = variable;
          resultOut.Name = variable->Name;
          return;
        }
      }
      // Usages
      else if (LocalVariableReferenceNode* typedNode = Type::DynamicCast<LocalVariableReferenceNode*>(node))
      {
        Variable* variable = typedNode->AccessedVariable;
        if (variable != nullptr)
        {
          this->InitializeDefinitionInfo(resultOut, variable);
          resultOut.DefinedVariable = variable;
          resultOut.Name = variable->Name;
          return;
        }
      }
      else if (MemberAccessNode* typedNode = Type::DynamicCast<MemberAccessNode*>(node))
      {
        if (typedNode->AccessedMember != nullptr)
        {
          this->InitializeDefinitionInfo(resultOut, typedNode->AccessedMember);
          resultOut.DefinedField = typedNode->AccessedField;
          resultOut.DefinedGetterSetter = typedNode->AccessedGetterSetter;
          resultOut.DefinedFunction = typedNode->AccessedFunction;
          resultOut.DefinedMember = typedNode->AccessedMember;
          resultOut.Name = resultOut.DefinedMember->Name;
          return;
        }

        if (typedNode->OverloadedFunctions != nullptr && typedNode->OverloadedFunctions->Empty() == false)
        {
          Function* function = typedNode->OverloadedFunctions->Front();
          this->InitializeDefinitionInfo(resultOut, function);
          resultOut.DefinedFunction = function;
          resultOut.Name = function->Name;
          return;
        }
      }
      else if (SyntaxType* typedNode = Type::DynamicCast<SyntaxType*>(node))
      {
        Type* resolvedType = typedNode->ResolvedType;
        if (resolvedType != nullptr)
        {
          this->InitializeDefinitionInfo(resultOut, resolvedType);
          return;
        }
      }
      else if (ExpressionNode* typedNode = Type::DynamicCast<ExpressionNode*>(node))
      {
        Type* resultType = typedNode->ResultType;
        if (resultType != nullptr)
        {
          this->InitializeDefinitionInfo(resultOut, resultType);
          return;
        }
      }

      // If we just have a node then use its description
      if (node != nullptr)
      {
        String comments = node->GetMergedComments();
        BoundType* nodeType = ZilchVirtualTypeId(node);
        resultOut.Name = nodeType->Name;
        resultOut.ResolvedType = nodeType;
        resultOut.ElementLocation = node->Location;
        resultOut.NameLocation = node->Location;
        if (!nodeType->Description.Empty())
        {
          resultOut.ToolTip = nodeType->Description;
        }
        else
        {
          String code;
          size_t startPosition = (size_t)-1;
          size_t endPosition = (size_t)-1;
          for (size_t i = 0; i < tokens.Size(); ++i)
          {
            UserToken& token = tokens[i];

            if (token.Location.StartLine == node->Location.StartLine)
            {
              if (startPosition == (size_t)-1)
              {
                startPosition = token.Location.StartPosition;
                code = token.Location.Code;
              }

              endPosition = token.Location.EndPosition;
            }
            else if (token.Location.StartLine > node->Location.StartLine)
            {
              break;
            }
          }

          if (!code.Empty() && startPosition != (size_t)-1 && endPosition != (size_t)-1 && startPosition != endPosition)
          {
            String line = code.SubStringFromByteIndices(startPosition, endPosition);
            resultOut.ToolTip = line;
          }
          else
          {
            static const String Node("Node");
            static const size_t NodeStringByteCount = Node.SizeInBytes();

            if (nodeType->Name.EndsWith(Node))
              resultOut.ToolTip = nodeType->Name.SubStringFromByteIndices(0, nodeType->Name.SizeInBytes() - NodeStringByteCount);
            else
              resultOut.ToolTip = nodeType->Name;
          }
        }

        // If the node has any comments then attach them
        if (!comments.Empty())
        {
          resultOut.ToolTip = BuildString(
            comments,
            "\n",
            resultOut.ToolTip);
        }
        return;
      }
    }
  }

  //***************************************************************************
  void Project::GetAutoCompleteInfo(Module& dependencies, size_t cursorPosition, StringParam cursorOrigin, AutoCompleteInfo& resultOut)
  {
    // First query auto complete type and function information
    this->GetAutoCompleteInfoInternal(dependencies, cursorPosition, cursorOrigin, resultOut);

    // We basically consider the auto complete a success if it found anything that wasn't the error type (and not null)
    resultOut.Success = (resultOut.NearestType != nullptr && resultOut.NearestType != Core::GetInstance().ErrorType);

    // If we're trying to access a bound type...
    Zilch::BoundType* boundType = Zilch::Type::DynamicCast<Zilch::BoundType*>(resultOut.NearestType);

    // If this wasn't a bound type, check first if its an indirect type
    if (boundType == nullptr)
    {
      // In general we can access indirect types exactly like how we access bound types...
      Zilch::IndirectionType* indirectType = Zilch::Type::DynamicCast<Zilch::IndirectionType*>(resultOut.NearestType);

      // Just set the bound type to be the reference type and continue on
      if (indirectType != nullptr)
        boundType = indirectType->ReferencedType;
    }

    // Make sure we have a valid type, but ignore integer literals
    if (boundType != nullptr)
    {
      // Query the dependencies for all extension functions and properties for the bound type
      // These functors also walk up base types and find any of their properties
      AutoCompletePropertyFunctionQuery queryFunctor;
      queryFunctor.Info = &resultOut;
      queryFunctor.CursorOrigin = cursorOrigin;
      ForEachGetterSetter(resultOut.IsStatic, dependencies, boundType, queryFunctor);
      ForEachFunction(resultOut.IsStatic, dependencies, boundType, queryFunctor);
    }

    // If we have no actual function overloads (we still may be performing a call on a delegate or functor of some sort)...
    if (resultOut.FunctionOverloads.Empty())
    {
      // If the expression is a delegate type then generate an overload for the delegate
      // call (we technically won't know descriptions or, in the future, parameter names...)
      DelegateType* delegateType = Type::DynamicCast<DelegateType*>(resultOut.NearestType);
      if (delegateType != nullptr)
        this->AddAutoCompleteOverload(resultOut, delegateType);
      
      // We always set the name to be "delegate" when we don't know where the actual function came from
      resultOut.FunctionName = Grammar::GetKeywordOrSymbol(Grammar::Delegate);
    }
    else
    {
      // Now we handle any real overloads that we might have found (only when accessing a member on a type can we deal with overloads)
      for (size_t i = 0; i < resultOut.FunctionOverloads.Size(); ++i)
      {
        // Grab the current overloaded function
        Function* function = resultOut.FunctionOverloads[i];

        // Create a completion overload for it (filling out return type and parameters/names)
        CompletionOverload& overload = this->AddAutoCompleteOverload(resultOut, function->FunctionType);

        // Set the description (this could be an inherited description)
        overload.Description = function->Description;
      
        // If we haven't gotten a function name yet, get it from this overload
        if (resultOut.FunctionName.Empty())
        {
          // Show a special name for constructors
          if (function->Name == ConstructorName)
            resultOut.FunctionName = Grammar::GetKeywordOrSymbol(Grammar::Constructor);
          else
            resultOut.FunctionName = function->Name;
        }
      
        ErrorIf(function->Name != resultOut.FunctionName && function->Name != ConstructorName,
          "All function names in the overload list should match");
      }
    }

    // If we have completion overloads, then try to find some sort of 'best' overload to show the user, in
    // the event that the IDE does not have good support for an overload list or 'call-tips'
    if (resultOut.CompletionOverloads.Empty() == false)
    {
      // Right now, we're only considering overloads that have descrptions (looking for the one with the most parameters)
      int bestParameterCount = -1;

      // Walk through all the completion overloads and look for the first with a description
      for (size_t i = 0; i < resultOut.CompletionOverloads.Size(); ++i)
      {
        // Grab the current overload for convenience
        CompletionOverload& overload = resultOut.CompletionOverloads[i];

        // We only consider overloads tha have a description, then we look for the one with the most parameters
        int parameterCount = (int)overload.Parameters.Size();
        if (overload.Description.Empty() == false && parameterCount > bestParameterCount)
        {
          // This one is now the 'best', but we need to keep looking
          bestParameterCount = parameterCount;
          resultOut.BestCompletionOverload = (int)i;
        }
      }
      
      // If we still didn't find a good overload with a description...
      if (resultOut.BestCompletionOverload == -1)
      {
        // Just find the one with the most parameters, this should always get at least one
        // because all overloads have 0 or more parameters (and we start at -1)
        bestParameterCount = -1;
        
        // Walk through all the completion overloads and look for the first with a description
        for (size_t i = 0; i < resultOut.CompletionOverloads.Size(); ++i)
        {
          // Grab the current overload for convenience
          CompletionOverload& overload = resultOut.CompletionOverloads[i];

          // We only consider overloads tha have a description, then we look for the one with the most parameters
          int parameterCount = (int)overload.Parameters.Size();
          if (parameterCount > bestParameterCount)
          {
            // This one is now the 'best', but we need to keep looking
            bestParameterCount = parameterCount;
            resultOut.BestCompletionOverload = (int)i;
          }
        }
      }
    }

    // Finally, sort all completions and remove redundant entries
    Sort(resultOut.CompletionEntries.All());

    // If the user wants us to remove duplicate entries...
    if (resultOut.RemoveDuplicateNameEntries)
    {
      // The last entry name that we hit (so we can compare the next one and see if its the same)
      String lastName;

      // Walk through all entries and only keep the first one for any duplicates (should already be sorted with longest descriptions first)
      for (size_t i = 0; i < resultOut.CompletionEntries.Size();)
      {
        // If the current entry has the same name as the last...
        CompletionEntry& entry = resultOut.CompletionEntries[i];
        if (entry.Name == lastName)
        {
          // Note: We don't need to update 'lastName' because we know its the same!
          // Remove this entry from the completions (this will iterate us forward since it moves all entries in front back by one)
          resultOut.CompletionEntries.EraseAt(i);
        }
        else
        {
          // This is a new name, so store it so we can compare next time
          lastName = entry.Name;
          ++i;
        }
      }
    }
  }

  //***************************************************************************
  void Project::GetAutoCompleteInfoInternal(Module& dependencies, size_t cursorPosition, StringParam cursorOrigin, AutoCompleteInfo& resultOut)
  {
    // Temporary set tolerant mode to true (recall it on any exiting of this function)
    Zero::SetAndRecallOnDestruction<bool> changeTolerantMode(&this->TolerantMode, true);

    // Always assume we're parsing an instance/expression
    // Later on if we fail, we'll try to parse a type and therefore it may be a static
    resultOut.IsStatic = false;

    // Compile the entirety of the project and get the syntax tree out of it
    // We MUST store the library or all the resources will be released
    SyntaxTree syntaxTree;
    Array<UserToken> tokens;
    resultOut.IncompleteLibrary = this->Compile(DefaultLibraryName, dependencies, EvaluationMode::Project, syntaxTree, tokens);

    // Get all the syntax nodes under the cursor
    Array<SyntaxNode*> nodes;
    syntaxTree.GetNodesAtCursor(cursorPosition, cursorOrigin, nodes);

    // If we received no nodes, then return nothing (CodeDefinition will remain empty)
    if (nodes.Empty())
      return;

    for (int i = (int)nodes.Size() - 1; i >= 0; --i)
    {
      SyntaxNode* node = nodes[i];

      if (node->IsGenerated)
        continue;

      ExpressionNode* expression = nullptr;

      if (MemberAccessNode* memberAccess = Type::DynamicCast<MemberAccessNode*>(node))
      {
        expression = memberAccess->LeftOperand;
        resultOut.PartialMemberName = memberAccess->Name;
      }
      else if (FunctionCallNode* call = Type::DynamicCast<FunctionCallNode*>(node))
      {
        expression = call->LeftOperand;

        resultOut.CallArgumentIndex = 0;
        // Find the argument that our cursor query is within
        for (size_t j = 0; j < call->Arguments.Size(); ++j)
        {
          if (cursorPosition >= call->Arguments[j]->Location.EndPosition)
            resultOut.CallArgumentIndex = (int)j + 1;
        }
      }
      else
      {
        continue;
      }

      // We could have a Call node that exists on an Attribute
      // with no LeftOperand, therefore expression can be null
      if (expression == nullptr)
        continue;

      // The result type may be null if it was unable to resolve... return whatever we found
      resultOut.NearestType = expression->ResultType;

      // If the value we're accessing is a value node specifically, then it's a literal
      // (note that LocalVariableReferenceNode inherits from ValueNode and is NOT a literal, hence the ZilchGetDerivedType == check).
      resultOut.IsLiteral = (expression->ZilchGetDerivedType() == ZilchTypeId(ValueNode));

      // Look to see if we're accessing a single function or a bunch of overloads
      const FunctionArray* overloads = nullptr;
      Function* singleFunction = nullptr;

      // If the expression is a member access...
      if (MemberAccessNode* memberAccess = Type::DynamicCast<MemberAccessNode*>(expression))
      {
        overloads = memberAccess->OverloadedFunctions;
        singleFunction = memberAccess->AccessedFunction;
      }
      // If this is a creation call, we also want to pull out constructor overloads
      else if (StaticTypeNode* staticType = Type::DynamicCast<StaticTypeNode*>(expression))
      {
        resultOut.IsStatic = true;
        overloads = staticType->OverloadedConstructors;
        singleFunction = staticType->ConstructorFunction;
      }

      // If we resolved to an overload group... (but did not pick one yet)
      if (overloads != nullptr)
      {
        // Also let the user know what the overloads are
        resultOut.FunctionOverloads = *overloads;
      }
      // If we just resolved a single function...
      else if (singleFunction != nullptr)
      {
        // Add the single function (so that the user can get more documentation from it)
        resultOut.FunctionOverloads.PushBack(singleFunction);
      }
      break;
    }
  }
}
