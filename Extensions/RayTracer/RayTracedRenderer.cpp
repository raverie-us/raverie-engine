///////////////////////////////////////////////////////////////////////////////
///
/// \file RayTracedRenderer.cpp
///  Implementation of the ray-traced renderer class.
///
/// Authors: Trevor Sundberg, Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "RayTracedRenderer.hpp"
#include "Platform/Utilities.hpp"
#include "Engine/Time.hpp"
#include "Engine/JobSystem.hpp"
#include "Engine/ThreadDispatch.hpp"
#include "Common/Utility/Atomic.hpp"
#include "RayTracer/RayTracer.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(FrameCompleted);
}

//Vec2 ScreenPosition(PixelBuffer* buffer, uint x, uint y)
//{
//  Vec2 screenPos = Vec2(real(x), real(y));
//  screenPos.x /= buffer->Width;
//  screenPos.y /= buffer->Height;
//  screenPos.x *= 2.0f;
//  screenPos.y *= 2.0f;
//  screenPos.x -= 1.0f;
//  screenPos.y -= 1.0f;
//  screenPos.x += real(0.5) / real(buffer->Width);
//  screenPos.y += real(0.5) / real(buffer->Height);
//  screenPos.y *= -1.0f;
//  return screenPos;
//}

//------------------------------------------------------------ Ray Trace Functor
class RayTraceFunctor : public PixelSampler
{
public:
  RayTraceFunctor(ViewState& state, Camera& camera, RayTracedProvider& provider,
                  Vec4Param clearColor)
    : mState(state), mCamera(camera), mProvider(provider),
      mClearColor(clearColor)
  {
    //
  }

  void GetRay(Vec2Param pixelCenter, Vec3Ptr start, Vec3Ptr direction)
  {
    mPixelCenter = pixelCenter;
    Vec3 ndcPosition(pixelCenter.x, pixelCenter.y, 0);

    Mat4 perspective = mState.Projection.Inverted();
    Vec3 viewPosition = Math::TransformPointCol(perspective, ndcPosition);

    Mat4 view = mState.View.Inverted();
    Vec3 worldPosition = Math::TransformPointCol(view, viewPosition);

    *start = mState.EyePosition;
    if(mCamera.mPerspectiveMode == PerspectiveMode::Perspective)
    {
      *direction = (worldPosition - mState.EyePosition).Normalized();
    }
    else
    {
      *direction = mState.EyeDirection;
    }
  }

  Vec4 RayTraceColor(Vec3Param start, Vec3Param direction) const
  {
    IntVec2 pixel = IntVec2(int(mPixelCenter.x * mState.ViewportSize.x),
                            int(mPixelCenter.y * mState.ViewportSize.y));
    return mProvider.RayTraceColor(mClearColor, start, direction, pixel);
  }

private:
  ViewState&         mState;
  Camera&            mCamera;
  RayTracedProvider& mProvider;
  Vec2               mPixelCenter;
  Vec4               mClearColor;
};

