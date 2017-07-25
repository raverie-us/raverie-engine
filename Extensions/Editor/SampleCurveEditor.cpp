///////////////////////////////////////////////////////////////////////////////
///
/// \file SampleCurveEditor.cpp
/// Implementation of the SampleCurveEditor Composite.
/// 
/// Authors: Joshua Claeys
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//---------------------------------------------------------- Sample Curve Editor
//******************************************************************************
SampleCurveEditor::SampleCurveEditor(Composite* parent, GraphWidget* graph)
 : CurveEditor(parent)
{
  mGraph = graph;
}

//******************************************************************************
Vec2 SampleCurveEditor::ToPixelPosition(Vec2 graphPos)
{
  return mGraph->ToPixelPosition(graphPos);
}

//******************************************************************************
Vec2 SampleCurveEditor::ToGraphPosition(Vec2 pixelPos)
{
  return mGraph->ToGraphPosition(pixelPos);
}

//******************************************************************************
Vec2 SampleCurveEditor::ClampPixelPosition(Vec2 pixelPos)
{
  return mGraph->ClampPos(pixelPos);
}

//---------------------------------------------------------- Sample Curve Object
//******************************************************************************
SampleCurveObject::SampleCurveObject(SampleCurveEditor* editor,
                                     SampleCurve* curve)
  : CurveObject(editor)
{
  mIgnoreCurveEditorEvents = false;

  ConnectThisTo(this, Events::PushDebugSamples, OnPushDebugSamples);
  ConnectThisTo(this, Events::CurveModified, OnCurveModified);
  ConnectThisTo(this, Events::ControlPointModified, OnControlPointModified);
  ConnectThisTo(Z::gEditor, Events::Save, OnSave);

  SetCurve(curve);
}

//******************************************************************************
void SampleCurveObject::GetCurve(Vec3Array& curve)
{
  CurveObject::GetCurve(curve);

  if(!curve.Empty())
  {
    if(curve.Front().x > 0.0f)
    {
      Vec3 newFront(0, curve.Front().y, 0);
      curve.InsertAt(0, newFront);
    }

    if(curve.Back().x < 1.0f)
    {
      Vec3 newBack(1, curve.Back().y, 0);
      curve.PushBack(newBack);
    }
  }
}

//******************************************************************************
void SampleCurveObject::RemoveControlPoint(ControlPoint* controlPoint)
{
  if(mControlPoints.Size() == 2)
  {
    DoNotifyWarning("Cannot delete", "There must be at least 2 control points in a SampleCurve.");
    return;
  }

  CurveObject::RemoveControlPoint(controlPoint);
}

//******************************************************************************
void SampleCurveObject::SetCurve(SampleCurve* curve)
{
  // Set the curve
  mSampleCurve = curve;

  mError = curve->mError;

  // Load the curve for editing
  LoadFromCurve(curve);
}

//******************************************************************************
SampleCurve* SampleCurveObject::GetSampleCurve()
{
  return mSampleCurve;
}

//******************************************************************************
void SampleCurveObject::LoadFromCurve(SampleCurve* curve)
{
  mIgnoreCurveEditorEvents = true;
  // Clear any previous data on the curve editor
  Clear();

  // Walk each control point of the SampleCurve
  SampleCurve::ControlPointArray& controlPoints = curve->GetControlPoints();
  for(uint i = 0; i < controlPoints.Size(); ++i)
  {
    // Get the control point from the curve
    SampleCurve::ControlPoint& curvePoint = controlPoints[i];

    Vec2 pos = curvePoint.GetPosition();
    Vec2 tIn = curvePoint.TangentIn;
    Vec2 tOut = curvePoint.TangentOut;
    uint editorFlags = curvePoint.EditorFlags;

    // Create a control point in the editor
    CreateControlPoint(pos, tIn, tOut, editorFlags);
  }

  mIgnoreCurveEditorEvents = false;
}

//******************************************************************************
void SampleCurveObject::BakeToCurve(SampleCurve* curve)
{
  // Clear the SampleCurve
  SampleCurve::ControlPointArray& controlPoints = curve->GetControlPoints();
  controlPoints.Clear();

  // Walk each control point in the editor and add them to the SampleCurve
  forRange(auto point, GetControlPoints())
  {
    Vec2 pos = point->mGraphPosition;
    Vec2 in = point->mTangentIn->GetGraphDirection();
    Vec2 out = point->mTangentOut->GetGraphDirection();

    // Add it to the SampleCurve
    curve->AddControlPoint(pos, in, out, point->mEditorFlags);
  }

  // Re-bake the curve
  curve->Bake();
}

