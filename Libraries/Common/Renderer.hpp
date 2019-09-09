// MIT Licensed (see LICENSE.md).

#pragma once

namespace Zero
{

// Implemented by api specific renderer (just call C++ delete on the renderer to
// destroy it)
class Renderer;
Renderer* CreateRenderer(OsHandle windowHandle, String& error);

extern const String cPostVertex;
StringParam GetCoreVertexFragmentName(CoreVertexType::Enum type);

// Base types for renderer to implement resource render data
class MaterialRenderData
{
public:
  String mCompositeName;
  u64 mResourceId;
};
class MeshRenderData
{
};
class TextureRenderData
{
};

class RenderTasks;
class RenderQueues;

/// Information about the active graphics hardware.
class GraphicsDriverSupport
{
public:
  GraphicsDriverSupport();

  // Must be filled out by the renderer in initialization.
  /// If block compression formats can be sampled by hardware.
  bool mTextureCompression;
  /// If blend settings for the output merger stage can be independently set for
  /// multiple render targets.
  bool mMultiTargetBlend;
  /// If texture sampler settings can be uniquely specified per sampler shader
  /// input.
  bool mSamplerObjects;

  // For detecting Intel drivers to handle driver bugs.
  bool mIntel;
};

// Do not reorder, value used to calculate number of vertices per primitive
DeclareEnum3(PrimitiveType, Points, Lines, Triangles);

// Semantics are used to communicate what this element is
// with the vertex shader
DeclareEnum17(VertexSemantic,
              Position,
              Normal,
              Tangent,
              Bitangent,
              Uv,
              UvAux,
              Color,
              ColorAux,
              BoneWeights,
              BoneIndices,
              Aux0,
              Aux1,
              Aux2,
              Aux3,
              Aux4,
              Aux5,
              None);

// Type of element in vertex
DeclareEnum6(VertexElementType,
             Byte,
             Short,
             Half,
             Real,
             // Normalized Types map 0 to 1
             NormByte,
             NormShort);

DeclareEnum3(IndexElementType, Byte, Ushort, Uint);

#pragma pack(push, 1)
class VertexAttribute
{
public:
  VertexAttribute(){};
  VertexAttribute(VertexSemantic::Enum semantic, VertexElementType::Enum type, byte count, byte offset);

  ByteEnum<VertexSemantic::Enum> mSemantic;
  ByteEnum<VertexElementType::Enum> mType;
  byte mCount;
  byte mOffset;
};
#pragma pack(pop)

class MeshBone
{
public:
  String mName;
  Math::Mat4 mBindTransform;
};

class StreamedVertex
{
public:
  StreamedVertex()
  {
  }
  StreamedVertex(Vec3 position, Vec2 uv, Vec4 color, Vec2 uvAux = Vec2::cZero) :
      mPosition(position),
      mUv(uv),
      mColor(color),
      mUvAux(uvAux)
  {
  }