//---------------------------------------------------------- Ray Traced Renderer
// Implement the meta
//ZeroDefineType(RayTracedRenderer);
//
//// Constructor
//RayTracedRenderer::RayTracedRenderer()
//{
//  // Clear the final texture pointer (for safety)
//  mFinal = NULL;
//
//  // Make the clear color black (for dramatic effect :P)
//  mClearColor = Vec4(0, 0, 0, 1);
//
//  // We don't yet know the size of the viewport
//  mViewportWidth = 0;
//  mViewportHeight = 0;
//  mViewportTotal = 0;
//
//  // Start off on pixel zero (upper left)
//  mCurrentPixel = 0;
//
//  // The target draw time is 12 milliseconds (more than half the frame time)
//  mTargetDrawTimeMs = 12.0f;
//
//  // The ray-tracer should be active
//  mActive = true;
//  mInternallyActive = true;
//
//  // Just set the test coordinates to be the upper left
//  mTestX = 0;
//  mTestY = 0;
//
//  // We are not currently picking test coordinates
//  mPicking = false;
//
//  // Use a sample scale of 1 pixel per ray
//  mSampleScale = 1;
//
//  // We don't yet have a buffer that we're rendering to
//  mBuffer = NULL;
//
//  // Assume we have 8 threads
//  mThreadCount = 8;
//
//  // By default, we allow threading if the provider allows it
//  mAllowThreading = true;
//}
//
//// Bind any needed properties
//void RayTracedRenderer::InitializeMeta(MetaType* meta)
//{
//  BindBase(Renderer);
//  ZeroBindDocumented();
//  ZeroBindSetup(SetupMode::DefaultSerialization);
//  ZeroBindDependency(RayTracedProvider);
//
//  ZilchBindFieldProperty(mActive);
//  ZilchBindFieldProperty(mTargetDrawTimeMs);
//  ZilchBindFieldProperty(mAllowThreading);
//  ZilchBindFieldProperty(mThreadCount);
//  ZilchBindGetterSetterProperty(SampleScale);
//
//  ZilchBindFieldProperty(mTestX);
//  ZilchBindFieldProperty(mTestY);
//  ZilchBindMethodProperty(PickTestCoordinates);
//  ZilchBindMethodProperty(RunTest);
//
//  ZilchBindMethodProperty(RestartDrawing);
//}
//
//// Initialize the component
//void RayTracedRenderer::Initialize(CogInitializer& initializer)
//{
//  // Initialize our base
//  EditorRenderer::Initialize(initializer);
//
//  // Get the provider
//  mProvider = GetOwner()->has(RayTracedProvider);
//}
//
//// Destructor
//RayTracedRenderer::~RayTracedRenderer()
//{
//  DeleteObjectsInContainer(mBuffers);
//  SafeDelete(mFinal);
//}
//
//// Is this renderer directly rendering to the viewport?
//bool RayTracedRenderer::IsDirect()
//{
//  // We always render to a texture, even in the case of the EditorRenderer
//  return false;
//}
//
//// Return the final texture to use
//Texture* RayTracedRenderer::PostTexture()
//{
//  // We always render to a texture, even in the case of the EditorRenderer
//  return mFinal->ColorTexture;
//}
//
//// Get a buffer of a given sample size (or create it if it doesn't exist)
//PixelBuffer* RayTracedRenderer::GetBuffer(uint sampleSize)
//{
//  uint width = mViewportWidth / mSampleScale;
//  uint height = mViewportHeight / mSampleScale;
//
//  PixelBuffer*& buffer = mBuffers[sampleSize];
//
//  if(buffer == NULL)
//  {
//    buffer = new PixelBuffer(ToByteColor(mClearColor), width, height);
//  }
//
//  return buffer;
//}
//
//// Initialize the renderer's size
//void RayTracedRenderer::InitializeBuffers(IntVec2 bufferSize)
//{
//  // Initialize our base class viewport
//  EditorRenderer::InitializeBuffers(bufferSize);
//
//  int width = bufferSize.x;
//  int height = bufferSize.y;
//
//  // Store the width and height
//  mViewportWidth = width;
//  mViewportHeight = height;
//
//  // Compute the total number of pixels
//  mViewportTotal = width * height;
//
//  // Create the current buffer
//  mBuffer = GetBuffer(mSampleScale);
//
//  // Create the texture
//  mFinal = new RenderRig();
//  mFinal->Initialize(width, height, TextureFormat::RGBA8, TextureFormat::Depth32);
//}
//
//// Run a test ray cast on the given coordinates
//void RayTracedRenderer::RunTest()
//{
//  Cog* cameraCog = mLastCameraObject;
//
//  if(cameraCog == NULL)
//    return;
//
//  Camera* camera = cameraCog->has(Camera);
//
//  if(camera == NULL)
//    return;
//
//  if(mBuffer == NULL)
//    return;
//
//  Vec2 screenPos = ScreenPosition(mBuffer, mTestX, mTestY);
//
//  Vec3 start, direction;
//  GetRay(screenPos, mLastViewState, camera, &start, &direction);
//
//  mProvider->RayTraceTest(mClearColor, start, direction, IntVec2(mTestX, mTestY));
//}
//
//// Let the user pick where the test coordinates are
//void RayTracedRenderer::PickTestCoordinates()
//{
//  mPicking = true;
//}
//
//// Set the sample scale
//void RayTracedRenderer::SetSampleScale(uint scale)
//{
//  if(mSampleScale != scale)
//  {
//    RestartDrawing();
//
//    if(scale >= 1)
//    {
//      mSampleScale = scale;
//    }
//    else
//    {
//      mSampleScale = 1;
//    }
//
//    mBuffer = GetBuffer(mSampleScale);
//  }
//}
//
//// Get the sample scale
//uint RayTracedRenderer::GetSampleScale()
//{
//  return mSampleScale;
//}
//
//void RayTracedRenderer::GetRay(Vec2Param screenPos, ViewState& state,
//                               Camera* camera, Vec3* start, Vec3* direction)
//{
//  Vec3 ndcPosition(screenPos.x, screenPos.y, 0);
//
//  Mat4 perspective = state.Projection.Inverted();
//  Vec3 viewPosition = Math::TransformPointCol(perspective, ndcPosition);
//
//  Mat4 view = state.View.Inverted();
//  Vec3 worldPosition = Math::TransformPointCol(view, viewPosition);
//
//  *start = state.EyePosition;
//  if(camera->mPerspectiveMode == PerspectiveMode::Perspective)
//  {
//    *direction = (worldPosition - state.EyePosition).Normalized();
//  }
//  else
//  {
//    *direction = state.EyeDirection;
//  }
//}
//
//// Reset the ray-tracing position
//void RayTracedRenderer::RestartDrawing()
//{
//  mCurrentPixel = 0;
//}
//
//// Is the renderer active?
//bool RayTracedRenderer::IsActive()
//{
//  return mActive && mInternallyActive;
//}
//
//// Draw the progress that we've made in ray-tracing
//void RayTracedRenderer::DrawProgress()
//{
//  uint nextLine = ((mCurrentPixel - 1) / mBuffer->Width) + 1;
//
//  if(nextLine < mBuffer->Height)
//  {
//    for(uint x = 0; x < mBuffer->Width; ++x)
//    {
//      mBuffer->SetPixel(x, nextLine, Color::Red);
//    }
//  }
//}
//
//void RayTracedRenderer::RenderPixel(ViewState& state, Camera* camera, uint pixel)
//{
//  uint x, y;
//  mBuffer->GetCoordinates(pixel, &x, &y);
//
//  RayTraceFunctor rayTraceFunctor = RayTraceFunctor(state, *camera, *mProvider,
//                                                    mClearColor);
//
//  Vec2 screenPos = ScreenPosition(mBuffer, x, y);
//  Vec2 pixelDim = Vec2(real(1.0) / real(mBuffer->Width),
//                       real(1.0) / real(mBuffer->Height));
//  Pixel pixelData = Pixel(screenPos, pixelDim);
//  Vec4 finalColor = mProvider->RayTraceColor(pixelData, &rayTraceFunctor);
//
//  finalColor.x = Math::Clamp(finalColor.x);
//  finalColor.y = Math::Clamp(finalColor.y);
//  finalColor.z = Math::Clamp(finalColor.z);
//  finalColor.w = Math::Clamp(finalColor.w);
//
//  ByteColor finalByteColor = ToByteColor(finalColor);
//  mBuffer->SetPixel(x, y, finalByteColor);
//}

