// MIT Licensed (see LICENSE.md).

#pragma once

namespace Raverie
{

namespace Events
{
DeclareEvent(RenderTasksUpdate);
DeclareEvent(RenderTasksUpdateInternal);
} // namespace Events

/// A list for custom specifying which Graphicals and in what draw order for a
/// RenderPass.
class GraphicalRangeInterface
{
public:
  RaverieDeclareType(GraphicalRangeInterface, TypeCopyMode::ReferenceType);

  /// Add a Graphical to the list.
  void Add(Graphical* graphical);

  /// Remove all Graphicals from the list.
  void Clear();

  /// Current number of Graphicals in the list.
  uint GetCount();

  Array<Graphical*> mGraphicals;
};

/// Interface used to define unique render settings for a base RenderGroup and
/// its sub RenderGroups.
class SubRenderGroupPass : public SafeId32
{
public:
  RaverieDeclareType(SubRenderGroupPass, TypeCopyMode::ReferenceType);

  SubRenderGroupPass(RenderTasksEvent* renderTasksEvent, RenderGroup& baseRenderGroup);

  /// Resets interface back to the initial creation state with a given base
  /// RenderGroup.
  void Reset(RenderGroup& baseRenderGroup);
  /// Settings to use for the base or all sub RenderGroups that do not have
  /// specified settings. Without defaults, the base or any sub RenderGroup
  /// without settings will not render.
  void SetDefaultSettings(GraphicsRenderSettings& defaultSettings, MaterialBlock& defaultPass);
  /// Define the settings to use for a specific RenderGroup.
  /// Given RenderGroup must be a child of the base RenderGroup, or the base
  /// itself, that this was initialized with.
  void AddSubSettings(GraphicsRenderSettings& subSettings, RenderGroup& subGroup, MaterialBlock& subPass);
  /// Explicitely exclude a RenderGroup from rendering when there are default
  /// settings. Given RenderGroup must be a child of the base RenderGroup, or
  /// the base itself, that this was initialized with.
  void ExcludeSubRenderGroup(RenderGroup& subGroup);

  // Internal
  bool ValidateSettings(GraphicsRenderSettings& renderSettings, MaterialBlock& renderPass);
  bool ValidateRenderGroup(RenderGroup& renderGroup);

  struct SubData
  {
    GraphicsRenderSettings mRenderSettings;
    HandleOf<RenderGroup> mRenderGroup;
    HandleOf<MaterialBlock> mRenderPass;
    bool mRender;
  };

  RenderTasksEvent* mRenderTasksEvent;
  HandleOf<RenderGroup> mBaseRenderGroup;
  Array<SubData> mSubData;
};

/// Interface for adding tasks for the renderer, essentially defining a
/// rendering pipeline.
class RenderTasksEvent : public Event
{
public:
  RaverieDeclareType(RenderTasksEvent, TypeCopyMode::ReferenceType);

  RenderTasksEvent();
  ~RenderTasksEvent();

  /// Object with the CameraViewport component that this event is getting tasks
  /// for.
  Cog* GetCameraViewportCog();
  /// Size of the UI viewport, or the resolution on CameraViewport if not
  /// rendering to viewport.
  IntVec2 GetViewportSize();

  // Dimensions are clamped to [1, 4096]
  /// Returns a RenderTarget for use when adding render tasks. Target only valid
  /// during this event.
  HandleOf<RenderTarget> GetRenderTarget(IntVec2 size, TextureFormat::Enum format);
  /// Returns a RenderTarget for use when adding render tasks. Target only valid
  /// during this event.
  HandleOf<RenderTarget> GetRenderTarget(IntVec2 size, TextureFormat::Enum format, SamplerSettings& samplerSettings);
  /// Returns a RenderTarget for use when adding render tasks. Target only valid
  /// during this event. Will render to the given texture instead of an
  /// internally managed texture.
  HandleOf<RenderTarget> GetRenderTarget(HandleOf<Texture> texture);

  /// Creates the interface used to define unique render settings for a base
  /// RenderGroup and its sub RenderGroups. The given RenderGroup is used to
  /// define the hierarchy, or sub hierarchy, that should be rendered. The given
  /// RenderGroup also defines the sort order for all objects that are within
  /// its hierarchy. Returned SubRenderGroupPass is only valid during this
  /// event.
  HandleOf<SubRenderGroupPass> CreateSubRenderGroupPass(RenderGroup& baseGroup);

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

  /// Renders a group of objects with the given settings. The RenderPass
  /// fragment defines what data is written to RenderTargets.
  void AddRenderTaskRenderPass(GraphicsRenderSettings& renderSettings, RenderGroup& renderGroup, MaterialBlock& renderPass);
  /// Renders a group of objects with the given settings. The RenderPass
  /// fragment defines what data is written to RenderTargets.
  void AddRenderTaskRenderPass(GraphicsRenderSettings& renderSettings, GraphicalRangeInterface& graphicalRange, MaterialBlock& renderPass);

  /// Renders all objects within a RenderGroup hierarchy, sorted in the order
  /// defined by the base RenderGroup, and can use unique render settings for
  /// each RenderGroup in the hierarchy.
  void AddRenderTaskSubRenderGroupPass(SubRenderGroupPass& subRenderGroupPass);

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

  /// Returns a RenderTarget for rendering to the CameraViewport's FinalTexture
  /// that represents the end result of the renderer.
  HandleOf<RenderTarget> GetFinalTarget(IntVec2 size, TextureFormat::Enum format);
  /// Returns a RenderTarget for rendering to the CameraViewport's FinalTexture
  /// that represents the end result of the renderer.
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

  // All created SubRenderGroupPasses have to be destroyed when the event is
  // done.
  Array<SubRenderGroupPass*> mSubRenderGroupPasses;
};

class RenderTaskHelper
{
public:
  RenderTaskHelper(RenderTaskBuffer& buffer);

  template <typename T>
  T* NewRenderTask();

  void AddRenderTaskClearTarget(RenderSettings& renderSettings, Vec4 color, float depth, uint stencil, uint stencilWriteMask);
  void AddRenderTaskRenderPass(RenderSettings& renderSettings, uint renderGroupIndex, StringParam renderPassName, uint shaderInputRangesId, uint subRenderGroupCount = 0, bool render = true);
  void AddRenderTaskPostProcess(RenderSettings& renderSettings, StringParam postProcessName, uint shaderInputsId);
  void AddRenderTaskPostProcess(RenderSettings& renderSettings, MaterialRenderData* materialRenderData, uint shaderInputsId);
  void AddRenderTaskBackBufferBlit(RenderTarget* colorTarget, ScreenViewport viewport);
  void AddRenderTaskTextureUpdate(Texture* texture);

  bool ValidateRenderTargets(RenderSettings& renderSettings);

  RenderTaskBuffer& mBuffer;
};

template <typename T>
T* RenderTaskHelper::NewRenderTask()
{
  ErrorIf(mBuffer.mCurrentIndex % MaxPrimtiveSize != 0, "Should always be aligned");

  uint newIndex = mBuffer.mCurrentIndex + sizeof(T);
  if (newIndex > mBuffer.mRenderTaskData.Size())
  {
    size_t newSize = Math::Max(mBuffer.mRenderTaskData.Size() * 2, (size_t)newIndex);
    mBuffer.mRenderTaskData.Resize(newSize);
  }

  T* renderTask = new (&mBuffer.mRenderTaskData[mBuffer.mCurrentIndex]) T;
  mBuffer.mCurrentIndex = newIndex;
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

} // namespace Raverie
