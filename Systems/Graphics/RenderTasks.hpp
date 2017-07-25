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
  uint mRenderGroupIndex;
  String mRenderPassName;
  uint mShaderInputsId;
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
  void AddRenderTaskRenderPass(RenderSettings& renderSettings, uint renderGroupIndex, StringParam renderPassName, uint shaderInputRangesId);
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
};

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
