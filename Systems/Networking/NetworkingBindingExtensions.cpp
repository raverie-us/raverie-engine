////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------------------- Meta Net Property
ZilchDefineType(MetaNetProperty, builder, type)
{
  ZilchBindField(mNetPropertyConfig)->AddAttribute(PropertyAttributes::cOptional);
  ZilchBindField(mNetChannelConfig)->AddAttribute(PropertyAttributes::cOptional);
}

}//namespace Zero
