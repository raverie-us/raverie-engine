/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

// Defines
#define ZilchSaveAndVerifyTokenPosition()                   \
  TokenPositionVerifier __verifier(&this->TokenPositions);  \
  this->SaveTokenPosition();

namespace Zilch
{
  //***************************************************************************
  // Structure that helps us to verify that our parser is working properly
  class TokenPositionVerifier
  {
  public:
    PodArray<size_t>* Positions;
    size_t Count;

    TokenPositionVerifier(PodArray<size_t>* positions) :
      Positions(positions),
      Count(positions->Size())
    {
    }

    ~TokenPositionVerifier()
    {
      ErrorIf(this->Count != this->Positions->Size(),
        "Token count was not equal when we left the stack as when we started");
    }
  };

  //***************************************************************************
  Parser::Parser(Project& project) :
    TokenIndex(0),
    Errors(*(CompilationErrors*)&project),
    ParentProject(&project)
  {
    ZilchErrorIfNotStarted(Parser);
  }
  
  //***************************************************************************
  void Parser::ParseIntoTree(const Array<UserToken>& tokens, SyntaxTree& syntaxTree, EvaluationMode::Enum evaluation)
  {
    // If we have no tokens, don't do anything
    if (IsTokenStreamEmpty(tokens))
      return;

    // Clear all token positions, just in case we reuse this parser
    this->TokenIndex = 0;
    this->TokenPositions.Clear();

    // Store the tokenizer
    this->TokenStream = &tokens;

    // If we're evaluating an entire project, parse all the classes
    if (evaluation == EvaluationMode::Project)
    {
      // Get the root node for convenience
      RootNode* root = syntaxTree.Root;
      
      // Specifies if we parsed anything inside the script
      bool parsedSomething;
      
      // Parse the things that can show up inside a script (the outer most scope)
      do
      {
        // We haven't parsed anything yet this iteration...
        parsedSomething = false;

        // Attempt to parse a class definition
        parsedSomething |= root->NonTraversedNonOwnedNodesInOrder.Add(root->Classes.Add(this->Class())) != nullptr;

        // Attempt to parse an enum definition
        parsedSomething |= root->NonTraversedNonOwnedNodesInOrder.Add(root->Enums.Add(this->Enum())) != nullptr;
      }
      while (parsedSomething == true);
    }
    // Otherwise, we're just evaluating a single expression
    else
    {
      // Get the location where the expression is occuring
      CodeLocation location = tokens.Front().Location;

      // We create a fake class node to hold the expression, since all expressions
      // must be inside functions, and all functions must be inside classes
      ClassNode* classNode = new ClassNode();
      classNode->Location = location;

      // All static classes are by choice a reference class (even if it cannot be allocated0
      classNode->CopyMode = TypeCopyMode::ReferenceType;

      // The class has a special generated name that cannot be accessed from within code
      classNode->Name.Location = location;
      classNode->Name.Token = ExpressionProgram;

      // Now we need to generate a special function to put the expression within
      FunctionNode* functionNode = new FunctionNode();
      functionNode->Location = location;

      // The function will have a special name that cannot be accessed from within code
      functionNode->Name.Location = location;
      functionNode->Name.Token = ExpressionMain;

      // Let the return type be an any type...
      functionNode->ReturnType = new AnySyntaxType();

      // The function is going to be static so that it can be invoked without a context
      functionNode->IsStatic = true;

      // Parse just one expression from the tokens (this may fail and return null!)
      syntaxTree.SingleExpressionScope = functionNode;
      syntaxTree.SingleExpressionIndex = functionNode->Statements.Size();
      StatementNode* singleStatement = this->Statement(true);

      // If the statement is an expression, then we want to add an automatic return
      ExpressionNode* singleExpression = Type::DynamicCast<ExpressionNode*>(singleStatement);
      if (singleExpression != nullptr)
      {
        // Create the return statement to return the expression
        ReturnNode* returnNode = new ReturnNode();
        returnNode->ReturnValue = singleExpression;
        returnNode->Location = location;

        // Our statement is now just the parent return node
        singleStatement = returnNode;
      }

      // Add the expression as a single statement to the function
      functionNode->Statements.Add(singleStatement);

      // If this was not an expression, then always Insert an implicit return at the end
      // Technically we could have issues if all code paths of their statement returned (but we're going to remove that error)
      if (singleExpression == nullptr)
      {
        // Always Insert an implicit 'return null' at the end
        ValueNode* nullNode = new ValueNode();
        nullNode->Value = UserToken(Grammar::Null, &location);
        nullNode->Location = location;
        ReturnNode* returnNode = new ReturnNode();
        returnNode->ReturnValue = nullNode;
        returnNode->Location = location;
        functionNode->Statements.Add(returnNode);
      }

      // Add the psuedo function to the class
      classNode->Functions.Add(functionNode);
      classNode->NonTraversedNonOwnedNodesInOrder.Add(functionNode);

      // Finally, attach the class (and therefore function / expression) to the root
      syntaxTree.Root->Classes.Add(classNode);
      syntaxTree.Root->NonTraversedNonOwnedNodesInOrder.Add(classNode);
    }

    // If we somehow parsed everything, but didn't get to the end, we should throw an error
    if (this->TokenIndex != this->TokenStream->Size() - 1)
    {
      // Grab the last token we hit
      UserToken token = (*this->TokenStream)[this->TokenIndex];

      // Show an error message that prints out the token we hit
      return this->Errors.Raise(token.Location, ErrorCode::ParsingNotComplete, token.Token.c_str(), Grammar::GetName(token.TokenId).c_str());
    }

    // If we parsed everything but there were attributes that never got attached to anything...
    if (this->LastAttributes.Empty() == false)
    {
      // Show an error message to tell the user that attributes never got attached
      this->Errors.Raise(this->LastAttributes.Back()->Location, ErrorCode::AttributesNotAttached);

      // Delete the leftover attributes and clear the last attribute list
      ZilchForEach(AttributeNode* node, this->LastAttributes)
        delete node;
      this->LastAttributes.Clear();
      return;
    }
  }

  //***************************************************************************
  bool Parser::IsTokenStreamEmpty(const Array<UserToken>& tokens)
  {
    // If it's strictly empty, just return
    if (tokens.Empty())
      return true;

    // Otherwise, check if the size is one and the front element is end
    if (tokens.Size() == 1 && tokens.Front().TokenId == Grammar::End)
      return true;

    // If we got here, it must not be empty
    return false;
  }

  //***************************************************************************
  void Parser::ParseExpressionInFunctionAndClass(const Array<UserToken>& expression, const Array<UserToken>& function, const Array<UserToken>& classTokensWithoutFunction, SyntaxTree& syntaxTree)
  {
    // If we have no expression tokens, don't do anything
    if (IsTokenStreamEmpty(expression))
      return;

    // Get the location where the expression is occuring
    CodeLocation location = expression.Front().Location;

    // The resulting class node (either partially parsed or generaed)
    ClassNode* classNode = nullptr;

    // If we have the ability to parse a class...
    if (IsTokenStreamEmpty(function) == false)
    {
      // Clear all token positions and set the token stream to the class tokens
      this->TokenIndex = 0;
      this->TokenPositions.Clear();
      this->TokenStream = &classTokensWithoutFunction;

      // Attempt to parse the class (remember, this class is guaranteed to not have the function within it)
      classNode = this->Class();
    }

    // If we either didn't have the ability to parse a class, or we failed on every attempt...
    if (classNode == nullptr)
    {
      // We create a fake class node to hold the expression, since all expressions
      // must be inside functions, and all functions must be inside classes
      classNode = new ClassNode();
      classNode->Location = location;

      // All static classes are by choice a reference class (even if it cannot be allocated0
      classNode->CopyMode = TypeCopyMode::ReferenceType;

      // The class has a special generated name that cannot be accessed from within code
      classNode->Name.Location = location;
      classNode->Name.Token = ExpressionProgram;
    }

    // Parse the given tokens as a function
    GenericFunctionNode* functionNode = nullptr;

    // If we have the ability to parse a function...
    if (IsTokenStreamEmpty(function) == false)
    {
      // To figure out what to do, just read the first token
      const UserToken& firstToken = function.Front();

      // Clear all token positions and set the token stream to the function tokens
      this->TokenIndex = 0;
      this->TokenPositions.Clear();
      this->TokenStream = &function;

      // Is this a function?
      {
        FunctionNode* node = this->Function();
        if (node != nullptr)
        {
          functionNode = node;

          // Add the psuedo function to the class
          classNode->Functions.Add(node);
          classNode->NonTraversedNonOwnedNodesInOrder.Add(node);
        }
      }
      
      // Is this a constructor?
      {
        ConstructorNode* node = this->Constructor();
        if (node != nullptr)
        {
          functionNode = node;

          // Add the psuedo function to the class
          classNode->Constructors.Add(node);
          classNode->NonTraversedNonOwnedNodesInOrder.Add(node);
        }
      }

      // Is this a destructor?
      {
        DestructorNode* node = this->Destructor();
        if (node != nullptr)
        {
          functionNode = node;

          // Add the psuedo function to the class
          classNode->Destructor = node;
          classNode->NonTraversedNonOwnedNodesInOrder.Add(node);
        }
      }

      switch (firstToken.TokenId)
      {
        case Grammar::Get:
        {
          // We're supposed to have read the get/set token, so advance by 1
          ++this->TokenIndex;
          BoundSyntaxType* type = new BoundSyntaxType();
          type->TypeName = "Integer";
          MemberVariableNode* memberVariable = new MemberVariableNode();
          memberVariable->IsGetterSetter = true;
          memberVariable->Name.Token = "[Generated]";
          memberVariable->ResultSyntaxType = type;
          FunctionNode* node = this->GetSetFunctionBody(memberVariable, true);
          if (node != nullptr)
          {
            functionNode = node;
            memberVariable->Get = node;

            // Add the psuedo function to the class via member variable
            classNode->Variables.Add(memberVariable);
            classNode->NonTraversedNonOwnedNodesInOrder.Add(memberVariable);
          }
          break;
        }
        
        case Grammar::Set:
        {
          // We're supposed to have read the get/set token, so advance by 1
          ++this->TokenIndex;
          BoundSyntaxType* type = new BoundSyntaxType();
          type->TypeName = "Integer";
          MemberVariableNode* memberVariable = new MemberVariableNode();
          memberVariable->IsGetterSetter = true;
          memberVariable->Name.Token = "[Generated]";
          memberVariable->ResultSyntaxType = type;
          FunctionNode* node = this->GetSetFunctionBody(memberVariable, false);
          if (node != nullptr)
          {
            functionNode = node;
            memberVariable->Set = node;

            // Add the psuedo function to the class via member variable
            classNode->Variables.Add(memberVariable);
            classNode->NonTraversedNonOwnedNodesInOrder.Add(memberVariable);
          }
          break;
        }
      }
    }

    // If we either didn't have the ability to parse a function, or we failed on every attempt...
    if (functionNode == nullptr)
    {
      // Now we need to generate a special function to put the expression within
      FunctionNode* node = new FunctionNode();
      functionNode = node;
      node->Location = location;

      // The function will have a special name that cannot be accessed from within code
      node->Name.Location = location;
      node->Name.Token = ExpressionMain;

      // The function is going to be static so that it can be invoked without a context
      node->IsStatic = true;

      // Add the psuedo function to the class
      classNode->Functions.Add(node);
      classNode->NonTraversedNonOwnedNodesInOrder.Add(node);
    }

    // Again, clear all token positions and set the token stream to the expression tokens
    this->TokenIndex = 0;
    this->TokenPositions.Clear();
    this->TokenStream = &expression;

    // Parse just one expression from the tokens (this may fail and return null!)
    ExpressionNode* singleExpression = this->Expression();

    // As long as we got a valid single expression...
    if (singleExpression != nullptr)
    {
      // Find the scope that the expression probably exists within
      // (the latest scope before the expression's location)
      // Even if this cannot find a scope, it should always return the function node itself
      ScopeNode* latestScope = FindNearestScope(functionNode, singleExpression->Location);
      
      // We really don't want to crash doing auto-complete, but this is a serious error
      if (latestScope == nullptr)
      {
        // For now, just attach the expression to the class node
        // This will make sure everything gets deleted as expected, and we'll attempt to do some sort of auto-complete
        // Warning: This is not at all correct!
        syntaxTree.SingleExpressionScope = classNode;
        syntaxTree.SingleExpressionIndex = classNode->Statements.Size();
        classNode->Statements.Add(singleExpression);
      }
      else
      {
        // Add the expression as a single statement to the latest scope (at the end of it)
        // so it will be evaluated in the context of that scope
        // Note that the found scope could just be the function itself
        syntaxTree.SingleExpressionScope = latestScope;
        syntaxTree.SingleExpressionIndex = latestScope->Statements.Size();
        latestScope->Statements.Add(singleExpression);
      }
    }

    // Finally, attach the class (and therefore function / expression) to the root
    syntaxTree.Root->Classes.Add(classNode);
    syntaxTree.Root->NonTraversedNonOwnedNodesInOrder.Add(classNode);
  }

  //***************************************************************************
  ScopeNode* Parser::FindNearestScope(SyntaxNode* node, const CodeLocation& location)
  {
    // If this node that we're testing happens to come before the location...
    // Technically we should be testing if the location's end is within the node's end,
    // however it's slightly more tolerant to just check if the location's start is before the end
    // This should never happen, but we'll handle it anyways
    bool isLocationInsideNode =
      location.StartLine >= node->Location.StartLine &&
      (location.StartLine != node->Location.StartLine || location.StartCharacter >= node->Location.StartCharacter) &&
      location.StartLine <= node->Location.EndLine &&
      (location.StartLine != node->Location.EndLine || location.StartCharacter <= node->Location.EndCharacter);

    // We only want to consider nodes that we're inside of
    if (isLocationInsideNode == false)
    {
      return nullptr;
    }

    // We know that child nodes always come after our own node in a script
    // Therefore, we test ourself first, and then test children (to get the latest scope)
    ScopeNode* lastScope = Type::DynamicCast<ScopeNode*>(node);

    // Get all the children of this current node, as they could also be scopes
    NodeChildren children;
    node->PopulateChildren(children);

    // Loop through all the child nodes, some of which could be scopes
    for (size_t i = 0; i < children.Size(); ++i)
    {
      // Grab the current child
      SyntaxNode* child = (*children[i]);

      // Recursively find any scopes in that node
      ScopeNode* found = FindNearestScope(child, location);

      // If we found anything....
      if (found != nullptr)
      {
        // We know here that the found scope must come after any previously found scopes, so replace it!
        lastScope = found;
      }
    }

    // Return whatever we found (this could be null!)
    return lastScope;
  }

  //***************************************************************************
  SyntaxType* Parser::ParseType(const Array<UserToken>& type)
  {
    // Clear all token positions and set the token stream to the type tokens
    this->TokenIndex = 0;
    this->TokenPositions.Clear();
    this->TokenStream = &type;

    // Now attempt to read the type and see what we find
    return this->ReadTypeInfo();
  }

  //***************************************************************************
  void Parser::ErrorHere(ErrorCode::Enum errorCode, ...)
  {
    // Start a variadic argument list
    va_list argList;
    va_start(argList, errorCode);

    // Call the other error function
    ErrorHereArgs(errorCode, argList);

    // End the argument list
    va_end(argList);
  }

  //***************************************************************************
  void Parser::ErrorHereArgs(ErrorCode::Enum errorCode, StringParam extra, va_list argList)
  {
    // When raising an error 'here', we don't actually want the current token,
    // but rather the last token since our token index always points at the next token
    int previousIndex = (int)this->TokenIndex;
    --previousIndex;

    // Make sure we weren't at the first token, and if so cap our index at 0
    if (previousIndex < 0)
    {
      previousIndex = 0;
    }

    // Get the previous token
    const UserToken& previousToken = (*this->TokenStream)[(size_t)previousIndex];
    const UserToken& nextToken = (*this->TokenStream)[this->TokenIndex];

    // If we have a difference in lines...
    if (nextToken.Location.PrimaryLine != previousToken.Location.PrimaryLine)
    {
      // Invoke the error as if it was on the previous one
      this->Errors.RaiseArgs(previousToken.Location, extra, LocationArray(), errorCode, argList);
    }
    else
    {
      // Since it's on the same line, show the error at the next token
      this->Errors.RaiseArgs(nextToken.Location, extra, LocationArray(), errorCode, argList);
    }
  }

  //***************************************************************************
  void Parser::ErrorHereArgs(ErrorCode::Enum errorCode, va_list argList)
  {
    return ErrorHereArgs(errorCode, String(), argList);
  }
  
  //***************************************************************************
  void Parser::SetNodeLocationStartHere(SyntaxNode* node)
  {
    // Get the token where the last saved position was
    SetNodeLocationStartToToken(node, (*this->TokenStream)[this->TokenIndex]);
  }

  //***************************************************************************
  void Parser::SetNodeLocationPrimaryHere(SyntaxNode* node)
  {
    // Get the token where the last saved position was
    SetNodeLocationPrimaryToToken(node, (*this->TokenStream)[this->TokenIndex]);
  }

  //***************************************************************************
  void Parser::SetNodeLocationEndHere(SyntaxNode* node)
  {
    // When setting the end, we don't actually want the current token, but rather the last token
    // since our token index always points at the next token (which would be correct if we were starting)
    int previousIndex = (int)this->TokenIndex;
    --previousIndex;

    // Make sure we weren't at the first token, and if so cap our index at 0
    if (previousIndex < 0)
    {
      previousIndex = 0;
    }

    // Get the token where the last saved position was
    SetNodeLocationEndToToken(node, (*this->TokenStream)[(size_t)previousIndex]);
  }

