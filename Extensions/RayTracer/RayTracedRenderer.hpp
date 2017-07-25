///////////////////////////////////////////////////////////////////////////////
///
/// \file RayTracedRenderer.hpp
///  Declaration of the ray-traced renderer class.
///
/// Authors: Benjamin Strukus, Trevor Sundberg
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
//#include "Editor/EditorRenderer.hpp"
#include "RayTracer/RayTracer.hpp"

namespace Zero
{
// Any events we send out
namespace Events
{
  DeclareEvent(FrameCompleted);
}

// What color are the objects?
DeclareEnum5(ObjectColorMode, ModelColor, 
                              PhysicsModes, 
                              Normals, 
                              InverseNormals, 
                              AbsNormals);

//-------------------------------------------------------------- Frame Completed
// An event structure that is used when a frame has completed rendering
//struct FrameCompleted : public Event
//{
//  PixelBuffer* Buffer;
//};

//---------------------------------------------------------- Ray Traced Provider
// An interface that a user can implement to provide their own custom ray-tracing code
class RayTracedProvider : public Component
{
public:
  // Declare the ray trace provider
  ZeroDeclareType(RayTracedProvider);

  // Bind any needed properties
  static void InitializeMeta(MetaType* meta);

  // Initialize the component and bind any needed interfaces
  virtual void Initialize(CogInitializer& initializer);

  // Perform a ray-trace given a ray, and output the color that was determined
  virtual Vec4 RayTraceColor(Vec4Param backgroundColor, Vec3Param start, 
                             Vec3Param direction, IntVec2Param pixelPosition) = 0;

  //virtual Vec4 RayTraceColor(const Pixel& pixel, PixelSampler* pixelSampler) = 0;

  // Perform a test ray-trace given a ray (this is for debugging)
  virtual void RayTraceTest(Vec4Param backgroundColor, Vec3Param start, 
                            Vec3Param direction, IntVec2Param pixelPosition);

  // Is the ray-trace provider thread safe? Can we call 'RayTraceColor' from 
  // different threads?
  virtual bool IsThreadSafe();

  // An overridable method that is called when we begin rendering to a frame
  //virtual void BeginFrame(Space* space, PixelBuffer* buffer);

  // An overridable method that is called when we end rendering to a frame
  //virtual void EndFrame(Space* space, PixelBuffer* buffer);
};

//---------------------------------------------------------- Ray Traced Renderer
/// Ray-traced Scene renderer
//class RayTracedRenderer : public EditorRenderer
//{
//public:
//  // Declare the ray traced renderer
//  ZeroDeclareType(RayTracedRenderer);
//
//  // Friends
//  friend class ScanlineJob;
//
//  // Bind any needed properties
//  static void InitializeMeta(MetaType* meta);
//
//  // Constructor
//  RayTracedRenderer();
//
//  // Destructor
//  ~RayTracedRenderer();
//
//  // Initialize the component
//  void Initialize(CogInitializer& initializer);
//
//  // Is this renderer directly rendering to the viewport?
//  bool IsDirect() override;
//
//  // Return the final texture to use
//  Texture* PostTexture() override;
//
//  // Initialize the renderer's size
//  void InitializeBuffers(IntVec2 bufferSize) override;
//
//  // Render the space with the engine
//  void RenderSpace(DrawContext& dc, ViewState& state, Camera* camera, 
//                           GraphicsSpace* space, DisplayRender* displayRender);
//
//  // Render out a particular pixel and set it on the buffer
//  void RenderPixel(ViewState& state, Camera* camera, uint pixel);
//
//  // Set a pixel in the buffer
//  inline void SetPixel(uint x, uint y, ByteColor color);
//
//private:
//
//  // Initialize the buffers with the size (called every time we start a render over again)
//  void RestartRender();
//
//  // Draw the progress that we've made in ray-tracing
//  void DrawProgress();
//
//  // Reset the ray-tracing position
//  void RestartDrawing();
//
//  // Is the renderer active?
//  bool IsActive();
//
//  // Run a test ray cast on the given coordinates
//  void RunTest();
//
//  // Let the user pick where the test coordinates are
//  void PickTestCoordinates();
//
//  // Set the sample scale
//  void SetSampleScale(uint scale);
//
//  // Get the sample scale
//  uint GetSampleScale();
//
//  // Get a buffer of a given sample size (or create it if it doesn't exist)
//  PixelBuffer* GetBuffer(uint sampleSize);
//
//  // Get a picking ray from the screen position and view/camera state
//  static void GetRay(Vec2Param screenPos, ViewState& state, Camera* camera, 
//                     Vec3* start, Vec3* direction);
//
//private:
//
//  // Do we even want to allow threading at all?
//  bool mAllowThreading;
//
//  // The number of threads we want to launch
//  uint mThreadCount;
//
//  // Store the job completion count
//  s32 mJobCompletionCount;
//
//  // The dimensions of the viewport
//  uint mViewportWidth;
//  uint mViewportHeight;
//  uint mViewportTotal;
//
//  // The ray trace provider (who actually does the ray casting)
//  RayTracedProvider* mProvider;
//
//  // The final texture that we spit out
//  RenderRig* mFinal;
//
//  // Is the ray-tracer active (togglable by the user)
//  bool mActive;
//
//  // Is the ray-tracer internally active? (sometimes it goes offline based on certain events)
//  // This flag is not controlled by the user
//  bool mInternallyActive;
//
//  // The target frame time (in milliseconds) for ray tracing
//  float mTargetDrawTimeMs;
//
//  // Store the current pixel that we're on
//  uint mCurrentPixel;
//
//  // Store the test coordinates
//  uint mTestX;
//  uint mTestY;
//
//  // Are we currently picking test coordinates?
//  bool mPicking;
//
//  // Store the last view state
//  ViewState mLastViewState;
//
//  // Store the last camera cog
//  CogId mLastCameraObject;
//
//  // Store all the ray-trace buffers that we use
//  // The index is the down-scale of the buffer
//  HashMap<uint, PixelBuffer*> mBuffers;
//
//  // Store the current buffer that we're rendering to
//  PixelBuffer* mBuffer;
//
//  // The scale of the samples that we take (also indicates which buffer we are using)
//  uint mSampleScale;
//};

}//namespace Zero
