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

static const u64 cFragmentShaderInputsId = 0;
static const u64 cGlobalShaderInputsId = 1;
static const u64 cGraphicalShaderInputsId = 2;

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

class ScreenViewport
{
public:
  int x, y, width, height;
};

class RenderTask
{
public:
  byte mId;
};

class RenderTaskClearTarget : public RenderTask
{
public:
  RenderSettings mRenderSettings;
  Vec4 mColor;
  float mDepth;
  uint mStencil;
  uint mStencilWriteMask;
};

class RenderTaskRenderPass : public RenderTask
{
public:
  RenderSettings mRenderSettings;
  // Index or id of the RenderGroup that these settings are for.
  uint mRenderGroupIndex;
  // Name of the RenderPass fragment for shader lookups. Inputs are mapped when creating this task.
  String mRenderPassName;
  // Id used to lookup all shader input data for the RenderPass fragment and all Materials
  // that will be used in this render task.
  uint mShaderInputsId;
  // If not zero, this is the number of contiguous RenderTaskRenderPass objects in memory
  // after this one. The renderer must account for this.
  uint mSubRenderGroupCount;
  // Allows sub RenderGroups to be set as excluded from rendering.
  // Only mRenderGroupIndex is valid when this is false.
  bool mRender;
};

class RenderTaskPostProcess : public RenderTask
{
public:
  RenderSettings mRenderSettings;
  String mPostProcessName;
  MaterialRenderData* mMaterialRenderData;
  uint mShaderInputsId;
};

class RenderTaskBackBufferBlit : public RenderTask
{
public:
  TextureRenderData* mColorTarget;
  uint mTextureWidth;
  uint mTextureHeight;
  ScreenViewport mViewport;
};

class RenderTaskTextureUpdate : public RenderTask
{
public:
  TextureRenderData* mRenderData;
  uint mWidth;
  uint mHeight;
  TextureType::Enum mType;
  TextureFormat::Enum mFormat;
  TextureAddressing::Enum mAddressingX;
  TextureAddressing::Enum mAddressingY;
  TextureFiltering::Enum mFiltering;
  TextureCompareMode::Enum mCompareMode;
  TextureCompareFunc::Enum mCompareFunc;
  TextureAnisotropy::Enum mAnisotropy;
  TextureMipMapping::Enum mMipMapping;
};

class RenderTaskBuffer
{
public:

  RenderTaskBuffer();

  void Clear();

  template <typename T>
  T* NewRenderTask();

  void AddRenderTaskClearTarget(RenderSettings& renderSettings, Vec4 color, float depth, uint stencil, uint stencilWriteMask);
  void AddRenderTaskRenderPass(RenderSettings& renderSettings, uint renderGroupIndex, StringParam renderPassName, uint shaderInputsId, uint subRenderGroupCount = 0, bool render = true);
  void AddRenderTaskPostProcess(RenderSettings& renderSettings, StringParam postProcessName, uint shaderInputsId);
  void AddRenderTaskPostProcess(RenderSettings& renderSettings, MaterialRenderData* materialRenderData, uint shaderInputsId);
  void AddRenderTaskBackBufferBlit(RenderTarget* colorTarget, ScreenViewport viewport);
  void AddRenderTaskTextureUpdate(Texture* texture);

  bool ValidateRenderTargets(RenderSettings& renderSettings);

  uint mTaskCount;
  uint mCurrentIndex;
  Array<byte> mRenderTaskData;
};

class RenderTaskRange
{
public:
  bool operator<(const RenderTaskRange& other) const;

  int mRenderOrder;
  uint mTaskIndex;
  uint mTaskCount;
  uint mFrameBlockIndex;
  uint mViewBlockIndex;
};

class RenderTasks
{
public:
  void Clear();

  Array<RenderTaskRange> mRenderTaskRanges;
  RenderTaskBuffer mRenderTaskBuffer;

  HashMap<Pair<u64, uint>, IndexRange> mShaderInputRanges;
  Array<ShaderInput> mShaderInputs;
  uint mShaderInputsVersion;
};

