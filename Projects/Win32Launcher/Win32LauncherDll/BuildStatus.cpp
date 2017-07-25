///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------------- Tweakables
namespace BuildColors
{
  const cstr cLocation = "LauncherUi/BuildColors";
  Tweakable(Vec4, Installed,    Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, NotInstalled, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, Deprecated,   Vec4(1,1,1,1), cLocation);
}

namespace BuildStatusUi
{
  const cstr cLocation = "LauncherUi/BuildStatus";
  Tweakable(Vec4, TextColor,                       Vec4(1,1,1,1),   cLocation);
  Tweakable(Vec4, InstallLatestColor,              Vec4(1,1,1,1),   cLocation);
  Tweakable(Vec4, ListBackground,                  Vec4(1,1,1,1),   cLocation);
  Tweakable(Vec4, ListBackgroundHighlight,         Vec4(1,1,1,1),   cLocation);
  Tweakable(Vec4, ListBackgroundClicked,           Vec4(1,1,1,1),   cLocation);
  Tweakable(Vec4, ListBackgroundSelected,          Vec4(1,1,1,1),   cLocation);
  Tweakable(Vec4, ListBackgroundSelectedHighlight, Vec4(1,1,1,1),   cLocation);
  Tweakable(Vec4, ArrowColor,                      Vec4(1,1,1,1),   cLocation);
  Tweakable(Vec2, BuildSelectorSize,               Pixels(121, 30), cLocation);
}

//----------------------------------------------------------------------- Events
namespace Events
{
  DefineEvent(BuildSelected);
  DefineEvent(BuildStateChanged);
}

//-------------------------------------------------------------------ZeroBuildEvent
ZilchDefineType(LauncherBuildEvent, builder, type)
{
}

//----------------------------------------------------------------- Build Status
//******************************************************************************
BuildStatusView::BuildStatusView(Composite* parent, ZeroBuild* version) :
  Composite(parent)
{
  mBuild = nullptr;

  SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Pixels(0, -3), Thickness::cZero));

  mBuildText = new Text(this, mLauncherRegularFont, 11);
  mBuildText->SetColor(BuildStatusUi::TextColor);
  mBuildText->SetText("No Build");
  mBuildText->mAlign = TextAlign::Right;
  mBuildText->SizeToContents();

  mInstallState = new Text(this, mLauncherBoldFont, 10);
  mInstallState->mAlign = TextAlign::Right;

  SetVersion(version);
}

//******************************************************************************
void BuildStatusView::UpdateTransform()
{
  Composite::UpdateTransform();
}

//******************************************************************************
void BuildStatusView::SetVersion(ZeroBuild* version)
{
  // Disconnect from the old version if it exists
  if(mBuild)
    mBuild->GetDispatcher()->Disconnect(this);
  
  if(version)
  {
    SetVersion(version->GetBuildId());
    ConnectThisTo(version, Events::InstallStarted, OnBuildStateChanged);
    ConnectThisTo(version, Events::InstallCompleted, OnBuildStateChanged);
    ConnectThisTo(version, Events::UninstallStarted, OnBuildStateChanged);
    ConnectThisTo(version, Events::UninstallCompleted, OnBuildStateChanged);
  }
  else
  {
    mBuildText->SetText("No Build");
    mBuildText->SizeToContents();
  }

  mBuild = version;

  UpdateInstallState();
}

//******************************************************************************
void BuildStatusView::SetVersion(const BuildId& buildId)
{
  // Disconnect from the old version if it exists
  if(mBuild)
  {
    mBuild->GetDispatcher()->Disconnect(this);
    mBuild = nullptr;
  }

  String text = buildId.ToDisplayString();
  mBuildText->SetText(text);
  mBuildText->SizeToContents();
}

//******************************************************************************
void BuildStatusView::UpdateInstallState()
{
  if(mBuild == nullptr)
  {
    mInstallState->SetText("NOT AVAILABLE");
    mInstallState->SetColor(BuildColors::Deprecated);
  }
  else
  {
    InstallState::Type state = mBuild->mInstallState;

    if(state != InstallState::Installed && mBuild->IsBad())
    {
      mInstallState->SetText("DEPRECATED");
      mInstallState->SetColor(BuildColors::Deprecated);
    }
    else
    {
      switch(state)
      {
      case InstallState::NotInstalled:
        mInstallState->SetText("NOT INSTALLED");
        mInstallState->SetColor(BuildColors::NotInstalled);
        break;
      case InstallState::Installed:
        mInstallState->SetText("INSTALLED");
        mInstallState->SetColor(BuildColors::Installed);
        break;
      case InstallState::Installing:
        mInstallState->SetText("INSTALLING...");
        mInstallState->SetColor(BuildColors::NotInstalled);
        break;
      case InstallState::Uninstalling:
        mInstallState->SetText("UNINSTALLING...");
        mInstallState->SetColor(BuildColors::NotInstalled);
        break;
      }
    }
  }

  mInstallState->SizeToContents();
  SizeToContents();
  MarkAsNeedsUpdate();
}