  Vec3 mPosition;
  Vec2 mUv;
  Vec4 mColor;
  Vec2 mUvAux;
};

// 1.375MB per block at 44 bytes per StreamedVertex.
typedef PodBlockArray<StreamedVertex, 15> StreamedVertexArray;

/// Type of the texture, must match sampler type in shaders
/// Texture2D - Standard 2 dimensional texture
/// TextureCube - Uses texture as a cubemap
///   Faces are extracted from the image using aspect ratio to determine layout
DeclareEnum2(TextureType, Texture2D, TextureCube);

DeclareEnum25(TextureFormat,
              None,
              R8,
              RG8,
              RGB8,
              RGBA8, // byte
              R16,
              RG16,
              RGB16,
              RGBA16, // short
              R16f,
              RG16f,
              RGB16f,
              RGBA16f, // half float
              R32f,
              RG32f,
              RGB32f,
              RGBA32f, // float
              SRGB8,
              SRGB8A8, // gamma
              Depth16,
              Depth24,
              Depth32,
              Depth32f, // depth
              Depth24Stencil8,
              Depth32fStencil8Pad24); // depth-stencil

// Face identifiers for TextureCube, None is used for Texture2D
DeclareEnum7(TextureFace, None, PositiveX, PositiveY, PositiveZ, NegativeX, NegativeY, NegativeZ);

/// Block compression, lossy hardware supported formats with very high memory
/// savings None - No compression will be used BC1 - RGB stored at 1/2 byte per
/// pixel
///   Used for color maps that don't need alpha, normal maps
/// BC2 - RGB w/ low precision alpha stored at 1 byte per pixel
///   No common usages
/// BC3 - RGB w/ alpha stored at 1 byte per pixel
///   Used for color maps that need alpha
/// BC4 - R stored at 1/2 byte per pixel
///   Used for single channel maps like height, specular, roughness
/// BC5 - RG stored at 1 byte per pixel
///   Used for two channel maps like normals with reconstructed Z
/// BC6 - RGB floats stored at 1 byte per pixel
///   Used for high dynamic range images
DeclareEnum7(TextureCompression, None, BC1, BC2, BC3, BC4, BC5, BC6);

/// How to address the texture with uv's outside of the range [0, 1]
/// Clamp - Uses the last pixel at the border of the image
/// Repeat - Wraps to the opposite side and continues to sample the image
/// Mirror - Similar to Repeat but reverses image direction
DeclareEnum3(TextureAddressing, Clamp, Repeat, Mirror);

/// How pixels are sampled when viewing image at a different size
/// Nearest - Gets the closest pixel unaltered
/// Bilinear - Gets the 4 closest pixels and linearly blends between them
/// Trilinear - Same as bilinear with an additional linear blend between mip
/// levels
DeclareEnum3(TextureFiltering, Nearest, Bilinear, Trilinear);

/// How pixels are sampled when the ratio of pixels viewed along its u/v
/// directions is not 1:1 (Typically when viewing a texture at an angle) The
/// options represent how large of a ratio will be accounted for when sampling
/// x1 = 1:1 (no anisotropy), x16 = 16:1 (high anisotropy), x16 being the best
/// quality
DeclareEnum5(TextureAnisotropy, x1, x2, x4, x8, x16);

/// Progressively scaled down versions of the image are produced
/// to preserve image integrity when viewed at smaller scales
/// None - No mipmaps are generated
/// PreGenerated - Mipmaps are generated by the engine
///   Uses higher quality filtering than the gpu
///   Required for cubemaps in order to get perspective correct filtering over
///   face edges
/// GpuGenerated - Mipmaps are generated by the gpu at load time
DeclareEnum3(TextureMipMapping, None, PreGenerated, GpuGenerated);

DeclareEnum4(NineSlices, Left, Top, Right, Bottom);

class MipHeader
{
public:
  uint mFace;
  uint mLevel;
  uint mWidth;
  uint mHeight;
  uint mDataOffset;
  uint mDataSize;
};

/// Used when requesting a RenderTarget to configure how its texture is sampled.
class SamplerSettings
{
public:
  SamplerSettings();

  /// How to treat uv coordinates outside of [0, 1] along the Texture's width.
  TextureAddressing::Enum mAddressingX;
  /// How to treat uv coordinates outside of [0, 1] along the Texture's height.
  TextureAddressing::Enum mAddressingY;
  /// How samples should be blended under minification/magnification.
  TextureFiltering::Enum mFiltering;
  /// If sampling in hardware should perform comparison instead of fetching.
  /// Requires using SamplerShadow2d in the shader.
  TextureCompareMode::Enum mCompareMode;
  /// Which method of comparison should be used if CompareMode is set to Enable.
  TextureCompareFunc::Enum mCompareFunc;

  // Internal

  // Converts all settings on current object to one compact integer
  u32 GetSettings();

  // Compacts values that can be binary or'd together, used for hashing
  // Converted values use an extra bit so that unused values can be detected
  static u32 AddressingX(TextureAddressing::Enum addressingX);
  static u32 AddressingY(TextureAddressing::Enum addressingY);
  static u32 Filtering(TextureFiltering::Enum filtering);
  static u32 CompareMode(TextureCompareMode::Enum compareMode);
  static u32 CompareFunc(TextureCompareFunc::Enum compareFunc);

