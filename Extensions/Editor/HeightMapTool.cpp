///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg, Ryan Edgemon
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
/// The default name we give any created height maps
const String cHeightMapName("HeightMap");
const String cHeightMapArchetype("DefaultHeightMap");

/// Constants
const float MinRadius = 0.001f;

//---------------------------------------------------------- Height Map Sub Tool
ZilchDefineType(HeightMapSubTool, builder, type)
{
  // These options are referred to directly by pointer on the import options (unsafe for script)
  type->HandleManager = ZilchManagerId(PointerManager);
  type->Add(new TypeNameDisplay());

  ZeroBindExpanded();
}

bool HeightMapSubTool::LeftMouseDown(HeightMap* map, ViewportMouseEvent* e)
{
  return false;
}

bool HeightMapSubTool::LeftMouseUp(HeightMap* map, ViewportMouseEvent* e)
{
  return false;
}

bool HeightMapSubTool::MouseScroll(HeightMap* map, ViewportMouseEvent* e)
{
  return false;
}

void HeightMapSubTool::Draw(HeightMap* map)
{
  const float CrossScale = 0.2f;
  gDebugDraw->Add(Debug::LineCross(map->GetWorldPosition(mLocalToolPosition), map->GetUnitsPerPatch() * CrossScale));
}

//----------------------------------------------------- Height Manipulation Tool
/// Meta
ZilchDefineType(HeightManipulationTool, builder, type)
{
  ZilchBindFieldProperty(mRadius);
  ZilchBindFieldProperty(mFeatherRadius);

  ZeroBindExpanded();
}

HeightManipulationTool::HeightManipulationTool()
{
  mRadius = 1.0f;
  mFeatherRadius = 1.0f;
  mOperation = nullptr;
}

void HeightManipulationTool::PerformQuery(HeightMap* map, ViewportMouseEvent* e)
{
  HeightMapCellRange range(map, mLocalToolPosition, mRadius, mFeatherRadius);

  HeightMapStateManager* state;

  if(mOperation == nullptr)
  {
    mOperation = new HeightMapUndoRedo(map);
    mOperation->SetAABB(range);

    HashMap<HeightMap*, HeightMapStateManager>::InsertResult result = 
      mAlteredMaps.InsertNoOverwrite(map, HeightMapStateManager(map));

    state = &result.mValue->second;
    state->StartBrushStroke(mRadius, mFeatherRadius);
  }
  else
  {
    mOperation->UpdateAABB(range);
    state = &mAlteredMaps[map];
  }

  state->AddPointToStroke(mLocalToolPosition);

  ApplyToCells(range, e);
  range.SignalPatchesModified();
}

bool HeightManipulationTool::LeftMouseUp(HeightMap* map, ViewportMouseEvent* e)
{
  OperationQueue* queue = Z::gEditor->GetOperationQueue( );
  queue->Queue(mOperation);

    // Operation committed, prep for next OnMouseDown.
  mOperation = nullptr;

  mAlteredMaps[map].EndBrushStroke();

  return true;
}

bool HeightManipulationTool::LeftMouseDown(HeightMap* map, ViewportMouseEvent* e)
{
  PerformQuery(map, e);
  return true;
}

void HeightManipulationTool::LeftMouseMove(HeightMap* map, ViewportMouseEvent* e)
{
  PerformQuery(map, e);
}

bool HeightManipulationTool::MouseScroll(HeightMap* map, ViewportMouseEvent* e)
{
  if(e->ShiftPressed)
  {
    mRadius += e->Scroll.y * 0.1f;
    mFeatherRadius += e->Scroll.y * 0.1f;

    if (mRadius < MinRadius)
      mRadius = MinRadius;
    if (mFeatherRadius < MinRadius)
      mFeatherRadius = MinRadius;
  }

  // Tell any derived classes that the radius has changed
  OnRadiusChanged();

  return e->ShiftPressed;
}

float HeightManipulationTool::GetRadius()
{
  return mRadius;
}

void HeightManipulationTool::SetRadius(float value)
{
  if (value < MinRadius)
    value = MinRadius;

  mRadius = value;

  // Tell any derived classes that the radius has changed
  OnRadiusChanged();
}

float HeightManipulationTool::GetFeatherRadius()
{
  return mFeatherRadius;
}

void HeightManipulationTool::SetFeatherRadius(float value)
{
  if (value < MinRadius)
    value = MinRadius;

  mFeatherRadius = value;

  // Tell any derived classes that the radius has changed
  OnRadiusChanged();
}

