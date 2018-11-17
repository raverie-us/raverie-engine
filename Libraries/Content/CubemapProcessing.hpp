// Authors: Nathan Carlson
// Copyright 2015, DigiPen Institute of Technology

#pragma once

namespace Zero
{

void ExtractCubemapFaces(Status& status, Array<MipHeader>& mipHeaders, Array<byte*>& imageData, TextureFormat::Enum format);
void MipmapCubemap(Array<MipHeader>& mipHeaders, Array<byte*>& imageData, TextureFormat::Enum format, bool compressed);

} // namespace Zero
