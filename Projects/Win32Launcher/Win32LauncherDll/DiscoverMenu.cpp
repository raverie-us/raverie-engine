///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace DiscoverUi
{
  const cstr cLocation = "LauncherUi/Discover";
  Tweakable(Vec4, SpacerColor,   Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, DevUpdateText, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, DevUpdateDate, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, NewTextColor, Vec4(1,1,1,1), cLocation);
}

//-------------------------------------------------------------- Tile Separators
/// Used in-between the tiles.
class TileSeparators : public Composite
{
public:
  //****************************************************************************
  TileSeparators(Composite* parent, Vec4Param color) : Composite(parent)
  {
    mImage = CreateAttached<Element>(cWhiteSquare);
    mImage->SetColor(color);
  }

  //****************************************************************************
  void UpdateTransform() override
  {
    mImage->SetSize(GetSize());
    Composite::UpdateTransform();
  }

  Element* mImage;
};

//******************************************************************************
void CreateSpacer(Composite* parent)
{
  TileSeparators* e = new TileSeparators(parent, DiscoverUi::SpacerColor);
  e->SetMinSize(Pixels(8,8));
  e->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);
  e->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1);
}

//---------------------------------------------------------------- Discover Tile
//******************************************************************************
DiscoverTile::DiscoverTile(Composite* parent, StringParam text,
                           StringParam fontStyle, float titleHeight,
                           StringParam url) :
  Composite(parent),
  mUrl(url)
{
  SetSizing(SizeAxis::Y, SizePolicy::Flex, 1);

  // Create an invisible background to make the entire widget interactive
  mInteractive = CreateAttached<Element>(cWhiteSquare);
  mInteractive->SetColor(Vec4(0,0,0,0));

  // The background for the title
  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBackground->SetColor(Vec4(0,0,0,0.75f));
  mBackground->SetSize(Vec2(parent->GetSize().x, titleHeight));

  // The title text
  mTitle = new Text(this, fontStyle);
  mTitle->SetText(text);
  mTitle->SizeToContents();
  mTitle->SetTranslation(Pixels(4,0,0));
  mTitle->SetColor(Vec4(0.6, 0.6, 0.6, 1));

  mTitleBgHeight = titleHeight;

  ConnectThisTo(this, Events::MouseEnter, OnMouseEnter);
  ConnectThisTo(this, Events::LeftClick, OnLeftClick);
  ConnectThisTo(this, Events::MouseExit, OnMouseExit);
}

//******************************************************************************
void DiscoverTile::UpdateTransform()
{
  mInteractive->SetSize(mSize);
  mBackground->mSize.x = mSize.x;
  Composite::UpdateTransform();
}

//******************************************************************************
void DiscoverTile::OnMouseEnter(MouseEvent* e)
{
  // Grow the background and highlight the text
  ActionGroup* seq = new ActionGroup(this);
  float height = mTitleBgHeight * 1.2f;
  seq->Add(SizeWidgetAction(mBackground, Vec2(mSize.x, height), 0.15));
  seq->Add(Fade(mTitle, Vec4(1,1,1,1), 0.3));
}

//******************************************************************************
void DiscoverTile::OnLeftClick(MouseEvent* e)
{
  Os::SystemOpenNetworkFile(mUrl.c_str());
}

//******************************************************************************
void DiscoverTile::OnMouseExit(MouseEvent* e)
{
  // Shrink the background and dim the text
  ActionGroup* seq = new ActionGroup(this);
  float height = mTitleBgHeight * 1.2f;
  seq->Add(SizeWidgetAction(mBackground, Vec2(mSize.x, mTitleBgHeight), 0.15));
  seq->Add(Fade(mTitle, Vec4(0.6, 0.6, 0.6, 1), 0.3));
}