void HeightManipulationTool::Draw(HeightMap* map)
{
  HeightMapSubTool::Draw(map);

  // Grab the transform and get the up axis
  auto tx = map->GetOwner()->has(Transform);
  Vec3 upAxis = tx->TransformNormal(Vec3::cYAxis);

  // Draw the radius and feather for the brush
  gDebugDraw->Add(Debug::Circle(map->GetWorldPosition(mLocalToolPosition), upAxis, mRadius).OnTop(true).Color(Color::Red));
  gDebugDraw->Add(Debug::Circle(map->GetWorldPosition(mLocalToolPosition), upAxis, mRadius + mFeatherRadius).OnTop(true).Color(Color::Green));
}

//------------------------------------------------------------- Raise/Lower Tool
ZilchDefineType(RaiseLowerTool, builder, type)
{
  ZilchBindFieldProperty(mStrength)->Add(new EditorRange(0,1,0.001f));
  ZilchBindFieldProperty(mRelative);

  ZeroBindExpanded();
}

RaiseLowerTool::RaiseLowerTool()
{
  mStrength = 0.1f;
  mRelative = true;
}

void RaiseLowerTool::ApplyToCells(HeightMapCellRange& range, ViewportMouseEvent* e)
{
  // By default, we are raising the height map
  float direction = 1.0f;
  mOperation->mName = "HeightMapRaise";

  if(e->ShiftPressed)
  {
    // We are lowering the height map
    direction = -1.0f;
    mOperation->mName = "HeightMapLower";
  }

  forRange(HeightMapCell cell, range)
  {
    // Get a modifiable reference to the current height of the cell
    float& height = cell.Patch->GetHeight(cell.Index);
    float preDeltaHeight = height;

    // Raise or lower the height map and apply influence
    height += direction * mStrength * cell.Influence;

    mOperation->AddCell(cell, preDeltaHeight, height);
  }

}

//---------------------------------------------------------- Smooth/Sharpen Tool
ZilchDefineType(SmoothSharpenTool, builder, type)
{
  ZilchBindFieldProperty(mStrength)->Add(new EditorRange(0,1,0.001f));
  ZilchBindField(mUniformSamples);
  ZilchBindField(mRandomSamples);
  ZilchBindField(mRandomSampleDistance);
  ZilchBindField(mAutoDetermineSamples);

  ZeroBindExpanded();
}

SmoothSharpenTool::SmoothSharpenTool()
  : mRandom(1234)
{
  mStrength = 0.1f;
  mUniformSamples = 0;
  mRandomSamples = 0;
  mRandomSampleDistance = 0;
  mAutoDetermineSamples = true;
}

inline void SampleAverage(HeightMap* map, int dx, int dy, float influence,
                          HeightMapCell& cell, const AbsoluteIndex& index,
                          float& totalHeight, float& totalWeight)
{
  // Determine an index inside the current patch
  CellIndex inPatchIndex = cell.Index + CellIndex(dx, dy);

  // We will be filling this out with a sample
  float sample;

  // If the cell index is still within our patch
  if (inPatchIndex.x >= 0 &&
      inPatchIndex.y >= 0 &&
      inPatchIndex.x < HeightPatch::Size &&
      inPatchIndex.y < HeightPatch::Size)
  {
    // Get the current sample (in our own patch)
    sample = cell.Patch->GetHeight(inPatchIndex);
  }
  else
  {
    // Get the current sample (which will be indexing another patch)
    sample = map->SampleHeight(index + AbsoluteIndex(dx, dy), Math::cInfinite);
  }

  // As long as the sample was valid
  if (sample != Math::cInfinite)
  {
    // Total up all samples for an average
    totalHeight += sample * influence;
    totalWeight += influence;
  }
}

void SmoothSharpenTool::Refresh(HeightMap* map)
{
  if (mAutoDetermineSamples)
  {
    DetermineSamples(map);
  }
}

void SmoothSharpenTool::DetermineSamples(HeightMap* map)
{
  // Using the radius of the tool and the patch size, determine a base number of samples
  const int samples = int((mRadius / map->GetUnitsPerPatch()) * 0.1f * HeightPatch::Size);

  // We only start the random samples when our sample size reaches 3
  mRandomSamples = Math::Max(samples - 3, 0);

  // Our uniform samples go up with standard samples, but then go down when random samples are introduced
  // We also always want at least 1 uniform sample, and at max 4
  mUniformSamples = Math::Max(Math::Min(samples, 4) - mRandomSamples, 1);

  // If we have more than 4 random samples, we have to increase uniform sampling to get rid of noise
  if (mRandomSamples > 0)
    mUniformSamples = Math::Max(mUniformSamples, 2);

  // We scale up our random sample size to index further outward
  mRandomSampleDistance = int(samples * 1.5f);
}

void SmoothSharpenTool::OnRadiusChanged()
{
  // Do we automatically determine samples?
  if (mAutoDetermineSamples)
  {
    // Grab the map from our parent tool
    HeightMap* map = mOwner->GetHeightMap();

    // If the map is valid...
    if (map != NULL)
    {
      // Determine the samples!
      DetermineSamples(map);
    }
  }
}


