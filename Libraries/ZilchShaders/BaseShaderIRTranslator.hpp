///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------------BaseShaderIRTranslator
/// An interface for a front-end shader translator (typically targetting some kind of IR).
/// This allows language agnostic front-end translation. As front-end translation
/// is typically a lot of work, there will likely not be more than one front-end
/// translator and different back-ends would instead be created.
class BaseShaderIRTranslator
{
public:
  virtual ~BaseShaderIRTranslator() {};

  /// Tell the translator what settings to use for translation (Names, render targets, etc...)
  virtual void SetSettings(ZilchShaderSpirVSettingsRef& settings) = 0;
  /// Initial setup for a translator so it can cache
  /// anything it needs. SetSettings must be called first.
  virtual void Setup() = 0;

  /// Translate a given project (and it's syntax tree) into the passed in library.
  /// Each shader type in the library will contain translated classes that can
  /// be put together with a backend (e.g. ZilchSpirVDisassemblerBackend)
  virtual bool Translate(Zilch::SyntaxTree& syntaxTree, ZilchShaderIRProject* project, ZilchShaderIRLibrary* library) = 0;

  // An intrusive reference count for memory handling
  ZilchRefLink(BaseShaderIRTranslator);
};

}//namespace Zero
