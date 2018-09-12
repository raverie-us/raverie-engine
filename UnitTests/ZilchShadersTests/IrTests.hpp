

void TestCompilation(SimpleZilchShaderIRGenerator& shaderGenerator, IZilchShaderIR* type, StringParam directory, ErrorReporter& reporter);
void TestShaderFileCompilation(SimpleZilchShaderIRGenerator& shaderGenerator, UnitTestPackage& unitTestPackage, StringParam filePath, StringParam directory, ErrorReporter& reporter);
void TestShaderCompilationOfDirectory(SimpleZilchShaderIRGenerator& shaderGenerator, UnitTestPackage& unitTestPackage, StringParam directory, ErrorReporter& reporter);
void TestCompositesOfDirectory(SimpleZilchShaderIRGenerator& shaderGenerator, UnitTestPackage& unitTestPackage, StringParam directory, ErrorReporter& reporter);
void TestCompositeDefinition(SimpleZilchShaderIRGenerator& shaderGenerator, SerializedShaderDefinition& shaderDef, UnitTestPackage& unitTestPackage, StringParam directory, ErrorReporter& reporter);
void TestDirectory(SimpleZilchShaderIRGenerator& shaderGenerator, UnitTestPackage& unitTestPackage, StringParam directory, ErrorReporter& reporter, bool recurse = true);
void TestRunning(SimpleZilchShaderIRGenerator& shaderGenerator, UnitTestPackage& unitTestPackage, StringParam directory, bool recurse = true);