void SmoothSharpenTool::Smooth(HeightMapCellRange& range)
{
  mOperation->mName = "HeightMapSmooth";

  // Compute the max distance that random samples can go
  int randomDistance = this->mRandomSampleDistance;
  const float maxRandomDistance = Math::Sqrt(float(randomDistance * randomDistance +
                                                   randomDistance * randomDistance));

  // Pull members into locals
  int uniformSamples = mUniformSamples;
  int randomSamples = mRandomSamples;

  forRange (HeightMapCell cell, range)
  {
    // Get a modifiable reference to the current height of the cell
    float& height = cell.Patch->GetHeight(cell.Index);

    // Get the absolute index of the cell
    AbsoluteIndex index = cell.Patch->Index * HeightPatch::Size +
                          CellIndex(cell.Index.x - HeightPatch::Size / 2,
                                    cell.Index.y - HeightPatch::Size / 2);

    // Store totals for averaging
    float totalHeight = 0.0f;
    float totalWeight = 0.0f;

    // Here we do a uniform distribution of samples
    for (int dy = -uniformSamples; dy <= uniformSamples; ++dy)
    {
      for (int dx = -uniformSamples; dx <= uniformSamples; ++dx)
      {
        // Sample and average the amounts
        SampleAverage(range.mHeightMap, dx, dy, 1.0f, cell, index, totalHeight, totalWeight);
      }
    }

    // Here we do a random samples
    if(randomDistance != 0)
    {
      for(int s = 0; s < randomSamples; ++s)
      {
        // Determine a random sample location
        int dx = (mRandom.Uint32() % (randomDistance * 2)) - randomDistance;
        int dy = (mRandom.Uint32() % (randomDistance * 2)) - randomDistance;

        // Determine the distance from the center
        const float distance = Math::Sqrt(float(dx * dx + dy * dy));

        // Compute the influence
        float influence = (maxRandomDistance - distance) / maxRandomDistance;

        // Sample and average the amounts
        SampleAverage(range.mHeightMap, dx, dy, influence, cell, index, totalHeight, totalWeight);

        // We let the middle influence the random samples more
        const float MiddleInfluence = 0.25f;
        totalHeight += height * MiddleInfluence;
        totalWeight += MiddleInfluence;
      }
    }

    // If we have no samples, then skip out
    if(totalWeight != 0.0f)
    {
      // Compute the average height
      float averageHeight = totalHeight / totalWeight;

      // Smooth out by getting the averages
      float smoothedHeight = Math::Lerp(height, averageHeight, mStrength);

      // Apply influence
      float preDeltaHeight = height;
      height = Math::Lerp(height, smoothedHeight, cell.Influence);

      mOperation->AddCell(cell, preDeltaHeight, height);
    }

  }

}

void SmoothSharpenTool::Sharpen(HeightMapCellRange& range)
{
  // Store totals for averaging
  float totalHeight = 0.0f;
  float totalWeight = 0.0f;

  mOperation->mName = "HeightMapSharpen";

  forRange (HeightMapCell cell, range)
  {
    // Get a modifiable reference to the current height of the cell
    float& height = cell.Patch->GetHeight(cell.Index);

    totalHeight += height * cell.Influence;
    totalWeight += cell.Influence;
  }

  // If we have no samples, then skip out
  if (totalWeight != 0.0f)
  {
    // Compute the average height
    float averageHeight = totalHeight / totalWeight;

    forRange (HeightMapCell cell, range)
    {
      // Get a modifiable reference to the current height of the cell
      float& height = cell.Patch->GetHeight(cell.Index);
      float preDeltaHeight = height;

      // Get the difference between the average and the current
      float diff = (height - averageHeight) * mStrength;

      // Apply influence and add in the difference to sharpen
      const float SharpenScale = 0.2f;
      height += diff * cell.Influence * SharpenScale;

      mOperation->AddCell(cell, preDeltaHeight, height);
    }

  }

}

void SmoothSharpenTool::ApplyToCells(HeightMapCellRange& range, ViewportMouseEvent* e)
{
  if(e->ShiftPressed)
    Sharpen(range);
  else
    Smooth(range);
}

//----------------------------------------------------------------- Flatten Tool
ZilchDefineType(FlattenTool, builder, type)
{
  ZilchBindFieldProperty(mHeight);
  ZilchBindFieldProperty(mSlopeNormal);
  ZilchBindFieldProperty(mSampleOnMouseDown);
  ZilchBindFieldProperty(mSampleNormal);

  ZeroBindExpanded();
}

