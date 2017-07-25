///////////////////////////////////////////////////////////////////////////////
///
/// \file Loading.cpp
/// 
///
/// Authors: Chris Peters, Nathan Carlson
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

LoadingWindow::LoadingWindow(Composite* composite)
  : Composite(composite, AttachType::Direct)
{
  mMainText = new Text(this, "NotoSans-Bold", 64);
  mMainText->SetColor(Vec4(1.0f));

  mPendingText = new Text(this, "NotoSans-Bold", 64);
  mPendingText->SetColor(Vec4(1.0f));
  mPendingText->SetText("...");
  mPendingText->SizeToContents();

  ConnectThisTo(GetRootWidget(), Events::WidgetUpdate, OnUpdate);
}

void LoadingWindow::Activate(StringParam taskName)
{
  // Create the dark screen
  mDarkScreen.SafeDestroy();
  ColorBlock* block = CreateBlackOut(GetParent(), AttachType::Direct);
  block->SetColor(Vec4(0.0f, 0.0f, 0.0f, 0.8f));
  mDarkScreen = block;

  // Reactivate and move above dark screen
  MoveToFront();
  SetActive(true);

  SetLoadingName(taskName);
  mTime = 0.0f;

  OsWindow* mainWindow = Z::gEditor->mOsWindow;
  mainWindow->SendProgress(ProgressType::Indeterminate, 0.0f);
}

void LoadingWindow::Deactivate()
{
  SetActive(false);

  // Fade and destroy the dark screen
  if (ColorBlock* block = mDarkScreen)
  {
    ActionSequence* seq = new ActionSequence(block);
    seq->Add(Fade(block, Vec4(0.0f), 0.3f));
    seq->Add(DestroyAction(block));
  }

  OsWindow* mainWindow = Z::gEditor->mOsWindow;
  mainWindow->SendProgress(ProgressType::None, 0.0f);
}

void LoadingWindow::SetLoadingName(StringParam text)
{
  mMainText->SetText(text);
  mMainText->SizeToContents();

  mPendingText->SetTranslation(mMainText->mTranslation + Vec3(mMainText->mSize.x, 0.0f, 0.0f));
}

void LoadingWindow::OnUpdate(UpdateEvent* event)
{
  if (!GetActive())
    return;

  mTime += event->Dt;
  if (mTime >= 0.5f)
  {
    static const String pendingText[4] = {"", ".", "..", "..."};
    static uint textIndex = 0;

    mTime = 0.0f;
    textIndex = (textIndex + 1) % 4;
    mPendingText->SetText(pendingText[textIndex]);
  }
}

void LoadingWindow::UpdateTransform()
{
  SetSize(mMainText->mSize);
  CenterToWindow(GetParent(), this, false);

  if (ColorBlock* darkScreen = mDarkScreen)
    darkScreen->SetSize(GetParent()->mSize);

  Composite::UpdateTransform();
}

}//namespace Zero
