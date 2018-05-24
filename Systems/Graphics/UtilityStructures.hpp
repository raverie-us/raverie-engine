// Authors: Nathan Carlson
// Copyright 2015, DigiPen Institute of Technology

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
  Vec2 GetTopRight() { return Vec2(BotRight.x, TopLeft.y); }
  Vec2 GetBotLeft() { return Vec2(TopLeft.x, BotRight.y); }
};

typedef HashSet<Shader*> ShaderSet;
typedef HashMap<String, Shader*> ShaderMap;
typedef HashMap<String, ShaderSet> ShaderSetMap;

void ShaderInputSetValue(ShaderInput& input, AnyParam value);

} // namespace Zero
