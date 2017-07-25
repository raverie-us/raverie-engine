// AttributeArgumentMustBeLiteral
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "All arguments given to an attribute must be literals (numbers, strings, true/false, typeid(Type), or null).";
  error.Name = "AttributeArgumentMustBeLiteral";
  error.Reason = String();

}

// AttributeNotComplete
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The attribute '%s' was not completed (missing ']').";
  error.Name = "AttributeNotComplete";
  error.Reason = String();

}

// AttributesNotAttached
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Attributes were left at the end of the scope of the program and could not be attached to anything.";
  error.Name = "AttributesNotAttached";
  error.Reason = String();

}

// AttributeTypeNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The attribute type was not found.";
  error.Name = "AttributeTypeNotFound";
  error.Reason = String();

}

// BaseClassInitializerMustComeFirst
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "To make the order of operations very clear, the base class initializer must come before any other initailizers (such as this).";
  error.Name = "BaseClassInitializerMustComeFirst";
  error.Reason = "Base classes are initialized before anything else, so it makes sense that it should actually come first.";

}

// BaseClassInitializerRequiresBaseClassInheritance
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "A base class initializer can only be called when your class '%s' inherits from another class.";
  error.Name = "BaseClassInitializerRequiresBaseClassInheritance";
  error.Reason = String();

}

// BaseClassMemberSameName
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The base class '%s' has a member of the same name '%s'. This is not allowed because it creates confusion for users and often breaks automated serialization patterns (fields and getter/setters only).";
  error.Name = "BaseClassMemberSameName";
  error.Reason = String();

}

// BaseInitializerRequired
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Since '%s' inherits from '%s' (the base type), we require a base initializer after the constructor.";
  error.Name = "BaseInitializerRequired";
  error.Reason = String();

}

// BinaryOperatorRightOperandNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The right operand is invalid or not found for the '%s' operator '%s'.";
  error.Name = "BinaryOperatorRightOperandNotFound";
  error.Reason = "When an operator between two operands, such as the '+' operator, has a left operand (e.g. '5 + ') it must have a right operand as well.";

  {
    ErrorExample& example = error.Examples.PushBack();
    example.ErrorCode = "1.0 + ;";
    example.FixedCode = "1.0 + 2.0;\r\n";
    example.ExplanationOfFix = "*TBD*";
  }

}

// BlockCommentNotComplete
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "A block comment started wasn't properly closed when we reached the end of the file.";
  error.Name = "BlockCommentNotComplete";
  error.Reason = "Commonly, users accidentally delete or mistype the end of a block comment, which can cause code to be unnecessarily commented out. This can be dangerous if it was unintentional, hence the error.";

  {
    ErrorExample& example = error.Examples.PushBack();
    example.ErrorCode = "var cat : Animal := new Cat();\r\n\r\n/*\r\nvar dog : Animal := new Dog();";
    example.FixedCode = "var cat : Animal := new Cat();\r\n\r\n/*\r\nvar dog : Animal := new Dog();\r\n*/";
    example.ExplanationOfFix = "In the error case, we started a block comment (/*) but never finished it with (*/).";
  }

}

// BlockCommentNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "A block comment end (*/) was found, but the beginning of the block comment (/*) was not found.";
  error.Name = "BlockCommentNotFound";
  error.Reason = String();

}

// BreakCountMustBeGreaterThanZero
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The break count is the number of loops we want to break out of, and therefore must be greater than or equal to 1.";
  error.Name = "BreakCountMustBeGreaterThanZero";
  error.Reason = String();

}

// BreakLoopNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "A break statement was used, but we couldn't find the loop that it was breaking out of.";
  error.Name = "BreakLoopNotFound";
  error.Reason = String();

}

// CannotCreateType
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The type '%s' can only be created outside of Zilch.";
  error.Name = "CannotCreateType";
  error.Reason = String();

}

// CannotReplaceTemplateInstanceWithNonTemplateType
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "We tried to replace a type in a template, but couldn't do it since the type was used as an actual template type, and the type passed in as a template argument was not templatable.";
  error.Name = "CannotReplaceTemplateInstanceWithNonTemplateType";
  error.Reason = String();

}

// CannotReplaceTemplateInstanceWithTemplateArguments
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Cannot replace a template instance with template arguments.";
  error.Name = "CannotReplaceTemplateInstanceWithTemplateArguments";
  error.Reason = String();

}

// CastTypeNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The casting operator 'as' is missing a cast type declaration (eg. x as Real).";
  error.Name = "CastTypeNotFound";
  error.Reason = "Casting allows us to convert one type to another. The 'as' operator was chosen because it gets rid of many unnecessary parentheses (eg C-style casts). The 'as' operator takes what we want to convert on the left and the type we want to convert it to on the right.";

  {
    ErrorExample& example = error.Examples.PushBack();
    example.ErrorCode = "class Animal\r\n{\r\n  function TestFn() : Real\r\n  {\r\n	return 5 as ;\r\n  }\r\n}";
    example.FixedCode = "class Animal\r\n{\r\n  function TestFn() : Real\r\n  {\r\n	return 5 as Real;\r\n  }\r\n}";
    example.ExplanationOfFix = "*TBD*";
  }

}

