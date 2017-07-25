///////////////////////////////////////////////////////////////////////////////
///
/// \file GraphView.hpp
/// Declaration of the GraphView.
/// 
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

struct RangeData
{
  bool AutoNormalized;
  float MinValue;
  float MaxValue;
};

struct EntryLabel
{
  bool PointContains(Vec2 point){return false;}
  void DrawSelection(DisplayRender* render){}

  String Name;
  Vec3 Translation;
  Vec2 Size;
};

class DataSampler : public EventObject
{
public:
  typedef DataSampler ZilchSelf;

  virtual float Sample()=0;
  virtual void Setup(RangeData& data, EntryLabel& label){};
  virtual ~DataSampler(){};
};

class MetaObjectSample : public DataSampler
{
public:

  MetaObjectSample(Object* object, StringParam propertyName)
  {
    mObject = object;
    mProperty = mObject.StoredType->GetProperty(propertyName);
  }

  MetaObjectSample(Object* object, Property* property)
  {
    mObject = object;
    mProperty = property;
  }

  void Setup(RangeData& data, EntryLabel& label)
  {
    data.AutoNormalized = true;
    label.Name = mProperty->Name;
  }

  float Sample() override
  {
    if(!mObject.IsNull())
    {
      Any variant = mProperty->GetValue(mObject);
      if(variant.Is<float>())
        return variant.Get<float>();
      else
        return float(variant.Get<int>());
    }
    return 0.0f;
  }

  Handle mObject;
  Property* mProperty;
};

class GraphEntry
{
public:
  GraphEntry()
  {
    data.MaxValue = 1;
    data.MinValue = 0;
    data.AutoNormalized = false;
  }

  void Update();
  float Normalize(float value);

  RangeData data;
  ByteColor Color;
  Array<float> Values;
  DataSampler* Sampler;
  EntryLabel Label;
};

/// A data graph ui widget. Used to draw a series of data points for
/// plotting changes in values over time.
class GraphView : public Widget
{
public:
  typedef GraphView ZilchSelf;

  GraphView(Composite* parent);

  void AddEntry(GraphEntry* entry);
  void AddSampler(DataSampler* sampler);

  void OnSetKeyPressed(ObjectEvent* event);
  void OnClose(ObjectEvent* event);

  void RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect) override;

private:
  //--------------------------------------------------------Mouse Event Response
  void OnMouseDown(MouseEvent* event);
  void OnMouseMove(MouseEvent* event);
  void OnMouseScroll(MouseEvent* event);

private:
  Vec3 GetLabelPosition(uint labelNumber);

  /// Draw the label for each graph entry.
  void DrawLabels(RenderFont* font, ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect);

  /// Draw the line graph
  void DrawLineGraph(Vec2Param size, ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect);
  void DrawGrid(Vec2Param size, RenderFont* font, ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect);

  /// Draw the pie graph
  void DrawPieGraph();

private:
  Array<GraphEntry*> Entries;
  /// List of available colors to Assign new entries.
  Array<ByteColor> mColors;
  /// Whether or not we show labels.
  bool mLabels;
  int mMouseOverLabel;
  RenderFont* mFont;
};

}//namespace Zero
