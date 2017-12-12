////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------------------------- Meta Net Property
class MetaNetProperty : public ReferenceCountedEventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// The net property type name.
  String mNetPropertyConfig;
  
  /// Desired net channel name.
  String mNetChannelConfig;
};

}//namespace Zero