FlattenTool::FlattenTool()
{
  mStrength = 0.1f;
  mHeight = 0.0f;
  mSlopeNormal = Vec3(0, 1, 0);
  mSampleOnMouseDown = true;
  mSampleNormal = false;
}

void FlattenTool::ApplyToCells(HeightMapCellRange& range, ViewportMouseEvent* e)
{
  mOperation->mName = "HeightMapFlatten";

  forRange (HeightMapCell cell, range)
  {
    // Get a modifiable reference to the current height of the cell
    float& height = cell.Patch->GetHeight(cell.Index);

    // Smooth out the flattening using the strength
    float smoothedHeight = Math::Lerp(height, mHeight, mStrength);

    // Apply influence and flatten to the specified height
    float preDeltaHeight = height;
    height = Math::Lerp(height, smoothedHeight, cell.Influence);

    mOperation->AddCell(cell, preDeltaHeight, height);
  }

}

bool FlattenTool::LeftMouseDown(HeightMap* map, ViewportMouseEvent* e)
{
  // If we want to sample when the mouse goes down
  if (mSampleOnMouseDown)
  {
    // Get the height
    float height = map->SampleHeight(mLocalToolPosition, Math::cInfinite);

    // If the height is a valid height...
    if (height != Math::cInfinite)
    {
      // Set the sampled height
      mHeight = height;
      return true;
    }

    // Otherwise, we didn't sample anything
    return false;
  }

  // Now let the manipulation tool handle it
  return HeightManipulationTool::LeftMouseDown(map, e);
}


//---------------------------------------------------------- Create/Destroy Tool
ZilchDefineType(CreateDestroyTool, builder, type)
{
  ZilchBindFieldProperty(mBaseHeight);
  ZilchBindFieldProperty(mUsePerlinNoise);
  ZilchBindFieldProperty(mPerlinFrequency);
  ZilchBindFieldProperty(mPerlinAmplitude);

  ZeroBindExpanded();
}

CreateDestroyTool::CreateDestroyTool()
{
  mBaseHeight = 0.0f;
  mUsePerlinNoise = true;
  mPerlinFrequency = 1.0f;
  mPerlinAmplitude = 10.0f;

  mOperation = nullptr;
}

bool CreateDestroyTool::LeftMouseDown(HeightMap* map, ViewportMouseEvent* e)
{
  if(mOperation == nullptr)
    mOperation = new HeightPatchUndoRedo(map);

  mOperation->SetNoise(mUsePerlinNoise, mBaseHeight, mPerlinFrequency, mPerlinAmplitude);

  //Create with click destroy with shift click
  if(!e->ShiftPressed)
  {
    auto patchIndex = map->GetPatchIndexFromLocal(mLocalToolPosition);
    if (map->GetPatchAtIndex(patchIndex) != NULL)
      return true;

    auto patch = map->CreatePatchAtIndex(patchIndex);

    mOperation->mName = "CreateHeightPatch";
    mOperation->AddPatch(true, patchIndex);

    if (mUsePerlinNoise)
    {
      map->ApplyNoiseToPatch(patch, mBaseHeight, mPerlinFrequency, mPerlinAmplitude);
    }
    else
    {
      for (size_t i = 0; i < HeightPatch::TotalSize; ++i)
      {
        patch->Heights[i] = mBaseHeight;
      }
    }

    map->SignalPatchModified(patch);
  }
  else
  {
    auto patchIndex = map->GetPatchIndexFromLocal(mLocalToolPosition);
    map->DestroyPatchAtIndex(patchIndex);

    mOperation->mName = "DestroyHeightPatch";
    mOperation->AddPatch(false, patchIndex);
  }
  
  return true;
}

void CreateDestroyTool::LeftMouseMove(HeightMap* map, ViewportMouseEvent* e)
{
  LeftMouseDown(map, e);
}

bool CreateDestroyTool::LeftMouseUp(HeightMap* map, ViewportMouseEvent* e)
{
  OperationQueue* queue = Z::gEditor->GetOperationQueue( );
  queue->Queue(mOperation);
    // Operation committed, prep for next OnMouseDown.
  mOperation = nullptr;

  return true;
}

void CreateDestroyTool::Draw(HeightMap* map)
{
}

//------------------------------------------------------------  WeightPainterTool
ZilchDefineType(WeightPainterTool, builder, type)
{
  ZilchBindFieldProperty(mTextureChannel);

  ZilchBindFieldProperty(mStrength)->Add(new EditorRange(0,1,0.001f));
  ZilchBindFieldProperty(mRadius);
  ZilchBindFieldProperty(mFeatherRadius);

  ZeroBindExpanded();
}

WeightPainterTool::WeightPainterTool()
{
  mTextureChannel = HeightTextureSelect::Texture0;
  mStrength = 0.1f;
  mRadius = 1.0f;
  mFeatherRadius = 1.0f;

  mOperation = nullptr;
}

