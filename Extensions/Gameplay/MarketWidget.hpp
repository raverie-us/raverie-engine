///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class MarketWidget : public WebBrowserWidget
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  MarketWidget(Composite* composite);

  void OnDownloadStarted(WebBrowserDownloadEvent* event);
};

} // namespace Zero