  //***************************************************************************
  void Parser::SetNodeLocationStartToLastSave(SyntaxNode* node)
  {
    // Get the token where the last saved position was
    SetNodeLocationStartToToken(node, (*this->TokenStream)[this->TokenPositions.Back()]);
  }

  //***************************************************************************
  void Parser::SetNodeLocationPrimaryToLastSave(SyntaxNode* node)
  {
    // Get the token where the last saved position was
    SetNodeLocationPrimaryToToken(node, (*this->TokenStream)[this->TokenPositions.Back()]);
  }

  //***************************************************************************
  void Parser::SetNodeLocationEndToLastSave(SyntaxNode* node)
  {
    // Get the token where the last saved position was
    SetNodeLocationEndToToken(node, (*this->TokenStream)[this->TokenPositions.Back()]);
  }

  //***************************************************************************
  void Parser::SetNodeLocationStartToToken(SyntaxNode* node, const UserToken& token)
  {
    // Set the line and character
    // Note that here we set the entire token, not just the start
    node->Location = token.Location;
  }

  //***************************************************************************
  void Parser::SetNodeLocationPrimaryToToken(SyntaxNode* node, const UserToken& token)
  {
    // Set the line and character
    node->Location.PrimaryCharacter = token.Location.PrimaryCharacter;
    node->Location.PrimaryLine = token.Location.PrimaryLine;
    node->Location.PrimaryPosition = token.Location.PrimaryPosition;
  }

  //***************************************************************************
  void Parser::SetNodeLocationEndToToken(SyntaxNode* node, const UserToken& token)
  {
    // Set the line and character
    node->Location.EndCharacter = token.Location.EndCharacter;
    node->Location.EndLine = token.Location.EndLine;
    node->Location.EndPosition = token.Location.EndPosition;
  }

  //***************************************************************************
  void Parser::SaveTokenPosition()
  {
    // Push the current token index onto the stack
    this->TokenPositions.PushBack(this->TokenIndex);
  }

  //***************************************************************************
  void Parser::RecallTokenPosition()
  {
    // Pop the token index and revert the current one
    this->TokenIndex = this->TokenPositions.Back();
    this->TokenPositions.PopBack();
  }

  //***************************************************************************
  void Parser::AcceptTokenPosition()
  {
    // Simply just pop the token position from the stack
    this->TokenPositions.PopBack();
  }

  //***************************************************************************
  bool Parser::AcceptAnyArgs(size_t parameters, const UserToken** out_token, va_list vl)
  {
    // Loop through all the extra arguments
    for (size_t i = 0; i < parameters; ++i)
    {
      // Retreive the symbol argument
      // We use int here because technically an enum can have any size (1, 2, etc)
      // but the va-args list will auto-promote the enum to int
      Grammar::Enum symbol = (Grammar::Enum)va_arg(vl, int /*Grammar::Enum*/);

      // Retrieve the current token
      const UserToken* token = &(*this->TokenStream)[this->TokenIndex];

      // If the next token in the token stream was that same symbol...
      if (token->TokenId == symbol)
      {
        // We found one of them! Close the args list, increment the token index, output the value, and return success
        ++this->TokenIndex;
        *out_token = token;
        return true;
      }
    }

    // Otherwise, if we got here we didn't find it :<
    return false;
  }

  //***************************************************************************
  bool Parser::AcceptAny(size_t parameters, const UserToken** out_token, ...)
  {
    // Create a variadic argument list to read the extra arguments
    va_list vl;
    va_start(vl, out_token);

    // Run the accept any that takes a va_list
    bool result = AcceptAnyArgs(parameters, out_token, vl);

    // Close the va_list and return the result
    va_end(vl);
    return result;
  }

  //***************************************************************************
  bool Parser::Expect(Grammar::Enum grammarConstant, ErrorCode::Enum errorCode, ...)
  {
    // Create a variadic argument list to read the extra arguments
    va_list vl;
    va_start(vl, errorCode);
    
    // Invoke the argument list version
    const UserToken* ignoredToken = nullptr;
    bool result = this->ExpectAndRetrieveArgs(grammarConstant, ignoredToken, errorCode, vl);
      
    // Finalize the argument list and return what we got back
    va_end(vl);
    return result;
  }

  //***************************************************************************
  bool Parser::ExpectAndRetrieveArgs(Grammar::Enum grammarConstant, const UserToken*& outToken, ErrorCode::Enum errorCode, va_list vl)
  {
    // Retrieve the next token from the stream and store it
    const UserToken* userToken = &(*this->TokenStream)[this->TokenIndex];

    // If the next token in the token stream was not the same symbol...
    if (userToken->TokenId != grammarConstant)
    {
      // Anything extra we decide to Append to the error
      String extra;

      bool foundWasKeyword = (userToken->TokenId >= Grammar::Abstract && userToken->TokenId <= Grammar::While);

      if (grammarConstant == Grammar::LowerIdentifier || grammarConstant == Grammar::UpperIdentifier)
      {
        if (foundWasKeyword)
        {
          extra = String::Format(" Note: '%s' is a keyword and cannot be used as a name.",
            userToken->Token.c_str());
        }
        // If we expected an upper identifier, but we got a lower one...
        else if (userToken->TokenId == Grammar::LowerIdentifier)
        {
          String expectedIdentifier = ToUpperCamelCase(userToken->Token);
          extra = String::Format(" Upper-camel case names are required here (use '%s' instead of '%s').",
            expectedIdentifier.c_str(), userToken->Token.c_str());
        }
        // If we expected a lower identifier, but we got an upper one...
        else if (userToken->TokenId == Grammar::UpperIdentifier)
        {
          String expectedIdentifier = ToLowerCamelCase(userToken->Token);
          extra = String::Format(" Lower-camel case names are required here (use '%s' instead of '%s').",
            expectedIdentifier.c_str(), userToken->Token.c_str());
        }
      }
      else
      {
        // Get string representations of the two tokens so we can display info to the user
        String foundSymbol    = Grammar::GetKeywordOrSymbol(userToken->TokenId);
        String expectedSymbol = Grammar::GetKeywordOrSymbol(grammarConstant);

        // Because we are expecting a particular token, format extra information so
        // that the error always says what we found and what we expected to find
        extra = String::Format(" We found '%s' but we expected to find '%s'.",
          foundSymbol.c_str(), expectedSymbol.c_str());
      }

      // Report an error here
      ErrorHereArgs(errorCode, extra, vl);

      // We found a token that wasn't what we were looking for, return failure
      va_end(vl);
      return false;
    }

    // Otherwise, if the out-token is not null, output the found token
    outToken = userToken;

    // We found all the given tokens! Push the token index out to the end of the list and return success
    ++this->TokenIndex;
    return true;
  }

  //***************************************************************************
  bool Parser::ExpectAndRetrieve(Grammar::Enum grammarConstant, const UserToken*& outToken, ErrorCode::Enum errorCode, ...)
  {
    // Create a variadic argument list to read the extra arguments
    va_list vl;
    va_start(vl, errorCode);

    // Invoke the argument list version
    bool result = this->ExpectAndRetrieveArgs(grammarConstant, outToken, errorCode, vl);

    // Finalize the argument list and return what we got back
    va_end(vl);
    return result;
  }

  //***************************************************************************
  bool Parser::Accept(size_t parameters, ...)
  {
    // Create a variadic argument list to read the extra arguments
    va_list vl;
    va_start(vl, parameters);

    // Compute the end token that we'll stop at
    const size_t end = parameters + this->TokenIndex;

    // Loop through all the tokens we were given (and loop over the tokens in the tokenizer)
    for (size_t i = this->TokenIndex; i < end; ++i)
    {
      // Retreive the symbol argument
      // We use int here because technically an enum can have any size (1, 2, etc)
      // but the va-args list will auto-promote the enum to int
      Grammar::Enum symbol = (Grammar::Enum)va_arg(vl, int /*Grammar::Enum*/);

      // If the next token in the token stream was not the same symbol...
      if ((*this->TokenStream)[i].TokenId != symbol)
      {
        // We found a token that wasn't what we were looking for, return failure
        va_end(vl);
        return false;
      }
    }

    // We found all the given tokens! Push the token index out to the end of the list and return success
    va_end(vl);
    this->TokenIndex = end;
    return true;
  }

  //***************************************************************************
  bool Parser::AcceptAndRetrieve(size_t parameters, ...)
  {
    // Create a variadic argument list to read the extra arguments
    va_list vl;
    va_start(vl, parameters);

    // Compute the end token that we'll stop at
    const size_t end = parameters + this->TokenIndex;

    // Loop through all the tokens we were given (and loop over the tokens in the tokenizer)
    for (size_t i = this->TokenIndex; i < end; ++i)
    {
      // Retreive the symbol argument and a pointer to the out token pointer
      // We use int here because technically an enum can have any size (1, 2, etc)
      // but the va-args list will auto-promote the enum to int
      Grammar::Enum symbol = (Grammar::Enum)va_arg(vl, int /*Grammar::Enum*/);

      // A pointer to a constant user token
      const UserToken** out_token = va_arg(vl, const UserToken**);

      // Retrieve the next token from the stream and store it
      const UserToken* user_token = &(*this->TokenStream)[i];

      // If the next token in the token stream was not the same symbol...
      if (user_token->TokenId != symbol)
      {
        // We found a token that wasn't what we were looking for, return failure
        va_end(vl);
        return false;
      }

      // Otherwise, if the out-token is not null, output the found token
      if (out_token != nullptr)        
        *out_token = user_token;
    }

    // We found all the given tokens! Push the token index out to the end of the list and return success
    va_end(vl);
    this->TokenIndex = end;
    return true;
  }

  //***************************************************************************
  bool Parser::ReadDelegateTypeContents(DelegateSyntaxType* delegateSyntaxType)
  {
    //// Is this a template type?
    //if (Accept(1, Grammar::BeginTemplate))
    //{
    //  // Parse arguments to the indexer until there are no more
    //  ZilchLoop
    //  {
    //    // Attempt to read another type
    //    SyntaxType* argumentType = this->ReadTypeInfo();

    //    // If the argument was valid...
    //    if (argumentType == nullptr)
    //    {
    //      // The argument was not found!
    //      ErrorHere(ErrorCode::TemplateArgumentNotFound);
    //    }
    //    else
    //    {
    //      // Add it to the arguments list
    //      dataSyntaxType->TemplateArguments.Add(argumentType);
    //    }

    //    // Attempt to read an argument separator, and if we don't find one, break out
    //    if (Accept(1, Grammar::ArgumentSeparator) == false)
    //    {
    //      break;
    //    }
    //  }

    //  // We now expect to close the template argument
    //  if (Expect(Grammar::EndTemplate, ErrorCode::TemplateTypeArgumentsNotComplete) == false)
    //  {
    //    // We didn't successfully parse a the type definition, so return a failure
    //    delete dataSyntaxType;
    //    RecallTokenPosition();
    //    return nullptr;
    //  }
    //}

    // Get the name of the function
    if (Expect(Grammar::BeginFunctionParameters, ErrorCode::FunctionArgumentListNotFound, "delegate"))
    {
      // Get all the parameters for the delegate
      do
      {
        // Store a temporary copy of the current parameter that we're reading in
        DelegateSyntaxParameter parameter;

        // Look for an identifier
        if (AcceptAndRetrieve(1, Grammar::LowerIdentifier, &parameter.Name))
        {
          // Look for a type specifier (followed by the parameter's type)
          if (Expect(Grammar::TypeSpecifier, ErrorCode::ParameterTypeSpecifierNotFound, parameter.Name->Token.c_str()))
          {
            // Attempt to read the type after the parameter
            if ((parameter.Type = this->ReadTypeInfo()) != nullptr)
            {
              // Add this parameter to the list of parameters
              delegateSyntaxType->Parameters.PushBack(parameter);
            }
            else
            {
              // Show an error
              ErrorHere(ErrorCode::ParameterTypeNotFound, parameter.Name->Token.c_str());
              return false;
            }
          }
          else
          {
            // We didn't read the type specifier
            return false;
          }
        }
      }
      while (Accept(1, Grammar::ArgumentSeparator));

      // Look for the end parenthasis
      if (Expect(Grammar::EndFunctionParameters, ErrorCode::FunctionArgumentListNotComplete, "delegate"))
      {
        // Return if we successfully parsed the type at the end or not
        return AcceptOptionalTypeSpecifier(delegateSyntaxType->Return, ErrorCode::DelegateReturnTypeNotFound);
      }
    }

    // If we got down here, we must have failed...
    return false;
  }

  //***************************************************************************
  BoundSyntaxType* Parser::ReadBoundTypeInfo()
  {
    ZilchSaveAndVerifyTokenPosition();

    // Grab the user token for the identifier
    const UserToken* typeToken;

    // Look for the type identifier
    if (this->AcceptAndRetrieve(1, Grammar::UpperIdentifier, &typeToken) == true)
    {
      // Create a new data syntax type
      BoundSyntaxType* boundSyntaxType = new BoundSyntaxType();
      this->SetNodeLocationStartToLastSave(boundSyntaxType);

      // Store away the type string
      boundSyntaxType->TypeName = typeToken->Token;

      // Is this a template type?
      if (this->Accept(1, Grammar::BeginTemplate))
      {
        // Parse arguments to the template there are no more
        ZilchLoop
        {
          // Attempt to read another type
          SyntaxNode* argument = this->ReadTypeInfo();

          // If the argument was valid...
          if (argument == nullptr)
          {
            // Parse an expression (right now we only support value types)
            argument = this->Expression();
            if (argument == nullptr)
            {
              // The argument was not found!
              this->ErrorHere(ErrorCode::TemplateArgumentNotFound);

              // We didn't successfully parse a the type definition, so return a failure
              delete boundSyntaxType;
              this->RecallTokenPosition();
              return nullptr;
            }
          }

          // Add it to the arguments list
          boundSyntaxType->TemplateArguments.Add(argument);

          // Attempt to read an argument separator, and if we don't find one, break out
          if (this->Accept(1, Grammar::ArgumentSeparator) == false)
          {
            break;
          }
        }

        // We now expect to close the template argument
        if (this->Expect(Grammar::EndTemplate, ErrorCode::TemplateTypeArgumentsNotComplete) == false)
        {
          // We didn't successfully parse a the type definition, so return a failure
          delete boundSyntaxType;
          this->RecallTokenPosition();
          return nullptr;
        }
      }

      // Accept the token position and return the data type
      this->AcceptTokenPosition();

      // We'll return the data syntax type
      return boundSyntaxType;
    }

    // Return nothing since we failed to parse
    this->RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  SyntaxType* Parser::ReadTypeInfo()
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // If this type represents the any type...
    if (this->Accept(1, Grammar::Any))
    {
      // There's nothing else to read about the any type, just return a new one
      this->AcceptTokenPosition();
      SyntaxType* anySyntaxType = new AnySyntaxType();
      this->SetNodeLocationStartToLastSave(anySyntaxType);
      return anySyntaxType;
    }
    // If this type represents a delegate type...
    else if (this->Accept(1, Grammar::Delegate))
    {
      // Create a new delegate syntax type
      DelegateSyntaxType* delegateSyntaxType = new DelegateSyntaxType();

      // Read the delegate type
      if (this->ReadDelegateTypeContents(delegateSyntaxType))
      {
        // Accept the token position and return the delegate type
        this->AcceptTokenPosition();
        this->SetNodeLocationStartToLastSave(delegateSyntaxType);

        // We'll return the delegate syntax type
        return delegateSyntaxType;
      }
      else
      {
        // We didn't successfully parse a the deligate definition, so return a failure
        delete delegateSyntaxType;
        this->RecallTokenPosition();
        return nullptr;
      }
    }
    else
    {
      // Store whether or not this type is a reference (uses the ref keyword)
      bool isIndirectionRef = false;
      if (Accept(1, Grammar::Ref))
      {
        // This is an indirection
        isIndirectionRef = true;
      }

      // Attempt to read a named type...
      BoundSyntaxType* boundSyntaxType = this->ReadBoundTypeInfo();

      // If we failed to read the named syntax type, return nothing
      if (boundSyntaxType == nullptr)
      {
        this->RecallTokenPosition();
        return nullptr;
      }

      // If this is an indirection of a type (ref)
      if (isIndirectionRef)
      {
        // Create a qualified syntax type
        IndirectionSyntaxType* indirectionSyntaxType = new IndirectionSyntaxType();
        this->SetNodeLocationStartToLastSave(indirectionSyntaxType);

        // Set the true type for the qualified type...
        indirectionSyntaxType->ReferencedType = boundSyntaxType;

        // Return the qualified syntax type
        this->AcceptTokenPosition();
        return indirectionSyntaxType;
      }
      else
      {
        // Return the true syntax type that we parsed
        this->AcceptTokenPosition();
        return boundSyntaxType;
      }
    }
  }

