///////////////////////////////////////////////////////////////////////////////
///
/// \file Project.cpp
/// Declaration of the Project class.
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//----------------------------------------------------------------------- Events
namespace Events
{
  DefineEvent(ProjectLoaded);
  DefineEvent(NoProjectLoaded);
}

//---------------------------------------------------------------------- Project
ZilchDefineType(ProjectSettings, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  type->AddAttribute(ObjectAttributes::cCore);

  ZilchBindFieldGetter(ProjectName);
  ZilchBindFieldGetter(ProjectFolder);
  ZilchBindFieldGetter(ContentFolder);
  ZilchBindFieldGetter(EditorContentFolder);

  ZilchBindFieldProperty(ProjectOwner);
}

//******************************************************************************
ProjectSettings::ProjectSettings()
{
  ProjectContentLibrary = nullptr;
  ProjectResourceLibrary = nullptr;
  mLastLoadedTime = 0;
}

//******************************************************************************
ProjectSettings::~ProjectSettings()
{

}

//******************************************************************************
void ProjectSettings::Serialize(Serializer& stream)
{
  SerializeNameDefault(ProjectName, String());
  SerializeNameDefault(ProjectOwner, String());
  SerializeNameDefault(DefaultLevel, String());
  SerializeNameDefault(ProjectSpace, String());
  SerializeNameDefault(ProjectEngineRevision, uint(0));
  SerializeNameDefault(AutoTakeProjectScreenshot, true);
  SerializeNameDefault(mGuid, (Guid)0);

  // If we didn't have a guid before for some reason
  // (upgrading a project, etc...), then generate one now
  if(mGuid == 0 && stream.GetMode() == SerializerMode::Loading)
    GenerateProjectGuid();
}

//******************************************************************************
String ProjectSettings::GetProjectName()
{
  return ProjectName;
}

//******************************************************************************
String ProjectSettings::GetProjectFolder()
{
  return ProjectFolder;
}

//******************************************************************************
String ProjectSettings::GetContentFolder()
{
  return ContentFolder;
}

//******************************************************************************
String ProjectSettings::GetEditorContentFolder()
{
  return EditorContentFolder;
}

//******************************************************************************
void ProjectSettings::Save(bool overwriteRevisionNumber)
{
  String projectFile = FilePath::CombineWithExtension(ProjectFolder, ProjectName, ".zeroproj");
  if(overwriteRevisionNumber)
    ProjectEngineRevision = GetRevisionNumber();
  SaveToDataFile(*mOwner, projectFile);
}

//******************************************************************************
String ProjectSettings::GetScreenshotFile()
{
  return FilePath::Combine(EditorContentFolder, "ProjectScreenshot.png");
}

//******************************************************************************
bool ProjectSettings::ScreenshotAvailable()
{
  // If it's already loaded, it must be available
  // What if the screen shot file was removed..?
  if(ScreenshotLoaded())
    return true;

  String screenshotFile = GetScreenshotFile();
  return FileExists(screenshotFile);
}

//******************************************************************************
Image* ProjectSettings::GetScreenshot(bool forceReload)
{
  String screenshotFile = GetScreenshotFile();

  if(FileExists(screenshotFile))
  {
    // Free it if a screen shot has already been loaded
    if(mScreenshot.Data != nullptr)
    {
      // If we're not force reloading the file, check when the file
      // was modified. If it hasn't been modified since we last loaded it,
      // we don't need to reload it
      if(!forceReload)
      {
        // If the screen shot hasn't been modified, return the old one
        TimeType modifiedTime = GetFileModifiedTime(screenshotFile);

        // The screen shot hasn't changed, so return the old one
        if(mLastLoadedTime == modifiedTime)
          return &mScreenshot;
      }
      
      // Deallocate before we load the new one
      mScreenshot.Deallocate();
    }

    Status status;
    LoadFromPng(status, &mScreenshot, screenshotFile);

    // Store the time the file was last modified to compare the next
    // time the screen shot is used
    mLastLoadedTime = GetFileModifiedTime(screenshotFile);

    // Only return the image if it successfully loaded
    if(status.Succeeded())
      return &mScreenshot;

    return nullptr;
  }

  return nullptr;
}

