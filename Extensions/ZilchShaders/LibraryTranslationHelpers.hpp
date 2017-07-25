///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

Zilch::Function* GetMemberOverloadedFunction(Zilch::Type* type, StringParam fnName);
Zilch::Function* GetMemberOverloadedFunction(Zilch::Type* type, StringParam fnName, StringParam p0);
Zilch::Function* GetMemberOverloadedFunction(Zilch::Type* type, StringParam fnName, StringParam p0, StringParam p1);
Zilch::Function* GetMemberOverloadedFunction(Zilch::Type* type, StringParam fnName, StringParam p0, StringParam p1, StringParam p2);
Zilch::Function* GetMemberOverloadedFunction(Zilch::Type* type, StringParam fnName, StringParam p0, StringParam p1, StringParam p2, StringParam p3);

Zilch::Function* GetStaticFunction(Zilch::Type* type, StringParam fnName);
Zilch::Function* GetStaticFunction(Zilch::Type* type, StringParam fnName, StringParam p0);
Zilch::Function* GetStaticFunction(Zilch::Type* type, StringParam fnName, StringParam p0, StringParam p1);
Zilch::Function* GetStaticFunction(Zilch::Type* type, StringParam fnName, StringParam p0, StringParam p1, StringParam p2);
Zilch::Function* GetStaticFunction(Zilch::Type* type, StringParam fnName, StringParam p0, StringParam p1, StringParam p2, StringParam p3);
Zilch::Function* GetStaticFunction(Zilch::Type* type, StringParam fnName, StringParam p0, StringParam p1, StringParam p2, StringParam p3, StringParam p4);

Zilch::Field* GetStaticMember(Zilch::Type* type, StringParam memberName);
Zilch::Property* GetInstanceProperty(Zilch::Type* type, StringParam propName);
Zilch::Property* GetStaticProperty(Zilch::Type* type, StringParam propName);

Array<String> BuildParams(StringParam p0);
Array<String> BuildParams(StringParam p0, StringParam p1);
Array<String> BuildParams(StringParam p0, StringParam p1, StringParam p2);
Array<String> BuildParams(StringParam p0, StringParam p1, StringParam p2, StringParam p3);

String JoinRepeatedString(StringParam str, StringParam separator, size_t count);

}//namespace Zero