  //***************************************************************************
  LocalVariableNode* Parser::LocalVariable(bool initialized)
  {
    // Parse any optional attribute
    this->ParseAllOptionalAttributes();

    // If any errors occurred in parsing attributes, error here
    if (this->Errors.WasError)
      return nullptr;

    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // First look for the "var" keyword
    if (Accept(1, Grammar::Variable))
    {
      // Create a variable node
      LocalVariableNode* node = new LocalVariableNode();
      this->SetNodeLocationStartToLastSave(node);

      // Attach any attributes we parsed for this member variable
      this->AttachLastAttributeToNode(node->Attributes);
      
      // If this is a static variable...
      if (GetAttribute(node->Attributes, StaticAttribute) != nullptr)
          node->IsStatic = true;

      // Get the name of the variable
      const UserToken* variableName = nullptr;
      if (ExpectAndRetrieve(Grammar::LowerIdentifier, variableName, ErrorCode::VariableNameNotFound))
      {
        // Store the name of the node
        node->Name = *variableName;
        this->SetNodeLocationPrimaryToToken(node, *variableName);

        // Return if we successfully parsed the type at the end or not
        if (AcceptOptionalTypeSpecifier(node->ResultSyntaxType, ErrorCode::VariableTypeNotFound, node->Name.c_str()))
        {
          // If we don't need to be initialized, skip this next part
          if (initialized == false)
          {
            // Accept the token position, and return the variable node
            this->SetNodeLocationEndHere(node);
            this->AcceptTokenPosition();
            return node;
          }

          // Now make sure we are assigning to the variable
          if (Expect(Grammar::Assignment, ErrorCode::VariableMustBeInitialized, node->Name.c_str()))
          {
            // Now attempt to read an expression in
            node->InitialValue = Expression();

            // If we parsed an initial value...
            if (node->InitialValue != nullptr)
            {
              // Accept the token position, and return the variable node
              this->SetNodeLocationEndHere(node);
              this->AcceptTokenPosition();
              return node;
            }
            else
            {
              // Show an error message
              this->ErrorHere(ErrorCode::VariableInitialValueNotFound, node->Name.c_str());
            }
          }
        }
      }

      // We must have failed, so delete the node
      delete node;
    }

    // We didn't successfully parse a variable definition, so just recall the token position and return null
    RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  template <typename Node>
  void Parser::ApplyVirtualStaticExtensionAttributes(Node* node)
  {
    // If this is a static function...
    if (GetAttribute(node->Attributes, StaticAttribute) != nullptr)
        node->IsStatic = true;

    // If this is a virtual function...
    if (GetAttribute(node->Attributes, VirtualAttribute) != nullptr)
    {
      // If the node is static, then we cannot be marked as virtual
      if (node->IsStatic)
      {
        this->ErrorHere(ErrorCode::StaticCannotBeVirtual, node->Name.c_str());
        return;
      }

      // Mark the node as being virtual
      node->Virtualized = VirtualMode::Virtual;
    }

    // If this is an extension function...
    AttributeNode* extensionAttribute = GetAttribute(node->Attributes, ExtensionAttribute);
    if (extensionAttribute != nullptr)
    {
      // We emit this error message over and over for all cases of failure
      CodeLocation& location = extensionAttribute->Location;
      cstr attributeName = extensionAttribute->TypeName->Token.c_str();
      cstr errorMessage = " The attribute must take the form of [Extension(typeid(OtherType))] and the OtherType must be a class/struct type.";

      // Make sure the attribute has a call node that only takes one argument
      FunctionCallNode* callNode = extensionAttribute->AttributeCall;
      if (callNode == nullptr || callNode->Arguments.Size() != 1)
        return this->Errors.Raise(location, ErrorCode::InvalidAttribute, attributeName, errorMessage);

      // Make sure the only argument to the attribute call is a typeid node with a compile time type
      TypeIdNode* typeIdNode = Type::DynamicCast<TypeIdNode*>(callNode->Arguments[0]);
      if (typeIdNode == nullptr || typeIdNode->CompileTimeSyntaxType == nullptr)
        return this->Errors.Raise(location, ErrorCode::InvalidAttribute, attributeName, errorMessage);

      // Make sure the typeid's compile time type is a bound type (class/struct) not a delegate or indirect type
      BoundSyntaxType* extensionOwner = Type::DynamicCast<BoundSyntaxType*>(typeIdNode->CompileTimeSyntaxType);
      if (extensionOwner == nullptr)
        return this->Errors.Raise(location, ErrorCode::InvalidAttribute, attributeName, errorMessage);

      // We successfully parsed an extension owner
      node->ExtensionOwner = extensionOwner->Clone();
    }

    // If this is a virtual function...
    if (GetAttribute(node->Attributes, OverrideAttribute) != nullptr)
    {
      ErrorIf(node->Virtualized == VirtualMode::Overriding,
        "It is not possible to already be overriding");

      // If the node is non-virtual, then mark it as overriding
      if (node->Virtualized == VirtualMode::NonVirtual)
      {
        // If the node is static, then we cannot be marked as virtual or overriding
        if (node->IsStatic)
        {
          this->ErrorHere(ErrorCode::StaticCannotBeOverriding, node->Name.c_str());
          return;
        }

        // Mark the node as being overriding
        node->Virtualized = VirtualMode::Overriding;
      }
      else
      {
        // The node is both overriding and virtual, this is unnecessary
        this->ErrorHere(ErrorCode::UnnecessaryVirtualAndOverride, node->Name.c_str());
        return;
      }
    }
  }

  //***************************************************************************
  MemberVariableNode* Parser::MemberVariable()
  {
    // Parse any optional attribute
    this->ParseAllOptionalAttributes();

    // If any errors occurred in parsing attributes, error here
    if (this->Errors.WasError)
      return nullptr;

    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // First look for the "var" keyword
    if (this->Accept(1, Grammar::Variable))
    {
      // Create a variable node
      MemberVariableNode* node = new MemberVariableNode();
      this->SetNodeLocationStartToLastSave(node);

      // Attach any attributes we parsed for this member variable
      this->AttachLastAttributeToNode(node->Attributes);

      // The name token we attempt to read
      const UserToken* nameToken = nullptr;

      // Get the name of the variable
      if (this->ExpectAndRetrieve(Grammar::UpperIdentifier, nameToken, ErrorCode::VariableNameNotFound))
      {
        // Store the name of the node
        node->Name = *nameToken;

        // Look for any virtual and static attributes
        this->ApplyVirtualStaticExtensionAttributes(node);

        // If any errors occurred from the virtual/static parsing
        if (this->Errors.WasError)
        {
          // We didn't successfully parse the static/virtual/override attributes
          this->RecallTokenPosition();
          delete node;
          return nullptr;
        }

        // Look for a type after the member variable
        if (this->AcceptOptionalTypeSpecifier(node->ResultSyntaxType, ErrorCode::VariableTypeNotFound, node->Name.c_str()))
        {
          // As long as we got a valid type...
          if (node->ResultSyntaxType != nullptr)
          {
            // Accept the beginning scope
            if (this->Accept(1, Grammar::BeginScope))
            {
              // This is a property, thus we expect to find a get or a set, or both
              // They must always come in the order of 'get' first, then set
              node->IsGetterSetter = true;

              // Check if we have a getter
              if (this->Accept(1, Grammar::Get))
              {
                // Read in the get function body
                node->Get = this->GetSetFunctionBody(node, true);
              }

              // Check if we have a setter
              if (this->Accept(1, Grammar::Set))
              {
                // Read in the set function body
                node->Set = this->GetSetFunctionBody(node, false);
              }

              // Error checking, we don't want to find a get after a set
              if (this->Accept(1, Grammar::Get))
              {
                // Show an error message
                this->ErrorHere(ErrorCode::GetFoundAfterSet, node->Name.c_str());
              }
              // We expect to see the end of the scope
              else if (this->Expect(Grammar::EndScope, ErrorCode::PropertyDeclarationNotComplete, node->Name.c_str()))
              {
                // Accept the token position, and return the variable node
                this->SetNodeLocationEndHere(node);
                this->AcceptTokenPosition();
                return node;
              }
              // If we're doing auto-complete or something like it...
              else if (this->Errors.TolerantMode)
              {
                // Since we're being tolerant, just eat tokens until we hit the end of our scope
                // This is just an approximation, as there may be actual scope errors
                // This will return true if it finds the scope and will advance the token forward automatically
                if (this->MoveToScopeEnd())
                {
                  // Accept the token position, and return the variable node
                  this->SetNodeLocationEndHere(node);
                  this->AcceptTokenPosition();
                  return node;
                }
              }
            }
            // We are not a property, so make sure the user is initializing the variable
            else
            {
              if (this->Accept(1, Grammar::Assignment))
              {
                // Note: It is actually legal for a member variable to Overriding or Virtual
                // In the case that we're overriding, we'll still be a raw member when directly accessed, but
                // if we're accessed through an interface it will use the generated get/set
                // In the case that we're virtual, the member itself basically becomes a property
                // This needs to be handled specially, because in the PreInitialize we still wan't to Assign
                // a value to the raw field data, but then anyone who accessing the member after that goes
                // through a property get/set

                // Now attempt to read the initialization expression in
                node->InitialValue = Expression();
              
                // If we failed to parsed an initial value...
                if (node->InitialValue == nullptr)
                {
                  // Show an error message
                  this->ErrorHere(ErrorCode::VariableInitialValueNotFound, node->Name.c_str());

                  // We didn't successfully parse a variable definition, so just recall the token position and return null
                  RecallTokenPosition();
                  delete node;
                  return nullptr;
                }
              }

              // Attempt to read the statement separator
              if (this->Expect(Grammar::StatementSeparator, ErrorCode::VariableInitializationNotComplete, node->Name.c_str()))
              {
                // Accept the token position, and return the variable node
                this->SetNodeLocationEndHere(node);
                this->AcceptTokenPosition();
                return node;
              }
            }
          }
          else
          {
            // Show an error message
            this->ErrorHere(ErrorCode::MemberVariableTypesCannotBeInferred, node->Name.c_str());
          }
        }
      }

      // We must have failed, so delete the node
      delete node;
    }

    // We didn't successfully parse a variable definition, so just recall the token position and return null
    this->RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  void Parser::ParseAllOptionalAttributes()
  {
    // Parse all the optional atrtibutes
    while (ParseOneOptionalAttribute());
  }

  //***************************************************************************
  bool Parser::ParseOneOptionalAttribute()
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // Check to see if we're starting an attribute
    if (Accept(1, Grammar::BeginAttribute))
    {
      // Every attribute starts with a type name
      const UserToken* typeName;

      // Read the attribute name
      if (ExpectAndRetrieve(Grammar::UpperIdentifier, typeName, ErrorCode::AttributeTypeNotFound))
      {
        // Create an attribute node that we'll fill in below
        AttributeNode* node = new AttributeNode();
        this->SetNodeLocationStartToLastSave(node);

        // Save the type name on the node
        node->TypeName = typeName;

        // Read an optional function call after the attribute node
        node->AttributeCall = this->FunctionCall(nullptr);

        // If there was any sort of error, bail out
        if (this->Errors.WasError)
        {
          // We didn't successfully parse the function call, so just recall the token position and return false
          delete node;
          RecallTokenPosition();
          return false;
        }

        // Look for the ending of the attribute
        if (Expect(Grammar::EndAttribute, ErrorCode::AttributeNotComplete, typeName->Token.c_str()))
        {
          // Add this node to the list of attributes which
          // we will attach to the next valid node we find
          this->LastAttributes.Add(node);

          // Accept the token position, and return the attributes
          this->SetNodeLocationEndHere(node);
          AcceptTokenPosition();
          return true;
        }
        else
        {
          delete node;
        }
      }
    }

    // We didn't successfully parse an expression, so just recall the token position and return null
    RecallTokenPosition();
    return false;
  }
  
  //***************************************************************************
  void Parser::AttachLastAttributeToNode(NodeList<AttributeNode>& attributes)
  {
    // Output the list of attributes
    attributes = this->LastAttributes;

    // Clear out the last attributes
    this->LastAttributes.Clear();
  }

  //***************************************************************************
  EnumValueNode* Parser::EnumValue()
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // Stores the initializer type (reused variable)
    const UserToken* valueName;

    // Attempt to read the enum value name
    if (this->AcceptAndRetrieve(1, Grammar::UpperIdentifier, &valueName))
    {
      // Create a return node since this is a valid return statement
      EnumValueNode* node = new EnumValueNode();
      this->SetNodeLocationStartToLastSave(node);

      // Set the name of the enum
      node->Name = *valueName;
      
      // It's optional to set a specific integral value
      if (this->Accept(1, Grammar::Assignment))
      {
        // Read the custom value we want to give this enum entry
        if (this->ExpectAndRetrieve(Grammar::IntegerLiteral, node->Value, ErrorCode::EnumValueRequiresIntegerLiteral, node->Name.c_str()) == false)
        {
          // We didn't successfully parse an enum, so just recall the token position and return null
          this->RecallTokenPosition();
          delete node;
          return nullptr;
        }
      }

      // Accept the token position, and return the "return node"
      this->AcceptTokenPosition();
      this->SetNodeLocationEndHere(node);
      return node;
    }

    // We didn't successfully parse en enum, so just recall the token position and return null
    this->RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  EnumNode* Parser::Enum()
  {
    // Parse any optional attribute
    this->ParseAllOptionalAttributes();
    
    // If any errors occurred in parsing attributes, error here
    if (this->Errors.WasError)
      return nullptr;

    // Save the token position
    ZilchSaveAndVerifyTokenPosition();
    
    // This will store the keyword we used to star this object (class or struct)
    const UserToken* enumType;

    // First look for the "enum" or "flags" keyword
    if (this->AcceptAny(2, &enumType, Grammar::Enumeration, Grammar::Flags))
    {
      // We must be inside a class, so allocate a node for it
      EnumNode* node = new EnumNode();
      this->SetNodeLocationStartToLastSave(node);

      // We read the class or struct keyword, it's safe to assume no other node type could be here
      // Attach the attributes we read above to the class node
      this->AttachLastAttributeToNode(node->Attributes);

      // Mark if this is considered a flags enum or not...
      node->IsFlags = (enumType->TokenId == Grammar::Flags);
      
      // Get the name of the enum
      const UserToken* name;
      if (this->ExpectAndRetrieve(Grammar::UpperIdentifier, name, ErrorCode::EnumNameNotFound))
      {
        // Set the name of the class node
        node->Name = *name;
        
        // Are we inheriting from a type (or implementing an interface?)
        if (this->Accept(1, Grammar::Inheritance))
        {
          // Attempt to read a parent type for this enum
          node->Inheritance = this->ReadTypeInfo();

          // If we had an error reading a type...
          if (this->Errors.WasError)
          {
            // We didn't successfully parse a class definition, so just recall the token position and return null
            delete node;
            RecallTokenPosition();
            return nullptr;
          }
        }

        // Now dive into the scope...
        if (this->Expect(Grammar::BeginScope, ErrorCode::EnumBodyNotFound, node->Name.c_str()))
        {
          // Parse the things that can show up inside a class until we run out of those
          ZilchLoop
          {
            // Attempt to add an enum value
            if (node->Values.Add(this->EnumValue()))
            {
              // If we don't find a comma, break out since there should be nothing more
              // Note that we allow a trailing comma after the last enum value
              if (this->Accept(1, Grammar::ArgumentSeparator) == false)
              {
                break;
              }
            }
            // We didn't parse an enum value, so break out
            else
            {
              break;
            }
          }

          // If we hit the end of the class scope...
          bool finishedNode = Expect(Grammar::EndScope, ErrorCode::EnumBodyNotComplete, node->Name.c_str());

          // As long as we finished this node (either via correct parsing or tolerance)
          if (finishedNode == false && this->Errors.TolerantMode)
          {
            // Since we're being tolerant, just eat tokens until we hit the end of our scope
            // This is just an approximation, as there may be actual scope errors
            // This will return true if it finds the scope and will advance the token forward automatically
            finishedNode = this->MoveToScopeEnd();
          }

          // As long as we finished this node (either via correct parsing or tolerance)
          if (finishedNode)
          {
            // We read the entire class definition
            // Accept the token position, and return the class node
            this->AcceptTokenPosition();
            this->SetNodeLocationEndHere(node);
            return node;
          }
        }
      }

      // We must have failed, so delete the node
      delete node;
    }

    // We didn't successfully parse a class definition, so just recall the token position and return null
    this->RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  ClassNode* Parser::Class()
  {
    // Parse any optional attribute
    this->ParseAllOptionalAttributes();

    // If any errors occurred in parsing attributes, error here
    if (this->Errors.WasError)
      return nullptr;

    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // This will store the keyword we used to star this object (class or struct)
    const UserToken* objectType;

    // First look for the "class" or "struct" keyword
    if (AcceptAny(2, &objectType, Grammar::Class, Grammar::Struct))
    {
      // We must be inside a class, so allocate a node for it
      ClassNode* node = new ClassNode();
      this->SetNodeLocationStartToLastSave(node);

      // We read the class or struct keyword, it's safe to assume no other node type could be here
      // Attach the attributes we read above to the class node
      this->AttachLastAttributeToNode(node->Attributes);

      // Set if we are a value type or not
      if (objectType->TokenId == Grammar::Struct)
      {
        // It was declared as a struct, therefore it is a value type!
        node->CopyMode = TypeCopyMode::ValueType;
      }
      else
      {
        node->CopyMode = TypeCopyMode::ReferenceType;
      }

      // Get the name of the class
      const UserToken* name;
      if (ExpectAndRetrieve(Grammar::UpperIdentifier, name, ErrorCode::ClassNameNotFound))
      {
        // Set the name of the class node
        node->Name = *name;

        // Is this a template type?
        if (Accept(1, Grammar::BeginTemplate))
        {
          // Parse arguments to the template until there are no more
          ZilchLoop
          {
            // Get the name of the argument
            const UserToken* argumentName;

            // Look for an identifier...
            if (ExpectAndRetrieve(Grammar::UpperIdentifier, argumentName, ErrorCode::TemplateArgumentNotFound))
            {
              // Add it to the arguments list
              node->TemplateArguments.PushBack(argumentName);
            }
            else
            {
              // We didn't successfully parse a class definition, so just recall the token position and return null
              delete node;
              this->RecallTokenPosition();
              return nullptr;
            }

            // Attempt to read an argument separator, and if we don't find one, break out
            if (Accept(1, Grammar::ArgumentSeparator) == false)
            {
              break;
            }
          }

          // We now expect to close the template argument
          if (Expect(Grammar::EndTemplate, ErrorCode::TemplateTypeArgumentsNotComplete) == false)
          {
            // We didn't successfully parse a class definition, so just recall the token position and return null
            delete node;
            RecallTokenPosition();
            return nullptr;
          }
        }

        // Are we inheriting from a type (or implementing an interface?)
        if (Accept(1, Grammar::Inheritance))
        {
          // Parse all the interfaces / base class
          ZilchLoop
          {
            // Look for an identifier...
            if (SyntaxType* argumentType = this->ReadTypeInfo())
            {
              // Add it to the arguments list
              node->Inheritance.Add(argumentType);
            }
            else
            {
              // We didn't successfully parse a class definition, so just recall the token position and return null
              delete node;
              RecallTokenPosition();
              return nullptr;
            }

            // Attempt to read an argument separator, and if we don't find one, break out
            if (Accept(1, Grammar::ArgumentSeparator) == false)
            {
              break;
            }
          }
        }

        // Now dive into the scope...
        if (Expect(Grammar::BeginScope, ErrorCode::ClassBodyNotFound, node->Name.c_str()))
        {
          // Specifies if we parsed anything inside the current scope
          bool parsedSomething;

          // Clear the destructor first
          node->Destructor = nullptr;

          // Parse the things that can show up inside a class until we run out of those
          do
          {
            // We haven't parsed anything yet this iteration...
            parsedSomething = false;

            // Attempt to parse a variable
            parsedSomething |= node->NonTraversedNonOwnedNodesInOrder.Add(node->Variables.Add(this->MemberVariable())) != nullptr;

            // Attempt to parse a function definition
            parsedSomething |= node->NonTraversedNonOwnedNodesInOrder.Add(node->Functions.Add(this->Function())) != nullptr;

            // Attempt to parse a constructor definition
            parsedSomething |= node->NonTraversedNonOwnedNodesInOrder.Add(node->Constructors.Add(this->Constructor())) != nullptr;

            // Attempt to parse a 'sends' statement
            parsedSomething |= node->NonTraversedNonOwnedNodesInOrder.Add(node->SendsEvents.Add(this->SendsEvent())) != nullptr;

            // Attempt to parse a destructor definition
            DestructorNode* destructor = this->Destructor();
            if (destructor != nullptr)
            {
              if (node->Destructor == nullptr)
              {
                // Set the destructor
                node->NonTraversedNonOwnedNodesInOrder.Add(destructor);
                node->Destructor = destructor;
                parsedSomething = true;
              }
              else
              {
                // Show an error
                ErrorHere(ErrorCode::OnlyOneDestructorAllowed);

                // We didn't successfully parse a class definition, so just recall the token position and return null
                delete node;
                delete destructor;
                RecallTokenPosition();
                return nullptr;
              }
            }
          }
          while (parsedSomething == true);

          // If we hit the end of the class scope...
          bool finishedNode = Expect(Grammar::EndScope, ErrorCode::ClassBodyNotComplete, node->Name.c_str());

          // As long as we finished this node (either via correct parsing or tolerance)
          if (finishedNode == false && this->Errors.TolerantMode)
          {
            // Since we're being tolerant, just eat tokens until we hit the end of our scope
            // This is just an approximation, as there may be actual scope errors
            // This will return true if it finds the scope and will advance the token forward automatically
            finishedNode = this->MoveToScopeEnd();
          }

          // As long as we finished this node (either via correct parsing or tolerance)
          if (finishedNode)
          {
            // We read the entire class definition
            // Accept the token position, and return the class node
            AcceptTokenPosition();
            this->SetNodeLocationEndHere(node);
            return node;
          }
        }
      }

      // We must have failed, so delete the node
      delete node;
    }

    // We didn't successfully parse a class definition, so just recall the token position and return null
    RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  bool Parser::AcceptOptionalTypeSpecifier(SyntaxType*& outSyntaxType, ErrorCode::Enum notFound, ...)
  {
    // Start a variadic argument list
    va_list vl;
    va_start(vl, notFound);
    
    // Forward the arguments to the accept function
    bool result = this->AcceptOptionalTypeSpecifierArgs(outSyntaxType, notFound, vl);

    // We're done with the variadic list
    va_end(vl);
    return result;
  }

  //***************************************************************************
  bool Parser::AcceptOptionalTypeSpecifierArgs(SyntaxType*& outSyntaxType, ErrorCode::Enum notFound, va_list args)
  {
    // Look for the type specifier for the return types
    if (Accept(1, Grammar::TypeSpecifier))
    {
      // This will store the return types in a function
      outSyntaxType = this->ReadTypeInfo();

      // Check if we did not parse a return type (but we were expecting one)
      if (outSyntaxType == nullptr)
      {
        // Throw an error
        this->ErrorHereArgs(notFound, args);
        return false;
      }
    }
    else
    {
      // We have no return type
      outSyntaxType = nullptr;
    }

    // We got here, successfull!
    return true;
  }

  //***************************************************************************
  bool Parser::ExpectArgumentList(GenericFunctionNode* node, StringParam functionName, bool mustBeEmpty)
  {
    // Get the name of the function
    if (Expect(Grammar::BeginFunctionParameters, ErrorCode::FunctionArgumentListNotFound, functionName.c_str()))
    {
      // Store the parameter index
      size_t parameterIndex = 0;

      // Some special argument lists (such as the destructor) must be empty
      // Technically we could parse those as a separate function, but just to reuse code we put this here
      if (mustBeEmpty == false)
      {
        // Get all the arguments for the function
        do
        {
          // Parse a parameter node
          ParameterNode* parameter = Parameter();

          // If the parameter is null
          if (parameter == nullptr)
          {
            // If we've already parsed one argument (meaning we have a comma)
            if (parameterIndex > 0)
            {
              // Throw an error
              ErrorHere(ErrorCode::FunctionParameterNotFound);
              return false;
            }
          }
          else
          {
            // Fill in which parameter this is
            parameter->ParameterIndex = parameterIndex;
          }

          // Add the parameter parameter to the parameter list, and pass in its index
          node->Parameters.Add(parameter);

          // Increment the parameter index
          ++parameterIndex;
        }
        while (Accept(1, Grammar::ArgumentSeparator));
      }

      // Look for the end parenthasis
      if (Expect(Grammar::EndFunctionParameters, ErrorCode::FunctionArgumentListNotComplete, functionName.c_str()))
      {
        // We succeeded at parsing
        return true;
      }
    }

    // Otherwise, we failed at parsing
    return false;
  }

  //***************************************************************************
  bool Parser::ExpectScopeBody(GenericFunctionNode* node, StringParam functionName)
  {
    // Now look for the start scope
    if (Expect(Grammar::BeginScope, ErrorCode::FunctionBodyNotFound, functionName.c_str()))
    {
      // Parse all the statements in the function
      while (node->Statements.Add(Statement()));

      // Now look for the end scope.
      if (Expect(Grammar::EndScope, ErrorCode::FunctionBodyNotComplete, functionName.c_str()))
      {
        // We succeeded in parsing the body
        this->SetNodeLocationEndHere(node);
        return true;
      }
      // If we're in tolerant mode...
      else if (this->Errors.TolerantMode)
      {
        // Since we're being tolerant, just eat tokens until we hit the end of our scope
        // This is just an approximation, as there may be actual scope errors
        // This will return true if it finds the scope and will advance the token forward automatically
        bool result = this->MoveToScopeEnd();
        this->SetNodeLocationEndHere(node);
        return result;
      }
    }

    // Otherwise, we failed at parsing
    return false;
  }
  
  //***************************************************************************
  bool Parser::MoveToScopeEnd()
  {
    // We probably failed to parse a statement, and therefore we didn't hit the end of the scope
    // Alternatively, the scope could just be missing

    // We're going to parse until we hit a scope (skipping all remaining statements)
    // We could try to parse the rest of the statements, but generally they don't affect code completion
    // Note: This could be entirely incorrect in the case that the end scope is missing!
    // For example, in parsing a scope for a function if the end scope is missing, it will not stop until
    // it reaches the end scope for the class!

    // We're currently in a scope (we started above)
    // When this hits zero, we'll know we reached the end of our scope
    size_t scopeCounter = 1;

    // Loop through all the rest of the tokens
    for (size_t index = this->TokenIndex; index < this->TokenStream->Size(); ++index)
    {
      // Retrieve the current token
      const UserToken* token = &(*this->TokenStream)[index];
          
      // Based on the current token type...
      switch (token->TokenId)
      {
        // If we reach the end of a file, or another function, or another class...
        // In the future, this may break if we add sub-classes or anonymous/nested functions
        case Grammar::End:
        case Grammar::Function:
        case Grammar::Class:
        {
          // Leave the token index right at this position so this token will be the next thing parsed
          this->TokenIndex = index;
          return true;
        }

        case Grammar::BeginScope:
        {
          // Increment the scope counter since we just encountered another scope
          ++scopeCounter;
          break;
        }
        case Grammar::EndScope:
        {
          // Decrement the scope counter until it reaches 0
          --scopeCounter;

          // If we hit the end of our current scope
          if (scopeCounter == 0)
          {
            // Move the token forward so that the next token is whatever is after the end scope
            this->TokenIndex = index + 1;

            // Return that we accepted this entire scope!
            return true;
          }
          break;
        }
      }
    }

    // If we got here, we were unable to find the ending scope!
    return false;
  }

  //***************************************************************************
  FunctionNode* Parser::GenerateGetSetFunctionNode(MemberVariableNode* variable, bool isGet)
  {
    // Create a function node
    FunctionNode* node = new FunctionNode();
    this->SetNodeLocationStartToLastSave(node);

    // We create a clone of the type because, in both get/set cases
    // the node is responsible for cleaning up syntax types, and we
    // want to avoid any double delete situations
    SyntaxType* typeClone = (SyntaxType*)variable->ResultSyntaxType->Clone();

    // If this is a getter...
    if (isGet)
    {
      // Generate the name for the getter
      node->Name = variable->Name;
      node->Name.Token = BuildGetterName(variable->Name.Token);

      // Getters don't take any parameters, but return the type of the property
      node->ReturnType = typeClone;
    }
    // Otherwise it's a setter!
    else
    {
      // Generate the name for the setter
      node->Name = variable->Name;
      node->Name.Token = BuildSetterName(variable->Name.Token);

      // Create a parameter node since all setters take in the value being set
      ParameterNode* value = new ParameterNode();
      this->SetNodeLocationStartToLastSave(value);

      // The parameter is never defaulted
      value->InitialValue = nullptr;

      // The value's type is the same as the variable's type
      value->ResultSyntaxType = typeClone;

      // The name of the parameter is simply just 'value'
      value->Name.Location = value->Location;
      value->Name.Token = ValueKeyword;

      // Add the parameter to our get function
      node->Parameters.Add(value);
    }

    // The function is static, virtual, and extended based entirely off the owning member
    node->IsStatic = variable->IsStatic;
    node->Virtualized = variable->Virtualized;
    if (variable->ExtensionOwner != nullptr)
      node->ExtensionOwner = variable->ExtensionOwner->Clone();

    // Return the function node we created
    return node;
  }

  //***************************************************************************
  FunctionNode* Parser::GetSetFunctionBody(MemberVariableNode* variable, bool isGet)
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // Generate the function node
    FunctionNode* node = GenerateGetSetFunctionNode(variable, isGet);

    // Now look for the start scope
    if (this->ExpectScopeBody(node, node->Name.Token))
    {
      // Accept the token position, and return the variable node
      this->AcceptTokenPosition();
      return node;
    }

    // We didn't successfully parse an expression, so just recall the token position and return null
    this->RecallTokenPosition();
    delete node;
    return nullptr;
  }

  //***************************************************************************
  FunctionNode* Parser::Function()
  {
    // Parse any optional attribute
    this->ParseAllOptionalAttributes();

    // If any errors occurred in parsing attributes, error here
    if (this->Errors.WasError)
      return nullptr;

    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // Look for the function keyword
    if (Accept(1, Grammar::Function))
    {
      // Create a function node
      FunctionNode* node = new FunctionNode();
      this->SetNodeLocationStartToLastSave(node);

      // Attach any attributes to this function node
      this->AttachLastAttributeToNode(node->Attributes);

      // Get the name of the function
      const UserToken* functionName;
      if (this->ExpectAndRetrieve(Grammar::UpperIdentifier, functionName, ErrorCode::FunctionNameNotFound))
      {
        // Store the name on the function
        node->Name = *functionName;

        // Look for any virtual and static attributes
        this->ApplyVirtualStaticExtensionAttributes(node);

        // If any errors occurred from the virtual/static parsing
        if (this->Errors.WasError)
        {
          // We didn't successfully parse the static/virtual/override attributes
          this->RecallTokenPosition();
          delete node;
          return nullptr;
        }

        // Parse the argument list
        bool argumentsMustBeEmpty = false;
        if (this->ExpectArgumentList(node, node->Name.Token, argumentsMustBeEmpty))
        {
          // Attempt to read the type specifier
          this->AcceptOptionalTypeSpecifier(node->ReturnType, ErrorCode::FunctionReturnTypeNotFound, node->Name.c_str());

          // If an error occurred reading the type specifier
          if (this->Errors.WasError)
          {
            // We didn't successfully parse the return type, so just recall the token position and return null
            this->RecallTokenPosition();
            delete node;
            return nullptr;
          }

          // Now look for the start scope
          if (this->ExpectScopeBody(node, node->Name.Token))
          {
            // Accept the token position, and return the variable node
            this->SetNodeLocationEndHere(node);
            this->AcceptTokenPosition();

            // Return the node now that we've parsed it
            return node;
          }
        }
      }

      // We failed if we got here
      delete node;
    }

    // We didn't successfully parse an expression, so just recall the token position and return null
    this->RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  AttributeNode* Parser::GetAttribute(NodeList<AttributeNode>& attributes, StringParam name)
  {
    // Loop through all the attributes
    for (size_t i = 0; i < attributes.Size(); ++i)
    {
      // Grab the current attribute
      AttributeNode* attribute = attributes[i];

      // If this is a the attribute we were looking for...
      if (attribute->TypeName->Token == name)
        return attribute;
    }

    // Otherwise, we didn't find it
    return nullptr;
  }

  //***************************************************************************
  template <typename FunctionNodeType>
  FunctionNodeType* Parser::SpecializedFunction
  (
    Grammar::Enum type,
    String functionName,
    bool (Parser::*postArgs)(FunctionNodeType* node)
  )
  {
    // Parse any optional attribute
    this->ParseAllOptionalAttributes();

    // If any errors occurred in parsing attributes, error here
    if (this->Errors.WasError)
      return nullptr;

    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // Look for the given grammar keyword
    if (Accept(1, type))
    {
      // Create a specialized function node
      FunctionNodeType* node = new FunctionNodeType();
      this->SetNodeLocationStartToLastSave(node);

      // Parse the argument list
      bool argumentsMustBeEmpty = (type == Grammar::Destructor);
      if (this->ExpectArgumentList(node, functionName, argumentsMustBeEmpty))
      {
        // If we have a post arguments parser function...
        if (postArgs != nullptr)
        {
          // Apply the post arguments function
          if ((this->*postArgs)(node) == false)
          {
            // We failed the post arguments function, so recall and free the node
            delete node;
            RecallTokenPosition();
            return nullptr;
          }
        }

        // Now look for the start scope
        if (this->ExpectScopeBody(node, functionName))
        {
          // Accept the token position, and return the variable node
          this->SetNodeLocationEndHere(node);
          this->AcceptTokenPosition();

          // Attach any attributes to this function node
          this->AttachLastAttributeToNode(node->Attributes);
          return node;
        }
      }
    }

    // We didn't successfully parse an expression, so just recall the token position and return null
    RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  ConstructorNode* Parser::Constructor()
  {
    return SpecializedFunction<ConstructorNode>(Grammar::Constructor, "Constructor", &Parser::ConstructorInitializerList);
  }

  //***************************************************************************
  bool Parser::ConstructorInitializerList(ConstructorNode* node)
  {
    // If we have an initializer list...
    if (this->Accept(1, Grammar::InitializerList))
    {
      // Stores the initializer type (reused variable)
      const UserToken* initializerType;

      // The first thing we expect is a base class initializer
      if (this->AcceptAndRetrieve(1, Grammar::Base, &initializerType))
      {
        // Create an initializer node to represent the base class
        InitializerNode* initializer = new InitializerNode();
        initializer->InitializerType = initializerType;
        this->SetNodeLocationStartHere(initializer);

        // Let the constructor know that it actually has a base initializer
        node->BaseInitializer = initializer;

        // Parse the function call for the current initializer
        FunctionCallNode* call = this->FunctionCall(nullptr);

        // If we did not parse the base initializer function call...
        if (call == nullptr)
        {
          // We can't just have 'base' or 'this' sitting there
          this->ErrorHere(ErrorCode::FunctionCallExpectedAfterInitializer);
          delete initializer;
          return false;
        }
        else
        {
          // The left hand operand is the base class initializer
          call->LeftOperand = initializer;

          // Add the call to the statements
          node->Statements.Add(call);
        }
      }
      
      // Look for a comma delimiter
      if (this->Accept(1, Grammar::ArgumentSeparator))
      {
        // Grab the 'this' token (we don't parse it as a special keyword!)
        const UserToken* thisToken;
        
        // Look for a local base class initializer
        if (this->AcceptAndRetrieve(1, Grammar::LowerIdentifier, &thisToken))
        {
          // If we found the 'this' keyword...
          if (thisToken->Token == ThisKeyword)
          {
            // Create an initializer node to represent the base class
            InitializerNode* initializer = new InitializerNode();
            initializer->InitializerType = thisToken;
            node->ThisInitializer = initializer;
            this->SetNodeLocationStartHere(initializer);

            // Parse the function call for the current initializer
            FunctionCallNode* call = this->FunctionCall(nullptr);

            // If we did not parse the base initializer function call...
            if (call == nullptr)
            {
              // We can't just have 'base' or 'this' sitting there
              this->ErrorHere(ErrorCode::FunctionCallExpectedAfterInitializer);
              delete initializer;
              return false;
            }
            else
            {
              // The left hand operand is the base class initializer
              call->LeftOperand = initializer;

              // Add the call to the statements
              node->Statements.Add(call);
            }
          }
        }
      }

      // Check for a late base initializer...
      if (this->Accept(2, Grammar::ArgumentSeparator, Grammar::Base))
      {
        this->ErrorHere(ErrorCode::BaseClassInitializerMustComeFirst);
        return false;
      }
    }

    // If we got here, either we did not have an initializer 
    // list or we did have one and parsed it properly
    return true;
  }

  //***************************************************************************
  DestructorNode* Parser::Destructor()
  {
    return SpecializedFunction<DestructorNode>(Grammar::Destructor, "Destructor", nullptr);
  }

  //***************************************************************************
  SendsEventNode* Parser::SendsEvent()
  {
    // Parse any optional attribute
    this->ParseAllOptionalAttributes();

    // If any errors occurred in parsing attributes, error here
    if (this->Errors.WasError)
      return nullptr;

    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // First look for the "sends" keyword
    if (Accept(1, Grammar::Sends))
    {
      // Create a sends statement node
      SendsEventNode* node = new SendsEventNode();
      this->SetNodeLocationStartToLastSave(node);

      // Attach any attributes we parsed for this member variable
      this->AttachLastAttributeToNode(node->Attributes);

      // Get the name of the variable
      if (ExpectAndRetrieve(Grammar::UpperIdentifier, node->Name, ErrorCode::SendsEventStatementNameNotFound))
      {
        // Grab the string name for convenience
        String name = node->Name->Token;

        // Look for a type after the member variable
        SyntaxType* syntaxType = nullptr;
        if (AcceptOptionalTypeSpecifier(syntaxType, ErrorCode::SendsEventStatementTypeNotFound, name.c_str()))
        {
          // As long as we got a valid type...
          if (syntaxType != nullptr)
          {
            node->EventType = Type::DynamicCast<BoundSyntaxType*>(syntaxType);
            if(node->EventType != nullptr)
            {
              // Attempt to read the statement separator
              if(Expect(Grammar::StatementSeparator, ErrorCode::SendsEventStatementNotComplete, name.c_str()))
              {
                // Accept the token position, and return the variable node
                this->SetNodeLocationEndHere(node);
                AcceptTokenPosition();
                return node;
              }
            }
            else
            {
              delete syntaxType;
              ErrorHere(ErrorCode::GenericError, "The sends declaration can only take a class or struct type");
            }
          }
          else
          {
            // Show an error message
            ErrorHere(ErrorCode::SendsEventStatementTypeSpecifierNotFound, name.c_str());
          }
        }
      }

      // We must have failed, so delete the node
      delete node;
    }

    // We didn't successfully parse a variable definition, so just recall the token position and return null
    RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  ParameterNode* Parser::Parameter()
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // Attempt to read the name before continuing...
    const UserToken* name;

    // Look for an identifier
    if (AcceptAndRetrieve(1, Grammar::LowerIdentifier, &name))
    {
      // Look for a type specifier (followed by the parameter's type)
      if (Expect(Grammar::TypeSpecifier, ErrorCode::ParameterTypeSpecifierNotFound, name->Token.c_str()))
      {
        // Create a parameter node
        ParameterNode* node = new ParameterNode();
        this->SetNodeLocationStartToLastSave(node);

        // Assume the parameter is not yet defaulted
        node->InitialValue = nullptr;

        // Attempt to read the type after the parameter
        if ((node->ResultSyntaxType = this->ReadTypeInfo()) != nullptr)
        {
          // Store the name and type on the parameter node
          node->Name = *name;

          // Accept the token position, and return the variable node
          AcceptTokenPosition();
          this->SetNodeLocationEndHere(node);
          return node;
        }
        else
        {
          // Delete the node and show an error
          delete node;
          ErrorHere(ErrorCode::ParameterTypeNotFound, name->Token.c_str());
        }
      }
    }

    // We didn't successfully parse an expression, so just recall the token position and return null
    RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  ExpressionNode* Parser::BinaryOperatorRightToLeftAssociative(ExpressionFn currentPrecedence, ExpressionFn nextPrecedence, int parameters, ...)
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // Attempt to parse the left operand (look for a higher precedence expression)
    ExpressionNode* leftOperand = (this->*nextPrecedence)();

    // As long as we parced a left operand...
    if (leftOperand != nullptr)
    {
      // This will hold the operator that we parse below
      const UserToken* acceptedOperator;

      // Start a variadic argument list
      va_list vl;
      va_start(vl, parameters);

      // If we found the operator...
      if (AcceptAnyArgs(parameters, &acceptedOperator, vl))
      {
        // Attempt to parse the right operand as if it was another expression of the same precedence
        ExpressionNode* rightOperand = (this->*currentPrecedence)();

        // If we properly parsed the right operand...
        if (rightOperand != nullptr)
        {
          // Allocate an expression node that encapsulates the operator and operands
          BinaryOperatorNode* node = new BinaryOperatorNode();
          node->Location = leftOperand->Location;
          SetNodeLocationPrimaryToToken(node, *acceptedOperator);

          // Initialize the node
          node->Operator = acceptedOperator;

          // Add the left and right operands explicitly to the node
          node->LeftOperand   = leftOperand;
          node->RightOperand  = rightOperand;

          // Accept the token position, close the va_list, and return the expression node
          this->AcceptTokenPosition();
          this->SetNodeLocationEndHere(node);
          va_end(vl);
          return node;
        }
        else
        {
          // We have an improper right operand to an expression, so delete the left argument and close the va_list
          delete leftOperand;
          va_end(vl);

          // Show an error message
          this->ErrorHere
          (
            ErrorCode::BinaryOperatorRightOperandNotFound,
            Grammar::GetName(acceptedOperator->TokenId).c_str(),
            acceptedOperator->Token.c_str()
          );
          RecallTokenPosition();
          return nullptr;
        }
      }
      else
      {
        // Otherwise, we didn't find an operator, but the left-argument could still be a valid expression
        AcceptTokenPosition();
        return leftOperand;
      }
    }

    // We didn't successfully parse an expression, so just recall the token position and return null
    RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  ExpressionNode* Parser::BinaryOperatorLeftToRightAssociative(ExpressionFn nextPrecedence, int parameters, ...)
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // Attempt to parse the left operand (look for a higher precedence expression)
    ExpressionNode* leftOperand = (this->*nextPrecedence)();

    // As long as we parced a left operand...
    if (leftOperand != nullptr)
    {
      ZilchLoop
      {
        // This will hold the operator that we parse below
        const UserToken* acceptedOperator;

        // Start a variadic argument list
        va_list vl;
        va_start(vl, parameters);

        // If we found the operator...
        if (AcceptAnyArgs(parameters, &acceptedOperator, vl))
        {
          // Attempt to parse the right operand as if it was another expression of lower precedence
          ExpressionNode* rightOperand =  (this->*nextPrecedence)();

          // If we properly parsed the right operand...
          if (rightOperand != nullptr)
          {
            // Allocate an expression node that encapsulates the operator and operands
            BinaryOperatorNode* node = new BinaryOperatorNode();
            node->Location = leftOperand->Location;
            SetNodeLocationPrimaryToToken(node, *acceptedOperator);

            // Initialize the node
            node->Operator = acceptedOperator;

            // Add the left and right operands explicitly to the node
            node->LeftOperand   = leftOperand;
            node->RightOperand  = rightOperand;

            // Close the va_list and continue the iteration to find more operands
            va_end(vl);
            leftOperand = node;
            this->SetNodeLocationEndHere(node);
          }
          else
          {
            // We have an improper right operand to an expression, so delete the left argument and close the va_list
            delete leftOperand;
            va_end(vl);

            // Show an error message
            this->ErrorHere
            (
              ErrorCode::BinaryOperatorRightOperandNotFound,
              Grammar::GetName(acceptedOperator->TokenId).c_str(),
              acceptedOperator->Token.c_str()
            );

            // We didn't successfully parse an expression, so just recall the token position and return null
            RecallTokenPosition();
            return nullptr;
          }
        }
        else
        {
          // Otherwise, we didn't find an operator, but the left-argument could still be a valid expression
          AcceptTokenPosition();
          return leftOperand;
        }
      }
      
      // Technically unreachable (all paths above handled properly)
      // If we somehow got here, we failed
      //delete leftOperand;
    }

    // We didn't successfully parse an expression, so just recall the token position and return null
    RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  ExpressionNode* Parser::Expression()
  {
    // Note: See the top of 'Shared.cpp' for a reference on precedence
    return Expression00();
  }

  //***************************************************************************
  ExpressionNode* Parser::Expression00()
  {
    // Parse the assignment and compound assignment operators
    return BinaryOperatorRightToLeftAssociative(&Parser::Expression00, &Parser::Expression01, 12, Grammar::Assignment, Grammar::AssignmentAdd, Grammar::AssignmentSubtract, Grammar::AssignmentMultiply, Grammar::AssignmentDivide, Grammar::AssignmentModulo, Grammar::AssignmentExponent, Grammar::AssignmentLeftShift, Grammar::AssignmentRightShift, Grammar::AssignmentBitwiseAnd, Grammar::AssignmentBitwiseXor, Grammar::AssignmentBitwiseOr);
  }

  //***************************************************************************
  ExpressionNode* Parser::Expression01()
  {
    // Parse the logical or operator
    return BinaryOperatorLeftToRightAssociative(&Parser::Expression02, 1, Grammar::LogicalOr);
  }

  //***************************************************************************
  ExpressionNode* Parser::Expression02()
  {
    // Parse the logical and operator
    return BinaryOperatorLeftToRightAssociative(&Parser::Expression03, 1, Grammar::LogicalAnd);
  }

  //***************************************************************************
  ExpressionNode* Parser::Expression03()
  {
    // Parse the bitwise or operator
    return BinaryOperatorLeftToRightAssociative(&Parser::Expression04, 1, Grammar::BitwiseOr);
  }

  //***************************************************************************
  ExpressionNode* Parser::Expression04()
  {
    // Parse the bitwise xor operator
    return BinaryOperatorLeftToRightAssociative(&Parser::Expression05, 1, Grammar::BitwiseXor);
  }

  //***************************************************************************
  ExpressionNode* Parser::Expression05()
  {
    // Parse the bitwise and operator
    return BinaryOperatorLeftToRightAssociative(&Parser::Expression06, 1, Grammar::BitwiseAnd);
  }

  //***************************************************************************
  ExpressionNode* Parser::Expression06()
  {
    // Parse the equality and inequality operators
    return BinaryOperatorLeftToRightAssociative(&Parser::Expression07, 2, Grammar::Equality, Grammar::Inequality);
  }

  //***************************************************************************
  ExpressionNode* Parser::Expression07()
  {
    // Parse the comparison operators
    return BinaryOperatorLeftToRightAssociative(&Parser::Expression08, 4, Grammar::LessThan, Grammar::LessThanOrEqualTo, Grammar::GreaterThan, Grammar::GreaterThanOrEqualTo);
  }

  //***************************************************************************
  ExpressionNode* Parser::Expression08()
  {
    // Parse the bitshift operators
    return BinaryOperatorLeftToRightAssociative(&Parser::Expression09, 2, Grammar::BitshiftLeft, Grammar::BitshiftRight);
  }

  //***************************************************************************
  ExpressionNode* Parser::Expression09()
  {
    // Parse the additive operators
    return BinaryOperatorLeftToRightAssociative(&Parser::Expression10, 2, Grammar::Add, Grammar::Subtract);
  }

  //***************************************************************************
  ExpressionNode* Parser::Expression10()
  {
    // Parse the multaplicative operators and the modulo operator
    return BinaryOperatorLeftToRightAssociative(&Parser::Expression11, 3, Grammar::Multiply, Grammar::Divide, Grammar::Modulo);
  }

  //***************************************************************************
  ExpressionNode* Parser::Expression11()
  {
    // Parse the exponent operator
    return BinaryOperatorLeftToRightAssociative(&Parser::Expression12, 1, Grammar::Exponent);
  }

  //***************************************************************************
  ExpressionNode* Parser::Expression12()
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // This will store the unary left hand operator
    const UserToken* acceptedOperator;

    // Attempt to parse the unary left hand operator
    if (AcceptAny(9, &acceptedOperator, Grammar::AddressOf, Grammar::Dereference, Grammar::Positive, Grammar::Negative, Grammar::Increment, Grammar::Decrement, Grammar::LogicalNot, Grammar::BitwiseNot, Grammar::PropertyDelegate))
    {
      // Attempt to read in the unary operand
      ExpressionNode* operand = Expression13();

      // If we found an operand...
      if (operand != nullptr)
      {
        // The node we're creating...
        UnaryOperatorNode* node = nullptr;

        // If this is a property delegate node, then this operator is slightly different
        if (acceptedOperator->TokenId == Grammar::PropertyDelegate)
        {
          // Create a unary node (property delegate operator) to represent the operation
          node = new PropertyDelegateOperatorNode();
        }
        else
        {
          // Create a unary node to represent the operation
          node = new UnaryOperatorNode();
        }

        // Initialize the node so we know it's location
        this->SetNodeLocationStartToToken(node, *acceptedOperator);

        // Setup the node
        node->Operator = acceptedOperator;

        // Add the operand expression explicitly to the node
        node->Operand = operand;

        // Accept the token position, and return the expression node
        AcceptTokenPosition();
        this->SetNodeLocationEndHere(node);
        return node;
      }
      else
      {
        // Show an error message
        this->ErrorHere
        (
          ErrorCode::UnaryOperatorOperandNotFound,
          Grammar::GetName(acceptedOperator->TokenId).c_str(),
          acceptedOperator->Token.c_str()
        );
      }
    }
    else
    {
      // Just attempt to read the next level of precedence
      ExpressionNode* nextLevel = Expression13();

      // If the next precedence level returned us a node, we'll use that one
      if (nextLevel != nullptr)
      {
        // Accept the token position, and return the next-level expression node
        AcceptTokenPosition();
        return nextLevel;
      }
    }

    // We didn't successfully parse an expression, so just recall the token position and return null
    RecallTokenPosition();
    return nullptr;
  }
  
  //***************************************************************************
  ExpressionNode* Parser::Expression13()
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // Attempt to parse the left operand (look for a higher precedence expression)
    ExpressionNode* leftOperand = Expression14();

    // As long as we parced a left operand...
    if (leftOperand != nullptr)
    {
      ZilchLoop
      {
        // If we found the "as" operator...
        if (Accept(1, Grammar::As))
        {
          // Allocate a type-cast node that represents the casting operation
          TypeCastNode* node = new TypeCastNode();
          this->SetNodeLocationStartToLastSave(node);
          this->SetNodeLocationPrimaryHere(node);

          // Now attempt to grab the type that we're casting to
          if ((node->Type = this->ReadTypeInfo()) != nullptr)
          {
            // Add the left operand explicitly to the node
            node->Operand = leftOperand;

            // Close the va_list and continue the iteration to find more operands
            leftOperand = node;
            this->SetNodeLocationEndHere(node);
          }
          else
          {
            // We have an improper right operand to an expression, so delete the left argument and close the va_list
            delete node;
            delete leftOperand;
            ErrorHere(ErrorCode::CastTypeNotFound);

            // We didn't successfully parse an expression, so just recall the token position and return null
            RecallTokenPosition();
            return nullptr;
          }
        }
        else
        {
          // Otherwise, we didn't find an operator, but the left-argument could still be a valid expression
          AcceptTokenPosition();
          return leftOperand;
        }
      }
      
      // Technically unreachable (all paths above handled properly)
      // If we got down here, we failed
      //delete leftOperand;
    }

    // We didn't successfully parse an expression, so just recall the token position and return null
    RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  ExpressionNode* Parser::Expression14()
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // Attempt to read a value node
    ExpressionNode* value = Value();

    // If the value node parsed successfully...
    if (value != nullptr)
    {
      // This node will store any nodes we get from the post-expression call
      ExpressionNode* node = this->PostExpression(value);

      // If we found a valid post-expression node...
      if (node != nullptr)
      {
        // Collect all the post-expressions
        ZilchLoop
        {
          // To chain post expressions, make the node we found into the "value"
          value = node;

          // Attempt to read another post expression node
          ExpressionNode* nextNode = this->PostExpression(value);

          // If we successfully parsed the next expression node...
          if (nextNode != nullptr)
          {
            // Store it away to be used in the next iteration...
            node = nextNode;
          }
          else
          {
            // Exit since we no longer have a node
            break;
          }
        }

        // Accept the token position, and return the special operator node
        AcceptTokenPosition();
        return node;
      }

      // Accept the token position, and return the value node
      AcceptTokenPosition();
      return value;
    }

    // We didn't successfully parse an expression, so just recall the token position and return null
    RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  ExpressionNode* Parser::PostExpression(ExpressionNode* leftOperand)
  {
    // First, check if we're at an indexer call
    ExpressionNode* node = this->IndexerCall(leftOperand);

    // If so, return the node
    if (node != nullptr)
      return node;

    // Now check to see if we're at a function call
    node = this->FunctionCall(leftOperand);

    // If so, return the node
    if (node != nullptr)
      return node;

    // Now check to see if we're at a member access
    node = this->MemberAccess(leftOperand);

    // If so, return the node
    if (node != nullptr)
      return node;

    // Now check to see if this is an expression initializer
    node = this->ExpressionInitializer(leftOperand);

    // If so, return the node
    if (node != nullptr)
      return node;

    // We failed to find anything...
    return nullptr;
  }

  //***************************************************************************
  IndexerCallNode* Parser::IndexerCall(ExpressionNode* leftOperand)
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // Check to see if we're starting an index
    if (this->Accept(1, Grammar::BeginIndex))
    {
      // We started an index, so allocate an indexer node
      IndexerCallNode* node = new IndexerCallNode();
      this->SetNodeLocationStartToLastSave(node);

      // Parse arguments to the indexer until there are no more
      ZilchLoop
      {
        // Attempt to parse an argument to the indexer
        if (node->Arguments.Add(this->Expression()) == nullptr)
        {
          // Show an error
          this->ErrorHere(ErrorCode::IndexerIndicesNotFound);

          // We didn't successfully parse an expression, so just recall the token position and return null
          this->RecallTokenPosition();
          delete node;
          return nullptr;
        }

        // Attempt to read an argument separator, and if we don't find one, break out
        if (this->Accept(1, Grammar::ArgumentSeparator) == false)
          break;
      }

      // Now that we parsed all the arguments, we need to parse the end of the index call
      if (this->Accept(1, Grammar::EndIndex))
      {
        // Accept the token position, and return the indexer node
        this->AcceptTokenPosition();
        this->SetNodeLocationEndHere(node);
        node->LeftOperand = leftOperand;
        return node;
      }
      else
      {
        // Show an error message
        this->ErrorHere(ErrorCode::IndexerNotComplete);
        delete node;
      }
    }

    // We didn't successfully parse an expression, so just recall the token position and return null
    this->RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  FunctionCallNode* Parser::FunctionCall(ExpressionNode* leftOperand)
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // Check to see if we're starting a function call
    if (this->Accept(1, Grammar::BeginFunctionCall))
    {
      // We started a function call, so allocate an function call node
      FunctionCallNode* node = new FunctionCallNode();
      this->SetNodeLocationStartToLastSave(node);

      // Assume by default that we will not use named arguments
      // Note that if the function call has no arguments, it will not matter
      node->IsNamed = false;

      // Did we already parse the first argument
      bool parsedFirstArgument = false;

      // Parse arguments to the function call until there are no more
      ZilchLoop
      {
        // This will hold the argument name as we parse it
        const UserToken* argumentName = nullptr;

        // Look for an identifier and the named argument symbol
        if (this->AcceptAndRetrieve(2, Grammar::LowerIdentifier, &argumentName, Grammar::NameSpecifier, (void*)nullptr))
        {
          // Add the name to the list of function call names
          node->ArgumentNames.PushBack(argumentName->Token);

          // Attempt to parse an argument to the indexer
          if (node->Arguments.Add(Expression()) == nullptr)
          {
            // Show an error message
            this->ErrorHere(ErrorCode::FunctionCallNamedArgumentNotFound, argumentName->Token.c_str());

            // We didn't successfully parse an expression, so just recall the token position and return null
            this->RecallTokenPosition();
            delete node;
            return nullptr;
          }

          // We parsed the first argument
          parsedFirstArgument = true;

          // Since we parsed the named argument we must be using named calling
          node->IsNamed = true;

          // Attempt to read an argument separator, and if we don't find one, break out
          if (this->Accept(1, Grammar::ArgumentSeparator) == false)
          {
            break;
          }
        }
        // As long as we didn't yet parse the first argument...
        else if (parsedFirstArgument == false)
        {
          break;
        }
        else
        {
          // Show an error message
          this->ErrorHere(ErrorCode::FunctionCallNotComplete);

          // We didn't successfully parse an expression, so just recall the token position and return null
          this->RecallTokenPosition();
          delete node;
          return nullptr;
        }
      }

      // If we did not parse any arguments (which could be ok...)
      // We need to try parsing as if they were positional arguments
      if (parsedFirstArgument == false)
      {
        // Parse arguments to the function call until there are no more
        ZilchLoop
        {
          // Attempt to parse an argument to the indexer
          if (node->Arguments.Add(Expression()))
          {
            // We parsed the first argument
            parsedFirstArgument = true;

            // Attempt to read an argument separator, and if we don't find one, break out
            if (this->Accept(1, Grammar::ArgumentSeparator) == false)
            {
              break;
            }
          }
          // As long as we didn't yet parse the first argument...
          else if (parsedFirstArgument == false)
          {
            break;
          }
          else
          {
            // Show an error message
            this->ErrorHere(ErrorCode::FunctionCallNotComplete);

            // We didn't successfully parse an expression, so just recall the token position and return null
            this->RecallTokenPosition();
            delete node;
            return nullptr;
          }
        }
      }

      // Now that we parsed all the arguments, we need to parse the end of the function call
      if (this->Accept(1, Grammar::EndFunctionCall))
      {
        // Accept the token position, and return the function call node
        this->AcceptTokenPosition();
        this->SetNodeLocationEndHere(node);
        node->LeftOperand = leftOperand;
        return node;
      }
      else
      {
        // Show an error message
        this->ErrorHere(ErrorCode::FunctionCallNotComplete);
        delete node;
      }
    }

    // We didn't successfully parse an expression, so just recall the token position and return null
    this->RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  MemberAccessNode* Parser::MemberAccess(ExpressionNode* leftOperand)
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // Hold the access operator type
    const UserToken* accessOperator;

    // Check to see if we're starting a member access
    if (this->AcceptAny(2, &accessOperator, Grammar::Access, Grammar::NonVirtualAccess))
    {
      // Hold the member we attempted to access
      const UserToken* member;

      // Get the member name that we're trying to access
      if (this->ExpectAndRetrieve(Grammar::UpperIdentifier, member, ErrorCode::MemberAccessNameNotFound))
      {
        // We started a member access, so allocate the corresponding node
        MemberAccessNode* node = new MemberAccessNode();
        node->LeftOperand = leftOperand;
        this->SetNodeLocationStartToLastSave(node);

        // Set the operator
        node->Operator = accessOperator->TokenId;

        // Set the name
        node->Name = member->Token;

        // Accept the token position, and return the function call node
        this->AcceptTokenPosition();
        this->SetNodeLocationEndHere(node);
        return node;
      }
    }

    // We didn't successfully parse an expression, so just recall the token position and return null
    this->RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  StatementNode* Parser::Delete()
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // Check to see if we're starting a delete statement
    if (Accept(1, Grammar::Delete))
    {
      // Parse the expression which we are deleting
      ExpressionNode* deletedObject = Expression();

      // If we parsed an expression
      if (deletedObject != nullptr)
      {
        // Create a delete node since this is a valid return statement
        DeleteNode* node = new DeleteNode();
        this->SetNodeLocationStartToLastSave(node);

        // Set the deleted object, then add the expression as a child
        node->DeletedObject = deletedObject;

        // Accept the token position, and return the "return node"
        AcceptTokenPosition();
        this->SetNodeLocationEndHere(node);
        return node;
      }
    }

    // We didn't successfully parse an expression, so just recall the token position and return null
    RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  StatementNode* Parser::Return()
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // Checks whether this return ignores flow control errors
    bool isDebugReturn = false;

    // Check to see if we're starting a debug statement (could be other debug statements)
    if (Accept(1, Grammar::Debug))
    {
      // This is a debug return, which means it ignores flow control
      isDebugReturn = true;
    }

    // Check to see if we're starting a return statement
    if (Accept(1, Grammar::Return))
    {
      // Create a return node since this is a valid return statement
      ReturnNode* node = new ReturnNode();
      this->SetNodeLocationStartToLastSave(node);

      // Set the debug return flag depending upon what we read above
      node->IsDebugReturn = isDebugReturn;

      // Attempt to parse an expression (this expression can be missing, that is OK!)
      node->ReturnValue = Expression();

      // Accept the token position, and return the "return node"
      AcceptTokenPosition();
      this->SetNodeLocationEndHere(node);
      return node;
    }

    // We didn't successfully parse an expression, so just recall the token position and return null
    RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  StatementNode* Parser::Break()
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // Check to see if we're starting a break statement
    if (Accept(1, Grammar::Break))
    {
      // Create a break node since this is a valid break statement
      BreakNode* node = new BreakNode();
      this->SetNodeLocationStartToLastSave(node);

      // Grab the user token for the identifier
      const UserToken* scopeCount = nullptr;

      // Read an optional integer that tells us how many loop scopes to break out of
      if (AcceptAndRetrieve(1, Grammar::IntegerLiteral, &scopeCount))
      {
        // Read the break count
        node->ScopeCount = atoi(scopeCount->Token.c_str());

        // If the scope count is not valid
        if (node->ScopeCount <= 0)
        {
          // Throw an error
          ErrorHere(ErrorCode::BreakCountMustBeGreaterThanZero);

          // We didn't successfully parse an expression, so just recall the token position and return null
          RecallTokenPosition();
          delete node;
          return nullptr;
        }
      }
      else
      {
        // Otherwise, the scope count is assumed to be 1
        node->ScopeCount = 1;
      }

      // Accept the token position, and return the "break node"
      AcceptTokenPosition();
      this->SetNodeLocationEndHere(node);
      return node;
    }

    // We didn't successfully parse an expression, so just recall the token position and return null
    RecallTokenPosition();
    return nullptr;
  }
  
  //***************************************************************************
  StatementNode* Parser::DebugBreak()
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // Check to see if we're starting a debug break statement
    if (Accept(2, Grammar::Debug, Grammar::Break))
    {
      // Create a break node since this is a valid break statement
      DebugBreakNode* node = new DebugBreakNode();
      this->SetNodeLocationStartToLastSave(node);

      // Accept the token position, and return the "debug break node"
      AcceptTokenPosition();
      this->SetNodeLocationEndHere(node);
      return node;
    }

    // We didn't successfully parse an expression, so just recall the token position and return null
    RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  StatementNode* Parser::Continue()
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // Check to see if we're starting a continue statement
    if (Accept(1, Grammar::Continue))
    {
      // Create a continue node since this is a valid continue statement
      ContinueNode* node = new ContinueNode();
      this->SetNodeLocationStartToLastSave(node);

      // Accept the token position, and return the "return node"
      AcceptTokenPosition();
      this->SetNodeLocationEndHere(node);
      return node;
    }

    // We didn't successfully parse an expression, so just recall the token position and return null
    RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  StatementNode* Parser::Throw()
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // Check to see if we're starting a continue statement
    if (this->Accept(1, Grammar::Throw))
    {
      // Read the expression (we'll validate that it's an exception type in the syntaxer)
      ExpressionNode* exception = this->Expression();

      // As long as we read the exception expression (typically new Exception...)
      if (exception != nullptr)
      {
        // Create a throw node since we know everything we need
        ThrowNode* node = new ThrowNode();
        this->SetNodeLocationStartToLastSave(node);

        // Set the exception expression on the node
        node->Exception = exception;

        // Accept the token position, and return the "throw node"
        this->AcceptTokenPosition();
        this->SetNodeLocationEndHere(node);
        return node;
      }
      else
      {
        // Show an error and let us be recalled below
        this->ErrorHere(ErrorCode::ThrowExceptionExpressionNotFound);
      }
    }

    // We didn't successfully parse an expression, so just recall the token position and return null
    this->RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  StatementNode* Parser::Statement(bool optionalDelimiter)
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // First parse to see if we found a delimited statement
    if (StatementNode* node = DelimitedStatement())
    {
      // When parsing single expressions and statements, we allow the delimiter to be optional (because we know its singular)
      if (optionalDelimiter)
      {
        // Just eat the statement separator if there is one
        this->Accept(1, Grammar::StatementSeparator);

        // Accept the token position, and return the node
        AcceptTokenPosition();
        return node;
      }
      else
      {
        // Check to see that the statement was properly delimited
        if (this->Expect(Grammar::StatementSeparator, ErrorCode::StatementSeparatorNotFound))
        {
          // Accept the token position, and return the node
          AcceptTokenPosition();
          return node;
        }
        else
        {
          // We failed to find the ending semicolon, delete the node and unroll
          delete node;
        }
      }
    }
    else if (StatementNode* node = FreeStatement())
    {
      // Accept the token position, and return the node
      AcceptTokenPosition();
      return node;
    }

    // Otherwise, recall the old token position and return null
    RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  StatementNode* Parser::DelimitedStatement()
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // Look for a variable (don't check for delimiting because it will be automatically checked for later)
    if (LocalVariableNode* node = this->LocalVariable())
    {
      // Accept the token position, and return the node
      AcceptTokenPosition();
      node->IsUsedAsStatement = true;
      return node;
    }
    // Look for a return statement
    else if (StatementNode* node = this->Return())
    {
      // Accept the token position, and return the node
      AcceptTokenPosition();
      return node;
    }
    // Look for a delete statement
    else if (StatementNode* node = this->Delete())
    {
      // Accept the token position, and return the node
      AcceptTokenPosition();
      return node;
    }
    // Look for a break statement
    else if (StatementNode* node = this->Break())
    {
      // Accept the token position, and return the node
      AcceptTokenPosition();
      return node;
    }
    // Look for a debug break statement
    else if (StatementNode* node = this->DebugBreak())
    {
      // Accept the token position, and return the node
      AcceptTokenPosition();
      return node;
    }
    // Look for a continue statement
    else if (StatementNode* node = this->Continue())
    {
      // Accept the token position, and return the node
      AcceptTokenPosition();
      return node;
    }
    // Look for a throw exception statement
    else if (StatementNode* node = this->Throw())
    {
      // Accept the token position, and return the node
      AcceptTokenPosition();
      return node;
    }
    // Look for an expression
    else if (ExpressionNode* node = this->Expression())
    {
      // Mark that this expression is being used as a statement
      node->IsUsedAsStatement = true;

      // Accept the token position, and return the node
      AcceptTokenPosition();
      return node;
    }

    // Otherwise, recall the old token position and return null
    RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  StatementNode* Parser::FreeStatement()
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // Look for an if statement
    if (StatementNode* node = If())
    {
      // Accept the token position, and return the node
      AcceptTokenPosition();
      return node;
    }
    else if (StatementNode* node = For())
    {
      // Accept the token position, and return the node
      AcceptTokenPosition();
      return node;
    }
    else if (StatementNode* node = ForEach())
    {
      // Accept the token position, and return the node
      AcceptTokenPosition();
      return node;
    }
    else if (StatementNode* node = While())
    {
      // Accept the token position, and return the node
      AcceptTokenPosition();
      return node;
    }
    else if (StatementNode* node = DoWhile())
    {
      // Accept the token position, and return the node
      AcceptTokenPosition();
      return node;
    }
    else if (StatementNode* node = Loop())
    {
      // Accept the token position, and return the node
      AcceptTokenPosition();
      return node;
    }
    else if (StatementNode* node = Scope())
    {
      // Accept the token position, and return the node
      AcceptTokenPosition();
      return node;
    }
    else if (StatementNode* node = Timeout())
    {
      // Accept the token position, and return the node
      AcceptTokenPosition();
      return node;
    }

    // Otherwise, recall the old token position and return null
    RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  void Parser::IfBody(ExpressionNode*& condition, IfRootNode* root)
  {
    // Create an if node since this is a valid if statement
    IfNode* node = new IfNode();
    this->SetNodeLocationStartToLastSave(node);
    
    // Parse all the statements inside the if-statement
    if (this->ExpectScopedStatements(node->Statements, Grammar::If))
    {
      // Set the conditional expression on the if-node
      node->Condition = condition;

      // End the node here and attach it to the if parts
      this->SetNodeLocationEndHere(node);
      root->IfParts.Add(node);

      // As long as we have a condition, we can continue parsing else statements
      if (condition != nullptr)
      {
        // Parse the else statement (if it exists)
        this->Else(root);
      }

      // We're done by this point
      condition = nullptr;
      return;
    }

    // Delete the if-node if we got here
    delete node;
  }

  //***************************************************************************
  ExpressionNode* Parser::IfCondition()
  {
    // Check to see if we're starting an if statement
    if (this->Accept(1, Grammar::If))
    {
      // Look for the beginning parenthasis
      if (this->Expect(Grammar::BeginGroup, ErrorCode::IfConditionalExpressionNotFound))
      {
        // Attempt to parse the conditional expression
        ExpressionNode* condition = Expression();

        // If we properly parsed the conditional expression...
        if (condition != nullptr)
        {
          // Look for the end parenthasis and beginning of the if-statement scope
          if (this->Expect(Grammar::EndGroup, ErrorCode::IfConditionalExpressionNotComplete))
          {
            return condition;
          }

          // Delete the conditional expression node if we got here
          delete condition;
        }
        else
        {
          // We couldn't read the conditional expression
          this->ErrorHere(ErrorCode::IfConditionalExpressionNotFound);
        }
      }
    }

    // Otherwise, no condition was parsed
    return nullptr;
  }

  //***************************************************************************
  void Parser::Else(IfRootNode* root)
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // Check to see if we're starting an else statement
    if (this->Accept(1, Grammar::Else))
    {
      // Parse the condition (it's ok if this is null)
      ExpressionNode* condition = this->IfCondition();

      // Now parse the body of the else statement
      this->IfBody(condition, root);

      // As long as we got a valid node back
      if (this->Errors.WasError == false)
      {
        // Return the node
        this->AcceptTokenPosition();
        return;
      }
    }

    // Otherwise, no else was parsed (that's ok!)
    this->RecallTokenPosition();
  }

  //***************************************************************************
  StatementNode* Parser::Timeout()
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // Check to see if we're starting a timeout statement
    if (this->Accept(1, Grammar::Timeout))
    {
      // Look for the beginning of a conditional expression
      if (this->Expect(Grammar::BeginGroup, ErrorCode::TimeoutSecondsNotFound))
      {
        // The timeout always takes an integer, just to ensure that users don't try to go to
        // a precision that's lower than our timer (typically the standard C clock function)
        const UserToken* secondsToken = nullptr;
        if (this->ExpectAndRetrieve(Grammar::IntegerLiteral, secondsToken, ErrorCode::TimeoutSecondsExpectedIntegerLiteral))
        {
          // Look for the end parenthasis
          if (this->Expect(Grammar::EndGroup, ErrorCode::TimeoutSecondsNotComplete))
          {
            // Set the number of seconds on the node
            int seconds = atoi(secondsToken->Token.c_str());

            // If the user specified a zero or negative time...
            if (seconds <= 0)
            {
              // Inform the user, recall the token position back and return out
              this->ErrorHere(ErrorCode::TimeoutSecondsMustBeNonZeroPositive);
              this->RecallTokenPosition();
              return nullptr;
            }

            // We've parsed everything we need to ensure this is a timeout statement, so create it
            TimeoutNode* node = new TimeoutNode();
            this->SetNodeLocationStartToLastSave(node);

            // We've verified it can't be negative, so cast it to an unsigned and store it on the node
            node->Seconds = (size_t)seconds;
            
            // Parse all the statements inside the timeout-statement
            if (this->ExpectScopedStatements(node->Statements, Grammar::Timeout))
            {
              // Accept the token position, and return the "return node"
              this->AcceptTokenPosition();
              this->SetNodeLocationEndHere(node);
              return node;
            }

            // Delete the timeout-node if we got here
            delete node;
          }
        }
        else
        {
          // Show an error message
          this->ErrorHere(ErrorCode::TimeoutSecondsNotFound);
        }
      }
    }

    // We didn't successfully parse an expression, so just recall the token position and return null
    this->RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  StatementNode* Parser::Scope()
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // Check to see if we're starting a scope statement
    if (this->Accept(1, Grammar::Scope))
    {
      // Create a loop node since this is a valid loop statement
      ScopeNode* node = new ScopeNode();
      this->SetNodeLocationStartToLastSave(node);

      // Parse all the statements inside the while-statement
      if (this->ExpectScopedStatements(node->Statements, Grammar::Scope))
      {
        // Accept the token position, and return the "return node"
        this->AcceptTokenPosition();
        this->SetNodeLocationEndHere(node);
        return node;
      }

      // Delete the while-node
      delete node;
    }

    // We didn't successfully parse an expression, so just recall the token position and return null
    this->RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  IfRootNode* Parser::If()
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // Parse the if condition (including the if keyword)
    ExpressionNode* condition = IfCondition();

    // If a condition and keyword was read...
    if (condition != nullptr)
    {
      // The root of the if statement, which makes traversing the parts of the if statement more straightforward
      IfRootNode* ifRoot = new IfRootNode();
      this->SetNodeLocationStartToLastSave(ifRoot);

      // Parse the body of the if statement, from { to }
      this->IfBody(condition, ifRoot);

      // As long as the body was properly read...
      if (this->Errors.WasError == false)
      {
        // We didn't have an error, so we should have one child
        if (ifRoot->IfParts.Empty() == false)
        {
          // The first part of the if statement is the 
          ifRoot->IfParts.Front()->IsFirstPart = true;
        }

        // Accept the token position, and return the "return node"
        this->AcceptTokenPosition();
        this->SetNodeLocationEndHere(ifRoot);
        return ifRoot;
      }

      // If we got here, we failed
      delete ifRoot;

      // This condition could be nulled by the 'IfBody' call
      delete condition;
    }

    // We didn't successfully parse an expression, so just recall the token position and return null
    RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  StatementNode* Parser::ForEach()
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // This is effectively what this below code is translating a foreach loop to:
    //*
    //* foreach (var animal in farm.All)
    //* {
    //*   animal.Speak();
    //* }
    //* 
    //* for (var [animalRange] = farm.All.All; [animalRange].IsNotEmpty; [animalRange].MoveNext())
    //* {
    //*   var animal = [animalRange].Current;
    //*   animal.Speak();
    //* }
    //*

    // Check to see if we're starting a foreach statement
    if (this->Accept(1, Grammar::ForEach))
    {
      // Look for the beginning of the for loop declaration
      if (this->Expect(Grammar::BeginGroup, ErrorCode::ForLoopExpressionsNotFound))
      {
        // Attempt to parse the variable
        LocalVariableNode* valueVariable = this->LocalVariable(false);

        // If we didn't parse a variable...
        if (valueVariable == nullptr)
        {
          // Show an error since we failed to parse the condition
          this->ErrorHere(ErrorCode::ForEachVariableDeclarationNotFound);

          // We didn't successfully parse an expression, so just recall the token position and return null
          this->RecallTokenPosition();
          return nullptr;
        }

        // Look for the 'in' keyword
        if (this->Expect(Grammar::In, ErrorCode::ForEachInKeywordNotFound))
        {
          // Attempt to parse the range expression
          ExpressionNode* range = this->Expression();

          // If we parsed an expression...
          if (range != nullptr)
          {
            // Look for the end parenthasis
            if (this->Expect(Grammar::EndGroup, ErrorCode::ForLoopExpressionsNotComplete))
            {
              // Create a for node since this is a valid for statement
              ForEachNode* node = new ForEachNode();
              this->SetNodeLocationStartToLastSave(node);

              // Let any users know the original variable/range that came from the foreach
              node->NonTraversedVariable = valueVariable->Clone();
              node->NonTraversedRange = (ExpressionNode*)range->Clone();

              // Access the 'All' element of the range expression
              // Note that even ranges have a subsequent 'All' which returns itself
              MemberAccessNode* allAccessNode = new MemberAccessNode();
              allAccessNode->Location = range->Location;
              allAccessNode->Name = "All";
              allAccessNode->Operator = Grammar::Access;
              allAccessNode->LeftOperand = range;

              // Create a variable that will store the range
              LocalVariableNode* rangeVariable = new LocalVariableNode();
              rangeVariable->Location = range->Location;
              rangeVariable->Name.Location = range->Location;
              rangeVariable->Name.Token = BuildString("[", valueVariable->Name.Token, "Range]");
              rangeVariable->InitialValue = allAccessNode;
              node->RangeVariable = rangeVariable;

              // Create a local variable reference to the range variable
              LocalVariableReferenceNode* rangeLocal = new LocalVariableReferenceNode();
              rangeLocal->Value = rangeVariable->Name;

              // We need to access the 'MoveNext' function on the range
              MemberAccessNode* moveNextAccessNode = new MemberAccessNode();
              moveNextAccessNode->Location = range->Location;
              moveNextAccessNode->Name = "MoveNext";
              moveNextAccessNode->Operator = Grammar::Access;
              moveNextAccessNode->LeftOperand = rangeLocal;

              // We want to call the 'MoveNext' function
              FunctionCallNode* moveNextCallNode = new FunctionCallNode();
              moveNextCallNode->Location = range->Location;
              moveNextCallNode->LeftOperand = moveNextAccessNode;

              // For iteration, we call the 'MoveNext' functoin on the range
              node->Iterator = moveNextCallNode;

              // Access the 'IsNotEmpty' element of the range expression
              MemberAccessNode* isNotEmptyAccessNode = new MemberAccessNode();
              isNotEmptyAccessNode->Location = range->Location;
              isNotEmptyAccessNode->Name = "IsNotEmpty";
              isNotEmptyAccessNode->Operator = Grammar::Access;
              isNotEmptyAccessNode->LeftOperand = rangeLocal->Clone();

              // Our condition is if we're empty
              node->Condition = isNotEmptyAccessNode;

              // Access the 'Current' element of the range expression
              MemberAccessNode* currentAccessNode = new MemberAccessNode();
              currentAccessNode->Location = range->Location;
              currentAccessNode->Name = "Current";
              currentAccessNode->Operator = Grammar::Access;
              currentAccessNode->LeftOperand = rangeLocal->Clone();

              // We create this variable and initialize it as the first statement in the for loop
              valueVariable->InitialValue = currentAccessNode;
              node->Statements.Add(valueVariable);

              // Parse all the statements inside the foreach-statement
              if (this->ExpectScopedStatements(node->Statements, Grammar::ForEach))
              {
                // Accept the token position, and return the "return node"
                this->AcceptTokenPosition();
                this->SetNodeLocationEndHere(node);
                return node;
              }

              // We didn't successfully parse an expression, so just recall the token position and return null
              this->RecallTokenPosition();
              delete node;
              return nullptr;
            }

            // We failed, so delete anything we've created up to this point
            delete range;
          }
          else
          {
            // Show an error since we failed to parse the condition
            this->ErrorHere(ErrorCode::ForEachRangeExpressionNotFound);
          }

          // We failed, so delete anything we've created up to this point
          delete valueVariable;
        }
      }
    }

    // We didn't successfully parse an expression, so just recall the token position and return null
    this->RecallTokenPosition();
    return nullptr;
  }
  
  //***************************************************************************
  bool Parser::ExpectScopedStatements(NodeList<StatementNode>& statements, Grammar::Enum parentKeyword)
  {
    // Grab the keyword in case an error occurs
    const String& keyword = Grammar::GetKeywordOrSymbol(parentKeyword);

    // Look for the beginning of the if-statement scope
    if (this->Accept(1, Grammar::BeginScope))
    {
      // Parse all the statements inside the if-statement
      while (statements.Add(this->Statement()));

      // Now look for the end scope
      if (this->Expect(Grammar::EndScope, ErrorCode::ScopeBodyNotComplete, keyword.c_str()))
      {
        return true;
      }
      // If we didn't find the end of the scope, but we're in tolerant mode...
      else if (this->Errors.TolerantMode)
      {
        // Since we're being tolerant, just eat tokens until we hit the end of our scope
        // This is just an approximation, as there may be actual scope errors
        // This will return true if it finds the scope and will advance the token forward automatically
        return this->MoveToScopeEnd();
      }
    }
    // Try and parse a single statement, if that fails, then throw an error
    else if (statements.Add(this->Statement()))
    {
      return true;
    }
    // Otherwise, we failed to parse either {} or a single statement...
    else
    {
      this->ErrorHere(ErrorCode::ScopeBodyNotFound, keyword.c_str());
    }

    return false;
  }

  //***************************************************************************
  StatementNode* Parser::For()
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // Check to see if we're starting a for statement
    if (Accept(1, Grammar::For))
    {
      // Look for the beginning of the for loop declaration
      if (Expect(Grammar::BeginGroup, ErrorCode::ForLoopExpressionsNotFound))
      {
        // Attempt to parse the variable
        LocalVariableNode* variable = this->LocalVariable();

        // Alternatively, we could have an initialization expression (assume we don't for now)
        ExpressionNode* initialization = nullptr;

        // If we didn't parse a variable...
        if (variable == nullptr)
        {
          // Attempt to parse the initialization expression
          initialization = Expression();

          // If we do not have an initialization expression...
          if (initialization == nullptr)
          {
            // Show an error since we failed to parse the condition
            ErrorHere(ErrorCode::ForLoopExpressionsNotFound);

            // We didn't successfully parse an expression, so just recall the token position and return null
            RecallTokenPosition();
            return nullptr;
          }
        }

        // Look for the semicolon that separates initialization and condition
        if (Expect(Grammar::StatementSeparator, ErrorCode::ForLoopExpressionsNotComplete))
        {
          // Attempt to parse the condition expression
          ExpressionNode* condition = Expression();

          // If we parsed an expression...
          if (condition != nullptr)
          {
            // Look for the semicolon that separates initialization and condition
            if (Expect(Grammar::StatementSeparator, ErrorCode::ForLoopExpressionsNotComplete))
            {
              // Attempt to parse the iterator expression
              ExpressionNode* iterator = Expression();

              // If we parsed an expression...
              if (iterator != nullptr)
              {
                // Look for the end parenthasis
                if (Expect(Grammar::EndGroup, ErrorCode::ForLoopExpressionsNotComplete))
                {
                  // Create a for node since this is a valid for statement
                  ForNode* node = new ForNode();
                  this->SetNodeLocationStartToLastSave(node);

                  // Parse all the statements inside the for-statement
                  if (this->ExpectScopedStatements(node->Statements, Grammar::For))
                  {
                    // Set the variable (it could be null)
                    node->ValueVariable = variable;

                    // Set the initialization expression (it could be null)
                    node->Initialization = initialization;

                    // Set the conditional expression
                    node->Condition = condition;

                    // Set the iterator expression
                    node->Iterator = iterator;

                    // Accept the token position, and return the "return node"
                    AcceptTokenPosition();
                    this->SetNodeLocationEndHere(node);
                    return node;
                  }

                  // Delete the node since we failed
                  delete node;
                }
              }
              else
              {
                // Show an error since we failed to parse the iterator
                ErrorHere(ErrorCode::ForLoopExpressionsNotFound);
              }

              // We failed, so delete anything we've created up to this point
              delete iterator;
            }

            // We failed, so delete anything we've created up to this point
            delete condition;
          }
          else
          {
            // Show an error since we failed to parse the condition
            ErrorHere(ErrorCode::ForLoopExpressionsNotFound);
          }

          // We failed, so delete anything we've created up to this point
          delete variable;
          delete initialization;
        }
      }
    }

    // We didn't successfully parse an expression, so just recall the token position and return null
    RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  StatementNode* Parser::While()
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // Check to see if we're starting a while statement
    if (this->Accept(1, Grammar::While))
    {
      // Look for the beginning of a conditional expression
      if (this->Expect(Grammar::BeginGroup, ErrorCode::WhileConditionalExpressionNotFound))
      {
        // Attempt to parse the conditional expression
        ExpressionNode* condition = this->Expression();

        // If we properly parsed the conditional expression...
        if (condition != nullptr)
        {
          // Look for the end parenthasis
          if (this->Expect(Grammar::EndGroup, ErrorCode::WhileConditionalExpressionNotComplete))
          {
            // Create a while node since this is a valid while statement
            WhileNode* node = new WhileNode();
            this->SetNodeLocationStartToLastSave(node);

            // Parse all the statements inside the while-statement
            if (this->ExpectScopedStatements(node->Statements, Grammar::While))
            {
              // Set the conditional expression on the if-node
              node->Condition = condition;

              // Accept the token position, and return the "return node"
              AcceptTokenPosition();
              this->SetNodeLocationEndHere(node);
              return node;
            }

            // Delete the while-node
            delete node;
          }

          // Delete the conditional expression node
          delete condition;
        }
        else
        {
          // Show an error message
          ErrorHere(ErrorCode::WhileConditionalExpressionNotFound);
        }
      }
    }

    // We didn't successfully parse an expression, so just recall the token position and return null
    RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  StatementNode* Parser::DoWhile()
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // Check to see if we're starting a do while statement
    if (this->Accept(1, Grammar::Do))
    {
      // Create a do-while node since this is a valid while statement
      DoWhileNode* node = new DoWhileNode();
      this->SetNodeLocationStartToLastSave(node);

      // Parse all the statements inside the while-statement
      if (this->ExpectScopedStatements(node->Statements, Grammar::Do))
      {
        // Now parse the while statement
        if (Accept(1, Grammar::While))
        {
          // Look for the beginning of a conditional expression
          if (Expect(Grammar::BeginGroup, ErrorCode::DoWhileConditionalExpressionNotFound))
          {
            // Attempt to parse the conditional expression
            ExpressionNode* condition = Expression();

            // If we properly parsed the conditional expression...
            if (condition != nullptr)
            {
              // Look for the end parenthasis
              if (Expect(Grammar::EndGroup, ErrorCode::DoWhileConditionalExpressionNotComplete))
              {
                // Set the conditional expression on the if-node
                node->Condition = condition;

                // Accept the token position, and return the "return node"
                AcceptTokenPosition();
                this->SetNodeLocationEndHere(node);
                return node;
              }

              // Delete the conditional expression node
              delete condition;
            }
            else
            {
              // We failed to find the conditional expression
              ErrorHere(ErrorCode::DoWhileConditionalExpressionNotFound);
            }
          }
        }
      }

      // Delete the while-node
      delete node;
    }

    // We didn't successfully parse an expression, so just recall the token position and return null
    RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  StatementNode* Parser::Loop()
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // Check to see if we're starting a loop statement
    if (this->Accept(1, Grammar::Loop))
    {
      // Create a loop node since this is a valid loop statement
      LoopNode* node = new LoopNode();
      this->SetNodeLocationStartToLastSave(node);

      // Parse all the statements inside the while-statement
      if (this->ExpectScopedStatements(node->Statements, Grammar::Loop))
      {
        // Accept the token position, and return the "return node"
        AcceptTokenPosition();
        this->SetNodeLocationEndHere(node);
        return node;
      }

      // Delete the while-node
      delete node;
    }

    // We didn't successfully parse an expression, so just recall the token position and return null
    this->RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  StaticTypeNode* Parser::StaticTypeOrCreationCall()
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // We don't know if this node is going to be used as a constructor yet
    CreationMode::Enum creationMode = CreationMode::Invalid;

    // Lets see if it's a new creation call
    if (this->Accept(1, Grammar::New))
      creationMode = CreationMode::New;
    // Lets see if it's a local creation call
    else if (this->Accept(1, Grammar::Local))
      creationMode = CreationMode::Local;

    // Now lets attempt to get the type used in the new-call
    BoundSyntaxType* type = this->ReadBoundTypeInfo();
    if (type != nullptr)
    {
      // Create the node that we'll attach to the tree
      StaticTypeNode* node = new StaticTypeNode();
      node->Mode = creationMode;
      node->ReferencedSyntaxType = type;

      // Let the node know where it was started
      this->SetNodeLocationStartToLastSave(node);

      // Return the parsed constructor call (every new/local requires a constructor call)
      this->SetNodeLocationEndHere(node);
      this->AcceptTokenPosition();
      return node;
    }
    // If we didn't read a type, but we did have a new/local...
    else if (creationMode != CreationMode::Invalid)
    {
      // Show an error message
      this->ErrorHere(ErrorCode::CreatedTypeNotFound);
    }

    // Return null since we didn't parse anything
    this->RecallTokenPosition();
    return nullptr;
  }
  