bool WeightPainterTool::LeftMouseDown(HeightMap* map, ViewportMouseEvent* e)
{
  if(mOperation == nullptr)
    mOperation = new WeightMapUndoRedo(map);

  Paint(map);
  return true;
}

void WeightPainterTool::LeftMouseMove(HeightMap* map, ViewportMouseEvent* e)
{
  Paint(map);
}



bool WeightPainterTool::LeftMouseUp(HeightMap* map, ViewportMouseEvent* e)
{
  OperationQueue* queue = Z::gEditor->GetOperationQueue( );
  queue->Queue(mOperation);
    // Operation committed, prep for next OnMouseDown.
  mOperation = nullptr;

  return true;
}

void WeightPainterTool::Draw(HeightMap* map)
{
  HeightMapSubTool::Draw(map);

  // Grab the transform and get the up axis
  auto tx = map->GetOwner()->has(Transform);
  Vec3 upAxis = tx->TransformNormal(Vec3::cYAxis);

  // Draw the radius and feather for the brush
  gDebugDraw->Add(Debug::Circle(map->GetWorldPosition(mLocalToolPosition), upAxis, mRadius).OnTop(true).Color(Color::Red));
  gDebugDraw->Add(Debug::Circle(map->GetWorldPosition(mLocalToolPosition), upAxis, mRadius + mFeatherRadius).OnTop(true).Color(Color::Green));
}


void ClampWeights(Vec4& a)
{
  float total = a.x + a.y + a.z + a.w;
  a.x /= total;
  a.y /= total;
  a.z /= total;
  a.w /= total;
}

ByteColor ChangeWeights(ByteColor current, uint channel, float change)
{
  Vec4 weights = ToFloatColor(current);

  float targetChannel = weights[channel];
  float otherTotal = (1.0f - targetChannel);

  targetChannel += change;
  targetChannel = Math::Clamp(targetChannel, 0.0f, 1.0f);

  float targetChange = targetChannel - weights[channel];

  //Change all channel by the amount added to the target channel
  //using their original weights to balance it
  float otherChange =  (otherTotal != 0.0f) ? (1.0f / otherTotal) * -targetChange : 0;

  //Rebalance weights
  for(uint i=0;i<4;++i)
  {
    if(i==channel)
      weights[i] = targetChannel;
    else
      weights[i] += weights[i] * otherChange;
  }

  ClampWeights(weights);

  return ToByteColor(weights);
}


bool WeightPainterTool::MouseScroll(HeightMap* map, ViewportMouseEvent* e)
{
  if(e->ShiftPressed)
  {
    mRadius += e->Scroll.y * 0.1f;
    mFeatherRadius += e->Scroll.y * 0.1f;

    if (mRadius < MinRadius)
      mRadius = MinRadius;
    if (mFeatherRadius < MinRadius)
      mFeatherRadius = MinRadius;
  }

  return e->ShiftPressed;
}

void WeightPainterTool::Paint(HeightMap* map)
{
  // Get the index of the local position
  PatchIndex centerIndex = map->GetPatchIndexFromLocal(mLocalToolPosition);

  // Grab the patch at the found index
  //HeightPatch* patch = map->GetPatchAtIndex(centerIndex);

  HeightMapModel* heightMapModel = map->mOwner->has(HeightMapModel);
  if (heightMapModel == nullptr)
    return;

  Vec2 brushPosition = Vec2(mLocalToolPosition.x, mLocalToolPosition.y);

  float totalRadius = mRadius+mFeatherRadius;

  // How large is the total radius in indices (upper bound)
  int indexRadius = (int)Math::Ceil(totalRadius / map->GetUnitsPerPatch());

  // Mark the map as modified so it will save
  map->Modified();

  // Loop through the possible patches
  for (int y = -indexRadius; y <= indexRadius; ++y)
  {
    for (int x = -indexRadius; x <= indexRadius; ++x)
    {
      // Compute the possible patch index
      PatchIndex index = centerIndex;
      index.x += x;
      index.y += y;

      // Grab the patch at the found index
      HeightPatch* patch = map->GetPatchAtIndex(index);

      if(patch)
      {
        GraphicalHeightPatch* graphicalPatch =  heightMapModel->mGraphicalPatches.FindPointer(patch, nullptr);
        if (graphicalPatch == nullptr)
          continue;

        Vec2 patchPosition = map->GetLocalPosition(index);
        Vec2 pixelOffset = Vec2(-1,-1) * map->GetUnitsPerPatch() * 0.5;

        patchPosition = patchPosition + pixelOffset;

        uint textureSize = 128;
        float pixelToUint = (1.0f / float(textureSize)) *  map->GetUnitsPerPatch();

        for(uint x = 0; x < textureSize; ++x)
        {
          for(uint y = 0; y < textureSize; ++y)
          {
            ByteColor currentW = graphicalPatch->mWeightTexture->GetPixel(x, y);
            ByteColor preDeltaWeight = currentW;

            Vec2 pixelPosition = patchPosition + Vec2(float(x), float(y)) * pixelToUint;

            float distanceToBrush = Math::Distance(pixelPosition, brushPosition);

            float influence = FeatherInfluence(distanceToBrush, mRadius, mFeatherRadius);

            if(distanceToBrush < totalRadius)
            {
              currentW = ChangeWeights(currentW, mTextureChannel, influence * mStrength);

              graphicalPatch->mWeightTexture->SetPixel(x, y, currentW);
              mOperation->AddPixel(patch->Index, x, y, preDeltaWeight, currentW);
            }

          }

        }

        graphicalPatch->mWeightTexture->Upload();
      }
    }

  }

}