  // Retrieves individual enum values from the compacted integer
  // Results in 0 if the requested value was never set
  static TextureAddressing::Enum AddressingX(u32 samplerSettings);
  static TextureAddressing::Enum AddressingY(u32 samplerSettings);
  static TextureFiltering::Enum Filtering(u32 samplerSettings);
  static TextureCompareMode::Enum CompareMode(u32 samplerSettings);
  static TextureCompareFunc::Enum CompareFunc(u32 samplerSettings);

  // Adds a converted value only if it hasn't been set already
  static void AddValue(u32& samplerSettings, u32 value);
  // Uses values from defaults to fill any unset values
  static void FillDefaults(u32& samplerSettings, u32 defaultSettings);
};

uint GetPixelSize(TextureFormat::Enum format);

void SetPixelData(byte* data, uint index, Vec4 value, TextureFormat::Enum format);
void ReadPixelData(byte* data, uint index, Vec4& value, TextureFormat::Enum format);

void SetPixelDataByte(byte* data, uint index, Vec4 value, uint elementCount);
void SetPixelDataShort(byte* data, uint index, Vec4 value, uint elementCount);
void SetPixelDataHalfFloat(byte* data, uint index, Vec4 value, uint elementCount);
void SetPixelDataFloat(byte* data, uint index, Vec4 value, uint elementCount);
void SetPixelDataGamma(byte* data, uint index, Vec4 value, uint elementCount);

void ReadPixelDataByte(byte* data, uint index, Vec4& value, uint elementCount);
void ReadPixelDataShort(byte* data, uint index, Vec4& value, uint elementCount);
void ReadPixelDataHalfFloat(byte* data, uint index, Vec4& value, uint elementCount);
void ReadPixelDataFloat(byte* data, uint index, Vec4& value, uint elementCount);
void ReadPixelDataGamma(byte* data, uint index, Vec4& value, uint elementCount);

bool IsColorFormat(TextureFormat::Enum format);
bool IsShortColorFormat(TextureFormat::Enum format);
bool IsFloatColorFormat(TextureFormat::Enum format);
bool IsDepthFormat(TextureFormat::Enum format);
bool IsDepthStencilFormat(TextureFormat::Enum format);

void YInvertNonCompressed(byte* imageData, uint width, uint height, uint pixelSize);
void YInvertBlockCompressed(
    byte* imageData, uint width, uint height, uint dataSize, TextureCompression::Enum compression);

void BuildOrthographicTransformZero(
    Mat4& matrix, float verticalSize, float aspectRatio, float nearDistance, float farDistance);
void BuildOrthographicTransformGl(
    Mat4& matrix, float verticalSize, float aspectRatio, float nearDistance, float farDistance);
void BuildOrthographicTransformDx(
    Mat4& matrix, float verticalSize, float aspectRatio, float nearDistance, float farDistance);

void BuildPerspectiveTransformZero(
    Mat4& matrix, float verticalFov, float aspectRatio, float nearDistance, float farDistance);
void BuildPerspectiveTransformGl(
    Mat4& matrix, float verticalFov, float aspectRatio, float nearDistance, float farDistance);
void BuildPerspectiveTransformDx(
    Mat4& matrix, float verticalFov, float aspectRatio, float nearDistance, float farDistance);

class Shader
{
public:
  String mName;
  String mCoreVertex;
  String mComposite;
  String mRenderPass;
  bool mSentToRenderer = false;
};

class ShaderEntry
{
public:
  ShaderEntry()
  {
  }
  ShaderEntry(Shader* shader) :
      mCoreVertex(shader->mCoreVertex),
      mComposite(shader->mComposite),
      mRenderPass(shader->mRenderPass)
  {
  }

  String mCoreVertex;
  String mComposite;
  String mRenderPass;
  // Used for shader names in the library for lookup first,
  // and then they are the translated shaders' source for the renderer
  String mVertexShader;
  String mGeometryShader;
  String mPixelShader;
};

class ShaderInput
{
public:
  ShaderInput() : mShaderInputType(ShaderInputType::Invalid), mSamplerSettings(0)
  {
  }

  ShaderInputType::Enum mShaderInputType;
  String mTranslatedInputName;
  u32 mSamplerSettings;