  //***************************************************************************
  ExpressionNode* Parser::ExpressionInitializer(ExpressionNode* leftOperand)
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // Let the user know that the syntax has changed
    ZilchTodo("This can be removed once a considerable time period has passed or when we give it a new meaning");
    if (this->Accept(1, Grammar::OldBeginInitializer))
    {
      // Show an error and recall back to the saved position
      this->ErrorHere(ErrorCode::GenericError, "The expression initializer syntax has changed from using [] to using {}.");
      this->RecallTokenPosition();
      return nullptr;
    }

    // Attempt to parse the initializer now (initializes members and adds values to containers AFTER construction)
    if (this->Accept(1, Grammar::BeginInitializer))
    {
      // Wrap the left operand in a uniquely generated local variable
      // (so we can refer to it in special syntactical sugar cases)
      // For example, when we do member initializers or container initializers, we call .Add on this variable
      // We explicitly use this as an expression
      LocalVariableNode* leftVar = new LocalVariableNode(ExpressionInitializerLocal, this->ParentProject, nullptr);
      this->SetNodeLocationStartToLastSave(leftVar);
    
      // Create and setup the expression initializer node
      ExpressionInitializerNode* initializer = new ExpressionInitializerNode();
      this->SetNodeLocationStartHere(initializer);
    
      // Our initializer points at the unique local variable we created, and in tern that points at the actual left expression
      initializer->LeftOperand = leftVar;

      // Because we may allow a trailing comma (or empty initializers) then we may accept an initializer early
      bool acceptedOuterEndInitializer = false;

      // Loop until we have no more expressions to parse
      do
      {
        // First check to see if this is an entirely empty list or if we just have a trailing comma...
        if (this->Accept(1, Grammar::EndInitializer))
        {
          acceptedOuterEndInitializer = true;
          break;
        }

        // If we read in an add node (two forms, just an expression, and also a block [expressions])
        ExpressionInitializerAddNode* addNode = nullptr;

        // Save the name of the member we're initializing
        const UserToken* memberName = nullptr;

        // If we're initializing a member...
        if (this->AcceptAndRetrieve(2, Grammar::UpperIdentifier, &memberName, Grammar::Assignment, nullptr))
        {
          // Create the member initializer node
          ExpressionInitializerMemberNode* memberNode = new ExpressionInitializerMemberNode();
          this->SetNodeLocationStartHere(memberNode);
          initializer->InitailizeMembers.Add(memberNode);

          // Set the name of the member we're accessing
          memberNode->MemberName = *memberName;

          // Now read the value we want to initialize this member to
          if (ExpressionNode* initialValue = this->Expression())
          {
            // Store the initial value, we've read the entire member initializer!
            memberNode->Value = initialValue;
            
            // End the member node here
            this->SetNodeLocationEndHere(memberNode);

            // We previously generated a local variable so we could look up the variable by name
            LocalVariableReferenceNode* accessLeftVar = new LocalVariableReferenceNode();
            accessLeftVar->Location = memberNode->Location;
            accessLeftVar->Value = leftVar->Name;

            // Access the 'Add' method on the container
            MemberAccessNode* memberAccess = new MemberAccessNode();
            memberAccess->Location = memberNode->Location;
            memberAccess->Name = memberName->Token;
            memberAccess->Operator = Grammar::Access;
            memberAccess->LeftOperand = accessLeftVar;

            // Call the add method with the arguments we parsed
            BinaryOperatorNode* assignment = new BinaryOperatorNode();
            assignment->Location = memberNode->Location;
            assignment->LeftOperand = memberAccess;
            assignment->Operator = Tokenizer::GetAssignmentToken();
            assignment->RightOperand = (ExpressionNode*)memberNode->Value->Clone();

            // Lastly, add the assignment to the initialization statements
            initializer->InitializerStatements.Add(assignment);
          }
          else
          {
            // Show an error and recall back to the saved position
            this->ErrorHere(ErrorCode::CreationInitializeMemberExpectedInitialValue,
              memberName->Token.c_str(), memberName->Token.c_str());
            this->RecallTokenPosition();

            // Deleting the initializer should delete the constructor call and the creation node, and the type we parsed
            delete initializer;
            return nullptr;
          }
        }
        // Look for another 'add' element initializer
        else if (this->Accept(1, Grammar::BeginInitializer))
        {
          // We're reading another value (could be an argument list)
          addNode = new ExpressionInitializerAddNode();
          this->SetNodeLocationStartHere(addNode);
          initializer->AddValues.Add(addNode);
        
          // Because we may allow a trailing comma (or empty initializers) then we may accept an initializer early
          bool acceptedInnerEndInitializer = false;

          // Loop until we have no more expressions to parse
          do
          {
            // First check to see if this is an entirely empty list or if we just have a trailing comma...
            if (this->Accept(1, Grammar::EndInitializer))
            {
              acceptedInnerEndInitializer = true;
              break;
            }

            // Parse each expression and add it
            ExpressionNode* addValue = this->Expression();

            // We got in here, but we didn't expect another expression
            // We make an exception for the first expression
            if (addValue == nullptr)
            {
              // Show an error and recall back to the saved position
              this->ErrorHere(ErrorCode::CreationInitializerNotComplete);
              this->RecallTokenPosition();

              // Deleting the initializer should delete the constructor call and the creation node, and the type we parsed
              delete initializer;
              return nullptr;
            }

            // Add the next expression to our arguments list...
            addNode->Arguments.Add(addValue);
          }
          // While we read comma separators (which means we read the next expression)
          while (this->Accept(1, Grammar::ArgumentSeparator));

          // If we expect another expression...
          if (acceptedInnerEndInitializer == false && this->Expect(Grammar::EndInitializer, ErrorCode::CreationInitializerNotComplete) == false)
          {
            // Deleting the initializer should delete the constructor call and the creation node, and the type we parsed
            delete initializer;
            this->RecallTokenPosition();
            return nullptr;
          }
        }
        // If we're adding just one expression...
        else if (ExpressionNode* singleAddValue = this->Expression())
        {
          // We're reading another value (could be an argument list)
          addNode = new ExpressionInitializerAddNode();
          this->SetNodeLocationStartHere(addNode);
          initializer->AddValues.Add(addNode);
        
          // Add the expression we read to the add node
          addNode->Arguments.Add(singleAddValue);
        }
        else
        {
          // Show an error and recall back to the saved position
          this->ErrorHere(ErrorCode::CreationInitializerExpectedSubElement);
          this->RecallTokenPosition();

          // Deleting the initializer should delete the constructor call and the creation node, and the type we parsed
          delete initializer;
          return nullptr;
        }

        // If we read in an add node, then generate an actual 'Add' statement
        if (addNode != nullptr)
        {
          // End the add node here
          this->SetNodeLocationEndHere(addNode);
          
          // We previously generated a local variable so we could look up the creation node by name
          LocalVariableReferenceNode* accessLeftVar = new LocalVariableReferenceNode();
          accessLeftVar->Location = addNode->Location;
          accessLeftVar->Value = leftVar->Name;

          // Access the 'OperatorInsert' method on the container
          MemberAccessNode* addMember = new MemberAccessNode();
          addMember->Location = addNode->Location;
          addMember->Name = OperatorInsert;
          addMember->Operator = Grammar::Access;
          addMember->LeftOperand = accessLeftVar;

          // Call the add method with the arguments we parsed
          FunctionCallNode* addCall = new FunctionCallNode();
          addCall->Location = addNode->Location;
          addCall->LeftOperand = addMember;

          // Copy over the arguments to the function call
          for (size_t i = 0; i < addNode->Arguments.Size(); ++i)
          {
            // Clone the same expression arguments to appear in the add call arguments
            addCall->Arguments.Add((ExpressionNode*)addNode->Arguments[i]->Clone());
          }

          // Lastly, add the call to the initialization statements
          initializer->InitializerStatements.Add(addCall);
        }
      }
      // While we read comma separators (which means we read the next expression)
      while (this->Accept(1, Grammar::ArgumentSeparator));

      // If we expect another expression...
      if (acceptedOuterEndInitializer == false && this->Expect(Grammar::EndInitializer, ErrorCode::CreationInitializerNotComplete) == false)
      {
        // Deleting the constructor call should delete the creation node, all initializers, and the type we parsed
        delete initializer;
        this->RecallTokenPosition();
        return nullptr;
      }

      // The initializer ends here
      this->SetNodeLocationEndHere(initializer);
      this->AcceptTokenPosition();
      leftVar->InitialValue = leftOperand;
      return initializer;
    }