//******************************************************************************
void BuildStatusView::OnBuildStateChanged(Event*)
{
  UpdateInstallState();

  Event toSend;
  DispatchEvent(Events::BuildStateChanged, &toSend);
}

//------------------------------------------------------------------- Build List
//******************************************************************************
BuildList::BuildList(Composite* parent, VersionSelector* versionSelector,
                     ZeroBuild* selected, bool installedOnly) :
  Composite(parent),
  mSelected(selected),
  mVersionSelector(versionSelector)
{
  SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Pixels(0, 1), Thickness(1,1,1,1)));

  mBorder = CreateAttached<Element>(cWhiteSquareBorder);
  mBorder->SetColor(Vec4(0,0,0,1));

  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBackground->SetColor(BuildStatusUi::ListBackground);

  float latestHeight = 0.0f;

  ZeroBuild* latestBuild = versionSelector->GetLatestBuild();
  if(latestBuild && latestBuild->mInstallState != InstallState::Installed &&
     latestBuild->mInstallState != InstallState::Installing)
  {
    TextButton* installLatest = new TextButton(this, mLauncherBoldFont, 9);
    installLatest->mTextColor = ToByteColor(BuildStatusUi::InstallLatestColor);
    installLatest->SetText("+ INSTALL LATEST");
    installLatest->mBorder->SetVisible(false);
    installLatest->mBackgroundColor = ToByteColor(Vec4(1,1,1,0));
    installLatest->mBackgroundHoverColor = ToByteColor(BuildStatusUi::ListBackgroundHighlight);
    installLatest->mBackgroundClickedColor = ToByteColor(BuildStatusUi::ListBackgroundClicked);
    installLatest->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(20));

    latestHeight += installLatest->GetSize().y + Pixels(3);

    Element* splitter = CreateAttached<Element>(cWhiteSquare);
    splitter->SetColor(Vec4(0,0,0,1));
    splitter->SetNotInLayout(false);
    splitter->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);
    splitter->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(1));

    ConnectThisTo(installLatest, Events::ButtonPressed, OnInstallLatest);
  }

  mScrollArea = new ScrollArea(this);
  mScrollArea->DisableScrollBar(0);
  mScrollArea->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1);
  mScrollArea->mScrollSpeedScalar = 0.25f;
  Composite* clientArea = mScrollArea->GetClientWidget();

  ZeroBuildTagPolicy policy(mVersionSelector->mConfig);
  forRange(ZeroBuild* version, versionSelector->mVersions.All())
  {
    // Check if we should include this build (fitlers things like platform, branches, etc...)
    if(!policy.ShouldInclude(version))
      continue;

    // Only installed builds if specified
    if(!installedOnly || version->mInstallState == InstallState::Installed)
    {
      Element* background = clientArea->CreateAttached<Element>(cWhiteSquare);
      if(version == selected)
        background->SetColor(BuildStatusUi::ListBackgroundSelected);
      else
        background->SetColor(BuildStatusUi::ListBackground);

      BuildStatusView* build = new BuildStatusView(clientArea, version);
      build->SizeToContents();

      if(version == selected)
        build->mInstallState->SetColor(Vec4(0,0,0,0.4f));

      mEntries.PushBack(Entry(background, build));
    }
  }

  Vec2 selectorSize = BuildStatusUi::BuildSelectorSize;

  Vec2 clientSize;
  clientSize.x = selectorSize.x;
  clientSize.y = float(mEntries.Size()) * selectorSize.y;
  mScrollArea->SetClientSize(clientSize);

  // Only display 4 entries then add the scroll bar
  uint buildCount = Math::Min(4u, mEntries.Size());

  float buildListHeight = float(buildCount) * selectorSize.y;

  Vec2 size(clientSize.x, latestHeight + buildListHeight + Pixels(2));
  SetSize(size);

  Vec2 scrollAreaSize(size.x, buildListHeight);
  mScrollArea->SetSize(scrollAreaSize);

  ConnectThisTo(mScrollArea, Events::MouseMove, OnMouseMove);
  ConnectThisTo(mScrollArea, Events::MouseExit, OnMouseExitScrollArea);
  ConnectThisTo(mScrollArea, Events::LeftClick, OnLeftClick);
}