// ClassBodyNotComplete
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Class declaration '%s' does not have a closing '}'.";
  error.Name = "ClassBodyNotComplete";
  error.Reason = "In order for the compiler to understand where a class definition starts and ends, an opening ({) and closing (}) scope must be used. If the scope was not closed, the compiler would be forced to assume that the class encompasses all the code below it, which could be erroneous. There are a few likely cases that this could happen: the closing scope brace has been left off, a scope or parenthesis inside the class was not properly closed, or a variable or function declaration was garbled inside the class causing it to hault its parsing.";

  {
    ErrorExample& example = error.Examples.PushBack();
    example.ErrorCode = "class Animal\r\n{\r\n";
    example.FixedCode = "class Animal\r\n{\r\n}\r\n";
    example.ExplanationOfFix = "In the error case, we started the definition of the class 'Animal' but never closed the scope. We fixed it by simply adding a closing scope (}) to the end of the class.";
  }

}

// ClassBodyNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Class declaration '%s' does not have an opening '{'.";
  error.Name = "ClassBodyNotFound";
  error.Reason = "The compiler couldn't find the opening scope ({) for the class declaration, and therefore the class has no body. Check to make sure you added an opening scope and that the class name or inheritance list wasn't garbled. If you come from languages like C or C++ and you were trying to make a forward declaration, it is not required since the language globally shares all type-definitions between different scripts.";

  {
    ErrorExample& example = error.Examples.PushBack();
    example.ErrorCode = "class Animal\r\n";
    example.FixedCode = "class Animal\r\n{\r\n}";
    example.ExplanationOfFix = "In the error case, we forgot the opening scope ({). We fixed it by simply adding one in.";
  }

}

// ClassNameNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Class declaration is missing or invalid (we were unable to find the class name).";
  error.Name = "ClassNameNotFound";
  error.Reason = "Most likely the class name was invalid due to unnecessary symbols or to using a keyword in its place, or the class name was missing.";

  {
    ErrorExample& example = error.Examples.PushBack();
    example.ErrorCode = "class\r\n{\r\n}";
    example.FixedCode = "class Animal\r\n{\r\n}";
    example.ExplanationOfFix = "In the error case, we forgot to add the name of the class after the 'class' keyword. We fixed this by adding the name 'Animal' after the class, thus giving the type a name.";
  }

}

// CompositionCycleDetected
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "A cycle of composition was detected. Eg. Type A includes Type B, Type B includes Type C, and Type C includes Type A (which is illegal).";
  error.Name = "CompositionCycleDetected";
  error.Reason = String();

}

// ConditionMustBeABooleanType
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The expression resulted in a '%s'. Any condition must result in a Boolean (true/false) type.";
  error.Name = "ConditionMustBeABooleanType";
  error.Reason = String();

}

// ConstructorCallNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The new/local constructor call was not found.";
  error.Name = "ConstructorCallNotFound";
  error.Reason = String();

}

// ConstructorCannotAccessStaticMembers
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "You cannot access members on a type created with new/local";
  error.Name = "ConstructorCannotAccessStaticMembers";
  error.Reason = String();

}

// ContinueLoopNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "A continue statement was used, but we couldn't find the loop that it was continuing from.";
  error.Name = "ContinueLoopNotFound";
  error.Reason = String();

}

// CreatedTypeNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The new/local operator was missing a created type.";
  error.Name = "CreatedTypeNotFound";
  error.Reason = String();

}

// CreationInitializeMemberExpectedInitialValue
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "When initializing the member '%s', we need to specify a value expression to initialize to, eg '%s = 9'.";
  error.Name = "CreationInitializeMemberExpectedInitialValue";
  error.Reason = String();

}

// CreationInitializerExpectedSubElement
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Failed to parse a sub-element of the creation initializer. The sub-element can be a member initialization 'Name: Value', an expression that can be added to a container, or a list of expressions to be added '{Value1, Value2, ...}'.";
  error.Name = "CreationInitializerExpectedSubElement";
  error.Reason = String();

}

// CreationInitializerNotComplete
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Creation initializer read something it did not understand or is not properly closed with a '}'.";
  error.Name = "CreationInitializerNotComplete";
  error.Reason = String();

}

// CustomError
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "An error occurred: '%s'";
  error.Name = "CustomError";
  error.Reason = String();

}

// DelegateReturnTypeNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Delegate declaration is missing the return type declaration, yet the type specifier ':' was found.";
  error.Name = "DelegateReturnTypeNotFound";
  error.Reason = String();

}

// DeletingNonReferenceType
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Unable to delete a non reference type.";
  error.Name = "DeletingNonReferenceType";
  error.Reason = String();

}

// DeletingNonWritableValue
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Objects that are not writable cannot be deleted.";
  error.Name = "DeletingNonWritableValue";
  error.Reason = String();

}

// DoWhileConditionalExpressionNotComplete
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Do while statement conditional expression is not properly closed.";
  error.Name = "DoWhileConditionalExpressionNotComplete";
  error.Reason = String();

}

// DoWhileConditionalExpressionNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Do while statement conditional expression was not found.";
  error.Name = "DoWhileConditionalExpressionNotFound";
  error.Reason = String();

}