    // We didn't parse anything
    this->RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  ExpressionNode* Parser::TypeId()
  {
    // We only need to get the type-id token because we need to know its location
    // so that we can tell the node where it started from
    const UserToken* typeIdToken;

    // Lets see if it's a type-id
    if (this->AcceptAndRetrieve(1, Grammar::TypeId, &typeIdToken))
    {
      // Check for the first parenthesis
      if (this->Expect(Grammar::BeginGroup, ErrorCode::TypeIdExpressionNotFound))
      {
        // Below is a little bit of a wonky piece of logic
        // We may want to introduce a 'static type id' instead
        // Basically we need to attempt to read a type, and then if that fails
        // we need to recall back and read an expression
        // We know that we properly read the type when it parses and we also read
        // an ending parenthesis ')'. Note that the user could be inputting
        // an expression, but the expression could start with a type (TypeMemberAccess...)
        this->SaveTokenPosition();

        // We may alternatively try to get the typeid of a type
        ZilchTodo("Investigate changing typeid(Type) to just parsing an expression because now we have "
                  "StaticTypeNode (Decorate StaticTypeNode would need to allow typeid as a parent)");
        SyntaxType* type = this->ReadTypeInfo();

        // Read the value we want to get the type of
        ExpressionNode* value = nullptr;

        // If we read the type, and we know it's the whole type because we hit the ending parentheses...
        if (type != nullptr && this->Accept(1, Grammar::EndGroup))
        {
          // Accept the read so we can move on
          this->AcceptTokenPosition();
        }
        // If no type was read, or we didn't read the end parentheses
        // then attempt to read a value expression...
        else
        {
          // First attempt to delete the type that was read (nullptr safe to delete)
          delete type;
          type = nullptr;

          // Recall the position back so we can read the expression
          this->RecallTokenPosition();
          
          // Read the value expression that we want to get rtti for
          value = this->Expression();
        }

        // As long as we read the type-id expression / type
        if (type != nullptr || value != nullptr)
        {
          // Make sure we read the ending parenthesis (note, if we read a type, then that already happened!)
          if (type != nullptr || this->Expect(Grammar::EndGroup, ErrorCode::TypeIdExpressionNotComplete))
          {
            // Create a new call node
            TypeIdNode* node = new TypeIdNode();
            this->SetNodeLocationStartToToken(node, *typeIdToken);

            // Set the value expression or type that we want to get rtti for
            node->CompileTimeSyntaxType = type;
            node->Value = value;

            // Return the node we just parsed
            this->SetNodeLocationEndHere(node);
            return node;
          }
        }
        else
        {
          // Show an error since we didn't parse the expression
          this->ErrorHere(ErrorCode::TypeIdExpressionNotFound);
        }
      }
    }

    // Return null since we didn't parse anything
    return nullptr;
  }