//******************************************************************************
bool ProjectSettings::ScreenshotLoaded()
{
  return mScreenshot.Data != nullptr;
}

//******************************************************************************
bool ProjectSettings::NewScreenshotAvailable()
{
  if(!ScreenshotAvailable())
    return false;

  if(!ScreenshotLoaded())
    return true;

  TimeType modifiedTime = GetFileModifiedTime(GetScreenshotFile());
  return mLastLoadedTime < modifiedTime;
}

//******************************************************************************
// Saves a rect of the given image to the given file
void CreateSubImage(Image& image, Image& subImage, IntVec2 size,
                    IntVec2 offset)
{
  // Allocate an image of the new size
  subImage.Allocate((int)size.x, (int)size.y);

  for(int x = 0; x < size.x; ++x)
  {
    for(int y = 0; y < size.y; ++y)
    {
      ImagePixel& pixel = image.GetPixel(x + offset.x, y + offset.y);
      subImage.SetPixel(x, y, pixel);
    }
  }
}

//******************************************************************************
void ProjectSettings::SaveScreenshotFromImage(Image& image)
{
  float width = float(image.Width);
  float height = float(image.Height);
  Vec2 size = Vec2(width, height);
  Vec2 offset = Vec2::cZero;

  float currentRatio = width / height;
  float targetRatio = 16.0f / 9.0f;

  Vec2 scaleAspect;
  if(targetRatio < currentRatio)
    scaleAspect = Vec2(targetRatio / currentRatio, 1);
  else
    scaleAspect = Vec2(1, currentRatio / targetRatio);

  Vec2 newSize =  size * scaleAspect;
  offset = (size - newSize) * 0.5f;

  IntVec2 intSize((int)newSize.x, (int)newSize.y);
  IntVec2 intOffset((int)offset.x, (int)offset.y);

  Image subImage;
  CreateSubImage(image, subImage, intSize, intOffset);

  // Save the letter-boxed image to the file
  Status status;
  CreateDirectoryAndParents(EditorContentFolder);
  SaveToPng(status, &subImage, GetScreenshotFile());
}

//******************************************************************************
void ProjectSettings::GenerateProjectGuid()
{
  mGuid = GenerateUniqueId64();
}

//******************************************************************************
Guid ProjectSettings::GetProjectGuid()
{
  return mGuid;
}

//---------------------------------------------------- Content Library Reference
ZilchDefineType(ContentLibraryReference, builder, type)
{
  ZilchBindFieldProperty(mContentLibraryName);
}

//******************************************************************************
void ContentLibraryReference::Serialize(Serializer& stream)
{
  SerializeNameDefault(mContentLibraryName, String());
}

//--------------------------------------------------------------- Shared Content
ZilchDefineType(SharedContent, builder, type)
{
  ZeroBindComponent();
  //ZeroBindSetup(SetupMode::DefaultSerialization);

  ZilchBindFieldProperty(ExtraContentLibraries);
}

//******************************************************************************
void SharedContent::Serialize(Serializer& stream)
{
  SerializeName(ExtraContentLibraries);
}

//---------------------------------------------------------- Project Description
ZilchDefineType(ProjectDescription, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZilchBindFieldProperty(ShortDescription);
  ZilchBindFieldProperty(LongDescription);
  ZilchBindFieldProperty(Tags);
}

