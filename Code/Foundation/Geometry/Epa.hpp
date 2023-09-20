// MIT Licensed (see LICENSE.md).
#pragma once

namespace Intersection
{

class Epa
{
public:
  void Init(const Simplex& simplex);
  Vec3 GetClosestFaceNormal(void);
  float GetClosestDistance(void);
  void GetClosestFace(CSOVertex* retFace);
  bool Expand(CSOVertex newPoint);
  void DebugPoint(CSOVertex debugPoint);
  bool DebugStep(void);
  void DrawDebug(void);

private:
  struct Edge
  {
    Edge(void)
    {
    }
    Edge(unsigned i0, unsigned i1) : p0(i0), p1(i1)
    {
    }
    bool operator==(const Edge& other) const
    {
      if (p0 == other.p0 && p1 == other.p1)
        return true;
      if (p0 == other.p1 && p1 == other.p0)
        return true;
      return false;
    }
    unsigned p0, p1;
  };

  struct Face
  {
    Face(void)
    {
    }
    Face(unsigned i0, unsigned i1, unsigned i2) : p0(i0), p1(i1), p2(i2)
    {
    }
    unsigned p0, p1, p2;
    Vec3 normal;
  };

  static const size_t InvalidIndex = Raverie::Array<Edge>::InvalidIndex;

  void AddEdge(unsigned p0, unsigned p1);

  Raverie::Array<CSOVertex> mVertices;
  Raverie::Array<Edge> mEdges;
  Raverie::Array<Face> mFaces;
  unsigned mIndexClosest;
  float mDistClosest;

  Raverie::Array<unsigned> mVisibleFaces;
  CSOVertex mDebugPoint;
  unsigned mStep;
};

} // namespace Intersection