// DuplicateLocalVariableName
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "A local variable (or parameter) with the same name '%s' has already been defined (or you attempted to name your parameter a reserved name, such as 'this').";
  error.Name = "DuplicateLocalVariableName";
  error.Reason = String();

}

// DuplicateMemberName
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "A member with the same name '%s' has already been defined.";
  error.Name = "DuplicateMemberName";
  error.Reason = String();

}

// DuplicateTypeName
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "A type with the same name '%s' has already been defined in the '%s' library.";
  error.Name = "DuplicateTypeName";
  error.Reason = String();

}

// EnumBodyNotComplete
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Enum/flags declaration '%s' does not have a closing '}'.";
  error.Name = "EnumBodyNotComplete";
  error.Reason = String();

}

// EnumBodyNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Enum/flags declaration '%s' does not have an opening '{'.";
  error.Name = "EnumBodyNotFound";
  error.Reason = String();

}

// EnumDuplicateValue
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "A value of the same name '%s' has already been declared in the enum/flags '%s'. Names must only be used once.";
  error.Name = "EnumDuplicateValue";
  error.Reason = String();

}

// EnumNameNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Enum/flags declaration is missing a name.";
  error.Name = "EnumNameNotFound";
  error.Reason = String();

}

// EnumValueRequiresIntegerLiteral
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "When assigning a specific value to '%s' by using '=', you must specify an integer literal (e.g. 5).";
  error.Name = "EnumValueRequiresIntegerLiteral";
  error.Reason = String();

}

// ExternalTypeNamesCollide
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The type '%s' from the library '%s' collides with another type of the same name from the library '%s'. Currently this is always an error, but in the future it will not be an error until someone tries to reference that type in their code.";
  error.Name = "ExternalTypeNamesCollide";
  error.Reason = String();

}

// ForEachInKeywordNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The foreach loop requires an 'in' keyword, which specifies which range we're iterating through.";
  error.Name = "ForEachInKeywordNotFound";
  error.Reason = String();

}

// ForEachRangeExpressionNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The foreach loop expects a range expression after the 'in' keyword.";
  error.Name = "ForEachRangeExpressionNotFound";
  error.Reason = String();

}

// ForEachVariableDeclarationNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The variable declaration in a foreach loop was not found.";
  error.Name = "ForEachVariableDeclarationNotFound";
  error.Reason = String();

}

// ForLoopExpressionsNotComplete
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "For loop expressions were not complete.";
  error.Name = "ForLoopExpressionsNotComplete";
  error.Reason = String();

}

// ForLoopExpressionsNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "For loop expressions were not found.";
  error.Name = "ForLoopExpressionsNotFound";
  error.Reason = String();

}

// FunctionArgumentListNotComplete
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Function declaration '%s' has an invalid argument list.";
  error.Name = "FunctionArgumentListNotComplete";
  error.Reason = "Argument list in function declaration must end with a ')' for the compiler to know when it is finished.";

  {
    ErrorExample& example = error.Examples.PushBack();
    example.ErrorCode = "class Animal\r\n{\r\n  function TestFn(\r\n  {\r\n  }\r\n}";
    example.FixedCode = "class Animal\r\n{\r\n  function TestFn()\r\n  {\r\n  }\r\n}";
    example.ExplanationOfFix = "Just adding a ')' finishes the argument list.";
  }

}

// FunctionArgumentListNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Function declaration '%s' is missing the argument list opening '('.";
  error.Name = "FunctionArgumentListNotFound";
  error.Reason = "Function declarations must contain an argument list opening with '(' and ending with ')'.";

  {
    ErrorExample& example = error.Examples.PushBack();
    example.ErrorCode = "class Animal\r\n{\r\n  function TestFn\r\n  {\r\n  }\r\n}";
    example.FixedCode = "class Animal\r\n{\r\n  function TestFn()\r\n  {\r\n  }\r\n}";
    example.ExplanationOfFix = "Added an empty argument list, but an argument list nonetheless.";
  }

}

// FunctionBodyNotComplete
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Function declaration '%s' does not have a closing '}'.";
  error.Name = "FunctionBodyNotComplete";
  error.Reason = "For the compiler to know the entirety of a function, the function body needs to begin and end with curly braces.";

  {
    ErrorExample& example = error.Examples.PushBack();
    example.ErrorCode = "class Animal\r\n{\r\n  var Number : Integer := 9;\r\n\r\n  function TestFn() : Real\r\n  {\r\n    return 1.0;\r\n\r\n  function AnotherFn() : Real\r\n  {\r\n    return 1.0;\r\n  }\r\n}\r\n";
    example.FixedCode = "class Animal\r\n{\r\n  var Number : Integer := 9;\r\n\r\n  function TestFn() : Real\r\n  {\r\n    return 1.0;\r\n  }\r\n  function AnotherFn() : Real\r\n  {\r\n    return 1.0;\r\n  }\r\n}";
    example.ExplanationOfFix = "The solution to this case is to add the closing bracket to the TestFn.";
  }

}

// FunctionBodyNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Function declaration '%s' does not have an opening '{'.";
  error.Name = "FunctionBodyNotFound";
  error.Reason = "The compiler couldn't find the opening scope ({) for the function definition. Functions consist of a series of statements and expressions, and the compiler knows which of those belong to the function by what's inside the curly braces.";

  {
    ErrorExample& example = error.Examples.PushBack();
    example.ErrorCode = "class Animal\r\n{\r\n  function TestFn() : Real\r\n}\r\n";
    example.FixedCode = "class Animal\r\n{\r\n  function TestFn() : Real\r\n  {\r\n    return 1.0;\r\n  }\r\n}";
    example.ExplanationOfFix = "Adding the rest of the function, such as the braces and return call.";
  }

}

// FunctionCallExpectedAfterInitializer
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "We expect a function call after any initializer (base, this, member initialization, etc).";
  error.Name = "FunctionCallExpectedAfterInitializer";
  error.Reason = String();

}

// FunctionCallNamedArgumentNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The right hand side of a named argument '%s' is missing or invalid.";
  error.Name = "FunctionCallNamedArgumentNotFound";
  error.Reason = String();

}

// FunctionCallNotComplete
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "A function call was not properly closed with ')', or one of the inner expressions was invalid.";
  error.Name = "FunctionCallNotComplete";
  error.Reason = String();

}

// FunctionCallOnNonCallableType
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The left hand side of a function call () must be a callable type, like a function or delegate.";
  error.Name = "FunctionCallOnNonCallableType";
  error.Reason = String();

}

// FunctionNameNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Function declaration is missing a name.";
  error.Name = "FunctionNameNotFound";
  error.Reason = "Function must have a name in order to be called.";

  {
    ErrorExample& example = error.Examples.PushBack();
    example.ErrorCode = "class Animal\r\n{\r\n  function ()\r\n  {\r\n  }\r\n}";
    example.FixedCode = "class Animal\r\n{\r\n  function Foo()\r\n  {\r\n  }\r\n}";
    example.ExplanationOfFix = "test";
  }

}

// FunctionParameterNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "An argument specified (,) was found, but no parameter was defined to the right of it.";
  error.Name = "FunctionParameterNotFound";
  error.Reason = String();

}

// FunctionReturnTypeNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Function declaration '%s' has a return type specifier ':' but is missing the return type declaration.";
  error.Name = "FunctionReturnTypeNotFound";
  error.Reason = "':' was used after the parameter list, which signifies to the compiler that the function will be returning an object. The ':' must be followed by the type of object that will be returned. If you want nothing to be returned (ex 'void'), omit the usage of the ':'.";

  {
    ErrorExample& example = error.Examples.PushBack();
    example.ErrorCode = "function TestFn() :\r\n{\r\n}";
    example.FixedCode = "function TestFn()\r\n{\r\n}";
    example.ExplanationOfFix = "Fixed by removing the incomplete return type syntax.";
  }

}

// GenericError
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "%s";
  error.Name = "GenericError";
  error.Reason = String();

}

// GetFoundAfterSet
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "When writing the property '%s', the 'get' must always come before the 'set', just for the sake of consistency.";
  error.Name = "GetFoundAfterSet";
  error.Reason = String();

}

// GroupingOperatorNotComplete
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "A grouping operator was not properly closed, or the expression in the grouping was missing or invalid.";
  error.Name = "GroupingOperatorNotComplete";
  error.Reason = String();

}

// IfConditionalExpressionNotComplete
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "If statement conditional expression is not properly closed.";
  error.Name = "IfConditionalExpressionNotComplete";
  error.Reason = String();

}

// IfConditionalExpressionNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "If statement conditional expression was not found.";
  error.Name = "IfConditionalExpressionNotFound";
  error.Reason = String();

}

// IndexerIndicesNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The index or indices were invalid or not found for the index operator '[]'.";
  error.Name = "IndexerIndicesNotFound";
  error.Reason = String();

}

// IndexerNotComplete
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "An index operator was not closed with ']' or its contents were invalid.";
  error.Name = "IndexerNotComplete";
  error.Reason = String();

}

// InternalError
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "An internal error occurred: '%s'";
  error.Name = "InternalError";
  error.Reason = String();

}

// InvalidAttribute
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The attribute '%s' is not valid.%s";
  error.Name = "InvalidAttribute";
  error.Reason = String();

}

// InvalidBinaryOperation
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The binary '%s' operator '%s' is not valid with '%s' and '%s'.";
  error.Name = "InvalidBinaryOperation";
  error.Reason = String();

}

// InvalidEscapeInStringLiteral
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "An invalid escape sequence '\\%c' was used in a string literal.";
  error.Name = "InvalidEscapeInStringLiteral";
  error.Reason = String();

}

// InvalidNumberOfTemplateArguments
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "We expected %d template argument(s) but were given %d.";
  error.Name = "InvalidNumberOfTemplateArguments";
  error.Reason = String();

}

// InvalidTypeCast
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Attempting to cast between two unrelated types '%s' and '%s' (like trying to convert a car into an apple).";
  error.Name = "InvalidTypeCast";
  error.Reason = String();

}