//class ScanlineJob : public Job
//{
//public:
//  // The scan line we're rendering
//  uint StartPixel;
//
//  // The number of pixels in this scan line (width)
//  uint PixelCount;
//
//  // The view state
//  ViewState State;
//
//  // The camera we're rendering from
//  Camera* RenderCamera;
//
//  // Store a pointer back to the renderer
//  RayTracedRenderer* Renderer;
//
//  int Execute() override
//  {
//    uint endPixel = StartPixel + PixelCount;
//
//    for (uint pixel = StartPixel; pixel < endPixel; ++pixel)
//    {
//      Renderer->RenderPixel(State, RenderCamera, pixel);
//    }
//
//    // We completed the job...
//    AtomicPreIncrement(&Renderer->mJobCompletionCount);
//    return 0;
//  }
//};

//// Render the space with the engine
//void RayTracedRenderer::RenderSpace(DrawContext& dc, ViewState& state, Camera* camera,
//                                    GraphicsSpace* space, DisplayRender* displayRender)
//{
//  if(mFinal == NULL)
//    return;
//
//  // Store away the last view state
//  mLastViewState = state;
//
//  // Store away the last camera (its owner)
//  mLastCameraObject = camera->GetOwner();
//
//  if(IsActive())
//  {
//    // Get the owning space of the graphics provider
//    Space* ownerSpace = space->GetSpace();
//
//    // If we're at the beginning then send a frame begin event
//    if(mCurrentPixel == 0)
//    {
//      mProvider->BeginFrame(ownerSpace, mBuffer);
//    }
//
//    // If the ray tracer is thread safe...
//    if(mProvider->IsThreadSafe() && mAllowThreading)
//    {
//      // How many threads (jobs) we expect to complete
//      mJobCompletionCount = 0;
//
//      // Get the starting scanline
//      uint startLine = mCurrentPixel / mBuffer->Width;
//
//      // The number of scan lines we're rendering
//      uint scanLineCount = Math::Min(mThreadCount, mBuffer->Height - startLine);
//
//      // We need to create all the jobs (hopefully for each thread)
//      for(uint i = 0; i < scanLineCount; ++i)
//      {
//        // Setup the scanline job
//        auto job = new ScanlineJob();
//        job->RenderCamera = camera;
//        job->State = state;
//        job->StartPixel = startLine * mBuffer->Width;
//        job->PixelCount = mBuffer->Width;
//        job->Renderer = this;
//
//        // Queue up the job
//        Z::gJobs->AddJob(job);
//
//        // Move to the next line
//        ++startLine;
//      }
//
//      // Loop until all jobs finish
//      while(mJobCompletionCount != scanLineCount)
//      {
//        // Sleep until the next time we check
//        Os::Sleep(1);
//      }
//
//      // Compute the current pixel
//      mCurrentPixel = startLine * mBuffer->Width;
//    }
//    else
//    {
//      Timer timer;
//
//      // The number of milliseconds per second
//      const float cMillisecondsPerSecond = 1000.0f;
//
//      // While we have time to render...
//      while((float)timer.UpdateAndGetTime() < (mTargetDrawTimeMs / cMillisecondsPerSecond))
//      {
//        RenderPixel(state, camera, mCurrentPixel);
//
//        ++mCurrentPixel;
//
//        if(mCurrentPixel == mBuffer->Total)
//        {
//          break;
//        }
//      }
//    }
//
//    if(mCurrentPixel >= mBuffer->Total)
//    {
//      FrameCompleted event;
//      event.Buffer = mBuffer;
//
//      GetDispatcher()->Dispatch(Events::FrameCompleted, &event);
//      RestartDrawing();
//
//      mProvider->EndFrame(ownerSpace, mBuffer);
//    }
//
//    DrawProgress();
//
//    mBuffer->Upload();
//
//    PostProcess(dc, state, mBuffer->Image, NULL, mFinal, "PostCopy");
//
//    mFinal->BeginRender(dc);
//
//    // Render the debug drawing
//    if(mDrawDebugObjects)
//    {
//      dc.Clear(cClearModeDepthStencil);
//      RenderDebugElements(dc, space, state);
//      RenderDebugText(dc, state, space);
//    }
//
//    mFinal->EndRender(dc);
//  }
//  else
//  {
//    mFinal->BeginRender(dc);
//
//    dc.SetClearColor(mClearColor);
//    dc.Clear(cClearModeAll);
//
//
//    // Use the editor renderer
//    EditorRenderer::RenderSpace(dc, state, camera, space, displayRender);
//    mFinal->EndRender(dc);
//  }
//}

