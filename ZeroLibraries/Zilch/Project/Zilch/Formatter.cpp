/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  CodeFormat::CodeFormat() :
    IsTabs(false),
    Identation(2),
    StripWhiteSpaceFromEmptyLines(false),
    IndentGlobalDefault(IndentStyle::Indented),
    IndentClassContents(IndentStyle::UseGlobalDefault),
    IndentEnumContents(IndentStyle::UseGlobalDefault),
    IndentFunctionContents(IndentStyle::UseGlobalDefault),
    IndentPropertyContents(IndentStyle::UseGlobalDefault),
    IndentGetSetContents(IndentStyle::UseGlobalDefault),
    IndentScopeContents(IndentStyle::UseGlobalDefault),
    LineStyleGlobalDefaultScope(LineStyle::NextLine),
    LineStyleClassScope(LineStyle::UseGlobalDefault),
    LineStyleEnumScope(LineStyle::UseGlobalDefault),
    LineStyleFunctionScope(LineStyle::UseGlobalDefault),
    LineStylePropertyScope(LineStyle::UseGlobalDefault),
    LineStyleGetSetScope(LineStyle::UseGlobalDefault),
    LineStyleBlockScope(LineStyle::UseGlobalDefault),
    LineStyleInitializerList(LineStyle::SameLine),
    SpaceStyleGlobalDefaultColon(SpaceStyle::BeforeAndAfter),
    SpaceStyleInheritanceColon(SpaceStyle::UseGlobalDefault),
    SpaceStyleInitializerListColon(SpaceStyle::UseGlobalDefault),
    SpaceStyleTypeColon(SpaceStyle::UseGlobalDefault),
    SpaceStyleNamedArgumentColon(SpaceStyle::UseGlobalDefault),
    SpaceStyleGlobalDefaultComma(SpaceStyle::After),
    SpaceStyleInheritanceComma(SpaceStyle::UseGlobalDefault),
    SpaceStyleInitializerListComma(SpaceStyle::UseGlobalDefault),
    SpaceStyleFunctionDefinitionParameterComma(SpaceStyle::UseGlobalDefault),
    SpaceStyleFunctionCallParameterComma(SpaceStyle::UseGlobalDefault),
    SpaceStyleTemplateDefinitionParameterComma(SpaceStyle::UseGlobalDefault),
    SpaceStyleTemplateInstantiationParameterComma(SpaceStyle::UseGlobalDefault),
    SpaceStyleGlobalDefaultParenthesis(SpaceStyle::None),
    SpaceStyleFunctionDefinitionBeginParenthesis(SpaceStyle::None),
    SpaceStyleFunctionDefinitionEndParenthesis(SpaceStyle::None),
    SpaceAfterComment(true),
    CommentWordWrapLength(100)
  {
  }

  //***************************************************************************
  ZilchCodeBuilder::ZilchCodeBuilder() :
    Indentation(0),
    Line(1),
    Character(1)
  {
  }

  //***************************************************************************
  void ZilchCodeBuilder::WriteKeywordOrSymbol(Grammar::Enum token)
  {
    this->Write(Grammar::GetKeywordOrSymbol(token));
  }

  //***************************************************************************
  void ZilchCodeBuilder::WriteKeywordOrSymbolSpaceStyle(Grammar::Enum token, SpaceStyle::Enum specific, SpaceStyle::Enum globalDefault)
  {
    // This tells us if we want to place spaces before or after (or around) the token
    SpaceStyle::Enum spaceStyle = GetSpaceStyle(specific, globalDefault);

    // Prepend a space if we require it
    if (spaceStyle == SpaceStyle::BeforeAndAfter || spaceStyle == SpaceStyle::Before)
    {
      this->WriteSpace();
    }

    this->WriteKeywordOrSymbol(token);

    // Append a space if we require it
    if (spaceStyle == SpaceStyle::BeforeAndAfter || spaceStyle == SpaceStyle::After)
    {
      this->WriteSpace();
    }
  }
  
  //***************************************************************************
  void ZilchCodeBuilder::BeginScope(ScopeType::Enum scope)
  {
    // Scopes are responsible for starting their own line, and the next
    // For that matter, any kind of statement or thing that belongs on
    // its own line is repsonsible for emitting its own line before

    // Technically these should always be set as long as the 'scope' value is valid, however we need
    // to initialize them to defaults so the compiler doesn't complain about 'potentially uninitialized' values
    LineStyle::Enum specificLineStyle = LineStyle::NextLine;
    IndentStyle::Enum specificIndentStyle = IndentStyle::Indented;

    // Depending on the scope type we're adding, we could have different rules
    switch (scope)
    {
      // We're adding scope to a class (Contains properties, fields, methods, etc)
      case ScopeType::Class:
        specificLineStyle = this->Format.LineStyleClassScope;
        specificIndentStyle = this->Format.IndentClassContents;
        break;
      // We're adding scope to a class (Contains properties, fields, methods, etc)
      case ScopeType::Enumeration:
        specificLineStyle = this->Format.LineStyleEnumScope;
        specificIndentStyle = this->Format.IndentEnumContents;
        break;
      // We're adding scope to a function's statements
      case ScopeType::Function:
        specificLineStyle = this->Format.LineStyleFunctionScope;
        specificIndentStyle = this->Format.IndentFunctionContents;
        break;
      // We're adding scope to a property
      case ScopeType::Property:
        specificLineStyle = this->Format.LineStylePropertyScope;
        specificIndentStyle = this->Format.IndentPropertyContents;
        break;
      // We're adding scope to a get/set
      case ScopeType::GetSet:
        specificLineStyle = this->Format.LineStyleGetSetScope;
        specificIndentStyle = this->Format.IndentGetSetContents;
        break;
      // We're adding scope to any other type of block (if statement, while loop, etc)
      case ScopeType::Block:
        specificLineStyle = this->Format.LineStyleBlockScope;
        specificIndentStyle = this->Format.IndentScopeContents;
        break;
    }

    // For every situation, we could have a specific line or indent style, or we could fall back to the global
    LineStyle::Enum lineStyle = GetLineStyle(specificLineStyle, this->Format.LineStyleGlobalDefaultScope);
    IndentStyle::Enum indentStyle = GetIndentStyle(specificIndentStyle, this->Format.IndentGlobalDefault);

    // It would be nice if scope was just a simple integer, however, because some styles require that certain
    // scopes are indented, and others are not, and in some the braces themselves are indented...
    // We need to push this structure into a stack instead (a stack of scopes)
    ScopeStyle scopeStyle;
    scopeStyle.InnardsIndented = (indentStyle == IndentStyle::Indented);

    // Based on the line style...
    switch (lineStyle)
    {
      case LineStyle::NextLine:
        // Just write out a standard indented line
        this->WriteLineIndented();
        break;

      case LineStyle::NextLineIndented:
        // Just write out a standard indented line with one extra indent
        this->WriteLineIndented();
        this->WriteIndent();

        // In this case, it means that even the ending brace needs to be indented...
        scopeStyle.BracesIndented = true;
        break;

      case LineStyle::SameLine:
        // We always write out a space (this should probably become an option in the future)
        this->Write(" ");
        break;
    }

    // Now write out the 'begin scope' token, typically '{'
    this->WriteKeywordOrSymbol(Grammar::BeginScope);

    // Add the scope to the scope stack so that when we 'EndScope' / pop it off, we know what to do
    this->Scopes.PushBack(scopeStyle);

    // If we're indenting the inside, then increase the indentation for the next line
    if (scopeStyle.InnardsIndented)
    {
      ++this->Indentation;
    }

    // If the user also requested the braces be indented, our standard behavior is to indent the innards yet again
    if (scopeStyle.BracesIndented)
    {
      ++this->Indentation;
    }
  }
  
  //***************************************************************************
  void ZilchCodeBuilder::EndScope()
  {
    // Trim any previously writen lines or spaces
    this->TrimEnd();

    // Make sure we have scopes to end, and we didn't call this one too many times
    ErrorIf(this->Scopes.Empty(),
      "Attempting to pop a scope when there we're no scopes present");

    // Get the latest scope
    ScopeStyle& scope = this->Scopes.Back();

    // If it's innards were indented, decrease the indent
    if (scope.InnardsIndented)
    {
      --this->Indentation;
    }

    // Note: When emitting anything, we always assume we're on the last line, which means we must emit our own line
    // Note: If you look in BeginScope, having 'BracesIndented' will actually increase the tabbing again
    // At this point in time, we've only decreased the tabbing due to 'InnardsIndented', which means
    // that this line we emit will still have tabbing for braces, which is good for when we emit the end brace
    this->WriteLineIndented();

    // If it's braces were indented, decrease the indent again (see 2nd note above and BeginScope)
    if (scope.BracesIndented)
    {
      --this->Indentation;
    }
    
    // Now write out the 'end scope' token, typically '}'
    this->WriteKeywordOrSymbol(Grammar::EndScope);
    
    // We've officially finished this scope off!
    this->Scopes.PopBack();
  }

  //**************************************************************************
  void ZilchCodeBuilder::WriteLine()
  {
    // Invoke the base so we just write a standard newline
    StringBuilderExtended::WriteLine();

    // Also increment our line count
    ++this->Line;
  }

  //**************************************************************************
  void ZilchCodeBuilder::WriteLineIndented()
  {
    // Create a new line
    this->WriteLine();

    // Write indents up to the indentation level we're at
    for (size_t i = 0; i < this->Indentation; ++i)
    {
      this->WriteIndent();
    }
  }

  //**************************************************************************
  void ZilchCodeBuilder::Write(const UserToken& token)
  {
    this->Write(token.Token);
  }

  //**************************************************************************
  void ZilchCodeBuilder::WriteLineStyle(LineStyle::Enum specific, LineStyle::Enum globalDefault)
  {
    // Get the resulting line style
    LineStyle::Enum lineStyle = GetLineStyle(specific, globalDefault);

    // Based on the line style...
    switch (lineStyle)
    {
      case LineStyle::NextLine:
        // Just write out a standard indented line
        this->WriteLineIndented();
        break;

      case LineStyle::NextLineIndented:
        // Just write out a standard indented line with one extra indent
        this->WriteLineIndented();
        this->WriteIndent();
        break;

      case LineStyle::SameLine:
        // We always write out a space (this should probably become an option in the future)
        this->Write(" ");
        break;
    }
  }

  //***************************************************************************
  void ZilchCodeBuilder::WriteIndent()
  {
    // We make constants so we don't keep allocating this
    static const String Tab("\t");
    static const String Space(" ");

    // If we use tabs to format, then indent with tabs
    if (this->Format.IsTabs)
    {
      this->Repeat(this->Format.Identation, Tab);
    }
    else
    {
      this->Repeat(this->Format.Identation, Space);
    }
  }

  //***************************************************************************
  void ZilchCodeBuilder::WriteSpace()
  {
    this->Write(" ");
  }
  
  //***************************************************************************
  void ZilchCodeBuilder::WriteSingleLineComment(StringParam text)
  {
    this->Write("//");
    if (this->Format.SpaceAfterComment)
      this->WriteSpace();

    this->Write(text);
  }
  
  //***************************************************************************
  void ZilchCodeBuilder::TrimEnd()
  {
    size_t amountToBackup = 0;
    for (long long i = (long long)(this->GetSize() - 1); i >= 0; --i)
    {
      byte b = (*this)[(size_t)i];
      if (isspace(b) == false)
        break;

      ++amountToBackup;
    }

    this->Backup(amountToBackup);
  }

  //***************************************************************************
  String ZilchCodeBuilder::ToString()
  {
    //HACK need to run post pass to trim trailing space / empty lines
    return StringBuilderExtended::ToString();
  }

  //***************************************************************************
  size_t ZilchCodeBuilder::GetLine()
  {
    return this->Line;
  }

  //***************************************************************************
  ZilchCodeBuilder::ScopeStyle::ScopeStyle() :
    BracesIndented(false),
    InnardsIndented(false)
  {
  }

  //***************************************************************************
  ScopeLastNode::ScopeLastNode() :
    LastNode(nullptr),
    AssociatedScope(nullptr)
  {
  }

  //***************************************************************************
  CodeFormatterContext::CodeFormatterContext()
  {
  }

  //***************************************************************************
  LineStyle::Enum ZilchCodeBuilder::GetLineStyle(LineStyle::Enum specific, LineStyle::Enum globalDefault)
  {
    ErrorIf(globalDefault == LineStyle::UseGlobalDefault,
      "The global default cannot be set to 'LineStyle::UseGlobalDefault'");

    // Return the specific line style as long as it's not falling back on the default
    if (specific != LineStyle::UseGlobalDefault)
    {
      return specific;
    }

    // Return the default line format
    return globalDefault;
  }

  //***************************************************************************
  IndentStyle::Enum ZilchCodeBuilder::GetIndentStyle(IndentStyle::Enum specific, IndentStyle::Enum globalDefault)
  {
    ErrorIf(globalDefault == IndentStyle::UseGlobalDefault,
      "The global default cannot be set to 'IndentStyle::UseGlobalDefault'");

    // Return the specific indent style as long as it's not falling back on the default
    if (specific != IndentStyle::UseGlobalDefault)
    {
      return specific;
    }

    // Return the default indent format
    return globalDefault;
  }
  
  //***************************************************************************
  SpaceStyle::Enum ZilchCodeBuilder::GetSpaceStyle(SpaceStyle::Enum specific, SpaceStyle::Enum globalDefault)
  {
    ErrorIf(globalDefault == IndentStyle::UseGlobalDefault,
      "The global default cannot be set to 'SpaceStyle::UseGlobalDefault'");

    // Return the specific space style as long as it's not falling back on the default
    if (specific != SpaceStyle::UseGlobalDefault)
    {
      return specific;
    }

    // Return the default space format
    return globalDefault;
  }

  //***************************************************************************
  CodeFormatter::CodeFormatter()
  {
    this->Walker.RegisterNonLeafBase(&CodeFormatter::FormatCommentsAndLines);
    
    this->Walker.Register(&CodeFormatter::FormatBinaryOperator);
    this->Walker.Register(&CodeFormatter::FormatUnaryOperator);
    this->Walker.RegisterDerived<PropertyDelegateOperatorNode>(&CodeFormatter::FormatUnaryOperator);
    this->Walker.Register(&CodeFormatter::FormatTypeCast);
    this->Walker.Register(&CodeFormatter::FormatIndexerCall);
    this->Walker.Register(&CodeFormatter::FormatFunctionCall);
    this->Walker.Register(&CodeFormatter::FormatMemberAccess);
    this->Walker.Register(&CodeFormatter::FormatStaticTypeNode);
    this->Walker.Register(&CodeFormatter::FormatLocalVariable);
    this->Walker.Register(&CodeFormatter::FormatParameter);
    this->Walker.Register(&CodeFormatter::FormatMemberVariable);
    this->Walker.Register(&CodeFormatter::FormatValue);
    this->Walker.Register(&CodeFormatter::FormatStringInterpolant);
    this->Walker.Register(&CodeFormatter::FormatDelete);
    this->Walker.Register(&CodeFormatter::FormatReturn);
    this->Walker.Register(&CodeFormatter::FormatIf);
    this->Walker.Register(&CodeFormatter::FormatSendsEvent);
    this->Walker.Register(&CodeFormatter::FormatBreak);
    this->Walker.Register(&CodeFormatter::FormatDebugBreak);
    this->Walker.Register(&CodeFormatter::FormatContinue);
    this->Walker.Register(&CodeFormatter::FormatLoop);
    this->Walker.Register(&CodeFormatter::FormatWhile);
    this->Walker.Register(&CodeFormatter::FormatDoWhile);
    this->Walker.Register(&CodeFormatter::FormatFor);
    // this->Walker.RegisterDerived<ForEachNode>(&CodeFormatter::FormatFor);
    this->Walker.Register(&CodeFormatter::FormatForEach);
    this->Walker.Register(&CodeFormatter::FormatFunction);
    /*Initializer*/
    this->Walker.Register(&CodeFormatter::FormatConstructor);
    this->Walker.Register(&CodeFormatter::FormatDestructor);
    this->Walker.Register(&CodeFormatter::FormatClass);
    //this->Walker.Register(&CodeFormatter::FormatTypeDefine);
    this->Walker.RegisterDerived<LocalVariableReferenceNode>(&CodeFormatter::FormatValue);
    this->Walker.Register(&CodeFormatter::FormatThrow);
    this->Walker.Register(&CodeFormatter::FormatTypeId);
    this->Walker.Register(&CodeFormatter::FormatEnumValue);
    this->Walker.Register(&CodeFormatter::FormatEnum);
    
    // This must come after all statements and expressions are registered
    this->Walker.RegisterNonLeafBase(&CodeFormatter::FormatStatement);
  }

  //***************************************************************************
  String CodeFormatter::FormatTree(SyntaxTree& syntaxTree, const CodeFormat& format)
  {
    // Create the context and setup the format rules
    CodeFormatterContext context;
    context.Builder.Format = format;

    // Get the nodes in the order they were declared
    NodeList<SyntaxNode>& inOrderNodes = syntaxTree.Root->NonTraversedNonOwnedNodesInOrder;

    // Walk the given syntax tree and emit code for each type of node
    for (size_t i = 0; i < inOrderNodes.Size(); ++i)
    {
      // Traverse the current node
      SyntaxNode* childNode = inOrderNodes[i];
      this->Walker.Walk(this, childNode, &context);
    }

    // Finally compact the string builder into a single string
    return context.Builder.ToString();
  }

  //***************************************************************************
  bool CodeFormatter::IsDirectlyWithinScope(SyntaxNode* node)
  {
    // Note: This should be solved with interfaces or virtual functions
    if (Type::DynamicCast<RootNode*>(node->Parent) != nullptr)
      return true;

    if (Type::DynamicCast<ClassNode*>(node->Parent) != nullptr)
      return true;

    if (Type::DynamicCast<EnumNode*>(node->Parent) != nullptr)
      return true;

    return StatementNode::IsNodeUsedAsStatement(node);
  }

  //***************************************************************************
  size_t CodeFormatter::CountAttributes(SyntaxNode* node)
  {
    // Note: This should be solved with interfaces or virtual functions
    if (LocalVariableNode* attributeParent = Type::DynamicCast<LocalVariableNode*>(node))
    {
      return attributeParent->Attributes.Size();
    }

    if (MemberVariableNode* attributeParent = Type::DynamicCast<MemberVariableNode*>(node))
    {
      return attributeParent->Attributes.Size();
    }

    if (GenericFunctionNode* attributeParent = Type::DynamicCast<GenericFunctionNode*>(node))
    {
      return attributeParent->Attributes.Size();
    }

    if (ClassNode* attributeParent = Type::DynamicCast<ClassNode*>(node))
    {
      return attributeParent->Attributes.Size();
    }

    if (EnumNode* attributeParent = Type::DynamicCast<EnumNode*>(node))
    {
      return attributeParent->Attributes.Size();
    }

    return 0;
  }
  
  //***************************************************************************
  void CodeFormatter::FormatCommentsAndLines(SyntaxNode*& node, CodeFormatterContext* context)
  {
    // This will always run, even if other handlers will handle it
    
    ZilchCodeBuilder& builder = context->Builder;
    
    if (IsDirectlyWithinScope(node))
    {
      bool scopeFound = false;

      for (int i = (int)(context->FormatScopes.Size() - 1); i >= 0; --i)
      {
        ScopeLastNode& scope = context->FormatScopes[i];

        if (scope.AssociatedScope == node->Parent)
        {
          scopeFound = true;
          break;
        }
      }

      ScopeLastNode* foundScope = nullptr;

      if (scopeFound)
      {
        while (context->FormatScopes.Back().AssociatedScope != node->Parent)
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

      if (foundScope->LastNode != nullptr)
      {
        lineDifference = (int)(node->Location.StartLine - foundScope->LastNode->Location.EndLine);
      }
      
      lineDifference -= (int)node->Comments.Size();
      lineDifference -= (int)CountAttributes(node);

      // Since we will always emit one line, we skip one
      for (int i = 1; i < lineDifference; ++i)
      {
        builder.WriteLineIndented();
      }

      foundScope->LastNode = node;
    }


    //if (Type::DynamicCast<ClassNode*>(node->Parent) != nullptr)
    //{
    //  
    //  if (context->LastMember != nullptr)
    //  {
    //    
    //  }
    //}
    //else if (Type::DynamicCast<RootNode*>(node->Parent) != nullptr)
    //{
    //  if (context->LastClassOrEnum != nullptr)
    //  {
    //    lineDifference = (int)(node->Location.StartLine - context->LastClassOrEnum->Location.EndLine);
    //  }
    //}
    //else if (StatementNode::IsNodeUsedAsStatement(node))
    //{
    //  if (context->LastStatement != nullptr)
    //  {
    //    lineDifference = node->Location.StartLine - context->LastStatement->Location.EndLine;
    //  }
    //}



    for (size_t i = 0; i < node->Comments.Size(); ++i)
    {
      String& comment = node->Comments[i];

      builder.WriteLineIndented();

      builder.WriteKeywordOrSymbol(Grammar::CommentLine);

      size_t leadingSpaces = 0;
      StringRange range = comment.All();
      while (range.Empty() == false)
      {
        Rune rune = range.Front();
        range.PopFront();

        if (rune.mValue == ' ')
          ++leadingSpaces;
        else
          break;
      }

      range = comment.All();

      if (builder.Format.SpaceAfterComment)
      {
        if (leadingSpaces == 0)
        {
          builder.WriteSpace();
        }
      }
      else
      {
        if (leadingSpaces == 1)
        {
          range.IncrementByRune();
        }
      }
      
      builder.Write(range);
    }

    if (StatementNode::IsNodeUsedAsStatement(node))
    {
      // The only exception we have to the rule of 'you must write a line before you write yourself'
      // is in expressions, which we don't know if they are going to be used as an expression or a statement
      // Therefore we handle newlines before expressions here
      builder.WriteLineIndented();
    }

    // If someone else ends up handling the children, that is fine
    // but we need to explicitly say that we didn't handle any children
    context->Flags = WalkerFlags::ChildrenNotHandled;
  }

  //***************************************************************************
  void CodeFormatter::FormatDelete(DeleteNode*& node, CodeFormatterContext* context)
  {
    ZilchCodeBuilder& builder = context->Builder;

    builder.WriteKeywordOrSymbol(Grammar::Delete);
    builder.WriteSpace();
    context->Walker->Walk(this, node->DeletedObject, context);
  }

  //***************************************************************************
  void CodeFormatter::FormatBreak(BreakNode*& node, CodeFormatterContext* context)
  {
    ZilchCodeBuilder& builder = context->Builder;
    builder.WriteKeywordOrSymbol(Grammar::Break);
  }

  //***************************************************************************
  void CodeFormatter::FormatDebugBreak(DebugBreakNode*& node, CodeFormatterContext* context)
  {
    ZilchCodeBuilder& builder = context->Builder;
    builder.WriteKeywordOrSymbol(Grammar::Debug);
    builder.WriteSpace();
    builder.WriteKeywordOrSymbol(Grammar::Break);
  }

  //***************************************************************************
  void CodeFormatter::FormatThrow(ThrowNode*& node, CodeFormatterContext* context)
  {
    ZilchCodeBuilder& builder = context->Builder;
    builder.WriteKeywordOrSymbol(Grammar::Throw);
    builder.WriteSpace();
    context->Walker->Walk(this, node->Exception, context);
  }

  //***************************************************************************
  void CodeFormatter::FormatContinue(ContinueNode*& node, CodeFormatterContext* context)
  {
    ZilchCodeBuilder& builder = context->Builder;
    builder.WriteKeywordOrSymbol(Grammar::Continue);
  }

  //***************************************************************************
  void CodeFormatter::FormatForEach(ForEachNode*& node, CodeFormatterContext* context)
  {
    ZilchCodeBuilder& builder = context->Builder;
    builder.WriteKeywordOrSymbol(Grammar::ForEach);
    
    // HACK, this needs a space style!
    builder.WriteKeywordOrSymbolSpaceStyle(Grammar::BeginGroup, SpaceStyle::UseGlobalDefault, SpaceStyle::Before);

    if (node->NonTraversedVariable != nullptr)
    {
      // We don't want to do the standard walk here, because it would treat it as a statement!
      this->FormatLocalVariable(node->NonTraversedVariable, context);
    }

    builder.WriteSpace();
    builder.WriteKeywordOrSymbol(Grammar::In);
    builder.WriteSpace();
    
    if (node->NonTraversedRange != nullptr)
    {
      context->Walker->Walk(this, node->NonTraversedRange, context);
    }

    // HACK, this needs a space style!
    builder.WriteKeywordOrSymbolSpaceStyle(Grammar::EndGroup, SpaceStyle::UseGlobalDefault, SpaceStyle::None);

    builder.BeginScope(ScopeType::Block);
    context->Walker->Walk(this, node->Statements, context);
    builder.EndScope();
  }

  //***************************************************************************
  void CodeFormatter::FormatFor(ForNode*& node, CodeFormatterContext* context)
  {
    ZilchCodeBuilder& builder = context->Builder;
    builder.WriteKeywordOrSymbol(Grammar::For);
    
    // HACK, this needs a space style!
    builder.WriteKeywordOrSymbolSpaceStyle(Grammar::BeginGroup, SpaceStyle::UseGlobalDefault, SpaceStyle::Before);

    if (node->Initialization != nullptr)
    {
      context->Walker->Walk(this, node->Initialization, context);
    }
    
    if (node->ValueVariable != nullptr)
    {
      // We don't want to do the standard walk here, because it would treat it as a statement!
      this->FormatLocalVariable(node->ValueVariable, context);
    }
    
    // HACK, this needs a space style!
    builder.WriteKeywordOrSymbolSpaceStyle(Grammar::StatementSeparator, SpaceStyle::UseGlobalDefault, SpaceStyle::After);

    if (node->Condition != nullptr)
    {
      context->Walker->Walk(this, node->Condition, context);
    }
    
    // HACK, this needs a space style!
    builder.WriteKeywordOrSymbolSpaceStyle(Grammar::StatementSeparator, SpaceStyle::UseGlobalDefault, SpaceStyle::After);

    if (node->Iterator != nullptr)
    {
      context->Walker->Walk(this, node->Iterator, context);
    }

    // HACK, this needs a space style!
    builder.WriteKeywordOrSymbolSpaceStyle(Grammar::EndGroup, SpaceStyle::UseGlobalDefault, SpaceStyle::None);

    builder.BeginScope(ScopeType::Block);
    context->Walker->Walk(this, node->Statements, context);
    builder.EndScope();
  }

  //***************************************************************************
  void CodeFormatter::FormatDoWhile(DoWhileNode*& node, CodeFormatterContext* context)
  {
    ZilchCodeBuilder& builder = context->Builder;
    
    builder.WriteKeywordOrSymbol(Grammar::Do);

    builder.BeginScope(ScopeType::Block);
    context->Walker->Walk(this, node->Statements, context);
    builder.EndScope();

    // HACK, may need line style here...
    builder.WriteLineIndented();
    builder.WriteKeywordOrSymbol(Grammar::While);

    // HACK, this needs a space style!
    builder.WriteKeywordOrSymbolSpaceStyle(Grammar::BeginGroup, SpaceStyle::UseGlobalDefault, SpaceStyle::Before);
    context->Walker->Walk(this, node->Condition, context);
    // HACK, this needs a space style!
    builder.WriteKeywordOrSymbolSpaceStyle(Grammar::EndGroup, SpaceStyle::UseGlobalDefault, SpaceStyle::None);
  }

  //***************************************************************************
  void CodeFormatter::FormatLoop(LoopNode*& node, CodeFormatterContext* context)
  {
    ZilchCodeBuilder& builder = context->Builder;
    
    builder.WriteKeywordOrSymbol(Grammar::Loop);
    builder.BeginScope(ScopeType::Block);
    context->Walker->Walk(this, node->Statements, context);
    builder.EndScope();
  }

  //***************************************************************************
  void CodeFormatter::FormatWhile(WhileNode*& node, CodeFormatterContext* context)
  {
    ZilchCodeBuilder& builder = context->Builder;
    
    builder.WriteKeywordOrSymbol(Grammar::While);

    // HACK, this needs a space style!
    builder.WriteKeywordOrSymbolSpaceStyle(Grammar::BeginGroup, SpaceStyle::UseGlobalDefault, SpaceStyle::Before);
    context->Walker->Walk(this, node->Condition, context);
    // HACK, this needs a space style!
    builder.WriteKeywordOrSymbolSpaceStyle(Grammar::EndGroup, SpaceStyle::UseGlobalDefault, SpaceStyle::None);

    builder.BeginScope(ScopeType::Block);
    context->Walker->Walk(this, node->Statements, context);
    builder.EndScope();
  }

  //***************************************************************************
  void CodeFormatter::FormatIf(IfNode*& node, CodeFormatterContext* context)
  {
    ZilchCodeBuilder& builder = context->Builder;

    if (node->IsFirstPart == false)
    {
      builder.WriteKeywordOrSymbol(Grammar::Else);
    }

    if (node->Condition != nullptr)
    {
      if (node->IsFirstPart == false)
      {
        builder.WriteSpace();
      }

      builder.WriteKeywordOrSymbol(Grammar::If);
      // HACK, this needs a space style!
      builder.WriteKeywordOrSymbolSpaceStyle(Grammar::BeginGroup, SpaceStyle::UseGlobalDefault, SpaceStyle::Before);
      context->Walker->Walk(this, node->Condition, context);
      // HACK, this needs a space style!
      builder.WriteKeywordOrSymbolSpaceStyle(Grammar::EndGroup, SpaceStyle::UseGlobalDefault, SpaceStyle::None);
    }

    builder.BeginScope(ScopeType::Block);
    context->Walker->Walk(this, node->Statements, context);
    builder.EndScope();
  }

  //***************************************************************************
  void CodeFormatter::FormatReturn(ReturnNode*& node, CodeFormatterContext* context)
  {
    ZilchCodeBuilder& builder = context->Builder;
    
    builder.WriteKeywordOrSymbol(Grammar::Return);
    
    if (node->ReturnValue != nullptr)
    {
      builder.WriteSpace();
      context->Walker->Walk(this, node->ReturnValue, context);
    }
  }

  //***************************************************************************
  void CodeFormatter::FormatTypeId(TypeIdNode*& node, CodeFormatterContext* context)
  {
    ZilchCodeBuilder& builder = context->Builder;
    builder.WriteKeywordOrSymbol(Grammar::TypeId);
    
    // HACK, this needs a space style!
    // NOTE: Probably the same space style as function calling!
    builder.WriteKeywordOrSymbol(Grammar::BeginFunctionCall);

    if (node->Value != nullptr)
    {
      context->Walker->Walk(this, node->Value, context);
    }
    else
    {
      builder.Write(node->CompileTimeSyntaxType->ToString());
    }

    // HACK, this needs a space style!
    builder.WriteKeywordOrSymbol(Grammar::EndFunctionCall);
  }

  //***************************************************************************
  void CodeFormatter::FormatStringInterpolant(StringInterpolantNode*& node, CodeFormatterContext* context)
  {
    // The string interpolant doesn't really need to do anything,
    // just walk it's children (which are expressions and string literals)
    ZilchCodeBuilder& builder = context->Builder;
    context->Walker->GenericWalkChildren(this, node, context);
  }

  //***************************************************************************
  void CodeFormatter::FormatValue(ValueNode*& node, CodeFormatterContext* context)
  {
    ZilchCodeBuilder& builder = context->Builder;
    builder.Write(node->Value.Token);
  }

  //***************************************************************************
  void CodeFormatter::FormatUnaryOperator(UnaryOperatorNode*& node, CodeFormatterContext* context)
  {
    // Note: Our formatter makes the assumption that all unary operators are to the left
    ZilchCodeBuilder& builder = context->Builder;

    // HACK, this needs a space style!
    builder.WriteKeywordOrSymbolSpaceStyle(node->Operator->TokenId, SpaceStyle::UseGlobalDefault, SpaceStyle::None);
    context->Walker->Walk(this, node->Operand, context);
  }

  //***************************************************************************
  void CodeFormatter::FormatTypeCast(TypeCastNode*& node, CodeFormatterContext* context)
  {
    ZilchCodeBuilder& builder = context->Builder;

    context->Walker->Walk(this, node->Operand, context);
    
    // HACK, this needs a space style!
    builder.WriteKeywordOrSymbolSpaceStyle(Grammar::As, SpaceStyle::UseGlobalDefault, SpaceStyle::BeforeAndAfter);

    builder.Write(node->Type->ToString());
  }

  //***************************************************************************
  void CodeFormatter::FormatIndexerCall(IndexerCallNode*& node, CodeFormatterContext* context)
  {
    ZilchCodeBuilder& builder = context->Builder;
    
    context->Walker->Walk(this, node->LeftOperand, context);

    // HACK, this needs a space style!
    builder.WriteKeywordOrSymbolSpaceStyle(Grammar::BeginIndex, SpaceStyle::UseGlobalDefault, SpaceStyle::None);

    for (size_t i = 0; i < node->Arguments.Size(); ++i)
    {
      ExpressionNode* argument = node->Arguments[i];

      context->Walker->Walk(this, argument, context);
      
      bool isNotLast = (i != (node->Arguments.Size() - 1));
      if (isNotLast)
      {
        // HACK, this needs a space style!
        builder.WriteKeywordOrSymbolSpaceStyle(Grammar::ArgumentSeparator, SpaceStyle::UseGlobalDefault, builder.Format.SpaceStyleGlobalDefaultComma);
      }
    }

    // HACK, this needs a space style!
    builder.WriteKeywordOrSymbolSpaceStyle(Grammar::EndIndex, SpaceStyle::UseGlobalDefault, SpaceStyle::None);
  }

  //***************************************************************************
  void CodeFormatter::FormatBinaryOperator(BinaryOperatorNode*& node, CodeFormatterContext* context)
  {
    ZilchCodeBuilder& builder = context->Builder;

    // We only need a grouping operator if our parent is another binary operator with a higher precedence
    bool needsGroup = false;

    BinaryOperatorNode* parentBinOp = Type::DynamicCast<BinaryOperatorNode*>(node->Parent);
    if (parentBinOp != nullptr)
    {
      Shared& shared = Shared::GetInstance();

      UntypedOperator ourPrecedence = shared.GetOperatorPrecedence(node->Operator->TokenId, OperatorArity::Binary);
      UntypedOperator parentPrecedence = shared.GetOperatorPrecedence(parentBinOp->Operator->TokenId, OperatorArity::Binary);

      // If the parent has a higher precedence, then we need to wrap ourselves in a group
      // Note: Higher precedence is denoted by a lower number, which is silly but seems to be how
      // other languages do it (http://en.cppreference.com/w/cpp/language/operator_precedence)
      if (parentPrecedence.Precedence < ourPrecedence.Precedence)
      {
        needsGroup = true;
      }
      // If we have the same level of precedence, then there's a chance we may need parenthesis based on
      else if (parentPrecedence.Precedence == ourPrecedence.Precedence)
      {
        // Technically, if the operator is actually associative (as in math associative) we can ignore
        // grouping when it's parent operator is the same *unless* we allow operator overloading to change that

        // If we have a left to right associativity
        if (parentPrecedence.Associativity == OperatorAssociativity::LeftToRight)
        {
          if (parentBinOp->RightOperand == node)
          {
            needsGroup = true;
          }
        }
        else
        {
          if (parentBinOp->LeftOperand == node)
          {
            needsGroup = true;
          }
        }
      }
    }

    if (needsGroup)
    {
      // HACK, this needs a space style!
      builder.WriteKeywordOrSymbolSpaceStyle(Grammar::BeginGroup, SpaceStyle::UseGlobalDefault, SpaceStyle::None);
    }

    context->Walker->Walk(this, node->LeftOperand, context);

    SpaceStyle::Enum globalOperatorSpaceStyle = SpaceStyle::BeforeAndAfter;

    // HACK, maybe every operator needs it's own spacing, or access operators have a special mode
    if (node->Operator->TokenId == Grammar::Access          ||
        node->Operator->TokenId == Grammar::DynamicAccess   ||
        node->Operator->TokenId == Grammar::NonVirtualAccess)
    {
      globalOperatorSpaceStyle = SpaceStyle::None;
    }

    // HACK, this needs a space style!
    builder.WriteKeywordOrSymbolSpaceStyle(node->Operator->TokenId, SpaceStyle::UseGlobalDefault, globalOperatorSpaceStyle);
    
    context->Walker->Walk(this, node->RightOperand, context);

    if (needsGroup)
    {
      // HACK, this needs a space style!
      builder.WriteKeywordOrSymbolSpaceStyle(Grammar::EndGroup, SpaceStyle::UseGlobalDefault, SpaceStyle::None);
    }
  }
  
  //***************************************************************************
  void CodeFormatter::FormatStatement(StatementNode*& node, CodeFormatterContext* context)
  {
    // This will always run, even if other handlers will handle it
    
    // If this node is not being used as a direct statement
    // For example, an expression is a statement, but isn't always used as a statement
    if (StatementNode::IsNodeUsedAsStatement(node) == false)
      return;

    ZilchCodeBuilder& builder = context->Builder;
    
    // As long as the statement isn't a scoped based node (if, for, while, etc)
    // then we know it requires delimiting
    if (Type::DynamicCast<ScopeNode*>(node) == nullptr)
    {
      builder.WriteKeywordOrSymbol(Grammar::StatementSeparator);
    }

    // If someone else ends up handling the children, that is fine
    // but we need to explicitly say that we didn't handle any children
    context->Flags = WalkerFlags::ChildrenNotHandled;
  }

  //***************************************************************************
  void CodeFormatter::FormatClass(ClassNode*& node, CodeFormatterContext* context)
  {
    if (node->TemplateInstantiation != nullptr)
      return;
    
    ZilchCodeBuilder& builder = context->Builder;

    this->FormatAttributes(node->Attributes, builder);

    builder.WriteLineIndented();

    builder.WriteKeywordOrSymbol(Grammar::Class);

    builder.WriteSpace();
    builder.Write(node->Name);

    if (node->Inheritance.Empty() == false)
    {
      builder.WriteKeywordOrSymbolSpaceStyle
      (
        Grammar::Inheritance,
        builder.Format.SpaceStyleInheritanceColon,
        builder.Format.SpaceStyleGlobalDefaultColon
      );

      for (size_t i = 0; i < node->Inheritance.Size(); ++i)
      {
        SyntaxType* syntaxType = node->Inheritance[i];

        builder.Write(syntaxType->ToString());
      
        bool isNotLast = (i != (node->Inheritance.Size() - 1));
        if (isNotLast)
        {
          builder.WriteKeywordOrSymbolSpaceStyle
          (
            Grammar::ArgumentSeparator,
            builder.Format.SpaceStyleInheritanceComma,
            builder.Format.SpaceStyleGlobalDefaultComma
          );
        }
      }
    }

    builder.BeginScope(ScopeType::Class);

    // Get the nodes in the order they were declared
    NodeList<SyntaxNode>& inOrderNodes = node->NonTraversedNonOwnedNodesInOrder;

    // Walk the given syntax tree and emit code for each type of node
    for (size_t i = 0; i < inOrderNodes.Size(); ++i)
    {
      // Traverse the current node
      SyntaxNode* childNode = inOrderNodes[i];
      this->Walker.Walk(this, childNode, context);
    }

    builder.EndScope();
  }
  
  //***************************************************************************
  void CodeFormatter::FormatAttributes(NodeList<AttributeNode>& attributes, ZilchCodeBuilder& builder)
  {
    if (attributes.Empty() == false)
    {
      builder.WriteLineIndented();
    }

    for (size_t i = 0; i < attributes.Size(); ++i)
    {
      AttributeNode* attribute = attributes[i];
      builder.WriteKeywordOrSymbol(Grammar::BeginIndex);
      builder.Write(attribute->TypeName->Token);
      builder.WriteKeywordOrSymbol(Grammar::EndIndex);
    }
  }

  //***************************************************************************
  void CodeFormatter::FormatEnum(EnumNode*& node, CodeFormatterContext* context)
  {
    ZilchCodeBuilder& builder = context->Builder;

    this->FormatAttributes(node->Attributes, builder);

    builder.WriteLineIndented();

    if (node->IsFlags)
    {
      builder.WriteKeywordOrSymbol(Grammar::Flags);
    }
    else
    {
      builder.WriteKeywordOrSymbol(Grammar::Enumeration);
    }

    builder.WriteSpace();
    builder.Write(node->Name);

    if (node->Inheritance != nullptr)
    {
      builder.WriteKeywordOrSymbolSpaceStyle
      (
        Grammar::Inheritance,
        builder.Format.SpaceStyleInheritanceColon,
        builder.Format.SpaceStyleGlobalDefaultColon
      );

      builder.Write(node->Inheritance->ToString());
    }

    builder.BeginScope(ScopeType::Enumeration);

    // Walk all the values defined in the enum (in order)
    for (size_t i = 0; i < node->Values.Size(); ++i)
    {
      EnumValueNode* enumValueNode = node->Values[i];
      this->Walker.Walk(this, enumValueNode, context);
      // HACK, need a style for 'trailing comma'
      builder.WriteKeywordOrSymbol(Grammar::ArgumentSeparator);
    }

    builder.EndScope();
  }

  //***************************************************************************
  void CodeFormatter::FormatEnumValue(EnumValueNode*& node, CodeFormatterContext* context)
  {
    ZilchCodeBuilder& builder = context->Builder;
    builder.WriteLineIndented();
    
    builder.Write(node->Name);
    if (node->Value != nullptr)
    {
      // HACK, needs space style
      builder.WriteKeywordOrSymbolSpaceStyle
      (
        Grammar::Assignment,
        SpaceStyle::UseGlobalDefault,
        SpaceStyle::BeforeAndAfter
      );
      builder.Write(node->Value->Token);
    }
  }

  //***************************************************************************
  // NodeType is a GenericFunctionNode
  template <typename NodeType, typename FunctionType>
  void CodeFormatter::FormatGenericFunctionHelper(NodeType* node, CodeFormatterContext* context, FunctionType emitPostArgs)
  {
    ZilchCodeBuilder& builder = context->Builder;

    // HACK, this needs a space style!
    builder.WriteKeywordOrSymbol(Grammar::BeginFunctionParameters);

    for (size_t i = 0; i < node->Parameters.Size(); ++i)
    {
      ParameterNode* parameter = node->Parameters[i];

      builder.Write(parameter->Name);

      builder.WriteKeywordOrSymbolSpaceStyle
      (
        Grammar::TypeSpecifier,
        builder.Format.SpaceStyleTypeColon,
        builder.Format.SpaceStyleGlobalDefaultColon
      );

      builder.Write(parameter->ResultSyntaxType->ToString());

      bool isNotLast = (i != (node->Parameters.Size() - 1));
      if (isNotLast)
      {
        builder.WriteKeywordOrSymbolSpaceStyle
        (
          Grammar::ArgumentSeparator,
          builder.Format.SpaceStyleFunctionDefinitionParameterComma,
          builder.Format.SpaceStyleGlobalDefaultComma
        );
      }
    }

    // HACK, this needs a space style!
    builder.WriteKeywordOrSymbol(Grammar::EndFunctionParameters);

    emitPostArgs(node, builder);

    builder.BeginScope(ScopeType::Function);

    for (size_t i = 0; i < node->Statements.Size(); ++i)
    {
      StatementNode* statement = node->Statements[i];
      context->Walker->Walk(this, statement, context);
    }

    builder.EndScope();
  }
  
  //***************************************************************************
  void FormatFunctionPostArgs(FunctionNode* node, ZilchCodeBuilder& builder)
  {
    if (node->ReturnType != nullptr)
    {
      builder.WriteKeywordOrSymbolSpaceStyle
      (
        Grammar::TypeSpecifier,
        builder.Format.SpaceStyleTypeColon,
        builder.Format.SpaceStyleGlobalDefaultColon
      );

      builder.Write(node->ReturnType->ToString());
    }
  }

  //***************************************************************************
  void CodeFormatter::FormatFunction(FunctionNode*& node, CodeFormatterContext* context)
  {
    ZilchCodeBuilder& builder = context->Builder;
    
    this->FormatAttributes(node->Attributes, builder);

    builder.WriteLineIndented();

    builder.WriteKeywordOrSymbol(Grammar::Function);

    builder.WriteSpace();
    builder.Write(node->Name);
    
    this->FormatGenericFunctionHelper(node, context, FormatFunctionPostArgs);
  }

  //***************************************************************************
  void FormatConstructorPostArgs(ConstructorNode* node, ZilchCodeBuilder& builder)
  {
  }

  //***************************************************************************
  void CodeFormatter::FormatConstructor(ConstructorNode*& node, CodeFormatterContext* context)
  {
    ZilchCodeBuilder& builder = context->Builder;
    
    this->FormatAttributes(node->Attributes, builder);

    builder.WriteLineIndented();

    builder.WriteKeywordOrSymbol(Grammar::Constructor);
    
    this->FormatGenericFunctionHelper(node, context, FormatConstructorPostArgs);
  }

  //***************************************************************************
  void CodeFormatter::FormatSendsEvent(SendsEventNode*& node, CodeFormatterContext* context)
  {
    ZilchCodeBuilder& builder = context->Builder;
    
    builder.WriteLineIndented();
    builder.WriteKeywordOrSymbol(Grammar::Sends);
    builder.WriteSpace();
    builder.Write(node->Name->Token);
    builder.WriteKeywordOrSymbolSpaceStyle
    (
      Grammar::TypeSpecifier,
      builder.Format.SpaceStyleNamedArgumentColon,
      builder.Format.SpaceStyleGlobalDefaultColon
    );
    builder.Write(node->EventType->ToString());
    builder.WriteKeywordOrSymbol(Grammar::StatementSeparator);
  }

  //***************************************************************************
  void FormatDestructorPostArgs(DestructorNode* node, ZilchCodeBuilder& builder)
  {
  }

  //***************************************************************************
  void CodeFormatter::FormatDestructor(DestructorNode*& node, CodeFormatterContext* context)
  {
    ZilchCodeBuilder& builder = context->Builder;

    this->FormatAttributes(node->Attributes, builder);

    builder.WriteLineIndented();
    builder.WriteKeywordOrSymbol(Grammar::Destructor);
    
    this->FormatGenericFunctionHelper(node, context, FormatDestructorPostArgs);
  }

  //***************************************************************************
  void CodeFormatter::FormatFunctionCall(FunctionCallNode*& node, CodeFormatterContext* context)
  {
    ZilchCodeBuilder& builder = context->Builder;

    context->Walker->Walk(this, node->LeftOperand, context);

    // HACK, this needs a space style!
    builder.WriteKeywordOrSymbol(Grammar::BeginFunctionCall);

    for (size_t i = 0; i < node->Arguments.Size(); ++i)
    {
      ExpressionNode* expression = node->Arguments[i];

      if (node->ArgumentNames.Empty() == false)
      {
        String& name = node->ArgumentNames[i];
        builder.Write(name);

        builder.WriteKeywordOrSymbolSpaceStyle
        (
          Grammar::TypeSpecifier,
          builder.Format.SpaceStyleNamedArgumentColon,
          builder.Format.SpaceStyleGlobalDefaultColon
        );
      }
      
      context->Walker->Walk(this, expression, context);

      bool isNotLast = (i != (node->Arguments.Size() - 1));
      if (isNotLast)
      {
        builder.WriteKeywordOrSymbolSpaceStyle
        (
          Grammar::ArgumentSeparator,
          builder.Format.SpaceStyleFunctionCallParameterComma,
          builder.Format.SpaceStyleGlobalDefaultComma
        );
      }
    }

    // HACK, this needs a space style!
    builder.WriteKeywordOrSymbol(Grammar::EndFunctionCall);
  }

  //***************************************************************************
  void CodeFormatter::FormatMemberAccess(MemberAccessNode*& node, CodeFormatterContext* context)
  {
    ZilchCodeBuilder& builder = context->Builder;

    context->Walker->Walk(this, node->LeftOperand, context);
    builder.WriteKeywordOrSymbol(node->Operator);
    builder.Write(node->Name);
  }

  //***************************************************************************
  void CodeFormatter::FormatMemberVariable(MemberVariableNode*& node, CodeFormatterContext* context)
  {
    ZilchCodeBuilder& builder = context->Builder;
    this->FormatAttributes(node->Attributes, builder);
    
    builder.WriteLineIndented();
    builder.WriteKeywordOrSymbol(Grammar::Variable);
    builder.WriteSpace();
    builder.Write(node->Name);

    if (node->IsInferred() == false)
    {
      builder.WriteKeywordOrSymbolSpaceStyle
      (
        Grammar::TypeSpecifier,
        builder.Format.SpaceStyleTypeColon,
        builder.Format.SpaceStyleGlobalDefaultColon
      );
      builder.Write(node->ResultSyntaxType->ToString());
    }

    if (node->IsGetterSetter)
    {
      builder.BeginScope(ScopeType::Property);

      // Note: We don't actually want to walk the function nodes,
      // because they will emit function signatures, instead we just walk their statements
      // We also don't want to use 'GenericWalkChildren' because that will walk parameters

      if (node->Get != nullptr)
      {
        builder.WriteLineIndented();
        builder.WriteKeywordOrSymbol(Grammar::Get);
        builder.BeginScope(ScopeType::GetSet);
        context->Walker->Walk(this, node->Get->Statements, context);
        builder.EndScope();
      }
      
      if (node->Set != nullptr)
      {
        builder.WriteLineIndented();
        builder.WriteKeywordOrSymbol(Grammar::Set);
        builder.BeginScope(ScopeType::GetSet);
        context->Walker->Walk(this, node->Set->Statements, context);
        builder.EndScope();
      }
      
      builder.EndScope();
    }
    else if (node->InitialValue != nullptr)
    {
      // HACK, this needs a space style! 
      // PROBABLY BINARY OPERATOR SPACE STYLE
      builder.WriteKeywordOrSymbolSpaceStyle
      (
        Grammar::Assignment,
        SpaceStyle::UseGlobalDefault,
        SpaceStyle::BeforeAndAfter
      );
      context->Walker->Walk(this, node->InitialValue, context);
      builder.WriteKeywordOrSymbol(Grammar::StatementSeparator);
    }
  }

  //***************************************************************************
  void CodeFormatter::FormatParameter(ParameterNode*& node, CodeFormatterContext* context)
  {
    ZilchCodeBuilder& builder = context->Builder;
    builder.Write(node->Name);
    builder.WriteKeywordOrSymbolSpaceStyle
    (
      Grammar::TypeSpecifier,
      builder.Format.SpaceStyleTypeColon,
      builder.Format.SpaceStyleGlobalDefaultColon
    );
    builder.Write(node->ResultSyntaxType->ToString());
  }

  //***************************************************************************
  void CodeFormatter::FormatLocalVariable(LocalVariableNode*& node, CodeFormatterContext* context)
  {
    ZilchCodeBuilder& builder = context->Builder;
    this->FormatAttributes(node->Attributes, builder);

    builder.WriteKeywordOrSymbol(Grammar::Variable);
    builder.WriteSpace();
    builder.Write(node->Name);

    if (node->IsInferred() == false)
    {
      builder.WriteKeywordOrSymbolSpaceStyle
      (
        Grammar::TypeSpecifier,
        builder.Format.SpaceStyleTypeColon,
        builder.Format.SpaceStyleGlobalDefaultColon
      );
      builder.Write(node->ResultSyntaxType->ToString());
    }

    // Some variable nodes don't have initial values, or defaults
    if (node->InitialValue != nullptr)
    {
      // HACK, this needs a space style! 
      // PROBABLY BINARY OPERATOR SPACE STYLE
      builder.WriteKeywordOrSymbolSpaceStyle
      (
        Grammar::Assignment,
        SpaceStyle::UseGlobalDefault,
        SpaceStyle::BeforeAndAfter
      );
      context->Walker->Walk(this, node->InitialValue, context);
    }
  }

  //***************************************************************************
  void CodeFormatter::FormatStaticTypeNode(StaticTypeNode*& node, CodeFormatterContext* context)
  {
    ZilchCodeBuilder& builder = context->Builder;

    // If it's inferred, we don't bother writing anything
    if (node->Mode == CreationMode::New)
    {
      builder.WriteKeywordOrSymbol(Grammar::New);
      builder.WriteSpace();
    }
    else if (node->Mode == CreationMode::Local)
    {
      builder.WriteKeywordOrSymbol(Grammar::Local);
      builder.WriteSpace();
    }

    builder.Write(node->ReferencedSyntaxType->ToString());
  }
}