//******************************************************************************
void BuildList::UpdateTransform()
{
  Vec2 entrySize = BuildStatusUi::BuildSelectorSize;

  for(uint i = 0; i < mEntries.Size(); ++i)
  {
    Entry& entry = mEntries[i];

    Vec3 position = Vec3::cZero;
    position.y = float(i) * entrySize.y;

    Element* background = entry.first;
    background->SetTranslation(position);
    background->SetSize(entrySize);

    BuildStatusView* build = entry.second;
    
    position.x += entrySize.x - Pixels(15) - build->GetSize().x;
    position.y += Pixels(2);
    build->SetTranslation(position);
  }

  mBackground->SetTranslation(Pixels(1,1,0));
  mBackground->SetSize(mSize - Pixels(2,2));
  mBorder->SetSize(mSize);

  Composite::UpdateTransform();
}

//******************************************************************************
bool BuildList::TakeFocusOverride()
{
  this->HardTakeFocus();
  return true;
}

//******************************************************************************
void BuildList::OnMouseMove(MouseEvent* e)
{
  Vec2 localPosition = mScrollArea->GetClientWidget()->ToLocal(e->Position);
  int index = IndexFromPosition(localPosition);

  for(uint i = 0; i < mEntries.Size(); ++i)
  {
    Entry& entry = mEntries[i];
    Element* background = entry.first;
    bool selected = (entry.second->mBuild == mSelected);

    if(i == (int)index)
    {
      if(selected)
        background->SetColor(BuildStatusUi::ListBackgroundSelectedHighlight);
      else
        background->SetColor(BuildStatusUi::ListBackgroundHighlight);
    }
    else
    {
      if(selected)
        background->SetColor(BuildStatusUi::ListBackgroundSelected);
      else
        background->SetColor(BuildStatusUi::ListBackground);
    }
  }
}

//******************************************************************************
void BuildList::OnMouseExitScrollArea(MouseEvent* e)
{
  // Remove the highlight on all backgrounds
  forRange(Entry& entry, mEntries.All())
  {
    if(entry.second->mBuild == mSelected)
      entry.first->SetColor(BuildStatusUi::ListBackgroundSelected);
    else
      entry.first->SetColor(BuildStatusUi::ListBackground);
  }
}

//******************************************************************************
void BuildList::OnLeftClick(MouseEvent* e)
{
  if(e->Handled)
    return;

  Vec2 localPosition = mScrollArea->GetClientWidget()->ToLocal(e->Position);
  int index = IndexFromPosition(localPosition);

  if(index >= (int)mEntries.Size() || index < 0)
    return;

  LauncherBuildEvent eventToSend;
  eventToSend.mBuild = mEntries[index].second->mBuild;

  GetDispatcher()->Dispatch(Events::BuildSelected, &eventToSend);
}

//******************************************************************************
void BuildList::OnInstallLatest(Event*)
{
  ZeroBuild* latest = mVersionSelector->GetLatestBuild();

  // Install the latest
  mVersionSelector->InstallVersion(latest);

  LauncherBuildEvent eventToSend;
  eventToSend.mBuild = latest;

  GetDispatcher()->Dispatch(Events::BuildSelected, &eventToSend);
}

//******************************************************************************
int BuildList::IndexFromPosition(Vec2Param localPosition)
{
  Vec2 entrySize = BuildStatusUi::BuildSelectorSize;
  float index = localPosition.y / entrySize.y;
  return int(index);
}

//--------------------------------------------------------------- Build Selector
//******************************************************************************
BuildSelector::BuildSelector(Composite* parent, VersionSelector* versionSelector,
                             ZeroBuild* version) :
  Composite(parent),
  mVersionSelector(versionSelector)
{
  mInstalledOnly = true;

  SetSizing(SizePolicy::Fixed, BuildStatusUi::BuildSelectorSize);

  // The background is invisible so that it can still be clicked on
  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBackground->SetColor(Vec4(1,1,1,0));

  mCurrentBuild = new BuildStatusView(this, version);
  mCurrentBuild->SizeToContents();

  mArrow = CreateAttached<Element>("BuildSelectArrow");
  mArrow->SetColor(BuildStatusUi::ArrowColor);

  mBorder = CreateAttached<Element>(cWhiteSquareBorder);
  mBorder->SetColor(BuildColors::Deprecated);
  mBorder->SetVisible(false);

  ConnectThisTo(this, Events::MouseEnter, OnMouseEnter);
  ConnectThisTo(this, Events::LeftMouseDown, OnLeftMouseDown);
  ConnectThisTo(this, Events::MouseExit, OnMouseExit);
}