// InvalidUnaryOperation
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The unary operation is not valid on the type '%s'.";
  error.Name = "InvalidUnaryOperation";
  error.Reason = String();

}

// LocalCreateMustBeValueType
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The 'local' keyword can only be used to create value types (primitives and 'struct' types). '%s' is not a value type.";
  error.Name = "LocalCreateMustBeValueType";
  error.Reason = "The keyword 'local' is used to differentiate whether we are allocating something directly on the stack or on a class as a member. Locals must be value types because when they are copied we memory-copy the entire object (which isn't valid for reference types).";

}

// LocalVariableReferenceNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The variable '%s' could not be found. If the variable was defined above, but is in a nested scope, then we cannot access it.";
  error.Name = "LocalVariableReferenceNotFound";
  error.Reason = String();

}

// MemberAccessNameNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Member access operator found without a valid member on the right hand side.";
  error.Name = "MemberAccessNameNotFound";
  error.Reason = String();

}

// MemberIdExpressionNotComplete
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "In the memberid(...) expression we expect the value to be wrapped in parentheses, but we didn't find the closing ')'.";
  error.Name = "MemberIdExpressionNotComplete";
  error.Reason = String();

}

// MemberIdExpressionNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "In the memberid(...) expression we expect to find an expression wrapped in parentheses, but we couldn't find it.";
  error.Name = "MemberIdExpressionNotFound";
  error.Reason = String();

}

// MemberIdMustBeUsedOnAMember
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The expression passed to memberid(...) must be accessing a member.";
  error.Name = "MemberIdMustBeUsedOnAMember";
  error.Reason = String();

}

// MemberIdOverloadedFunctionsMustBeDisambiguated
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "If the memberid(...) operator is used on an overloaded function, it must be resolved with a cast.";
  error.Name = "MemberIdOverloadedFunctionsMustBeDisambiguated";
  error.Reason = String();

}

// MemberNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "A member by the name of '%s' (function, field, or property) could not be found on '%s'.";
  error.Name = "MemberNotFound";
  error.Reason = String();

}

// MemberVariableTypesCannotBeInferred
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The type of a member variable '%s' cannot be inferred.";
  error.Name = "MemberVariableTypesCannotBeInferred";
  error.Reason = String();

}

// MultipleInheritanceNotSupported
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "A class can only inherit from a single base class.";
  error.Name = "MultipleInheritanceNotSupported";
  error.Reason = "Mutltiple ineritance has the problems :B";

}

// NativeTypesRequireConstructors
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The native type '%s' has no constructors and cannot be created.";
  error.Name = "NativeTypesRequireConstructors";
  error.Reason = String();

}

// NoArgumentConstructorsProvided
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The type '%s' has no constructors that take arguments.";
  error.Name = "NoArgumentConstructorsProvided";
  error.Reason = String();

}

// NotAllPathsReturn
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "A function is declared to return a type, yet not all code paths through the function result in it returning something.";
  error.Name = "NotAllPathsReturn";
  error.Reason = String();

}

// OnlyOneDestructorAllowed
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "A class is only allowed to have one destructor (otherwise, which one would we call?).";
  error.Name = "OnlyOneDestructorAllowed";
  error.Reason = String();

}

// OverloadsCannotBeTheSame
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Two methods of the same name '%s' have the exact same signature, this is not allowed.";
  error.Name = "OverloadsCannotBeTheSame";
  error.Reason = String();

}

// ParameterTypeNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Parameter declaration '%s' has a type specifier ':' but is missing the type declaration.";
  error.Name = "ParameterTypeNotFound";
  error.Reason = "Parameter names must have types associated with them. Ambiguously-typed parameters are not allowed in Zilch.";

  {
    ErrorExample& example = error.Examples.PushBack();
    example.ErrorCode = "class Animal\r\n{\r\n  function TestFn(count : )\r\n  {\r\n  }\r\n}";
    example.FixedCode = "class Animal\r\n{\r\n  function TestFn(count : Int)\r\n  {\r\n  }\r\n}";
    example.ExplanationOfFix = "Simply add the type of the parameter and all is good.";
  }

}

// ParameterTypeSpecifierNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Parameter declaration '%s' is missing the type specifier ':'.";
  error.Name = "ParameterTypeSpecifierNotFound";
  error.Reason = "Function parameters must have types associated with them, denoted by the syntax 'parameter : parameterType'. Zilch does not support ambiguously-typed parameters.";

  {
    ErrorExample& example = error.Examples.PushBack();
    example.ErrorCode = "class Animal\r\n{\r\n  function TestFn(count )\r\n  {\r\n  }\r\n}";
    example.FixedCode = "class Animal\r\n{\r\n  function TestFn(count : Int)\r\n  {\r\n  }\r\n}";
    example.ExplanationOfFix = "Adding the type specifier, ':', and a type to the parameter 'count'.";
  }

}

// ParsingNotComplete
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Parsing could not be completed (we ran into something we didn't understand). The token we hit was '%s' with token type '%s'.";
  error.Name = "ParsingNotComplete";
  error.Reason = String();

}

// PluginLoadingFailed
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Plugin failed to load: %s";
  error.Name = "PluginLoadingFailed";
  error.Reason = String();

}