  static const size_t MaxSize = sizeof(Mat4);
  byte mValue[MaxSize];
};

typedef Pair<String, String> StringPair;
typedef Pair<String, StringPair> ShaderKey;

class AddMaterialInfo
{
public:
  MaterialRenderData* mRenderData;
  String mCompositeName;
  u64 mMaterialId;
};

class AddMeshInfo
{
public:
  MeshRenderData* mRenderData;
  uint mVertexSize;
  uint mVertexCount;
  byte* mVertexData;
  uint mIndexSize;
  uint mIndexCount;
  byte* mIndexData;
  Array<VertexAttribute> mVertexAttributes;
  PrimitiveType::Enum mPrimitiveType;
  Array<MeshBone> mBones;
};

class AddTextureInfo
{
public:
  TextureRenderData* mRenderData;
  uint mWidth;
  uint mHeight;
  uint mMipCount;
  uint mTotalDataSize;
  MipHeader* mMipHeaders;
  byte* mImageData;

  TextureType::Enum mType;
  TextureFormat::Enum mFormat;
  TextureCompression::Enum mCompression;
  TextureAddressing::Enum mAddressingX;
  TextureAddressing::Enum mAddressingY;
  TextureFiltering::Enum mFiltering;
  TextureCompareMode::Enum mCompareMode;
  TextureCompareFunc::Enum mCompareFunc;
  TextureAnisotropy::Enum mAnisotropy;
  TextureMipMapping::Enum mMipMapping;
  uint mMaxMipOverride;

  bool mSubImage;
  uint mXOffset;
  uint mYOffset;
};

class GetTextureDataInfo
{
public:
  TextureRenderData* mRenderData;
  TextureFormat::Enum mFormat;
  uint mWidth;
  uint mHeight;
  byte* mImage;
};

class ShowProgressInfo
{
public:
  ShowProgressInfo();

  TextureRenderData* mLoadingTexture;
  TextureRenderData* mLogoTexture;
  TextureRenderData* mWhiteTexture;
  TextureRenderData* mSplashTexture;
  uint mLogoFrameSize;
  float mCurrentPercent;
  float mTargetPercent;
  uint mProgressWidth;
  TextureRenderData* mFontTexture;
  Array<StreamedVertex> mProgressText;
  bool mSplashMode;
  float mSplashFade;
  Timer mTimer;
  Timer mPerJobTimer;
};

class Renderer
{
public:
  Renderer();
  virtual ~Renderer();

  // should move these to a file for api dependent utility functions
  virtual void
  BuildOrthographicTransform(Mat4Ref matrix, float size, float aspect, float nearPlane, float farPlane) = 0;
  virtual void BuildPerspectiveTransform(Mat4Ref matrix, float fov, float aspect, float nearPlane, float farPlane) = 0;
  virtual bool YInvertImageData(TextureType::Enum type)
  {
    return false;
  }

  // Called by main thread
  virtual MaterialRenderData* CreateMaterialRenderData() = 0;
  virtual MeshRenderData* CreateMeshRenderData() = 0;
  virtual TextureRenderData* CreateTextureRenderData() = 0;

  virtual void AddMaterial(AddMaterialInfo* info) = 0;
  virtual void AddMesh(AddMeshInfo* info) = 0;
  virtual void AddTexture(AddTextureInfo* info) = 0;
  virtual void RemoveMaterial(MaterialRenderData* data) = 0;
  virtual void RemoveMesh(MeshRenderData* data) = 0;
  virtual void RemoveTexture(TextureRenderData* data) = 0;

  virtual bool GetLazyShaderCompilation() = 0;
  virtual void SetLazyShaderCompilation(bool isLazy) = 0;
  virtual void AddShaders(Array<ShaderEntry>& entries, uint forceCompileBatchCount) = 0;
  virtual void RemoveShaders(Array<ShaderEntry>& entries) = 0;

  virtual void SetVSync(bool vsync) = 0;

  virtual void GetTextureData(GetTextureDataInfo* info) = 0;

  virtual void ShowProgress(ShowProgressInfo* info)
  {
  }

  virtual void DoRenderTasks(RenderTasks* renderTasks, RenderQueues* renderQueues) = 0;

  GraphicsDriverSupport mDriverSupport;