//******************************************************************************
void BuildSelector::UpdateTransform()
{
  mBackground->SetSize(mSize);
  mBorder->SetSize(mSize);

  Vec3 pos(0,2,0);
  pos.x = mSize.x - Pixels(15) - mCurrentBuild->GetSize().x;
  mCurrentBuild->SetTranslation(pos);
  Vec3 arrowPosition = Vec3(BuildStatusUi::BuildSelectorSize.mValue.x - Pixels(13), 7, 0);
  mArrow->SetTranslation(arrowPosition);

  Composite::UpdateTransform();
}

//******************************************************************************
void BuildSelector::SetBuild(ZeroBuild* version)
{
  mCurrentBuild->SetVersion(version);
  mCurrentBuild->MarkAsNeedsUpdate();
}

//******************************************************************************
void BuildSelector::SetBuild(const BuildId& buildId)
{
  mCurrentBuild->SetVersion(buildId);
  mCurrentBuild->MarkAsNeedsUpdate();
}

//******************************************************************************
ZeroBuild* BuildSelector::GetBuild()
{
  return mCurrentBuild->mBuild;
}

//******************************************************************************
void BuildSelector::OnMouseEnter(Event*)
{
  if(mBuildList.IsNull())
    mBackground->SetColor(BuildStatusUi::ListBackgroundHighlight);
}

//******************************************************************************
void BuildSelector::OnMouseExit(Event*)
{
  if(mBuildList.IsNull())
    mBackground->SetColor(Vec4(1,1,1,0));
}

//******************************************************************************
void BuildSelector::OnLeftMouseDown(MouseEvent* e)
{
  if(mBuildList.IsNotNull())
  {
    if(IsMouseOver())
      mBackground->SetColor(Vec4(1,1,1,0.05f));
    else
      mBackground->SetColor(Vec4(1,1,1,0));
    mBuildList.SafeDestroy();
    return;
  }

  mBackground->SetVisible(true);

  // we want to ignore the currently selected build
  Composite* popUp = GetRootWidget()->GetPopUp();
  BuildList* list = new BuildList(popUp, mVersionSelector, GetBuild(), mInstalledOnly);

  Vec3 translation = GetScreenPosition();
  translation.y -= list->GetSize().y;

  list->SetTranslation(translation);
  list->TakeFocus();

  ConnectThisTo(list, Events::BuildSelected, OnBuildSelected);
  ConnectThisTo(list, Events::FocusLostHierarchy, OnListFocusLost);
  ConnectThisTo(list, Events::FocusReset, OnListFocusReset);

  mBuildList = list;
}

//******************************************************************************
void BuildSelector::OnBuildSelected(LauncherBuildEvent* e)
{
  SetBuild(e->mBuild);
  CloseBuildList();

  DispatchEvent(Events::BuildSelected, e);
}

//******************************************************************************
void BuildSelector::OnListFocusLost(FocusEvent* e)
{
  // If we gained focus, we can let the left mouse down event response close
  // the build list. If we just closed it here, the left mouse down event
  // response would just open another one. This is a side-affect of the
  // handle system not resolving when an object is marked for destruction, but
  // not actually deleted yet
  bool weGainedFocus = this->IsAncestorOf(e->ReceivedFocus);
  if(!weGainedFocus)
    CloseBuildList();
}

//******************************************************************************
void BuildSelector::OnListFocusReset(Event*)
{
  CloseBuildList();
}

//******************************************************************************
void BuildSelector::CloseBuildList()
{
  mBuildList.SafeDestroy();
  mBackground->SetColor(Vec4(1,1,1,0));
}

//******************************************************************************
void BuildSelector::SetHighlight(bool state)
{
  if(state)
  {
    // Only start a new animation if there's not one already going
    if(GetActions()->IsEmpty())
    {
      ToColor0();
      mBorder->SetVisible(true);
    }
  }
  else
  {
    // Cancel all actions
    GetActions()->Cancel();

    mBorder->SetVisible(false);
  }
}

//******************************************************************************
void BuildSelector::ToColor0()
{
  Vec4 color = BuildColors::Deprecated;
  
  ActionSequence* sequence = new ActionSequence(this);
  sequence->Add(Fade(mBorder, color, 0.8f));
  sequence->Add(new CallAction<BuildSelector, &BuildSelector::ToColor1>(this));
}

//******************************************************************************
void BuildSelector::ToColor1()
{
  Vec4 color = BuildColors::Deprecated;
  color *= 0.7f;

  ActionSequence* sequence = new ActionSequence(this);
  sequence->Add(Fade(mBorder, color, 0.8f));
  sequence->Add(new CallAction<BuildSelector, &BuildSelector::ToColor0>(this));
}

}//namespace Zero
