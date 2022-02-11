// MIT Licensed (see LICENSE.md).
// This only exists because we want to treate SpirvHeaders as its own library,
// but since it doesn't export any symbosl some of the toolchain has errors.
// This makes a public symbol.
extern int SpirvHeaders;