// PropertyDeclarationNotComplete
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The declaration of the property '%s' must be closed by an ending '}'.";
  error.Name = "PropertyDeclarationNotComplete";
  error.Reason = String();

}

// PropertyDelegateOperatorRequiresProperty
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The property delegate operator '@' only works on properties (which also includes data members).";
  error.Name = "PropertyDelegateOperatorRequiresProperty";
  error.Reason = String();

}

// PropertyDelegateRequiresGetOrSet
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The property delegate operator '@' requires the property to have either a 'get' or 'set' (or both). The property '%s' has neither.";
  error.Name = "PropertyDelegateRequiresGetOrSet";
  error.Reason = String();

}

// ReadingFromAWriteOnlyValue
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "An attempt was made to read from a write only value.";
  error.Name = "ReadingFromAWriteOnlyValue";
  error.Reason = String();

}

// ReferencesOnlyToNamedValueTypes
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "References (using the 'ref' keyword) can only be made to named value types, such as 'struct' types or primitives. '%s' is not a named value type.";
  error.Name = "ReferencesOnlyToNamedValueTypes";
  error.Reason = String();

}

// ReferenceToUndefinedType
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The type '%s' was referenced, but could not be found.";
  error.Name = "ReferenceToUndefinedType";
  error.Reason = String();

}

// ReferenceToUndefinedTypeWithSimilarMember
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "It appears you are trying to reference the member '%s', and if so you must use 'this.%s'. Saying the name alone implies you are referring to a type, and the type '%s' could not be found.";
  error.Name = "ReferenceToUndefinedTypeWithSimilarMember";
  error.Reason = String();

}

// ReturnTypeMismatch
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The '%s' type of the return value did not match the function's '%s' return type.";
  error.Name = "ReturnTypeMismatch";
  error.Reason = String();

}

// ReturnValueNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The return statement is expected to return a '%s' value, since the function declares it so (the ':' at the end specifies the return type).";
  error.Name = "ReturnValueNotFound";
  error.Reason = String();

}

// ReturnValueUnexpected
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The return statement was not expected to return a '%s' value, since the function does not declare it (no ':' at the end).";
  error.Name = "ReturnValueUnexpected";
  error.Reason = String();

}

// ScopeBodyNotComplete
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The %s statement body is not properly closed.";
  error.Name = "ScopeBodyNotComplete";
  error.Reason = String();

}

// ScopeBodyNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The %s statement body was not found.";
  error.Name = "ScopeBodyNotFound";
  error.Reason = String();

}

// SendsEventStatementNameNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "In a 'sends' statement, we expect to see 'sends EventName : EventType;'. The name was not found.";
  error.Name = "SendsEventStatementNameNotFound";
  error.Reason = String();

}

// SendsEventStatementNotComplete
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "In a 'sends %s' statement, we expect to see 'sends EventName : EventType;'. The statement was not complete (probably missing a ';').";
  error.Name = "SendsEventStatementNotComplete";
  error.Reason = String();

}

// SendsEventStatementTypeNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "In a 'sends %s' statement, we expect to see 'sends EventName : EventType;'. The event type was not found.";
  error.Name = "SendsEventStatementTypeNotFound";
  error.Reason = String();

}

// SendsEventStatementTypeSpecifierNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "In a 'sends %s' statement, we expect to see 'sends EventName : EventType;'. The event type specifier ':' was not found.";
  error.Name = "SendsEventStatementTypeSpecifierNotFound";
  error.Reason = String();

}

// StatementSeparatorNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The statement must end with a semicolon.";
  error.Name = "StatementSeparatorNotFound";
  error.Reason = String();

}

// StatementsWillNotBeExecutedEarlyReturn
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The statement will not get hit because all code paths return before it would get to it. Use 'debug return' to exit early for debugging purposes.";
  error.Name = "StatementsWillNotBeExecutedEarlyReturn";
  error.Reason = String();

}

// StaticCannotBeOverriding
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The function or property '%s' marked as Static cannot also be marked as Override (because it can't be virtual).";
  error.Name = "StaticCannotBeOverriding";
  error.Reason = String();

}

// StaticCannotBeVirtual
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The function or property '%s' marked as Static cannot also be marked as Virtual.";
  error.Name = "StaticCannotBeVirtual";
  error.Reason = String();

}

// StaticTypeConstructorOrAccessNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "You can only refer to a type when constructing it or accessing static members upon it";
  error.Name = "StaticTypeConstructorOrAccessNotFound";
  error.Reason = String();

}

// StringInterpolantExpectedExpression
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "When we start a string interpolant with '`' we expect an expression to follow that we will stringify.";
  error.Name = "StringInterpolantExpectedExpression";
  error.Reason = String();

}

// StringInterpolantNotComplete
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "When we start a string interpolant with '`' we are expected to end it with another '`'.";
  error.Name = "StringInterpolantNotComplete";
  error.Reason = String();

}

// StringLiteralNotComplete
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "A string literal was started, but never finished before hitting the end of the line or stream.";
  error.Name = "StringLiteralNotComplete";
  error.Reason = String();

}