// Define the ray trace provider
ZeroDefineType(RayTracedProvider);

// Bind any needed properties
void RayTracedProvider::InitializeMeta(MetaType* meta)
{
  BindBase(Component);
  ZeroBindDocumented();
  ZeroBindInterface(RayTracedProvider);
}

// Initialize the component and bind any needed interfaces
void RayTracedProvider::Initialize(CogInitializer& initializer)
{
  GetOwner()->AddComponentInterface(RayTracedProvider::GetTypeId(), this);
}

// Perform a test ray-trace given a ray (this is for debugging)
void RayTracedProvider::RayTraceTest(Vec4Param backgroundColor, Vec3Param start,
                                     Vec3Param direction, IntVec2Param pixelPosition)
{
  // Breakpoint before the test
  Os::DebugBreak();

  // Run the actual ray-trace
  Vec4 result = RayTraceColor(backgroundColor, start, direction, pixelPosition);

  // Print out the ray-tracing color
  DebugPrint("RayTraceTest Color: (%g, %g, %g, %g)\n",
             result.x, result.y, result.z, result.w);
}

// Is the ray-trace provider thread safe? Can we call 'RayTraceColor' from
// different threads?
bool RayTracedProvider::IsThreadSafe()
{
  // By default, the best thing to assume is that no provider is thread safe
  return false;
}

//// An overridable method that is called when we begin rendering to a frame
//void RayTracedProvider::BeginFrame(Space* space, PixelBuffer* buffer)
//{
//}
//
//// An overridable method that is called when we end rendering to a frame
//void RayTracedProvider::EndFrame(Space* space, PixelBuffer* buffer)
//{
//}

}//namespace Zero