//******************************************************************************
void ProjectDescription::Serialize(Serializer& stream)
{
  SerializeNameDefault(ShortDescription, String());
  SerializeNameDefault(LongDescription, String());

  // This complicated logic is because proxy serialization doesn't properly work with arrays
  // so if we alter this structure to contain the information we need, older projects will
  // wipe out that information when they load (instead of saving it in proxy form) and we'll
  // lose all tag information. To deal with that we convert tags to a comma separated list of Name:TagType.
  if(stream.GetMode() == SerializerMode::Saving)
  {
    // Build the comma delimited list of tags with their types
    StringBuilder builder;
    forRange(StringParam tag, mProjectTags.All())
      builder.Append(BuildString(tag, ":Project,"));

    String Tags = builder.ToString();
    SerializeNameDefault(Tags, String());
  }
  else
  {
    String Tags;
    SerializeNameDefault(Tags, String());

    // First split the tags by the separator (comma)
    StringSplitRange splitRange = Tags.Split(",");
    for(; !splitRange.Empty(); splitRange.PopFront())
    {
      StringRange tag = splitRange.Front();
      // Make sure to account for any empty sets (especially since we don't leave off the last ',')
      if(tag.Empty())
        continue;

      // Split the string in half to find the name vs. type of the tag
      StringRange foundRange = tag.FindFirstOf(":");
      if(foundRange.Empty())
        continue;

      StringRange tagName = tag.SubString(tag.Begin(), foundRange.Begin());
      StringRange tagType = tag.SubString(foundRange.Begin() + 1, tag.End());
      // Remove any leading/trailing whitespace
      tagName = tagName.Trim();
      tagType = tagType.Trim();
      mProjectTags.Insert(tagName);
    }
  }
}

String ProjectDescription::GetTagsString(StringParam splitChar)
{
  String tagText;
  
  // Merge the tags into one list (and remove duplicates)
  TagList tags;
  forRange(StringParam tag, mProjectTags.All())
    tags.Insert(tag);

  // The tags are now in hashset order (random) so sort them alphabetically
  Array<String> sortedTags;
  forRange(StringParam tag, tags.All())
    sortedTags.PushBack(tag);
  Sort(sortedTags.All());

  // Now we can finally join all of the tags into one visual string
  tagText = String::JoinRange(splitChar, sortedTags.All());
  return tagText;
}

//--------------------------------------------------------- WindowLaunchSettings
ZilchDefineType(WindowLaunchSettings, builder, type)
{
  ZeroBindComponent();
  ZeroBindDocumented();
  ZeroBindSetup(SetupMode::DefaultSerialization);

  // Disabled usage of launch options popup for now
  //ZilchBindFieldProperty(mUseLaunchOptionsPopup)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  ZilchBindFieldProperty(mLaunchFullscreen);//->ZeroFilterEquality(mUseLaunchOptionsPopup, bool, false);
  ZilchBindFieldProperty(mWindowedResolution);//->ZeroFilterEquality(mUseLaunchOptionsPopup, bool, false);
}

void WindowLaunchSettings::Serialize(Serializer& stream)
{
  //SerializeNameDefault(mUseLaunchOptionsPopup, false);
  SerializeNameDefault(mLaunchFullscreen, true);
  SerializeNameDefault(mWindowedResolution, IntVec2(1280, 720));
}

//------------------------------------------------------------ FrameRateSettings
ZilchDefineType(FrameRateSettings, builder, type)
{
  ZeroBindComponent();
  ZeroBindDocumented();
  ZeroBindSetup(SetupMode::DefaultSerialization);

  ZilchBindFieldProperty(mVerticalSync);
  ZilchBindFieldProperty(mLimitFrameRate);
  ZilchBindGetterSetterProperty(FrameRate);
}

void FrameRateSettings::Serialize(Serializer& stream)
{
  SerializeNameDefault(mVerticalSync, false);
  SerializeNameDefault(mLimitFrameRate, true);
  SerializeNameDefault(mFrameRate, 60);
}

void FrameRateSettings::Initialize(CogInitializer& initializer)
{
}

int FrameRateSettings::GetFrameRate()
{
  return mFrameRate;
}

void FrameRateSettings::SetFrameRate(int frameRate)
{
  mFrameRate = Math::Max(frameRate, 1);
}

}//namespace Zero