// StructsCanOnlyContainValueTypes
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The struct '%s' can only contain value types (other structs or primitive values). The member '%s' is not a value type.";
  error.Name = "StructsCanOnlyContainValueTypes";
  error.Reason = "A struct is a value type, which means it must be trivially copyable in memory and must not have a destructor. Structs are typically used for efficiency because they can be quickly copied around and stack allocated, unlike classes which must be heap allocated.";

}

// TemplateArgumentNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The argument to a template type was not found.";
  error.Name = "TemplateArgumentNotFound";
  error.Reason = String();

}

// TemplateArgumentsMustBeConstantsOrTypes
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "A template argument must be a constant or a type.";
  error.Name = "TemplateArgumentsMustBeConstantsOrTypes";
  error.Reason = String();

}

// TemplateParameterTypeMismatch
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "A template parameter was of an incorrect type.";
  error.Name = "TemplateParameterTypeMismatch";
  error.Reason = String();

}

// TemplateTypeArgumentsNotComplete
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Arguments for a templated type were provided, but were never closed or completed.";
  error.Name = "TemplateTypeArgumentsNotComplete";
  error.Reason = String();

}

// ThrowExceptionExpressionNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "A throw statement always requires an exception to be thrown, e.g. 'throw new Exception();'. The exception part of the throw statement wasn't found.";
  error.Name = "ThrowExceptionExpressionNotFound";
  error.Reason = String();

}

// ThrowTypeMustDeriveFromException
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The exception we're throwing must derive from the 'Exception' type.";
  error.Name = "ThrowTypeMustDeriveFromException";
  error.Reason = String();

}

// TimeoutBodyNotComplete
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Timeout statement body is not properly closed.";
  error.Name = "TimeoutBodyNotComplete";
  error.Reason = String();

}

// TimeoutBodyNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Timeout statement body was not found.";
  error.Name = "TimeoutBodyNotFound";
  error.Reason = String();

}

// TimeoutSecondsExpectedIntegerLiteral
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The timeout statement only accepts Integer literals for seconds.";
  error.Name = "TimeoutSecondsExpectedIntegerLiteral";
  error.Reason = String();

}

// TimeoutSecondsMustBeNonZeroPositive
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The time in seconds given to the timeout statement must be positive and non-zero.";
  error.Name = "TimeoutSecondsMustBeNonZeroPositive";
  error.Reason = String();

}

// TimeoutSecondsNotComplete
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Timeout statement's seconds is not properly closed with a ')'.";
  error.Name = "TimeoutSecondsNotComplete";
  error.Reason = String();

}

// TimeoutSecondsNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The timeout statement must be followed by a '(', the time in seconds, and a closing ')'.";
  error.Name = "TimeoutSecondsNotFound";
  error.Reason = String();

}

// TypeIdExpressionNotComplete
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "In the typeid(...) expression we expect the value to be wrapped in parentheses, but we didn't find the closing ')'.";
  error.Name = "TypeIdExpressionNotComplete";
  error.Reason = String();

}

// TypeIdExpressionNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "In the typeid(...) expression we expect to find an expression wrapped in parentheses, but we couldn't find it.";
  error.Name = "TypeIdExpressionNotFound";
  error.Reason = String();

}

// UnableToResolveFunction
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The function '%s' exists, but could not be resolved since the types or names of the arguments used did not match. The arguments you gave were: %sThe possible choices were: %s";
  error.Name = "UnableToResolveFunction";
  error.Reason = String();

}

// UnaryOperatorOperandNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The operand was not found for the '%s' operator '%s'.";
  error.Name = "UnaryOperatorOperandNotFound";
  error.Reason = "The operators such as negation '-' and logical-not '!' must have an operand on the right-hand side for them to be valid syntax.";

  {
    ErrorExample& example = error.Examples.PushBack();
    example.ErrorCode = "class Animal\r\n{\r\n  function TestFn() : Real\r\n  {\r\n	return - ;\r\n  }\r\n}";
    example.FixedCode = "class Animal\r\n{\r\n  function TestFn() : Real\r\n  {\r\n	return -1.0 ;\r\n  }\r\n}";
    example.ExplanationOfFix = "*TBD*";
  }

}

// UnidentifiedSymbol
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Invalid symbol or character '%c' encountered.";
  error.Name = "UnidentifiedSymbol";
  error.Reason = "The compiler ran into a symbol or character that it didn't understand, and therefore cannot continue.";

  {
    ErrorExample& example = error.Examples.PushBack();
    example.ErrorCode = "function #IsEmpty() : Bool32\r\n{\r\n  return mCount == 0;\r\n}";
    example.FixedCode = "function IsEmpty() : Bool32\r\n{\r\n  return mCount == 0;\r\n}";
    example.ExplanationOfFix = "In the error case, a # sign was introduced before the name of the function. The tokenizer would reach this symbol and hault, not knowing what it is.";
  }

}

// UnnecessaryVirtualAndOverride
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "When marking the method '%s' as overriding, it is unnecessary to also mark it as Virtual.";
  error.Name = "UnnecessaryVirtualAndOverride";
  error.Reason = String();

}

