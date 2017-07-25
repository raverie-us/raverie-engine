///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------
// All of these have to be defined otherwise linker errors happen.
// The launcher can't export though so these are just stubs...
//-------------------------------------------------------------------

void ExportContentFolders(Cog* projectCog)
{
}

Exporter::Exporter()
{
}

void Exporter::ExportAndPlay(Cog* projectCog)
{
}

void Exporter::ExportGameProject(Cog* projectCog)
{
}

}//namespace Zero