//******************************************************************************
void SampleCurveObject::OnCurveModified(Event* e)
{
  SampleCurve* curve = mSampleCurve;
  if(curve == NULL || mIgnoreCurveEditorEvents)
    return;

  curve->Clear();
  forRange(CurveEditor::ControlPoint* cp, mControlPoints.All())
  {
    Vec2 pos = cp->mGraphPosition;
    Vec2 tanIn = cp->GetTangentIn();
    Vec2 tanOut = cp->GetTangentOut();
    uint editorFlags = cp->mEditorFlags;

    // Add the control point
    curve->AddControlPoint(pos, tanIn, tanOut, editorFlags);
  }
  
  MetaOperations::NotifyObjectModified(curve);

  TabModifiedEvent eventToSend(true);
  mEditor->DispatchBubble(Events::TabModified, &eventToSend);
}

//******************************************************************************
void SampleCurveObject::OnControlPointModified(CurveEvent* e)
{
  if(mIgnoreCurveEditorEvents)
    return;

  mIgnoreCurveEditorEvents = true;

  // Clamp the position of the point to the graph
  Vec2 pos = e->mControlPoint->mGraphPosition;
  Vec2 pixelPos = mEditor->ToPixelPosition(pos);
  pixelPos = mEditor->ClampPixelPosition(pixelPos);
  pos = mEditor->ToGraphPosition(pixelPos);

  e->mControlPoint->SetGraphPosition(pos);

  mIgnoreCurveEditorEvents = false;
}

//******************************************************************************
void SampleCurveObject::OnPushDebugSamples(Event* e)
{
  if(SampleCurve* curve = GetSampleCurve())
  {
    forRange(auto curveSample, curve->DebugSamples.All())
    {
      // Pull the data from the sample curve
      String sampleName = curveSample.first;
      float sampleTime = curveSample.second.LastSampledTime;
      Vec4 sampleColor = curveSample.second.Color;

      // Add the sample data to the curve editor
      CurveEditor::DebugSample editorSample;
      editorSample.Time = sampleTime;
      editorSample.Id = sampleName;
      editorSample.Color = sampleColor;
      mEditor->mDebugSamples.PushBack(editorSample);
    }

    // Clear the sample data from the curve
    curve->DebugSamples.Clear();
  }
}

//******************************************************************************
void SampleCurveObject::OnSave(Event* e)
{
  // Save the content item of the curve we're editing
  SampleCurve* curve = GetSampleCurve();
  if(curve)
    curve->mContentItem->SaveContent();

  TabModifiedEvent eventToSend(false);
  mEditor->DispatchBubble(Events::TabModified, &eventToSend);
}

//---------------------------------------------------- Multi Sample Curve Editor
//******************************************************************************
MultiSampleCurveEditor::MultiSampleCurveEditor(Composite* parent)
  : Composite(parent)
{
  SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Pixels(0,0), Thickness::cZero));
  CurveEditing::CurveEditorToolbar* toolBar;
  IconButton* linearTangents;
  IconButton* splitTangents;
  IconButton* weightedTangents;

  Composite* toolBarArea = new Composite(this);
  toolBarArea->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(4,0), Thickness::cZero));
  {
    toolBar = new CurveEditing::CurveEditorToolbar(toolBarArea, Pixels(40));

    // Linear Tangents
    linearTangents = new IconButton(toolBarArea);
    linearTangents->SetToolTip("Linear Tangents");
    linearTangents->SetIcon("LinearTangents");
    linearTangents->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(40));

    // Split Tangents
    splitTangents = new IconButton(toolBarArea);
    splitTangents->SetToolTip("Split Tangents");
    splitTangents->SetIcon("SplitTangents");
    splitTangents->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(40));

    // Weighted Tangents
    weightedTangents = new IconButton(toolBarArea);
    weightedTangents->SetToolTip("Weighted Tangents");
    weightedTangents->SetIcon("WeightedTangents");
    weightedTangents->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(40));
  }

  mGraphArea = new Composite(this);
  mGraphArea->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1);
  mGraphArea->SetClipping(true);
  {
    mGraph = new GraphWidget(mGraphArea);

    // We're adding a text box over the last labels
    mGraph->mHideLastLabel = true;

    mCurveEditor = new SampleCurveEditor(mGraphArea, mGraph);
    mCurveEditor->RegisterToolBar(toolBar);
    mCurveEditor->RegisterLinearTangentsButton(linearTangents);
    mCurveEditor->RegisterSplitTangentsButton(splitTangents);
    mCurveEditor->RegisterWeightedTangentsButton(weightedTangents);

    ConnectThisTo(mCurveEditor, Events::MetaDrop, OnMetaDrop);
    ConnectThisTo(mCurveEditor, Events::MetaDropTest, OnMetaDrop);

    // Create text boxes for changing the width and height of the graph
    mHeightTextBox = new TextBox(mGraphArea);
    mHeightTextBox->SetSize(Pixels(35, 20));
    mHeightTextBox->SetTranslation(Pixels(03, 10, 0));
    mHeightTextBox->SetNotInLayout(true);
    mHeightTextBox->SetEditable(true);
    mHeightTextBox->mBackgroundColor = ByteColorRGBA(49, 49, 49, 255);
    ConnectThisTo(mHeightTextBox, Events::TextBoxChanged, OnHeightChange);

    mWidthTextBox = new TextBox(mGraphArea);
    mWidthTextBox->SetSize(Pixels(35, 20));
    mWidthTextBox->SetTranslation(Pixels(230, 40, 0));
    mWidthTextBox->SetNotInLayout(true);
    mWidthTextBox->SetEditable(true);
    mWidthTextBox->mBackgroundColor = ByteColorRGBA(49, 49, 49, 255);
    ConnectThisTo(mWidthTextBox, Events::TextBoxChanged, OnWidthChange);
  }
}

