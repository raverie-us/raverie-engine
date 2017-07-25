#pragma once

namespace Zero
{

// Implemented by api specific renderer
void CreateRenderer(OsHandle windowHandle, String& error);
void DestroyRenderer();

// Base types for renderer to implement resource render data
class MaterialRenderData {};
class MeshRenderData {};
class TextureRenderData {};

/// Information about the active graphics hardware.
class GraphicsDriverSupport
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  GraphicsDriverSupport();

  // Must be filled out by the renderer in initialization.
  /// If block compression formats can be sampled by hardware.
  bool mTextureCompression;
  /// If blend settings for the output merger stage can be independently set for multiple render targets.
  bool mMultiTargetBlend;
  /// If texture sampler settings can be uniquely specified per sampler shader input.
  bool mSamplerObjects;
};

class Renderer
{
public:
  Renderer() {}
  virtual ~Renderer() {}

  // should move these to a file for api dependent utility functions
  virtual void BuildOrthographicTransform(Mat4Ref matrix, float size, float aspect, float near, float far) = 0;
  virtual void BuildPerspectiveTransform(Mat4Ref matrix, float fov, float aspect, float near, float far) = 0;
  virtual bool YInvertImageData(TextureType::Enum type) {return false;}

  // Called by main thread
  virtual void CreateRenderData(Material* material) = 0;
  virtual void CreateRenderData(Mesh* mesh) = 0;
  virtual void CreateRenderData(Texture* texture) = 0;

  virtual void AddMaterial(AddMaterialJob* job) = 0;
  virtual void AddMesh(AddMeshJob* job) = 0;
  virtual void AddTexture(AddTextureJob* job) = 0;
  virtual void RemoveMaterial(RemoveMaterialJob* job) = 0;
  virtual void RemoveMesh(RemoveMeshJob* job) = 0;
  virtual void RemoveTexture(RemoveTextureJob* job) = 0;

  virtual void AddShaders(AddShadersJob* job) = 0;
  virtual void RemoveShaders(RemoveShadersJob* job) = 0;

  virtual void SetVSync(SetVSyncJob* job) = 0;

  virtual void GetTextureData(GetTextureDataJob* job) = 0;

  virtual void ShowProgress(ShowProgressJob* job) {}

  virtual void DoRenderTasks(RenderTasks* renderTasks, RenderQueues* renderQueues) = 0;

  GraphicsDriverSupport mDriverSupport;
};

namespace Z
{
  extern Renderer* gRenderer;
}

} // namespace Zero