  //***************************************************************************
  ExpressionNode* Parser::MemberId()
  {
    // We only need to get the memberid token because we need to know its location
    // so that we can tell the node where it started from
    const UserToken* memberIdToken;

    // Lets see if it's a memberid
    if (this->AcceptAndRetrieve(1, Grammar::MemberId, &memberIdToken))
    {
      // Check for the first parenthesis
      if (this->Expect(Grammar::BeginGroup, ErrorCode::MemberIdExpressionNotFound))
      {
        // Read the value we want to get the member of
        ExpressionNode* value = this->Expression();
        MemberAccessNode* member = Type::DynamicCast<MemberAccessNode*>(value);
        if (value != nullptr && this->Expect(Grammar::EndGroup, ErrorCode::MemberIdExpressionNotComplete))
        {
          // As long as we read the member
          if (member != nullptr)
          {
            // Set the member expression on the new node
            MemberIdNode* node = new MemberIdNode();
            this->SetNodeLocationStartToToken(node, *memberIdToken);
            node->Member = member;

            // Return the node we just parsed
            this->SetNodeLocationEndHere(node);
            return node;
          }
          else
          {
            // Show an error since we didn't parse the expression
            this->ErrorHere(ErrorCode::MemberIdMustBeUsedOnAMember);
          }
        }
        else
        {
          // Show an error since we didn't parse the expression
          this->ErrorHere(ErrorCode::MemberIdExpressionNotFound);
        }

        // Delete any expression we might have read
        delete value;
      }
    }

    // Return null since we didn't parse anything
    return nullptr;
  }

