// MIT Licensed (see LICENSE.md).

#pragma once

namespace Raverie
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

class UniqueComposite
{
public:
  String mName;
  Array<String> mFragmentNames;
  HashSet<String> mFragmentNameMap;
  uint mReferences;
};

class UvRect
{
public:
  Vec2 TopLeft;
  Vec2 BotRight;
  Vec2 GetTopRight()
  {
    return Vec2(BotRight.x, TopLeft.y);
  }
  Vec2 GetBotLeft()
  {
    return Vec2(TopLeft.x, BotRight.y);
  }
};

struct ShaderPolicy
{
public:
  typedef ShaderPolicy this_type;

  inline bool Equal(const Shader* lhs, const Shader* rhs) const
  {
    return lhs->mName == rhs->mName;
  }

  inline size_t operator()(const Shader* value) const
  {
    return value->mName.Hash();
  }
};

typedef HashSet<Shader*, ShaderPolicy> ShaderSet;
typedef HashMap<String, Shader*> ShaderMap;
typedef HashMap<String, ShaderSet> ShaderSetMap;

void ShaderInputSetValue(ShaderInput& input, AnyParam value);

} // namespace Raverie
