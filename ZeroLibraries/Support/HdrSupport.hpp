#pragma once

namespace Zero
{

const uint cHdrPixelSize = 12;

void RgbeToRgb32f(byte* rgbe, float* rgb32f);
void Rgb32fToRgbe(float* rgb32f, byte* rgbe);

void LoadFromHdr(Status& status, byte** output, uint* width, uint* height, const byte* data, uint size);
void SaveToHdr(Status& status, byte* image, uint width, uint height, StringParam filename);

} // namespace Zero
