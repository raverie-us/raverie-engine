///////////////////////////////////////////////////////////////////////////////
///
/// \file TextResource.hpp
/// Implementation of simple text resource.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

TextBlock::TextBlock()
{
}

TextBlock::~TextBlock()
{

}

StringRange TextBlock::LoadTextData()
{
  if(Text.Empty())
    Text = ReadFileIntoString(LoadPath.c_str());
  return Text.All();
}

void TextBlock::Save(StringParam filename)
{
  LoadPath = filename;
  //Write the data out to the file
  WriteStringRangeToFile(filename, Text);
}

ZilchDefineType(TextBlock, builder, type)
{
  ZilchBindGetterProperty(Text);
  
  ZeroBindDocumented();
}

String TextBlock::GetText()
{
  LoadTextData();
  return Text;
}

void TextBlock::SetText(StringParam newText)
{
  Text = newText;
}

void TextBlock::ReloadData(StringRange data)
{
  Text = data;

  //Resource has been modified. Some objects (SpriteTextBlock) may need to reload data.
  ResourceEvent event;
  event.EventResource = this;
  TextBlockManager::GetInstance()->DispatchEvent(Events::ResourceModified, &event);
}

String TextBlock::GetFormat()
{
  return "Text";
}

class TextBlockLoader : public ResourceLoader
{
  void ReloadFromFile(Resource* resource, ResourceEntry& entry) override
  {
    TextBlock* text = (TextBlock*)resource;
    text->Text = String();
    text->LoadTextData();
  }

  HandleOf<Resource> LoadFromBlock(ResourceEntry& entry) override
  {
    TextBlock* text = new TextBlock();
    text->Text = String((cstr)entry.Block.Data, entry.Block.Size);
    TextBlockManager::GetInstance()->AddResource(entry, text);
    return text;
  }

  HandleOf<Resource> LoadFromFile(ResourceEntry& entry) override
  {
    TextBlock* text = new TextBlock();
    TextBlockManager::GetInstance()->AddResource(entry, text);
    text->DocumentSetup(entry);
    text->LoadTextData();
    return text;
  }
};

ImplementResourceManager(TextBlockManager, TextBlock);

TextBlockManager::TextBlockManager(BoundType* resourceType)
  :ResourceManager(resourceType)
{
  AddLoader("Text", new TextBlockLoader());
  DefaultResourceName = "DefaultTextBlock";
  mCanDuplicate = true;
  mCanAddFile = true;
  mOpenFileFilters.PushBack(FileDialogFilter("All Text Blocks", "*.txt;*.data"));
  mOpenFileFilters.PushBack(FileDialogFilter("*.txt"));
  mOpenFileFilters.PushBack(FileDialogFilter("*.data"));
  mCanReload = true;
  mCanCreateNew = true;
  mExtension = "txt";
}

}
