// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

Raverie::Function* GetMemberOverloadedFunction(Raverie::Type* type, StringParam fnName);
Raverie::Function* GetMemberOverloadedFunction(Raverie::Type* type, StringParam fnName, StringParam p0);
Raverie::Function* GetMemberOverloadedFunction(Raverie::Type* type, StringParam fnName, StringParam p0, StringParam p1);
Raverie::Function*
GetMemberOverloadedFunction(Raverie::Type* type, StringParam fnName, StringParam p0, StringParam p1, StringParam p2);
Raverie::Function* GetMemberOverloadedFunction(
    Raverie::Type* type, StringParam fnName, StringParam p0, StringParam p1, StringParam p2, StringParam p3);

Raverie::Function* GetStaticFunction(Raverie::Type* type, StringParam fnName);
Raverie::Function* GetStaticFunction(Raverie::Type* type, StringParam fnName, StringParam p0);
Raverie::Function* GetStaticFunction(Raverie::Type* type, StringParam fnName, StringParam p0, StringParam p1);
Raverie::Function*
GetStaticFunction(Raverie::Type* type, StringParam fnName, StringParam p0, StringParam p1, StringParam p2);
Raverie::Function* GetStaticFunction(
    Raverie::Type* type, StringParam fnName, StringParam p0, StringParam p1, StringParam p2, StringParam p3);
Raverie::Function* GetStaticFunction(Raverie::Type* type,
                                   StringParam fnName,
                                   StringParam p0,
                                   StringParam p1,
                                   StringParam p2,
                                   StringParam p3,
                                   StringParam p4);

Raverie::Function* GetConstructor(Raverie::Type* type, StringParam p0);
Raverie::Function* GetConstructor(Raverie::Type* type, Array<String>& params);

Raverie::Field* GetStaticMember(Raverie::Type* type, StringParam memberName);
Raverie::Property* GetInstanceProperty(Raverie::Type* type, StringParam propName);
Raverie::Property* GetStaticProperty(Raverie::Type* type, StringParam propName);

Array<String> BuildParams(StringParam p0);
Array<String> BuildParams(StringParam p0, StringParam p1);
Array<String> BuildParams(StringParam p0, StringParam p1, StringParam p2);
Array<String> BuildParams(StringParam p0, StringParam p1, StringParam p2, StringParam p3);

String JoinRepeatedString(StringParam str, StringParam separator, size_t count);

} // namespace Raverie