//---------------------------------------------------------------Height Map Tool
ZilchDefineType(HeightMapTool, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultConstructor);
  ZeroBindTag(Tags::Tool);

  if(DeveloperConfig* devConfig = Z::gEngine->GetConfigCog( )->has(Zero::DeveloperConfig))
  {
    ZilchBindFieldProperty(mShowPatchIndex);
    ZilchBindFieldProperty(mShowCellIndex);
    ZilchBindFieldProperty(mCellIndexType);
  }

  ZilchBindGetterSetterProperty(CurrentTool)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  ZilchBindFieldProperty(mSubTool);
}

HeightMapTool::HeightMapTool()
{
  mShowPatchIndex = false;
  mShowCellIndex = false;
  mCellIndexType = CellIndexType::Local;

  mAddHeightMapWidget = 0;

  // Add all the sub-tools
  mSubTools.PushBack(new CreateDestroyTool());
  mSubTools.PushBack(new RaiseLowerTool());
  mSubTools.PushBack(new SmoothSharpenTool());
  mSubTools.PushBack(new FlattenTool());
  mSubTools.PushBack(new WeightPainterTool());

  // Set us as the owner of each sub tool
  for (size_t i = 0; i < mSubTools.Size(); ++i)
  {
    mSubTools[i]->mOwner = this;
  }

  // By default, we use the create/destroy tool
  SetCurrentTool(HeightTool::CreateDestroy);
}

HeightMapTool::~HeightMapTool()
{
  // Cleanup all the sub-tools
  for (size_t i = 0; i < mSubTools.Size(); ++i)
  {
    delete mSubTools[i];
  }
}

void HeightMapTool::SetEditingMap(CogId cog, HeightMap* map)
{
  if (mSelection != cog)
  {
    mSubTool->Refresh(map);
    mSelection = cog;
  }
}

bool HeightMapTool::SetupLocalPosition(HeightMap* map, Viewport* viewport, ViewportMouseEvent* e)
{
  using namespace Intersection;

  // Get the ray from the mouse and viewport
  Ray worldRay = viewport->ScreenToWorldRay(e->Position);

  // Transform the ray into height map local space
  auto tx = map->GetOwner()->has(Transform);
  Ray localRay;
  localRay.Start = tx->TransformPointInverse(worldRay.Start);
  localRay.Direction = tx->TransformNormalInverse(worldRay.Direction);

  HeightMapRayRange range = map->CastLocalRay(localRay);

  IntersectionPoint point;

  if(range.Empty())
  {
    if (RayPlane(localRay.Start, localRay.Direction, Vec3::cYAxis, 0.0f, &point) == Intersection::None)
    {
      return false;
    }
  }
  else
  {
    point = range.Front().mIntersectionInfo;
  }

  // Ray cast against a plane
  mSubTool->mLocalToolPosition = Vec2(point.Points[0].x, point.Points[0].z);
  return true;
}

void HeightMapTool::Initialize(CogInitializer& initializer)
{
  ConnectThisTo(GetOwner(), Events::ToolActivate,   OnToolActivate);
  ConnectThisTo(GetOwner(), Events::ToolDeactivate, OnToolDeactivate);
  ConnectThisTo(GetOwner(), Events::LeftMouseDown, OnLeftMouseDown);
  ConnectThisTo(GetOwner(), Events::LeftMouseDrag, OnLeftMouseDrag);
  ConnectThisTo(GetOwner(), Events::LeftMouseUp, OnLeftMouseUp);
  ConnectThisTo(GetOwner(), Events::MouseMove, OnMouseMove);
  ConnectThisTo(GetOwner(), Events::MouseScroll, OnMouseScroll);
  ConnectThisTo(GetOwner(), Events::ToolDraw, OnToolDraw);
}

