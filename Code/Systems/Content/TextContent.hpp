// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

/// Text content item. Text content is content that is loaded
/// from plain text files. The editor can edit them with various
/// text editors.
class TextContent : public ContentComposition
{
public:
  RaverieDeclareType(TextContent, TypeCopyMode::ReferenceType);

  TextContent();
};

class BaseTextBuilder : public DirectBuilderComponent
{
public:
  RaverieDeclareType(BaseTextBuilder, TypeCopyMode::ReferenceType);

  void Generate(ContentInitializer& initializer) override;
  BaseTextBuilder(uint order, StringParam extension, StringParam resourceName) : DirectBuilderComponent(order, extension, resourceName)
  {
  }
};

const String TextResourceName = "Text";
const String TextExtension = ".txt";

/// Text Builder for generic text.
class TextBuilder : public BaseTextBuilder
{
public:
  RaverieDeclareType(TextBuilder, TypeCopyMode::ReferenceType);

  void SetDefaults()
  {
  }
  TextBuilder() : BaseTextBuilder(15, TextExtension, TextResourceName)
  {
  }
};

const String RaverieScriptResourceName = "RaverieScript";

/// Raverie Script File builder.
class RaverieScriptBuilder : public BaseTextBuilder
{
public:
  RaverieDeclareType(RaverieScriptBuilder, TypeCopyMode::ReferenceType);

  void SetDefaults()
  {
  }

  RaverieScriptBuilder() : BaseTextBuilder(15, FileExtensionManager::GetRaverieScriptTypeEntry()->GetDefaultExtensionWithDot(), RaverieScriptResourceName)
  {
  }
};

class RaverieFragmentBuilder : public BaseTextBuilder
{
public:
  RaverieDeclareType(RaverieFragmentBuilder, TypeCopyMode::ReferenceType);

  void SetDefaults()
  {
  }

  RaverieFragmentBuilder()
      // Increase the load order to 9 so that these load before materials
      // (since a material may need the block created from this fragment)
      :
      BaseTextBuilder(9, FileExtensionManager::GetRaverieFragmentTypeEntry()->GetDefaultExtensionWithDot(), "RaverieFragment")
  {
  }
};

} // namespace Raverie
