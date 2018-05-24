// Authors: Nathan Carlson
// Copyright 2015, DigiPen Institute of Technology

#pragma once

namespace Zero
{

namespace Events
{
  DeclareEvent(RenderTasksUpdate);
  DeclareEvent(RenderTasksUpdateInternal);
}

/// A list for custom specifying which Graphicals and in what draw order for a RenderPass.
class GraphicalRangeInterface
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Add a Graphical to the list.
  void Add(Graphical* graphical);

  /// Remove all Graphicals from the list.
  void Clear();
  
  /// Current number of Graphicals in the list.
  uint GetCount();

  Array<Graphical*> mGraphicals;
};

/// Interface for adding tasks for the renderer, essentially defining a rendering pipeline.
class RenderTasksEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  RenderTasksEvent();

  /// Object with the CameraViewport component that this event is getting tasks for.
  Cog* GetCameraViewportCog();
  /// Size of the UI viewport, or the resolution on CameraViewport if not rendering to viewport.
  IntVec2 GetViewportSize();

  // Dimensions are clamped to [1, 4096]
  /// Returns a RenderTarget for use when adding render tasks. Target only valid during this event.
  HandleOf<RenderTarget> GetRenderTarget(IntVec2 size, TextureFormat::Enum format);
  /// Returns a RenderTarget for use when adding render tasks. Target only valid during this event.
  HandleOf<RenderTarget> GetRenderTarget(IntVec2 size, TextureFormat::Enum format, SamplerSettings& samplerSettings);
  /// Returns a RenderTarget for use when adding render tasks. Target only valid during this event.
  /// Will render to the given texture instead of an internally managed texture.
  HandleOf<RenderTarget> GetRenderTarget(HandleOf<Texture> texture);

  /// Initializes all the internal texture data for the given RenderTargets.
  void AddRenderTaskClearTarget(RenderTarget* colorTarget, Vec4 color);
  /// Initializes all the internal texture data for the given RenderTargets.
  void AddRenderTaskClearTarget(RenderTarget* depthTarget, float depth);
  /// Initializes all the internal texture data for the given RenderTargets.
  void AddRenderTaskClearTarget(RenderTarget* depthTarget, float depth, uint stencil);
  /// Initializes all the internal texture data for the given RenderTargets.
  void AddRenderTaskClearTarget(RenderTarget* depthTarget, float depth, uint stencil, uint stencilWriteMask);
  /// Initializes all the internal texture data for the given RenderTargets.
  void AddRenderTaskClearTarget(RenderTarget* colorTarget, RenderTarget* depthTarget, Vec4 color, float depth);
  /// Initializes all the internal texture data for the given RenderTargets.
  void AddRenderTaskClearTarget(RenderTarget* colorTarget, RenderTarget* depthTarget, Vec4 color, float depth, uint stencil);
  /// Initializes all the internal texture data for the given RenderTargets.
  void AddRenderTaskClearTarget(RenderTarget* colorTarget, RenderTarget* depthTarget, Vec4 color, float depth, uint stencil, uint stencilWriteMask);
  /// Initializes all the internal texture data for the given RenderTargets.
  void AddRenderTaskClearTarget(GraphicsRenderSettings& renderSettings, Vec4 color);
  /// Initializes all the internal texture data for the given RenderTargets.
  void AddRenderTaskClearTarget(GraphicsRenderSettings& renderSettings, Vec4 color, float depth);
  /// Initializes all the internal texture data for the given RenderTargets.
  void AddRenderTaskClearTarget(GraphicsRenderSettings& renderSettings, Vec4 color, float depth, uint stencil);
  /// Initializes all the internal texture data for the given RenderTargets.
  void AddRenderTaskClearTarget(GraphicsRenderSettings& renderSettings, Vec4 color, float depth, uint stencil, uint stencilWriteMask);

  /// Renders a group of objects with the given settings. The RenderPass fragment defines what data is written to RenderTargets.
  void AddRenderTaskRenderPass(GraphicsRenderSettings& renderSettings, RenderGroup& renderGroup, MaterialBlock& renderPass);
  /// Renders a group of objects with the given settings. The RenderPass fragment defines what data is written to RenderTargets.
  void AddRenderTaskRenderPass(GraphicsRenderSettings& renderSettings, GraphicalRangeInterface& graphicalRange, MaterialBlock& renderPass);

  /// Invokes the pixel shader for every pixel of the RenderTargets.
  void AddRenderTaskPostProcess(RenderTarget* renderTarget, Material& material);
  /// Invokes the pixel shader for every pixel of the RenderTargets.
  void AddRenderTaskPostProcess(RenderTarget* renderTarget, MaterialBlock& postProcess);
  /// Invokes the pixel shader for every pixel of the RenderTargets.
  void AddRenderTaskPostProcess(GraphicsRenderSettings& renderSettings, Material& material);
  /// Invokes the pixel shader for every pixel of the RenderTargets.
  void AddRenderTaskPostProcess(GraphicsRenderSettings& renderSettings, MaterialBlock& postProcess);

  // Internal for the graphics engine.
  void AddRenderTaskBackBufferBlit(RenderTarget* colorTarget, ScreenViewport viewport);

  /// Returns a RenderTarget for rendering to the CameraViewport's FinalTexture that represents the end result of the renderer.
  HandleOf<RenderTarget> GetFinalTarget(IntVec2 size, TextureFormat::Enum format);
  /// Returns a RenderTarget for rendering to the CameraViewport's FinalTexture that represents the end result of the renderer.
  HandleOf<RenderTarget> GetFinalTarget(IntVec2 size, TextureFormat::Enum format, SamplerSettings& samplerSettings);

  // Internal
  uint GetUniqueShaderInputsId();
  void AddShaderInputs(Material* material, uint shaderInputsId);
  void AddShaderInputs(MaterialBlock* materialBlock, uint shaderInputsId);
  void AddShaderInputs(ShaderInputs* globalInputs, uint shaderInputsId);

  Cog* mCameraViewportCog;
  IntVec2 mViewportSize;

  RenderTasks* mRenderTasks;
  HandleOf<Texture> mFinalTexture;
  
  GraphicsSpace* mGraphicsSpace;
  Camera* mCamera;
};

