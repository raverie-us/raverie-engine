#pragma once

namespace Zero
{

// A bone object in the skeleton hierarchy
class BoneTransform
{
public:
  Cog* Object;
  Transform* Transform;
  uint SkeletonIndex;
};

class GraphicsRayCast
{
public:
  Ray mRay;
  Cog* mObject;
  Vec3 mPosition;
  Vec3 mNormal;
  Vec2 mUv;
  float mT;
};

class IndexRange
{
public:
  IndexRange() {}
  IndexRange(uint s, uint e) : start(s), end(e) {}

  uint Count() {return end - start;}

  uint start, end;
};

class UniqueComposite
{
public:
  String mName;
  Array<String> mFragmentNames;
  HashSet<String> mFragmentNameMap;
  uint mReferences;
};

class Shader
{
public:
  String mName;
  String mCoreVertex;
  String mComposite;
  String mRenderPass;
  ZilchShaderLibraryRef mLibrary;

  static Memory::Pool* sPool;
};

class ShaderEntry
{
public:
  ShaderEntry() {}
  ShaderEntry(Shader* shader)
    : mCoreVertex(shader->mCoreVertex), mComposite(shader->mComposite), mRenderPass(shader->mRenderPass) {}

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
  ShaderInput() : mShaderInputType(ShaderInputType::Invalid), mSamplerSettings(0) {}

  void SetValue(AnyParam value);

  ShaderInputType::Enum mShaderInputType;
  String mTranslatedInputName;
  u32 mSamplerSettings;

  static const size_t MaxSize = sizeof(Mat4);
  byte mValue[MaxSize];
};

class UvRect
{
public:
  Vec2 TopLeft;
  Vec2 BotRight;
  Vec2 GetTopRight() { return Vec2(BotRight.x, TopLeft.y); }
  Vec2 GetBotLeft() { return Vec2(TopLeft.x, BotRight.y); }
};

typedef HashSet<Shader*> ShaderSet;
typedef HashMap<String, Shader*> ShaderMap;
typedef HashMap<String, ShaderSet> ShaderSetMap;

} // namespace Zero