//------------------------------------------------------------- Dev Update Entry
//******************************************************************************
DevUpdateEntry::DevUpdateEntry(Composite* parent, DeveloperNotes& notes) :
  Composite(parent)
{
  SetLayout(CreateStackLayout());

  Composite* topRow = new Composite(this);
  topRow->SetLayout(CreateRowLayout());
  {
    TimeType currTime = Time::GetTime();
    CalendarDateTime dateTime = Time::GetLocalTime(currTime);

    cstr month = Time::GetMonthString(dateTime.Month);
    String date = String::Format("%s %i, %i", month, dateTime.Day, dateTime.Year);

    new Spacer(topRow, SizePolicy::Flex, Vec2(1));

    if(date == notes.mDate)
    {
      mNewText = new Text(topRow, mLauncherRegularFont, 18);
      mNewText->SetText("NEW");
      mNewText->SetColor(DiscoverUi::NewTextColor);

      new Spacer(topRow, SizePolicy::Fixed, Pixels(10,0));
    }

    mDate = new Text(topRow, "DevUpdateDate");
    mDate->SetText(notes.mDate);
    mDate->SizeToContents();
    mDate->mAlign = TextAlign::Right;
    mDate->SetColor(DiscoverUi::DevUpdateDate);
  }

  mUpdate = new MultiLineText(this, "DevUpdateText");
  mUpdate->SetText(notes.mNotes);
  mUpdate->SizeToContents();
  mUpdate->mBackground->SetActive(false);
  mUpdate->mBorder->SetActive(false);
  mUpdate->SetColor(DiscoverUi::DevUpdateText);
}

//---------------------------------------------------------------- Discover Menu
//******************************************************************************
DiscoverMenu::DiscoverMenu(Composite* parent, LauncherWindow* launcher) :
  Composite(parent),
  mLauncher(launcher)
{
  SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(0, 0), Thickness::cZero));

  Composite* left = new Composite(this);
  left->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Vec2::cZero, Thickness(Pixels(32, 27, 0, 0))));
  left->SetSizing(SizePolicy::Fixed, Pixels(672, 328));
  {
    mBackground = left->CreateAttached<Element>("DiscoverBgDefault");
    CreateTiles(left);

    Composite* fill = new Composite(left);
    fill->SetSizing(SizePolicy::Flex, Vec2(1));
  }

  Composite* spacer = new Composite(this);
  spacer->SetSizing(SizePolicy::Flex, Vec2(1));

  Composite* right = new Composite(this);
  right->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Pixels(0, 3), Thickness(Pixels(0,18,27,5))));
  right->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(300));
  right->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1);
  {
    // Used to indent the title to align with the entry text
    Composite* titleRow = new Composite(right);
    titleRow->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2::cZero, Thickness(Pixels(0,0,15,0))));

    Spacer* spacer = new Spacer(titleRow);
    spacer->SetSizing(SizePolicy::Flex, Vec2(1));

    Text* title = new Text(titleRow, "DevUpdateTitle");
    title->SetColor(DiscoverUi::DevUpdateText);
    title->SetText("DEV UPDATES");
    title->SizeToContents();
    title->mAlign = TextAlign::Right;

    mDevUpdatesArea = new ScrollArea(right);
    mDevUpdatesArea->SetSizing(SizePolicy::Flex, Vec2(1));
    mDevUpdatesArea->GetClientWidget()->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Pixels(0, 7), Thickness::cZero));
  }

  ConnectThisTo(this, Events::MenuDisplayed, OnMenuDisplayed);
  ConnectThisTo(mLauncher, Events::CheckForUpdates, OnCheckForUpdates);
  DownloadDevNotes();
}

//******************************************************************************
void DiscoverMenu::UpdateTransform()
{
  mBackground->SetSize(Pixels(640, 328));
  mBackground->SetTranslation(mTiles->GetTranslation());

  Composite::UpdateTransform();
}