  // Thread lock for the main thread to set any critical control flags.
  SpinLock mThreadLock;
  // Intel crashes when blitting to back buffer when window is minimized with
  // certain window style flags set. Flag is set to false when the window is
  // minimized.
  bool mBackBufferSafe;
};

class HandleIdInfo
{
public:
  u64 mId;
};

class ReferenceCountInfo
{
public:
  int mCount;
};

/// Settings for how pixel shader outputs are combined with the RenderTarget's
/// current values.
class BlendSettings
{
public:
  BlendSettings();
  BlendSettings(const BlendSettings& other);
  ~BlendSettings();
  BlendSettings& operator=(const BlendSettings& rhs);

  // Only used for safe handling of this type
  HandleIdInfo mHandleId;
  ReferenceCountInfo mReferenceCount;

  void SetBlendAlpha();
  void SetBlendAdditive();

  // TODO: Macro comments for auto-doc
  /// If blend equations should be applied to output.
  DeclareByteEnumGetSet(BlendMode::Enum, BlendMode);
  /// How source and destination values should be combined.
  DeclareByteEnumGetSet(BlendEquation::Enum, BlendEquation);
  /// What source value should be multiplied with before combined.
  DeclareByteEnumGetSet(BlendFactor::Enum, SourceFactor);
  /// What destination value should be multiplied with before combined.
  DeclareByteEnumGetSet(BlendFactor::Enum, DestFactor);

  // Separable color/alpha settings
  /// How source and destination values should be combined, for alpha channel if
  /// in separate mode.
  DeclareByteEnumGetSet(BlendEquation::Enum, BlendEquationAlpha);
  /// What source value should be multiplied with before combined, for alpha
  /// channel if in separate mode.
  DeclareByteEnumGetSet(BlendFactor::Enum, SourceFactorAlpha);
  /// What destination value should be multiplied with before combined, for
  /// alpha channel if in separate mode.
  DeclareByteEnumGetSet(BlendFactor::Enum, DestFactorAlpha);

  static void (*Constructed)(BlendSettings*);
  static void (*Destructed)(BlendSettings*);
};

/// Settings for how the depth buffer should control pixel output.
class DepthSettings
{
public:
  DepthSettings();
  DepthSettings(const DepthSettings& other);
  ~DepthSettings();
  DepthSettings& operator=(const DepthSettings& rhs);

  // Only used for safe handling of this type
  HandleIdInfo mHandleId;
  ReferenceCountInfo mReferenceCount;

  void SetDepthRead(TextureCompareFunc::Enum depthCompareFunc);
  void SetDepthWrite(TextureCompareFunc::Enum depthCompareFunc);

  void SetStencilTestMode(TextureCompareFunc::Enum stencilCompareFunc);
  void SetStencilIncrement();
  void SetStencilDecrement();

  // TODO: Macro comments for auto-doc
  // Depth settings
  /// If pixel depth should pass comparison to a depth buffer in order to
  /// output. And if value should be written to the depth buffer when comparison
  /// passes.
  DeclareByteEnumGetSet(DepthMode::Enum, DepthMode);
  /// Comparison function for depth test.
  DeclareByteEnumGetSet(TextureCompareFunc::Enum, DepthCompareFunc);

  // Stencil settings
  /// If pixels should also pass a comparison with the stencil buffer in order
  /// to output.
  DeclareByteEnumGetSet(StencilMode::Enum, StencilMode);
  /// Comparison function for stencil test.
  DeclareByteEnumGetSet(TextureCompareFunc::Enum, StencilCompareFunc);
  /// Operation to perform on stencil value if stencil test fails.
  DeclareByteEnumGetSet(StencilOp::Enum, StencilFailOp);
  /// Operation to perform on stencil value if stencil test passes but depth
  /// test fails.
  DeclareByteEnumGetSet(StencilOp::Enum, DepthFailOp);
  /// Operation to perform on stencil value if both stencil and depth tests
  /// pass.
  DeclareByteEnumGetSet(StencilOp::Enum, DepthPassOp);
  /// Bit mask for buffer value and test value when being compared.
  byte mStencilReadMask;
  /// Bit mask for which bits in the buffer can be modified.
  byte mStencilWriteMask;
  /// Value that will be used to compare against the stencil buffer for all
  /// pixels.
  byte mStencilTestValue;