/// Interface used to define unique render settings for a base RenderGroup and its sub RenderGroups.
class SubRenderGroupPass : public SafeId32
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SubRenderGroupPass(RenderTasksEvent* renderTasksEvent, RenderGroup& baseRenderGroup);

  /// Resets interface back to the initial creation state with a given base RenderGroup.
  void Reset(RenderGroup& baseRenderGroup);
  /// Settings to use for the base or all sub RenderGroups that do not have specified settings.
  /// Without defaults, the base or any sub RenderGroup without settings will not render.
  void SetDefaultSettings(RenderSettings& defaultSettings, MaterialBlock& defaultPass);
  /// Define the settings to use for a specific RenderGroup.
  /// Given RenderGroup must be a child of the base RenderGroup, or the base itself, that this was initialized with.
  void AddSubSettings(RenderSettings& subSettings, RenderGroup& subGroup, MaterialBlock& subPass);
  /// Explicitely exclude a RenderGroup from rendering when there are default settings.
  /// Given RenderGroup must be a child of the base RenderGroup, or the base itself, that this was initialized with.
  void ExcludeSubRenderGroup(RenderGroup& subGroup);

  // Internal
  bool ValidateSettings(RenderSettings& renderSettings, MaterialBlock& renderPass);
  bool ValidateRenderGroup(RenderGroup& renderGroup);

  struct SubData
  {
    RenderSettings mRenderSettings;
    HandleOf<RenderGroup> mRenderGroup;
    MaterialBlock mRenderPass;
    bool mRender;
  };

  RenderTasksEvent* mRenderTasksEvent;
  HandleOf<RenderGroup> mBaseRenderGroup;
  Array<SubData> mSubData;
};

/// Interface for adding tasks for the renderer, essentially defining a rendering pipeline.
class RenderTasksEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  RenderTasksEvent();
  ~RenderTasksEvent();

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

  /// Creates the interface used to define unique render settings for a base RenderGroup and its sub RenderGroups.
  /// The given RenderGroup is used to define the hierarchy, or sub hierarchy, that should be rendered.
  /// The given RenderGroup also defines the sort order for all objects that are within its hierarchy.
  /// Returned SubRenderGroupPass is only valid during this event.
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
  void AddRenderTaskClearTarget(RenderSettings& renderSettings, Vec4 color);
  /// Initializes all the internal texture data for the given RenderTargets.
  void AddRenderTaskClearTarget(RenderSettings& renderSettings, Vec4 color, float depth);
  /// Initializes all the internal texture data for the given RenderTargets.
  void AddRenderTaskClearTarget(RenderSettings& renderSettings, Vec4 color, float depth, uint stencil);
  /// Initializes all the internal texture data for the given RenderTargets.
  void AddRenderTaskClearTarget(RenderSettings& renderSettings, Vec4 color, float depth, uint stencil, uint stencilWriteMask);

  /// Renders a group of objects with the given settings. The RenderPass fragment defines what data is written to RenderTargets.
  void AddRenderTaskRenderPass(RenderSettings& renderSettings, RenderGroup& renderGroup, MaterialBlock& renderPass);
  /// Renders a group of objects with the given settings. The RenderPass fragment defines what data is written to RenderTargets.
  void AddRenderTaskRenderPass(RenderSettings& renderSettings, GraphicalRangeInterface& graphicalRange, MaterialBlock& renderPass);

  /// Renders all objects within a RenderGroup hierarchy, sorted in the order defined by the base RenderGroup,
  /// and can use unique render settings for each RenderGroup in the hierarchy.
  void AddRenderTaskSubRenderGroupPass(SubRenderGroupPass& subRenderGroupPass);

  /// Invokes the pixel shader for every pixel of the RenderTargets.
  void AddRenderTaskPostProcess(RenderTarget* renderTarget, Material& material);
  /// Invokes the pixel shader for every pixel of the RenderTargets.
  void AddRenderTaskPostProcess(RenderTarget* renderTarget, MaterialBlock& postProcess);
  /// Invokes the pixel shader for every pixel of the RenderTargets.
  void AddRenderTaskPostProcess(RenderSettings& renderSettings, Material& material);
  /// Invokes the pixel shader for every pixel of the RenderTargets.
  void AddRenderTaskPostProcess(RenderSettings& renderSettings, MaterialBlock& postProcess);

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

  // All created SubRenderGroupPasses have to be destroyed when the event is done.
  Array<SubRenderGroupPass*> mSubRenderGroupPasses;
};

//**************************************************************************************************
template <typename T>
T* RenderTaskBuffer::NewRenderTask()
{
  if (mCurrentIndex + sizeof(T) > mRenderTaskData.Size())
  {
    uint newSize = Math::Max(mRenderTaskData.Size() * 2, mCurrentIndex + sizeof(T));
    mRenderTaskData.Resize(newSize);
  }

  T* renderTask = new (&mRenderTaskData[mCurrentIndex]) T;
  mCurrentIndex += sizeof(T);
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