  //***************************************************************************
  ValueNode* Parser::CreateStringLiteral(const UserToken* token)
  {
    // Just allocate a standard value-node
    ValueNode* node = new ValueNode();

    // Let the node know where it started and ended (line / character, etc)
    this->SetNodeLocationStartHere(node);

    // Point the node's value to the parsed value
    node->Value = *token;
    node->Value.TokenId = Grammar::StringLiteral;

    // Return the value node
    return node;
  }

  //***************************************************************************
  StringInterpolantNode* Parser::StringInterpolant()
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // This will hold the token that we try and parse
    const UserToken* interpolantStart;
    
    // Check to see if we're starting a string interpolant
    if (this->AcceptAny(1, &interpolantStart, Grammar::BeginStringInterpolate))
    {
      // Create a string interpolant node
      StringInterpolantNode* node = new StringInterpolantNode();
      this->SetNodeLocationStartToLastSave(node);

      // Add the string value node
      node->Elements.Add(this->CreateStringLiteral(interpolantStart));

      ZilchLoop
      {
        // Now we should be able to parse an expression, since we started an interpolation
        if (node->Elements.Add(this->Expression()) == nullptr)
        {
          // Show an error and recall back to the saved position
          this->ErrorHere(ErrorCode::StringInterpolantExpectedExpression);
          this->RecallTokenPosition();
          delete node;
          return nullptr;
        }

        // Attempt to read the end of the interpolant
        const UserToken* interpolantEnd;
        if (this->AcceptAny(2, &interpolantEnd, Grammar::EndStringInterpolate, Grammar::EndBeginStringInterpolate))
        {
          // Add the string value node
          node->Elements.Add(this->CreateStringLiteral(interpolantEnd));

          // If we reached the true end of the interpolant
          if (interpolantEnd->TokenId == Grammar::EndStringInterpolate)
          {
            // Accept the token position, and return the node
            this->AcceptTokenPosition();
            this->SetNodeLocationEndHere(node);
            return node;
          }
        }
        else
        {
          // Show an error and recall back to the saved position
          this->ErrorHere(ErrorCode::StringInterpolantNotComplete);
          this->RecallTokenPosition();
          delete node;
          return nullptr;
        }
      }
    }