  // If separable front/back face settings are desired
  /// Comparison function for stencil test, for triangle back faces if in
  /// separate mode.
  DeclareByteEnumGetSet(TextureCompareFunc::Enum, StencilCompareFuncBackFace);
  /// Operation to perform on stencil value if stencil test fails, for triangle
  /// back faces if in separate mode.
  DeclareByteEnumGetSet(StencilOp::Enum, StencilFailOpBackFace);
  /// Operation to perform on stencil value if stencil test passes but depth
  /// test fails, for triangle back faces if in separate mode.
  DeclareByteEnumGetSet(StencilOp::Enum, DepthFailOpBackFace);
  /// Operation to perform on stencil value if both stencil and depth tests
  /// pass, for triangle back faces if in separate mode.
  DeclareByteEnumGetSet(StencilOp::Enum, DepthPassOpBackFace);
  /// Bit mask for buffer value and test value when being compared, for triangle
  /// back faces if in separate mode.
  byte mStencilReadMaskBackFace;
  /// Bit mask for which bits in the buffer can be modified, for triangle back
  /// faces if in separate mode.
  byte mStencilWriteMaskBackFace;
  /// Value that will be used to compare against the stencil buffer for all
  /// pixels, for triangle back faces if in separate mode.
  byte mStencilTestValueBackFace;

  static void (*Constructed)(DepthSettings*);
  static void (*Destructed)(DepthSettings*);
};

/// Contains all output targets and render settings needed for a render task.
class RenderSettings
{
public:
  RenderSettings();

  /// Settings to use when blending shader output with the ColorTarget,
  /// implicitly BlendSettings0.
  BlendSettings* GetBlendSettings();
  void SetBlendSettings(BlendSettings* blendSettings);

  /// Settings to use when doing depth/stencil testing with DepthTarget.
  DepthSettings* GetDepthSettings();
  void SetDepthSettings(DepthSettings* depthSettings);

  // Clears all targets and settings.
  void ClearAll();
  // Clears all set targets.
  void ClearTargets();
  // Clears all blend and depth settings.
  void ClearSettings();

  uint mTargetsWidth;
  uint mTargetsHeight;

  // Render data pointers.
  TextureRenderData* mColorTargets[8];
  TextureRenderData* mDepthTarget;

  // Texture pointers needed for validation and update check.
  void* mColorTextures[8];
  void* mDepthTexture;

  BlendSettings mBlendSettings[8];
  DepthSettings mDepthSettings;

  bool mSingleColorTarget;

  // TODO: Macro comments for auto-doc
  /// If a certain side of triangles should not be rendered. Front faces defined
  /// by counter-clockwise vertex winding.
  DeclareByteEnumGetSet(CullMode::Enum, CullMode);

  // Not exposed, only for old ui.
  ByteEnum<ScissorMode::Enum> mScissorMode;
};

class FrameNode;
class ViewNode;
class FrameBlock;
class ViewBlock;

class FrameNode
{
public:
  // Use for Extract only
  void* mGraphicalEntry;

  RenderingType::Enum mRenderingType;
  CoreVertexType::Enum mCoreVertexType;

  // Render data
  MaterialRenderData* mMaterialRenderData;
  MeshRenderData* mMeshRenderData;
  TextureRenderData* mTextureRenderData;

  // View independent transforms
  Mat4 mLocalToWorld;
  Mat3 mLocalToWorldNormal;

  Vec3 mObjectWorldPosition;

  IndexRange mBoneMatrixRange;
  IndexRange mIndexRemapRange;
  IndexRange mShaderInputRange;

  // temporary, needed by ui
  Vec4 mClip;
  // temporary, needed by gizmo debug draw
  float mBorderThickness;
  // temporary, needed for viewport blending
  uint mBlendSettingsIndex;
  bool mBlendSettingsOverride;
};

class ViewNode
{
public:
  // Use for Extract only
  void* mGraphicalEntry;

  // Index in the FrameBlock for this object's FrameNode.
  int mFrameNodeIndex;
  // Id used to map unique render settings for sub groups.
  int mRenderGroupId;

