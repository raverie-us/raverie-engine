// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

/// An interface for a front-end shader translator (typically targetting some
/// kind of IR). This allows language agnostic front-end translation. As
/// front-end translation is typically a lot of work, there will likely not be
/// more than one front-end translator and different back-ends would instead be
/// created.
class BaseShaderIRTranslator
{
public:
  virtual ~BaseShaderIRTranslator(){};

  /// Tell the translator what settings to use for translation (Names, render
  /// targets, etc...)
  virtual void SetSettings(RaverieShaderSpirVSettingsRef& settings) = 0;
  /// Initial setup for a translator so it can cache
  /// anything it needs. SetSettings must be called first.
  virtual void Setup() = 0;

  /// Translate a given project (and it's syntax tree) into the passed in
  /// library. Each shader type in the library will contain translated classes
  /// that can be put together with a backend (e.g.
  /// RaverieSpirVDisassemblerBackend)
  virtual bool Translate(Raverie::SyntaxTree& syntaxTree,
                         RaverieShaderIRProject* project,
                         RaverieShaderIRLibrary* library) = 0;

  // An intrusive reference count for memory handling
  RaverieRefLink(BaseShaderIRTranslator);
};

} // namespace Raverie