// VariableInitializationNotComplete
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Variable declaration '%s' is missing a semicolon or the initialization expression is invalid.";
  error.Name = "VariableInitializationNotComplete";
  error.Reason = "This error occurred because the compiler was trying to parse a variable declaration with initialization and expected to find a semicolon at the end, but did not. The causes could be as simple as the semicolon is missing, or that the semicolon is in fact there, but the initialization expression was somehow garbled. If you're curious as to why a semicolon is needed, they are necessary because they act as a separator and help the parser to differentiate statements from each other, just like periods in a sentence. Imagine if a period was missing from a sentance, or worse yet if a sentance was so confusing to the point that you would expected a period!";

  {
    ErrorExample& example = error.Examples.PushBack();
    example.ErrorCode = "var cat : Animal = new Cat()\r\nvar dog : Animal = new Dog();";
    example.FixedCode = "var cat : Animal = new Cat();\r\nvar dog : Animal = new Dog();";
    example.ExplanationOfFix = "In the error case, we forgot to put a semicolon (;) at the end of the 'cat' variable declaration. We fixed it by simply adding in a semicolon.";
  }

}

// VariableInitialValueNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Variable declaration '%s' has a missing or invalid initialization value.";
  error.Name = "VariableInitialValueNotFound";
  error.Reason = "Most probably the initial value of the variable was omitted, or a typo was made that caused the compiler to be unable to parse the initial value expression.";

  {
    ErrorExample& example = error.Examples.PushBack();
    example.ErrorCode = "var lives : Int32 := ;";
    example.FixedCode = "var lives : Int32 := 9;";
    example.ExplanationOfFix = "In the error case, the variable 'lives' was going to be initialized, but the value was left out. In the fixed case, we added a value (this value is known as the initialization expression).";
  }

}

// VariableMustBeInitialized
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Variable declaration '%s' is missing the initializer expression.";
  error.Name = "VariableMustBeInitialized";
  error.Reason = "All variables must be initialized for safety, and rather than letting the language decide some default value to initialize to, we forced the users to initialize all of their variables. Safety is a key concern for the language because it allows users to focus more on debugging logical bugs rather than bugs caused by indeterminate factors (like uninitialized variables).";

  {
    ErrorExample& example = error.Examples.PushBack();
    example.ErrorCode = "var lives : Int32;";
    example.FixedCode = "var lives : Int32 = 9;";
    example.ExplanationOfFix = "In the error case, the variable 'lives' did not have an initializer after it, and therefore would have had an undefined value. We fixed this by giving it an initialization expression so that we would ultimately always be in control of its value.";
  }

}

// VariableNameNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Variable declaration has a missing or invalid name.";
  error.Name = "VariableNameNotFound";
  error.Reason = "One of the most common reasons for this error is that a keyword was used as a variable's name, which is not allowed because it could confuse the compiler. Also look to see if the name was left off, or if somehow the name was garbled with other symbols. If you're wondering why a variable needs a name, the reason is because a name gives us a way to refer to it in other parts of code, much like how people have names so we can refer to them and differentiate them apart.";

  {
    ErrorExample& example = error.Examples.PushBack();
    example.ErrorCode = "var : Int32 := 9;";
    example.FixedCode = "var lives : Int32 := 9;";
    example.ExplanationOfFix = "In the error case, we forgot to name the variable. We fixed it simply by adding the variable name 'lives' after the 'var' keyword.";
  }

}

// VariableTypeMismatch
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The value being assigned to '%s' must be of type '%s'. Its type is '%s'.";
  error.Name = "VariableTypeMismatch";
  error.Reason = String();

}

// VariableTypeNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "Variable declaration '%s' has a type specifier ':' but is missing the type declaration.";
  error.Name = "VariableTypeNotFound";
  error.Reason = "Mosty likely the type was just left out or the type contained some invalid keyword or symbol. If the type specifier (:) operator was found, then the variable expects a type. If you actually meant to make the variable an inferred type, then leave off the (:) and type declaration altogether. Inferred variables are variables whose type is gleaned by the initialization value (this feature is better known as Type Inference).";

  {
    ErrorExample& example = error.Examples.PushBack();
    example.ErrorCode = "var lives : := 9;";
    example.FixedCode = "var lives : Int32 := 9;\r\n\r\n/* OR */\r\n\r\nvar lives := 9;";
    example.ExplanationOfFix = "In the error case, the type of lives was left out yet the type qualifier (:) was specified. We can fix it in two ways, first by actually specifying the type, and second by using type inference to infer the type automatically.";
  }

}

// WhileConditionalExpressionNotComplete
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "While statement conditional expression is not properly closed.";
  error.Name = "WhileConditionalExpressionNotComplete";
  error.Reason = String();

}

// WhileConditionalExpressionNotFound
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "While statement conditional expression was not found.";
  error.Name = "WhileConditionalExpressionNotFound";
  error.Reason = String();

}

// WritingToAReadOnlyValue
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "An attempt was made to write to a read only value.";
  error.Name = "WritingToAReadOnlyValue";
  error.Reason = String();

}

// CannotDeleteType
{
  ErrorInfo& error = this->Errors.PushBack();
  error.Error = "The type '%s' can only be deleted outside of Zilch.";
  error.Name = "CannotDeleteType";
  error.Reason = String();

}

