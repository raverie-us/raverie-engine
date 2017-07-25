///////////////////////////////////////////////////////////////////////////////
///
/// \file ScriptDebugEngine.hpp
/// Declaration of the DebugValue and ScriptDebugEngine classes.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Events sent by the DebugEngine.
namespace Events
{
  DeclareEvent(DebugBreak);
  DeclareEvent(DebugException);
  DeclareEvent(SyntaxError);
  DeclareEvent(UnhandledException);
}//namespace Events

//---------------------------------------------------------------- Code Location
class DocumentResource;

//----------------------------------------------------------- Debug Engine Event
class DebugEngineEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  DebugEngineEvent() { Script = NULL; }
  CodeLocation Location;
  String Message;
  /// The document resource that should be displayed due to an error
  DocumentResource* Script;
  bool Handled;
};

//------------------------------------------------------------------ Break Mode
namespace BreakMode
{
  /// Current break mode of the debugger.
  enum BreakMode
  {
    /// Break when it hits a break point or error. The default running state.
    BreakNormal,
    /// Break point is active so other breaks should not activate.
    BreakActive,
    /// Will break on first line and all break points. Used to break all.
    BreakOnAnyLine,
    /// Break when stack reaches target level and all break points.
    BreakOnStack,
    /// File break. Used when there has been a compile error or an error in a script file or other compiled file.
    BreakOnFile
  };
}//namespace BreakMode

//------------------------------------------------------------------ Debug Value
/// A Value in the debug engine
struct DebugValue
{
  enum ScopeType
  {
    Local,
    Global,
    Upvalue,
    Temp
  };

  enum ValueTypes
  {
    stNill = 0,
    stBoolean,
    stNumber,
    stString,
    stFunction,
    stObject
  };

  DebugValue(){}
  DebugValue(ScopeType scope, ValueTypes stype, StringParam name,
    StringParam stringValue, StringParam address, int stackLevel)
    : Scope(scope), ScriptType(stype), Name(name), StringValue(stringValue), 
    Address(address), StackLevel(stackLevel)
  {
  }

  cstr GetTypeName();
  cstr GetScopeName();

  //The scope of the variable.
  ScopeType Scope;
  //The script type of the variable.
  ValueTypes ScriptType;

  ////Name and value
  //Name of the value.
  String Name;
  //String representation of the value.
  String StringValue;

  /////Debug Engine Values
  //What stack level this value is located at. (Not for external use)
  int StackLevel;
  //Debug Engine address. Used to located the value. (Not for external use)
  String Address;

};

typedef void (*SystemBreakPump)();

//---------------------------------------------------------- Script Debug Engine
class ScriptDebugEngine : public EventObject
{
public:
  /////////Break points and Execution////////////

  //Sets the break mode. This can resume execution by setting BreakPointsOnly or
  //break at a particular stack level or the next line hit. See BreakMode for details.
  virtual void SetBreakMode(int newBreakMode, int targetStackLevel) = 0;

  //Set a break point in a particular file (filename 
  virtual void SetBreakPoint(CodeLocation& location) = 0;

  //Remove a break point in a particular file.
  virtual void RemoveBreakPoint(CodeLocation& location) = 0;

  //Remove all breakpoints from the debugger.
  virtual void ClearAllBreakPoints() = 0;

  //////Inspection Functions/////////
  //Function used to inspect the script environment
  //These can only be used during a debug break or break error.

  //Evaluate an identifier in the local stack context.
  virtual void EvaluateIdentifier(DebugValue& value, StringParam identifier) = 0;

  //Evaluate an expression in the debugger. Only works in a debug break
  virtual void EvaluateExpression(DebugValue& value, StringParam expression) = 0;

  ///Update the debug engine checking for infinite loops.
  virtual void Update() = 0;
  ////System Functions////////////

  //Set the pump used when a debug break is active.
  virtual void SetPump(SystemBreakPump pump) = 0;
};

}//namespace Zero
