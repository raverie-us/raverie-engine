///////////////////////////////////////////////////////////////////////////////
///
/// \file GridDraw.cpp
/// Declaration of the GridDraw class.
/// 
/// Authors: Trevor Sundberg
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

// Declare the meta
ZilchDefineType(GridDraw, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();

  ZilchBindFieldProperty(mActive);
  ZilchBindFieldProperty(mAxis);
  ZilchBindFieldProperty(mCellSize);
  ZilchBindFieldProperty(mLines);
  ZilchBindFieldProperty(mHighlightInterval);

  ZilchBindFieldProperty(mGridColor);
  ZilchBindFieldProperty(mGridHighlight);
  ZilchBindFieldProperty(mHalfCellOffset);
  ZilchBindFieldProperty(mDrawAxisOrigins);
  ZilchBindFieldProperty(mAlwaysDrawInEditor);
  ZilchBindFieldProperty(mDrawInGame);
  ZilchBindFieldProperty(mFollowEditorCamera);
}

// Serialize the members
void GridDraw::Serialize(Serializer& stream)
{
  SerializeNameDefault(mActive, true);
  SerializeNameDefault(mHalfCellOffset, false);
  SerializeNameDefault(mCellSize, 1.0f);
  SerializeEnumNameDefault(AxisDirection, mAxis, AxisDirection::Y);
  SerializeNameDefault(mLines, 100);
  SerializeNameDefault(mDrawAxisOrigins, true);
  SerializeNameDefault(mAlwaysDrawInEditor, true);
  SerializeNameDefault(mDrawInGame, false);
  SerializeNameDefault(mGridColor, Vec4(0.5f, 0.5f, 0.5f, 0.4f));
  SerializeNameDefault(mGridHighlight, Vec4(0, 0, 0, 0.4f));
  SerializeNameDefault(mHighlightInterval, 10);
  SerializeNameDefault(mFollowEditorCamera, false);
}

// Initialize the grid
void GridDraw::Initialize(CogInitializer& initializer)
{
  // Connect to the frame update
  ConnectThisTo(this->GetSpace(), Events::FrameUpdate, OnFrameUpdate);
}

// Occurs when the frame is updated on the space
void GridDraw::OnFrameUpdate(UpdateEvent* e)
{
  // If we want to draw this in editor mode, and we're currently in editor mode...
  if(this->GetSpace()->IsEditorMode())
  {
    if (this->mAlwaysDrawInEditor)
      this->Draw();
  }
  else if(mDrawInGame)
  {
    this->Draw();
  }
}

void GridDraw::SetAxis(AxisDirection::Enum axis)
{
  mAxis = axis;
}

void GridDraw::DebugDraw()
{
  if(!this->mAlwaysDrawInEditor)
  {
    this->Draw();
  }
}

void GetCustomGridColor(Vec3Param start, Vec3Param end, ByteColor* color)
{
  if(start.y == 0.0f && end.y == 0.0f && start.z == 0.0f && end.z == 0.0f)
    *color = ToByteColor(Vec4(1,0,0,0.7f));
  else if(start.x == 0.0f && end.x == 0.0f && start.z == 0.0f && end.z == 0.0f)
    *color = ToByteColor(Vec4(0,1,0,0.7f));
  else if(start.x == 0.0f && end.x == 0.0f && start.y == 0.0f && end.y == 0.0f)
    *color = ToByteColor(Vec4(0,0,1,0.7f));
}

// Draw the grid
void GridDraw::Draw()
{
  if(!mActive)
    return;

  size_t  lineToDraw = mLines;
  // Don't let them have too many lines, otherwise this will run forever
  lineToDraw = Math::Min(lineToDraw, size_t(1000));
 ++lineToDraw;

 if(mCellSize <= 0.0f)
   mCellSize = 1.0f;

 if(mHighlightInterval == 0)
   mHighlightInterval = 10;

  float intervalCount = (float)mLines;

  ByteColor gridColor = ToByteColor(mGridColor);
  ByteColor gridHighlight = ToByteColor(mGridHighlight);

  // Get the transform component
  Transform* tx = this->GetOwner()->has(Transform);
  Mat4 transformMatrix = Mat4::cIdentity;
  if(tx)
  {
    transformMatrix = tx->GetWorldMatrix();
  }

  Vec3 translation = Vec3(0,0,0);
  int highlightOffsets[] = {0,0,0};

  Cog* camera = this->GetSpace()->FindObjectByName(SpecialCogNames::EditorCamera);
  if(camera && mFollowEditorCamera)
  {
    tx = nullptr;
    EditorCameraController* controller = camera->has(EditorCameraController);
    if(controller)
      translation = controller->GetLookTarget();
    for(uint i=0;i<NumAxes;++i)
    {
      if(i != mAxis)
      {
        translation[i] = Snap(translation[i], mCellSize);
        highlightOffsets[i] = (int)Math::Round(translation[i] / mCellSize);
      }
      else
      {
        translation[i] = 0;
      }
    }
  }

  Vec3 splatCells = Vec3(mCellSize, mCellSize, mCellSize);
  if (mHalfCellOffset)
  {
    translation -= splatCells * 0.5f;
  }

  // The offset is used in multiple ways, but primarily it is the distance out that the lines are
  // drawn along two different axes that aren't the current axes
  Vec3 size = splatCells * intervalCount * 0.5f;

  // Loop through all the axes
  // The 'currentAxis' is the axis that we're stepping over to draw lines
  for (size_t currentAxis = 0; currentAxis < NumAxes; ++currentAxis)
  {
    // Loop through all the lines that we want to draw along that axis
    for (size_t currentLine = 0; currentLine < lineToDraw; ++currentLine)
    {
      Vec3 offset = size;
      // For the current axis, the offset is the distance along it where we want to draw the lines
      offset[currentAxis] = (currentLine - intervalCount * 0.5f) * mCellSize;

      // Loop through all the other axes
      for (size_t otherAxis = 0; otherAxis < NumAxes; ++otherAxis)
      {
        // Skip the current axis, since that would not be an "other axis"
        if (otherAxis == currentAxis)
          continue;

        // Compute which face we're doing (essentially the plane made by the current axis as a normal)
        size_t facePlaneIndex = NumAxes - (otherAxis ^ currentAxis);
        if(!(facePlaneIndex == mAxis))
          continue;

        ByteColor color = gridColor;
        if((currentLine + highlightOffsets[currentAxis]) % mHighlightInterval == 0)
         color = gridHighlight;

        // Put the start and end points of the line right at the center
        Vec3 start = Vec3::cZero;
        Vec3 end = Vec3::cZero;

        // Offset the start and end along the current axes
        start[currentAxis] += offset[currentAxis];
        end  [currentAxis] += offset[currentAxis];

        // Offset the start and end so that, on the other axes, they are from positive to negative
        start[otherAxis] += offset[otherAxis];
        end  [otherAxis] -= offset[otherAxis];

        if(mDrawAxisOrigins && !mFollowEditorCamera)
          GetCustomGridColor(start, end, &color);

        // Transform the start and end points
        if(tx)
        {
          start = Math::TransformPoint(transformMatrix, start);
          end   = Math::TransformPoint(transformMatrix, end);
        }

        start += translation;
        end += translation;

        if(mDrawAxisOrigins && mFollowEditorCamera)
          GetCustomGridColor(start, end, &color);

        // Draw the line
        gDebugDraw->Add(Debug::Line(start, end).Color(color));
      }
    }
  }
}

}