//******************************************************************************
void DiscoverMenu::CreateTiles(Composite* parent)
{
  cstr zeroHubUrl = "https://www.zeroengine.io";
  cstr docUrl = "https://docs.zeroengine.io";
  cstr zilchUrl = "https://zilch.zeroengine.io";
  cstr roadmapUrl = "https://roadmap.zeroengine.io";
  cstr digipenUrl = "https://www.digipen.edu";
  cstr qaUrl = "https://ask.zeroengine.io/";
  cstr marketUrl = "https://market.zeroengine.io";

  mTiles = new Composite(parent);
  mTiles->SetTranslation(Pixels(32, 27, 0));
  mTiles->SetSizing(SizePolicy::Fixed, Pixels(640, 328));
  mTiles->SetMinSize(Pixels(640, 328));
  mTiles->SetLayout(CreateRowLayout());
  {
    Composite* left = new Composite(mTiles);
    left->SetSizing(SizePolicy::Fixed, Pixels(316, 329));
    left->SetLayout(CreateStackLayout());
    {
      // Documentation tile
      Composite* zeroHub = new DiscoverTile(left, "ZEROHUB", "TileTitleText", Pixels(74), zeroHubUrl);
      zeroHub->SetSizing(SizePolicy::Fixed, Pixels(316, 170));

      // Spacer
      CreateSpacer(left);

      Composite* bottom = new Composite(left);
      bottom->SetSizing(SizePolicy::Fixed, Pixels(316, 150));
      bottom->SetLayout(CreateRowLayout());
      {
        // Zilch tile
        Composite* zilch = new DiscoverTile(bottom, "ZILCH", "TileTitleTextMedium", Pixels(44), zilchUrl);
        zilch->SetSizing(SizePolicy::Fixed, Pixels(154, 150));

        // Spacer
        CreateSpacer(bottom);

        // Roadmap
        Composite* roadmap = new DiscoverTile(bottom, "ROADMAP", "TileTitleTextMedium", Pixels(44), roadmapUrl);
        roadmap->SetSizing(SizePolicy::Fixed, Pixels(154, 150));
      }
    }

    // Spacer
    CreateSpacer(mTiles);

    Composite* right = new Composite(mTiles);
    right->SetSizing(SizePolicy::Fixed, Pixels(316, 328));
    right->SetLayout(CreateStackLayout());
    {
      // Documentation tile
      Composite* documentation = new DiscoverTile(right, "DOCUMENTATION", "TileTitleText", Pixels(74), docUrl);
      documentation->SetSizing(SizePolicy::Fixed, Pixels(316, 252));

      // Spacer
      CreateSpacer(right);

      Composite* bottom = new Composite(right);
      bottom->SetSizing(SizePolicy::Fixed, Pixels(316, 68));
      bottom->SetLayout(CreateRowLayout());
      {
        // Market place
        Composite* market = new DiscoverTile(bottom, "MARKET", "TileTitleTextSmall", Pixels(39), marketUrl);
        market->SetSizing(SizePolicy::Fixed, Pixels(240, 68));

        // Spacer
        CreateSpacer(bottom);

        // Q&A tile
        Composite* questionAnswer = new DiscoverTile(bottom, "Q&A", "TileTitleTextSmall", Pixels(39), qaUrl);
        questionAnswer->SetSizing(SizePolicy::Fixed, Pixels(68, 68));
      }
    }
  }
}

//******************************************************************************
void DiscoverMenu::DownloadDevNotes()
{
  BackgroundTask* task = mLauncher->mVersionSelector->DownloadDeveloperNotes();
  ConnectThisTo(task, Events::BackgroundTaskCompleted, OnDeveloperNotesDownloaded);
}

//******************************************************************************
void DiscoverMenu::OnCheckForUpdates(Event* e)
{
  DownloadDevNotes();
}

//******************************************************************************
void DiscoverMenu::OnDeveloperNotesDownloaded(BackgroundTaskEvent* e)
{
  if(e->mTask->IsCompleted())
  {
    // Clean all of the old entries
    for(size_t i = 0; i < mDevUpdateEntries.Size(); ++i)
      mDevUpdateEntries[i]->Destroy();
    mDevUpdateEntries.Clear();

    // Load the available builds into the version selector
    DownloadTaskJob* job = (DownloadTaskJob*)e->mTask->GetFinishedJob();
    
    Array<DeveloperNotes> DevUpdates;

    Status status;
    //load the data into an array
    DataTreeLoader loader;
    loader.OpenBuffer(status, job->mData);
    loader.SerializeFieldDefault("DevUpdates", DevUpdates, DevUpdates);

    for(uint i = 0; i < DevUpdates.Size(); ++i)
    {
      DevUpdateEntry* entry = new DevUpdateEntry(mDevUpdatesArea, DevUpdates[i]);
      entry->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(270));
      entry->SizeToContents();
      mDevUpdateEntries.PushBack(entry);
    }

    mDevUpdatesArea->GetClientWidget()->SizeToContents();
  }
}

//******************************************************************************
void DiscoverMenu::OnMenuDisplayed(Event* e)
{
  mLauncher->mMainButton->SetVisible(false);
  mLauncher->mSearch->SetVisible(false);
}

}//namespace Zero