    // We didn't successfully parse an expression, so just recall the token position and return null
    this->RecallTokenPosition();
    return nullptr;
  }

  //***************************************************************************
  ExpressionNode* Parser::Value()
  {
    // Save the token position
    ZilchSaveAndVerifyTokenPosition();

    // This will hold the token that we try and parse
    const UserToken* value;

    // Attempt to parse any value (including literals and identifiers) and retrieve the associated token
    if (this->AcceptAny(10, &value, Grammar::LowerIdentifier, Grammar::IntegerLiteral, Grammar::DoubleIntegerLiteral, Grammar::RealLiteral, Grammar::DoubleRealLiteral, Grammar::StringLiteral, Grammar::CharacterLiteral, Grammar::Null, Grammar::True, Grammar::False))
    {
      // We found a value, so make room for it
      ValueNode* node;
      
      // If the value is an lower-case identifier, then it must be a variable reference
      if (value->TokenId == Grammar::LowerIdentifier)
      {
        // Allocate an identifier node (which is also a value node)
        node = new LocalVariableReferenceNode();
      }
      // Otherwise, it's just a standard value
      else
      {
        // Just allocate a standard value-node
        node = new ValueNode();
      }

      // Let the node know where it started (line / character, etc)
      this->SetNodeLocationStartToLastSave(node);

      // Point the node's value to the parsed value
      node->Value = *value;

      // Accept the token position, and return the value node
      AcceptTokenPosition();
      this->SetNodeLocationEndHere(node);
      return node;
    }
    // Lets see if it's a string interpolant
    else if (StringInterpolantNode* node = this->StringInterpolant())
    {
      // Accept the token position, and return the value node
      AcceptTokenPosition();
      return node;
    }
    // Read a type (a constructor call or static member access could follow)
    // We also parse the new/local keywords here (in that case, we know it MUST be a constructor call)
    else if (StaticTypeNode* node = this->StaticTypeOrCreationCall())
    {
      // Accept the token position, and return the value node
      AcceptTokenPosition();
      return node;
    }
    // It might be a typeid, lets check that
    else if (ExpressionNode* node = this->TypeId())
    {
      // Accept the token position, and return the value node
      AcceptTokenPosition();
      return node;
    }
    // It might be a memberid, lets check that
    else if (ExpressionNode* node = this->MemberId())
    {
      // Accept the token position, and return the value node
      AcceptTokenPosition();
      return node;
    }
    // Lastly, try and see if we're just in a grouping operator
    else if (Accept(1, Grammar::BeginGroup))
    {
      // Attempt to read the expression inside the grouping
      ExpressionNode* expression = this->Expression();

      // If the expression was found...
      if (expression != nullptr)
      {
        // Look for the closing parenthasis
        if (Expect(Grammar::EndGroup, ErrorCode::GroupingOperatorNotComplete))
        {
          // Accept the token position, and return the expression node
          this->AcceptTokenPosition();
          return expression;
        }

        // If we got here, we failed to finish the grouping operator
        delete expression;
      }
    }

    // Otherwise, recall the old token position and return null
    this->RecallTokenPosition();
    return nullptr;
  }
}
