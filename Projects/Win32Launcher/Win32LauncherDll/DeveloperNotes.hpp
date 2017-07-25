///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Josh Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Represents information about a template project for the user. This may reside locally or on the server.
class DeveloperNotes
{
public:

  void Serialize(Serializer& stream);

  String mFileName;
  String mNotes;
  String mDate;
};

}//namespace Zero
