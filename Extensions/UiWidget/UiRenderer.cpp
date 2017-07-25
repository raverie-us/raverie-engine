///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015-216, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------------ Ui Renderer
//******************************************************************************
ZilchDefineType(UiRenderer, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDependency(UiRootWidget);

  ZilchBindMethod(Render);
}

//******************************************************************************
UiRenderer::UiRenderer()
{
  mStencilCount = 0;
  mStencilDrawMode = StencilDrawMode::None;

  BlendSettings blendSettings;
  blendSettings.SetBlendAlpha();

  DepthSettings depthSettings;
  depthSettings.SetDepthRead(TextureCompareFunc::LessEqual);
  depthSettings.SetStencilIncrement();

  mStencilAddSettings.mBlendSettings[0] = blendSettings;
  mStencilAddSettings.mDepthSettings = depthSettings;

  depthSettings.SetStencilDecrement();
  mStencilRemoveSettings.mBlendSettings[0] = blendSettings;
  mStencilRemoveSettings.mDepthSettings = depthSettings;

  depthSettings.SetStencilTestMode(TextureCompareFunc::Equal);
  mStencilTestSettings.mBlendSettings[0] = blendSettings;
  mStencilTestSettings.mDepthSettings = depthSettings;
}

//******************************************************************************
void UiRenderer::Serialize(Serializer& stream)
{
  
}

//******************************************************************************
void UiRenderer::Initialize(CogInitializer& initializer)
{
  mWidget = GetOwner()->has(UiWidget);
  mRootWidget = GetOwner()->has(UiRootWidget);
}

//******************************************************************************
void UiRenderer::Render(RenderTasksEvent* e, RenderTarget* color,
                        RenderTarget* depth, MaterialBlock* renderPass)
{
  if(e == nullptr || color == nullptr || depth == nullptr || renderPass == nullptr)
  {
    DoNotifyExceptionAssert("Cannot render Widgets", "All parameters must be satisfied.");
    return;
  }

  if(!IsDepthStencilFormat(depth->mTexture->mFormat))
  {
    DoNotifyExceptionAssert("Cannot render Widgets", "Depth target must have stencil (Depth24Stencil8).");
    return;
  }

  // Reset stencil values
  mStencilDrawMode = StencilDrawMode::None;
  mStencilCount = 0;

  // Render all widgets
  Vec4 colorTransform(1);
  RenderWidgets(e, color, depth, renderPass, mWidget, colorTransform);
  FlushGraphicals(e, color, depth, renderPass);
}

//******************************************************************************
void UiRenderer::RenderWidgets(RenderTasksEvent* e, RenderTarget* color, RenderTarget* depth,
                              MaterialBlock* renderPass, UiWidget* widget, Vec4Param colorTransform)
{
  // Don't render inactive widgets
  if(!widget->GetActive())
    return;

  // Build color transform
  Vec4 hierarchyColor = colorTransform * widget->mHierarchyColor;
  Vec4 localColor = hierarchyColor * widget->mLocalColor;

  // Set the color on graphicals
  Cog* widgetCog = widget->GetOwner();
  if(Sprite* sprite = widgetCog->has(Sprite))
    sprite->mVertexColor = localColor;
  if(SpriteText* spriteText = widgetCog->has(SpriteText))
    spriteText->mVertexColor = localColor;

  // Write out stencil if we're clipping our children
  if(widget->GetClipChildren())
    AddGraphical(e, color, depth, renderPass, widgetCog, StencilDrawMode::Add, 1);

  // Add the widget to be rendered
  if(widget->GetVisible())
    AddGraphical(e, color, depth, renderPass, widgetCog, StencilDrawMode::Test, 0);

  // Recurse to all children
  forRange(UiWidget& child, widget->GetChildren())
    RenderWidgets(e, color, depth, renderPass, &child, hierarchyColor);

  // Remove the written stencil data
  if(widget->GetClipChildren())
    AddGraphical(e, color, depth, renderPass, widgetCog, StencilDrawMode::Remove, -1);
}

//******************************************************************************
void UiRenderer::AddGraphical(RenderTasksEvent* e, RenderTarget* color, RenderTarget* depth,
                              MaterialBlock* renderPass, Cog* widgetCog,
                              StencilDrawMode::Enum stencilMode, uint stencilIncrement)
{
  if(Graphical* graphical = widgetCog->has(Graphical))
  {
    // If the stencil mode has changed, we need to commit all graphicals from the last group
    if(mStencilDrawMode != stencilMode)
      FlushGraphicals(e, color, depth, renderPass);

    mStencilDrawMode = stencilMode;
    mStencilCount += stencilIncrement;
    mGraphicals.Add(graphical);
  }
}

//******************************************************************************
void UiRenderer::FlushGraphicals(RenderTasksEvent* e, RenderTarget* color,
                                 RenderTarget* depth, MaterialBlock* renderPass)
{
  if(mGraphicals.GetCount() == 0)
    return;

  if(mStencilDrawMode == StencilDrawMode::Add)
  {
    mStencilAddSettings.SetDepthTarget(depth);
    e->AddRenderTaskRenderPass(mStencilAddSettings, mGraphicals, *renderPass);
  }
  else if(mStencilDrawMode == StencilDrawMode::Remove)
  {
    mStencilRemoveSettings.SetDepthTarget(depth);
    e->AddRenderTaskRenderPass(mStencilRemoveSettings, mGraphicals, *renderPass);
  }
  else if(mStencilDrawMode == StencilDrawMode::Test)
  {
    mStencilTestSettings.SetColorTarget(color);
    mStencilTestSettings.SetDepthTarget(depth);
    mStencilTestSettings.mDepthSettings.mStencilTestValue = mStencilCount;
    e->AddRenderTaskRenderPass(mStencilTestSettings, mGraphicals, *renderPass);
  }

  mGraphicals.Clear();
}

}//namespace Zero