void HeightMapTool::OnToolActivate(Event*)
{
  ConnectThisTo(Z::gEditor->GetSelection(), Events::SelectionFinal, OnSelectionFinal);

  HeightMap* heightMap = GetHeightMap();
  if (heightMap)
  {
    Cog* cog = heightMap->GetOwner();
    Z::gEditor->SelectOnly(cog);
    mSubTool->Refresh(heightMap);
  }
}

void HeightMapTool::OnToolDeactivate(Event*)
{
  mAddHeightMapWidget.SafeDestroy();
  Z::gEditor->GetSelection()->GetDispatcher()->Disconnect(this);
}

void HeightMapTool::OnLeftMouseDown(ViewportMouseEvent* e)
{
  // If we have no map that we are editing, ignore this event
  HeightMap* map = GetHeightMap();
  if (map == NULL)
    return;

  Viewport* viewport = e->GetViewport();

  // Set the local position on the sub tool and forward the event
  e->Handled = SetupLocalPosition(map, viewport, e) && mSubTool->LeftMouseDown(map, e);

  // Capture/Claim the mouse
  if(e->Handled)
    mMouseCapture = new HeightMapMouseCapture(e->GetMouse( ), viewport, this);
}

void HeightMapTool::OnLeftMouseDrag(ViewportMouseEvent* e)
{
  if(mMouseCapture.IsNull( ))
    return;

  e->Handled = true;
}

void HeightMapTool::OnLeftMouseUp(ViewportMouseEvent* e)
{
  if(mMouseCapture.IsNull( ))
    return;

  // If we have no map that we are editing, ignore this event
  HeightMap* map = GetHeightMap();
  if (map == NULL)
    return;

  Viewport* viewport = e->GetViewport();

  // Set the local position on the sub tool and forward the event
  e->Handled = SetupLocalPosition(map, viewport, e) && mSubTool->LeftMouseUp(map, e);
}

void HeightMapTool::OnMouseMove(ViewportMouseEvent* e)
{
  // If we have no map that we are editing, ignore this event
  HeightMap* map = GetHeightMap();
  if (map == NULL)
    return;

  Viewport* viewport = e->GetViewport();

  e->Handled = SetupLocalPosition(map, viewport, e);

  // Get the local position so we don't have to potentially do it twice
  if (SetupLocalPosition(map, viewport, e) == false)
    return;

  // If the left button is down...
  if(mMouseCapture.IsNotNull( ) && e->IsButtonDown(MouseButtons::Left))
    mSubTool->LeftMouseMove(map, e);
}

void HeightMapTool::OnMouseScroll(ViewportMouseEvent* e)
{
  if(mMouseCapture.IsNull( ))
    return;

  // If we have no map that we are editing, ignore this event
  HeightMap* map = GetHeightMap();
  if (map == NULL)
    return;

  Viewport* viewport = e->GetViewport();

  // Set the local position on the sub tool and forward the event
  e->Handled = SetupLocalPosition(map, viewport, e) && mSubTool->MouseScroll(map, e);
}

void HeightMapTool::OnToolDraw(Event*)
{
  // If we have no map that we are editing, ignore this event
  HeightMap* map = GetHeightMap();
  if (map == NULL)
    return;

  mAddHeightMapWidget.SafeDestroy();

  // Draw the current sub tool
  mSubTool->Draw(map);

  DebugDraw();
}

void HeightMapTool::OnSelectionFinal(SelectionChangedEvent* event)
{
  MetaSelection* selection = (MetaSelection*)event->Selection;
  Cog* cog = selection->GetPrimaryAs<Cog>();

  if (cog && cog->has(HeightMap))
  {
    // This behavior currently relies on MainPropertyView connecting to this event first
    Z::gEditor->ShowWindow("Tools");
  }
  else
  {
    // Return to select tool if no object has a HeightMap
    Z::gEditor->Tools->SelectToolIndex(0);
  }
}

void HeightMapTool::OnLeftClickAddHeightMapWidget(MouseEvent* e)
{
  e->Handled = true;
  CreateHeightMap();
}

void HeightMapTool::OnLeftMouseUpAddHeightMapWidget(MouseEvent* e)
{
  e->Handled = true;
}

void HeightMapTool::DebugDraw( )
{
  if(mShowCellIndex || mShowPatchIndex)
    DrawDebugIndexes( );
}