//******************************************************************************
void MultiSampleCurveEditor::UpdateTransform()
{
  Thickness margins(Pixels(40), Pixels(11), Pixels(20), Pixels(30));
  Rect graphRect = Rect::PointAndSize(Vec2::cZero, mGraphArea->GetSize());
  graphRect.RemoveThickness(margins);

  PlaceWithRect(graphRect, mGraph);
  PlaceWithRect(graphRect, mCurveEditor);

  // Position the width text box in the lower right of the client size
  Vec2 pos = mGraphArea->GetSize() - Pixels(40, 26);
  mWidthTextBox->SetTranslation(ToVector3(pos));

  Composite::UpdateTransform();
}

//******************************************************************************
SampleCurveObject* MultiSampleCurveEditor::LoadCurve(SampleCurve* curve)
{
  SampleCurveObject* curveObject = new SampleCurveObject(mCurveEditor, curve);

  // Set up the graph
  mGraph->SetWidthRange(curve->GetWidthMin(), curve->GetWidthMax());
  mGraph->SetHeightRange(curve->GetHeightMin(), curve->GetHeightMax());

  mHeightTextBox->SetText(String::Format("%0.1f", curve->GetWidthMax()));
  mWidthTextBox->SetText(String::Format("%0.1f", curve->GetHeightMax()));

  return curveObject;
}

//******************************************************************************
void MultiSampleCurveEditor::OnMetaDrop(MetaDropEvent* e)
{
  if(e->Testing)
  {
    e->Result = "Add Curve";
    e->Handled = true;
  }
  else if(SampleCurve* curve = e->Instance.Get<SampleCurve*>())
  {
    SampleCurveObject* curveObject = LoadCurve(curve);
    curveObject->SetCurveColor(Color::Green);
  }
}

//******************************************************************************
void MultiSampleCurveEditor::OnHeightChange(Event*)
{
  // Get the value from the text box
  float val;
  ToValue(mHeightTextBox->GetText().All(), val);

  forRange(CurveObject* curveObject, mCurveEditor->GetCurveObjects())
  {
    SampleCurveObject* sampleCurveObject = (SampleCurveObject*)curveObject;

    if(SampleCurve* curve = sampleCurveObject->GetSampleCurve())
    {
      curve->SetWidthMax(val);
      MetaOperations::NotifyObjectModified(curve);
    }
  }

  mGraph->SetWidthMax(val);

  TabModifiedEvent eventToSend(true);
  DispatchBubble(Events::TabModified, &eventToSend);
}

//******************************************************************************
void MultiSampleCurveEditor::OnWidthChange(Event*)
{
  // Get the value from the text box
  float val;
  ToValue(mWidthTextBox->GetText().All(), val);

  forRange(CurveObject* curveObject, mCurveEditor->mCurves.All())
  {
    SampleCurveObject* sampleCurveObject= (SampleCurveObject*)curveObject;

    if(SampleCurve* curve = sampleCurveObject->GetSampleCurve())
    {
      curve->SetHeightMax(val);
      MetaOperations::NotifyObjectModified(curve);
    }
  }

  mGraph->SetHeightMax(val);

  TabModifiedEvent eventToSend(true);
  DispatchBubble(Events::TabModified, &eventToSend);
}

}//namespace Zero
