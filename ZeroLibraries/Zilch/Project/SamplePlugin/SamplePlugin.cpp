/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2015, DigiPen Institute of Technology
\**************************************************************/

#include "SamplePlugin.hpp"

//***************************************************************************
ZilchDefineStaticLibraryAndPlugin(SampleLibrary, SamplePlugin)
{
  ZilchInitializeType(Sample);
}

//***************************************************************************
void SamplePlugin::Initialize(Zilch::BuildEvent* event)
{
}

//***************************************************************************
void SamplePlugin::Uninitialize()
{
}

//***************************************************************************
ZilchDefineType(Sample, builder, type)
{
  ZilchBindMethod(Run);
}

//***************************************************************************
void Sample::Run(StringBuilderExtended* builder, ArrayClass<Byte>* bytes)
{
  String s = builder->ToString();
  printf("It's ALIVE %s YAY!\n", s.c_str());
}