void HeightMapTool::DrawDebugIndexes( )
{
  static Array<Vec3> vertices;

  HeightMap* map = GetHeightMap();
  unsigned sID = map->GetSpace( )->GetId( ).Id;

  Transform* transform = map->GetOwner()->Has<Transform>();

  IntVec2 (HeightMap::*cellIndexFn)(Vec2Param);
  
  if(mCellIndexType == CellIndexType::Local)
    cellIndexFn = &HeightMap::GetCellIndexFromLocal;
  else if(mCellIndexType == CellIndexType::Absoulte)
    cellIndexFn = &HeightMap::GetNearestAbsoluteIndexFromLocal;

  forRange(HeightPatch* patch, map->GetAllPatches())
  {
    if(mShowPatchIndex)
    {
      char buffer[16] = { 0 };
      sprintf(buffer, "(%d,%d)", patch->Index.x, patch->Index.y);

      Vec3 p0(map->GetWorldPosition(patch->Index));
      gDebugDraw->Add(sID, Debug::Text(p0, 0.3f, buffer).Color(Color::MediumSpringGreen).OnTop(true).ViewAligned(true).ViewScaled(true));
    }

    if(!mShowCellIndex)
      continue;

    map->GetHeightPatchVertices(patch, vertices);
    forRange(Vec3& vertex, vertices.All( ))
    {
      Vec3 p0(transform->TransformPoint(vertex));
      IntVec2 index = (map->*cellIndexFn)(Vec2(vertex.x, vertex.z));

      char buffer[16] = { 0 };
      sprintf(buffer, "(%d,%d)", index.x, index.y);

      gDebugDraw->Add(sID, Debug::Text(p0, 0.3f, buffer).Color(Color::Black).OnTop(true).ViewAligned(true).ViewScaled(true));
    }

  }

}

void HeightMapTool::CreateHeightMap()
{
  Space* space = Z::gEditor->GetEditSpace();

  if (space)
  {
    Cog* created = space->CreateNamed(cHeightMapArchetype, cHeightMapName);
    if (created)
    {
      created->ClearArchetype();

      OperationQueue* queue = Z::gEditor->GetOperationQueue();
      ObjectCreated(queue, created);

      mAddHeightMapWidget.SafeDestroy();

      Z::gEditor->SelectOnly(created);
    }
  }
}

HeightMap* HeightMapTool::GetHeightMap()
{
  // Disabled creation
  HeightMap* heightMap = static_cast<HeightMap*>(Tool::GetOrCreateEditComponent(ZilchTypeId(HeightMap), cHeightMapName, cHeightMapArchetype, mLastEdited, false));
  if (heightMap == NULL && mAddHeightMapWidget.IsNull())
  {
    mAddHeightMapWidget = Tool::CreateViewportTextWidget("No HeightMap Object, Add New +");
    ConnectThisTo((ViewportTextWidget*)mAddHeightMapWidget, Events::LeftClick, OnLeftClickAddHeightMapWidget);
    ConnectThisTo((ViewportTextWidget*)mAddHeightMapWidget, Events::LeftMouseUp, OnLeftMouseUpAddHeightMapWidget);
  }
  return heightMap;
}

HeightTool::Enum HeightMapTool::GetCurrentTool()
{
  // Return the current tool enum
  return mCurrentTool;
}

void HeightMapTool::SetCurrentTool(HeightTool::Enum tool)
{
  // Store the current tool, and set the subtool from the array
  mCurrentTool = tool;
  mSubTool = mSubTools[(uint)tool];
}

//-------------------------------------------------------- HeightMapMouseCapture

/******************************************************************************/
HeightMapMouseCapture::HeightMapMouseCapture(Mouse* mouse, Viewport* viewport, HeightMapTool* tool)
  : MouseManipulation(mouse, viewport)
{
  mViewport = (ReactiveViewport*)viewport;
  mHeightMapTool = tool;
}

/******************************************************************************/
HeightMapMouseCapture::~HeightMapMouseCapture( )
{
}

/******************************************************************************/
void HeightMapMouseCapture::OnLeftMouseUp(MouseEvent* event)
{
  if(mViewport.IsNull())
    return;

  ViewportMouseEvent e(event);
  mViewport->InitViewportEvent(e);
  mHeightMapTool->OnLeftMouseUp(&e);
}

/******************************************************************************/
void HeightMapMouseCapture::OnMouseScroll(MouseEvent* event)
{
  if(mViewport.IsNull( ))
    return;

  ViewportMouseEvent e(event);
  mViewport->InitViewportEvent(e);
  mHeightMapTool->OnMouseScroll(&e);
}

/******************************************************************************/
void HeightMapMouseCapture::OnMouseMove(MouseEvent* event)
{
  if(mViewport.IsNull( ))
    return;

  ViewportMouseEvent e(event);
  mViewport->InitViewportEvent(e);
  mHeightMapTool->OnMouseMove(&e);
}

/******************************************************************************/
void HeightMapMouseCapture::OnLeftMouseDrag(MouseEvent* event)
{
  if(mViewport.IsNull( ))
    return;

  ViewportMouseEvent e(event);
  mViewport->InitViewportEvent(e);
  mHeightMapTool->OnLeftMouseDrag(&e);
}

}//namespace Zero