  // View dependent transforms
  Mat4 mLocalToView;
  Mat3 mLocalToViewNormal;
  Mat4 mLocalToPerspective;

  PrimitiveType::Enum mStreamedVertexType;
  uint mStreamedVertexStart;
  uint mStreamedVertexCount;
};

class FrameBlock
{
public:
  Array<FrameNode> mFrameNodes;
  RenderQueues* mRenderQueues;

  // Space data
  float mFrameTime;
  float mLogicTime;
};

class ViewBlock
{
public:
  Array<ViewNode> mViewNodes;
  Array<IndexRange> mRenderGroupRanges;

  // View transforms
  Mat4 mWorldToView;
  Mat4 mViewToPerspective;
  Mat4 mZeroPerspectiveToApiPerspective;

  float mNearPlane;
  float mFarPlane;
  Vec2 mViewportSize;
  Vec2 mInverseViewportSize;

  // Used by DebugGraphical
  Vec3 mEyePosition;
  Vec3 mEyeDirection;
  Vec3 mEyeUp;
  float mFieldOfView;
  float mOrthographicSize;
  bool mOrthographic;

  // For Graphicals to track Camera specific data
  u64 mCameraId;
};

class RenderQueues
{
public:
  void Clear();

  void AddStreamedLineRect(ViewNode& viewNode,
                           Vec3 pos0,
                           Vec3 pos1,
                           Vec2 uv0,
                           Vec2 uv1,
                           Vec4 color,
                           Vec2 uvAux0 = Vec2(0, 0),
                           Vec2 uvAux1 = Vec2(1, 1));
  void AddStreamedQuad(ViewNode& viewNode,
                       Vec3 pos0,
                       Vec3 pos1,
                       Vec2 uv0,
                       Vec2 uv1,
                       Vec4 color,
                       Vec2 uvAux0 = Vec2(0, 0),
                       Vec2 uvAux1 = Vec2(1, 1));
  void AddStreamedQuadTiled(ViewNode& viewNode,
                            Vec3 pos0,
                            Vec3 pos1,
                            Vec2 uv0,
                            Vec2 uv1,
                            Vec4 color,
                            Vec2 tileSize,
                            Vec2 uvAux0 = Vec2(0, 0),
                            Vec2 uvAux1 = Vec2(1, 1));
  void AddStreamedQuadNineSliced(ViewNode& viewNode,
                                 Vec3 pos0,
                                 Vec3 pos1,
                                 Vec2 uv0,
                                 Vec2 uv1,
                                 Vec4 color,
                                 Vec4 posSlices,
                                 Vec4 uvSlices,
                                 Vec2 uvAux0 = Vec2(0, 0),
                                 Vec2 uvAux1 = Vec2(1, 1));

  void AddStreamedQuadView(ViewNode& viewNode, Vec3 pos[4], Vec2 uv0, Vec2 uv1, Vec4 color);

  Array<FrameBlock> mFrameBlocks;
  Array<ViewBlock> mViewBlocks;
  StreamedVertexArray mStreamedVertices;

  uint mSkinningBufferVersion;
  Array<Mat4> mSkinningBuffer;
  Array<uint> mIndexRemapBuffer;

  // temporary, needed for viewport blending
  Array<BlendSettings> mBlendSettingsOverrides;

  /// This was for the old Ui system to add custom shader inputs. It's only set
  /// when rendering widgets.
  RenderTasks* mRenderTasks;
};

class ScreenViewport
{
public:
  int x, y, width, height;
};

class RenderTask
{
public:
  // We use a large type to ensure alignment on all platforms.
  union {
    u64 mId;
    MaxAlignmentType mAligned;
  };
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
  // Name of the RenderPass fragment for shader lookups. Inputs are mapped when
  // creating this task.
  String mRenderPassName;
  // Id used to lookup all shader input data for the RenderPass fragment and all
  // Materials that will be used in this render task.
  uint mShaderInputsId;
  // If not zero, this is the number of contiguous RenderTaskRenderPass objects
  // in memory after this one. The renderer must account for this.
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

static const u64 cFragmentShaderInputsId = 0;
static const u64 cGlobalShaderInputsId = 1;
static const u64 cGraphicalShaderInputsId = 2;

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

} // namespace Zero
