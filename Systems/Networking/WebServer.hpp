///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg.
/// Copyright 2018, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
namespace Events
{
  DeclareEvent(WebServerRequest);
}

class WebServer : public EventObject, public ThreadableLoop
{
public:
  ZilchDeclareType(WebServer, TypeCopyMode::ReferenceType);
};

} // namespace Zero