class RenderTaskHelper
{
public:
  RenderTaskHelper(RenderTaskBuffer& buffer);

  template <typename T>
  T* NewRenderTask();

  void AddRenderTaskClearTarget(RenderSettings& renderSettings, Vec4 color, float depth, uint stencil, uint stencilWriteMask);
  void AddRenderTaskRenderPass(RenderSettings& renderSettings, uint renderGroupIndex, StringParam renderPassName, uint shaderInputRangesId);
  void AddRenderTaskPostProcess(RenderSettings& renderSettings, StringParam postProcessName, uint shaderInputsId);
  void AddRenderTaskPostProcess(RenderSettings& renderSettings, MaterialRenderData* materialRenderData, uint shaderInputsId);
  void AddRenderTaskBackBufferBlit(RenderTarget* colorTarget, ScreenViewport viewport);
  void AddRenderTaskTextureUpdate(Texture* texture);

  bool ValidateRenderTargets(RenderSettings& renderSettings);

  RenderTaskBuffer& mBuffer;
};

//**************************************************************************************************
template <typename T>
T* RenderTaskHelper::NewRenderTask()
{
  if (mBuffer.mCurrentIndex + sizeof(T) > mBuffer.mRenderTaskData.Size())
  {
    uint newSize = Math::Max(mBuffer.mRenderTaskData.Size() * 2, mBuffer.mCurrentIndex + sizeof(T));
    mBuffer.mRenderTaskData.Resize(newSize);
  }

  T* renderTask = new (&mBuffer.mRenderTaskData[mBuffer.mCurrentIndex]) T;
  mBuffer.mCurrentIndex += sizeof(T);
  ++mBuffer.mTaskCount;
  return renderTask;
}

class RenderTasksUpdateData
{
public:
  RenderTasksEvent* mEvent;
  EventDispatcher* mDispatcher;
  GraphicsSpace* mGraphicsSpace;
  Camera* mCamera;
  int mRenderOrder;
  uint mTaskCount;
};

// Method for sending RenderTasksUpdate event and collecting results
void RenderTasksUpdateHelper(RenderTasksUpdateData& update);

} // namespace Zero
