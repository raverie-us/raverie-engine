// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

class EmptyRenderer : public Renderer
{
public:
  EmptyRenderer()
  {
  }
  ~EmptyRenderer() override
  {
  }

  void BuildOrthographicTransform(Mat4Ref matrix, float size, float aspect, float nearPlane, float farPlane) override
  {
  }
  void BuildPerspectiveTransform(Mat4Ref matrix, float fov, float aspect, float nearPlane, float farPlane) override
  {
  }
  bool YInvertImageData(TextureType::Enum type) override
  {
    return false;
  }

  MaterialRenderData* CreateMaterialRenderData() override
  {
    return nullptr;
  }
  MeshRenderData* CreateMeshRenderData() override
  {
    return nullptr;
  }
  TextureRenderData* CreateTextureRenderData() override
  {
    return nullptr;
  }

  void AddMaterial(AddMaterialInfo* info) override
  {
  }
  void AddMesh(AddMeshInfo* info) override
  {
  }
  void AddTexture(AddTextureInfo* info) override
  {
  }
  void RemoveMaterial(MaterialRenderData* data) override
  {
  }
  void RemoveMesh(MeshRenderData* data) override
  {
  }
  void RemoveTexture(TextureRenderData* data) override
  {
  }

  bool GetLazyShaderCompilation() override
  {
    return false;
  }
  void SetLazyShaderCompilation(bool isLazy) override
  {
  }
  void AddShaders(Array<ShaderEntry>& entries, uint forceCompileBatchCount) override
  {
  }
  void RemoveShaders(Array<ShaderEntry>& entries) override
  {
  }

  void SetVSync(bool vsync) override
  {
  }

  void GetTextureData(GetTextureDataInfo* info) override
  {
  }

  void ShowProgress(ShowProgressInfo* info) override
  {
  }

  void DoRenderTasks(RenderTasks* renderTasks, RenderQueues* renderQueues) override
  {
  }
};

Renderer* CreateRenderer(OsHandle windowHandle, String& error)
{
  return new EmptyRenderer();
}

} // namespace Zero
