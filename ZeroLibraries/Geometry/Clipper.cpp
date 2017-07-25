///////////////////////////////////////////////////////////////////////////////
///
/// \file Clipper.cpp
/// Implementation of polygon clipper.
/// 
/// Authors: Killian Koenig
/// Copyright 2013, DigiPen Institute of Technology
///
//////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

using Csg::Internal::Simplex;
using Csg::Internal::Vec2_t;
using Csg::Internal::float_t;
using Csg::Internal::LinkedVertex;

namespace 
{

// http://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
bool EqualsApprox(double A, double B) 
{
  double diff = std::abs(A - B);
  A = std::abs(A);
  B = std::abs(B);

  double largest = (B > A) ? B : A;
  return diff <= largest * DBL_EPSILON;
  
}

bool EqualsApprox(f32 A, f32 B) 
{
  return Math::Equal(A, B);
}

const Vec2_t operator+(const Vec2_t& a, const Vec2_t& b)
{
  return Vec2_t(a.x + b.x, a.y + b.y);
}

const Vec2_t operator*(const Vec2_t& v, float_t s)
{
  return Vec2_t(v.x * s, v.y * s);
}

const Vec2_t operator-(const Vec2_t& a, const Vec2_t& b)
{
  return Vec2_t(a.x - b.x, a.y - b.y);
}

float_t Dot(const Vec2_t& a, const Vec2_t& b)
{
  return a.x * b.x + a.y * b.y;
}

float_t Cross(const Vec2_t& a, const Vec2_t& b)
{
  return a.x * b.y - b.x * a.y;
}

void SafeNormalize(Vec2_t* v)
{
  float_t length = std::sqrt(Dot(*v, *v));
  if (length == float_t(0.0))
  {
    v->x = v->y = 0.0;
    return;
  }

  v->x /= length;
  v->y /= length;
}

template <typename T>
bool EqualsApprox(const T& a, const T& b)
{
  return EqualsApprox(a.x, b.x) && EqualsApprox(a.y, b.y);
}

template <typename T>
bool EqualsExact(const T& a, const T& b)
{
  return a.x == b.x && a.y == b.y;
}

bool operator==(const Simplex& a, const Simplex& b)
{
  return EqualsExact(a.p1, b.p1) && 
         EqualsExact(a.p2, b.p2) && 
         a.alpha == b.alpha;
}

// Since edge could have already been divided we need to figure out where
// in the vertex list we need to Insert the new point
void InsertPoint(Memory::Pool* pool,
                 InList<LinkedVertex<Vec2_t> >* vertices,
                 LinkedVertex<Vec2_t>* head,
                 LinkedVertex<Vec2_t>* tail, 
                 const Vec2_t& point)
{
  Vec2_t axis = tail->position - head->position;

  // If the point is not on the given edge do not Insert it
  // This will happen frequently when attempting to Insert touching points
  if(Dot(axis, point - head->position) < float_t(0.0) ||
     Dot(axis, point - tail->position) > float_t(0.0))
  {
    return;
  }

  // Compute the projection of the new vertex onto the edge axis
  float_t pointProj = Dot(axis, point);

  // Search forward along this edge until we find a vertex farther than the 
  // point in the edge direction or the tail vertex of original edge.
  LinkedVertex<Vec2_t>* vertex = vertices->NextWrap(head);
  while(vertex != tail)
  {
    float_t currProj = Dot(axis, vertex->position);

    // If this vertex is further along the edge axis than the new point,
    // we have found the tail vertex
    if (currProj >= pointProj)
    {
      break;
    }

    vertex = vertices->NextWrap(vertex);
  }

  // If a vertex of a shape intersects an edge of another shape
  // we will end up inserting the same point twice
  // Avoid this by comparing against the previous vertex
  if (EqualsExact(vertex->position, point))
  {
    return;
  }

  if (EqualsExact(head->position, point))
  {
    return;
  }

  LinkedVertex<Vec2_t>* newVertex = pool->AllocateType<LinkedVertex<Vec2_t> >();
  newVertex->position = point;
  vertices->InsertBefore(vertex, newVertex);
}

Intersection::IntersectionType TestSegmentSegment(Vec2_t a1, Vec2_t a2,
                                      Vec2_t b1, Vec2_t b2,
                                      Vec2_t* output)
{
  // L1 : a1 + t * v
  // L2 : b1 + s * w
  Vec2_t v = a2 - a1;
  Vec2_t w = b2 - b1;

  Vec2_t vn = v;
  SafeNormalize(&vn);

  Vec2_t wn = w;
  SafeNormalize(&wn);

  float_t dn = Cross(wn, vn);
  float_t d = Cross(w, v);

  // Either the segments are parallel, or one or both are degenerate
  // TODO -- Handle cases for degenerate segments
  if(std::abs(d) < std::numeric_limits<float_t>::min())
  { 
    float_t dotv = Dot(v, v);
    float_t dotw = Dot(w, w);

    float_t cEpsilon = std::numeric_limits<float_t>::epsilon();

    // If either segment is degenerate
    if (dotv < cEpsilon || dotw < cEpsilon)
    {
      return Intersection::None;
    }

    // If segments are not collinear
    Vec2_t delta = a1 - b2;
    if(std::abs(Cross(delta, v)) > cEpsilon)
    {
      return Intersection::None;
    }

    Vec2_t axis = v;
    SafeNormalize(&axis);

    float_t amin = Dot(a1, axis);
    float_t amax = Dot(a2, axis);

    if(amin > amax) Math::Swap(amin, amax);

    float_t bmin = Dot(b1, axis);
    float_t bmax = Dot(b2, axis);

    if(bmin > bmax) Math::Swap(bmin, bmax);

    if(amin > bmax || bmin > amax)
    {
      return Intersection::None;
    }

    return Intersection::Line;
  }

  // Intersection
  // a1 + t * v = b1 + s * w
  // Cross(w, a1 + t * v) = Cross(w, b1)
  // Cross(w, a1) + t * Cross(w, v) = Cross(w, b1)
  // t = (Cross(w, b1) - Cross(w, a1)) / Cross(w, v)
  // t = Cross(w, b1 - a1) / Cross(w, v)


  // Verify t lies on the segment [0, 1]
  float_t t = Cross(w, b1 - a1) / d;


  if(t < float_t(0.0) || t > float_t(1.0))
  {
    return Intersection::None;
  }

  // Verify s lies on the segment [0, 1]
  float_t s = Cross(v, b1 - a1) / d;

  if (s < float_t(0.0) || s > float_t(1.0))
  {
    return Intersection::None;
  }

  if (EqualsApprox(t + 1.0, 1.0))
  {
    *output = a1;
  }
  else if(EqualsApprox(s + 1.0, 1.0))
  {
    *output = b1;
  }
  else if (EqualsApprox(t, float_t(1.0)))
  {
    *output = a2;
  }
  else if (EqualsApprox(s, float_t(1.0)))
  {
    *output = b2;
  }
  else
  {
    *output = a1 + v * t;
  }

  return Intersection::Point;
}

// Test if a point lies on a line segment.
bool PointSegment(const Vec2_t& p, const Vec2_t& a, const Vec2_t& b)
{
  // Line is degenerate
  if(EqualsExact(a, b))
  {
    return EqualsExact(p, a);
  }

  // Verify that p is one the infinite line
  if(Cross(a - p, b - p) != float_t(0.0))
  {
    return false;
  }

  // Verify that p actually lies on the segment
  Vec2_t v = b - a;
  return Dot(v, p - b) <= float_t(0.0) && Dot(v, p - a) >= (0.0);
}

bool IsSameSign(float_t a, float_t b)
{
  return (a >= 0.0) == (b >= 0.0);
}

// Modified version of point in triangle test from Real-Time-Collision-Detection
bool Contains(const Simplex& simplex, const Vec2_t& p)
{
  Vec2_t b = simplex.p1;
  Vec2_t c = simplex.p2;

  float_t pab = Cross(p, b);
  float_t pbc = Cross(p - b, c - b);

  if(!IsSameSign(pab, pbc))
  {
    return false;
  }

  float_t pca = Cross(p - c, Vec2_t(0.0, 0.0) - c);

  if(!IsSameSign(pab, pca)) 
  {
    return false;
  }

   return true;
}

f32 ComputeEdgeValue(Csg::Operation::Type operation, 
                     const Simplex& simplex, 
                     const Array<Simplex>& chain)
{
  // Magic constants for Intersect, Union, Subtract
  static f32 operationTable[3][2] = {{ 1.f, 0.f }, {0.f, 1.f}, {1.f, 0.f} };

  f32 desiredA = operationTable[operation][0];
  f32 desiredB = operationTable[operation][1];

  // Check e is a directed edge of the other polygon
  for(size_t i = 0; i < chain.Size(); ++i)
  {
    if(EqualsExact(chain[i].p1, simplex.p1) && 
       EqualsExact(chain[i].p2, simplex.p2))
    {
      return desiredA;
    }
  }
 
  // Check if -e is a directed edge of the other polygon
  for(size_t i = 0; i < chain.Size(); ++i)
  {
    if(EqualsExact(chain[i].p2, simplex.p1) && 
       EqualsExact(chain[i].p1, simplex.p2))
    {
      return desiredB;
    }
  }

  // Compute the characteristic function of the midpoint of e
  Vec2_t q = (simplex.p1 + simplex.p2) * 0.5;

  for(size_t i = 0; i < chain.Size(); ++i)
  {
    const Simplex& edge = chain[i];
    bool inside = PointSegment(q, edge.p1, edge.p2);
    if(inside)
    {
      return 1.f;
    }
  }

  f32 beta = 0.f;
  for(size_t i = 0; i < chain.Size(); ++i)
  {
    const Simplex& chainSimplex = chain[i];

    Vec2_t zero(float_t(0.0), float_t(0.0));
    bool i1 = PointSegment(q, zero, chainSimplex.p1);
    bool i2 = PointSegment(q, zero, chainSimplex.p2);
    if(i1 || i2)
    {
      beta += chainSimplex.alpha * 0.5f;
    }
    else if(Contains(chainSimplex, q))
    {
      beta += chainSimplex.alpha;
    }
  }

  return beta;
}

} // namespace 


namespace Csg
{

using Internal::HighPrecisionContourArray;

namespace Internal
{

void Clean(ContourArray* contours,
           f32 angleTolerance,
           f32 distanceTolerance)
{
  // Remove duplicate / collinear points
  for(size_t i = 0; i < contours->Size(); ++i)
  {
    Array<Vec2>& contour = (*contours)[i];
    for(size_t j = 0; contour.Size() >= 3 && j < contour.Size(); )
    {
      size_t prevIndex = j ? j - 1 : contour.Size() - 1;
      size_t currIndex = j;
      size_t nextIndex = (j + 1) % contour.Size();    
      
      Vec2 prev = contour[prevIndex];
      Vec2 curr = contour[currIndex];
      Vec2 next = contour[nextIndex];

      Vec2 v1 = curr - prev;
      Vec2 v2 = next - prev;

      v1.AttemptNormalize();
      v2.AttemptNormalize();

      f32 sint = Cross(v1, v2);
      f32 distance = Math::Distance(prev, curr);
      if (Math::Abs(sint) < angleTolerance  ||
          distance < distanceTolerance)
      {
        contour.EraseAt(j);
      }
      else
      {
        ++j;
      }
    }    
  }

  for(size_t i = 0; i < contours->Size(); )
  {
    Array<Vec2>& contour = (*contours)[i];
    // If this has caused a degenerate contour, remove it
    if(contour.Size() < 3)
    {
      Zero::Swap(contour, contours->Back());
      contours->PopBack();
    }
    else
    {
      ++i;
    }
  }
}

void Transfer(InList<LinkedVertex<Vec2_t> >& vertices, Array<Vec2_t>* contour, Memory::Pool& pool)
{
  InList<LinkedVertex<Vec2_t> >::range r = vertices.All();
  contour->Clear();
  while(!r.Empty())
  {
    LinkedVertex<Vec2_t>& vertex = r.Front();
    contour->PushBack(vertex.position);
    r.PopFront();
    pool.DeallocateType<LinkedVertex<Vec2_t> >(&vertex);
  }
};

// Compute intersection points between all segments in two polygons
// and Insert them back into the original polygons
void Subdivide(Array<Vec2_t>* polygonA, Array<Vec2_t>* polygonB)
{
  size_t sizeA = polygonA->Size();
  size_t sizeB = polygonB->Size();

  // Use lists to build the new vertex data so we can Insert points efficiently
  InList<LinkedVertex<Vec2_t> > verticesA;
  InList<LinkedVertex<Vec2_t> > verticesB;

  // We need to map original vertices from their index in the pre-divided
  // polygons to their place in the post-divided lists
  Array<LinkedVertex<Vec2_t>*> lookupA;
  Array<LinkedVertex<Vec2_t>*> lookupB;

  size_t blocksPerPage = sizeA + sizeB;
  Memory::Pool pool("Vertex Pool", nullptr, sizeof(LinkedVertex<Vec2_t>), blocksPerPage);

  // Initialize vertex lists and array lookups
  lookupA.Resize(sizeA);
  for(size_t i = 0; i < sizeA; ++i)
  {
    LinkedVertex<Vec2_t>* vertex = pool.AllocateType<LinkedVertex<Vec2_t> >();
    vertex->position = (*polygonA)[i];
    lookupA[i] = vertex;
    verticesA.PushBack(vertex);
  }
  lookupB.Resize(sizeB);
  for(size_t i = 0; i < sizeB; ++i)
  {
    LinkedVertex<Vec2_t>* vertex = pool.AllocateType<LinkedVertex<Vec2_t> >();
    vertex->position = (*polygonB)[i];
    lookupB[i] = vertex;
    verticesB.PushBack(vertex);
  }

  // Compute all intersections/touching points between every edge pair
  for(size_t i = 0; i < sizeA; ++i)
  {
    // Edge A
    LinkedVertex<Vec2_t>* a1 = lookupA[i];
    LinkedVertex<Vec2_t>* a2 = lookupA[(i + 1) % sizeA];

    for(size_t j = 0; j < sizeB; ++j)
    {
      // Edge B
      LinkedVertex<Vec2_t>* b1 = lookupB[j];
      LinkedVertex<Vec2_t>* b2 = lookupB[(j + 1) % sizeB];

      // Compute edge/edge intersections
      Vec2_t point(float_t(0.0), float_t(0.0));
      Intersection::IntersectionType result = TestSegmentSegment(a1->position, a2->position,
                                                     b1->position, b2->position,
                                                     &point);

      // the intersection result between e1 and e2 is a line segment
      // We need to Insert the endpoints from each edge into the other polygon
      if(result == Intersection::Line)
      {
        // Insert edge B into polygon A
        InsertPoint(&pool, &verticesA, a1, a2, b1->position);
        InsertPoint(&pool, &verticesA, a1, a2, b2->position);          
        
        // Insert edge A into polygon B
        InsertPoint(&pool, &verticesB, b1, b2, a1->position);
        InsertPoint(&pool, &verticesB, b1, b2, a2->position);
      }
      else if(result == Intersection::Point)
      {
        // e1 and e2 have exactly 1 intersection point and this point
        // is a vertex of e1 or e2        
        // We only need to Insert this point into the polygon that does not
        // already contain this vertex        
        if (EqualsExact(point, a1->position) || 
            EqualsExact(point, a2->position))
        {
          InsertPoint(&pool, &verticesB, b1, b2, point);
        }
        else if (EqualsExact(point, b1->position) || 
                 EqualsExact(point, b2->position))
        {
          InsertPoint(&pool, &verticesA, a1, a2, point);
        }
        // e1 and e2 have exactly 1 intersection point that does not
        // coincide with any of the vertices of e1 or e2        
        else
        {
          InsertPoint(&pool, &verticesA, a1, a2, point);
          InsertPoint(&pool, &verticesB, b1, b2, point);
        }
      }
    }
  }

  // Put the final vertex lists back into the original contours
  Transfer(verticesA, polygonA, pool);
  Transfer(verticesB, polygonB, pool);
}

void Subdivide(HighPrecisionContourArray* contoursA,
               HighPrecisionContourArray* contoursB)
{
  for(size_t i = 0; i < contoursA->Size(); ++i)
  {
    for(size_t j = 0; j < contoursB->Size(); ++j)
    {
      Subdivide(&(*contoursA)[i], &(*contoursB)[j]);
    }
  }
}

// Construct a simplical chain from an arbitrary polygon
void Build(const HighPrecisionContourArray& contours, Array<Simplex>* chain)
{
  for(size_t i = 0; i < contours.Size(); ++i)
  {
    const Array<Vec2_t>& contour = contours[i];
    size_t edgeCount = contour.Size();
    size_t offset = chain->Size();
    chain->Resize(chain->Size() + edgeCount);

    for(size_t j = 0; j < edgeCount; ++j)
    {
      Simplex* simplex = &(*chain)[j + offset];
      simplex->p1 = contour[j];
      simplex->p2 = contour[(j + 1) % edgeCount];

      // Compute 2x the signed area of the triangle formed by the
      // simplex edge and the origin
      float_t area = simplex->p1.x * simplex->p2.y - 
                     simplex->p1.y * simplex->p2.x;

      if(area != float_t(0.0))
      {
        simplex->alpha = area >= float_t(0.0) ? 1.f : -1.f;
      }
      else
      {
        simplex->alpha = 0.f;
      }
    }    
  }
}

void Build(const Array<Vec2_t>& contour, Array<Simplex>* chain)
{
    size_t edgeCount = contour.Size();
    size_t offset = chain->Size();
    chain->Resize(chain->Size() + edgeCount);
    for(size_t i = 0; i < edgeCount; ++i)
    {
      Simplex* simplex = &(*chain)[i + offset];
      simplex->p1 = contour[i];
      simplex->p2 = contour[(i + 1) % edgeCount];

      // Compute 2x the signed area of the triangle formed by the
      // simplex edge and the origin
      float_t area = simplex->p1.x * simplex->p2.y - 
                     simplex->p1.y * simplex->p2.x;

      if(area != float_t(0.0))
      {
        simplex->alpha = area >= float_t(0.0) ? 1.f : -1.f;
      }
      else
      {
        simplex->alpha = 0.f;
      }
    }    
}

void Select(Csg::Operation::Enum operation,
            const Array<Simplex>& chainA, 
            const Array<Simplex>& chainB,
            Array<Simplex>* selectedA,
            Array<Simplex>* selectedB)
{
  // Magic constants for Intersect, Union, Subtract
  static f32 operationTable[3][2] = {{ 1.f, 1.f }, {0.f, 0.f}, {0.f, 1.f}};

  f32 desiredA = operationTable[operation][0];
  f32 desiredB = operationTable[operation][1];

  for(size_t i = 0; i < chainA.Size(); ++i)
  {
    const Simplex& simplex = chainA[i];
    f32 value = ComputeEdgeValue(operation, simplex, chainB);

    if(value == desiredA)
    {
      selectedA->PushBack(simplex);
    }
  }

  for(size_t i = 0; i < chainB.Size(); ++i)
  {
    const Simplex& simplex = chainB[i];
    f32 value = ComputeEdgeValue(operation, simplex, chainA);

    if(value == desiredB)
    {
      if(chainA.Contains(simplex) == false)
      {
        selectedB->PushBack(simplex);
      }
    }
  }

  if(operation == Csg::Operation::Subtract)
  {
    for(size_t i = 0; i < selectedB->Size(); ++i)
    {
      Math::Swap((*selectedB)[i].p1, (*selectedB)[i].p2);
    }

    if(selectedB->Size())
    {
      Reverse(selectedB->Begin(), selectedB->End());
    }
  }

}

// Build contours from sets of unconnected simplices
bool Merge(const Array<Simplex>& selectedA, 
           const Array<Simplex>& selectedB,
           ContourArray* results)
{
  Array<Simplex> edges;
  edges.Reserve(selectedA.Size() + selectedB.Size());
  edges.Append(selectedA.All());
  edges.Append(selectedB.All());

  Array<Array<Simplex> > partials;

  for(size_t i = 0; i < edges.Size(); ++i)
  {
    const Simplex& edge = edges[i];

    Array<size_t> connected;

    for(size_t j = 0; j < partials.Size(); ++j)
    {
      const Array<Simplex>& partial = partials[j];

      if(EqualsExact(edge.p1, partial.Back().p2))
      {
        connected.PushBack(j);
      }
      else if(EqualsExact(edge.p2, partial.Front().p1))
      {
        connected.PushBack(j); 
      }
    }

    // This can happen due to floating point error, since its highly unlikely
    // we will generate the correct contour at this point the algorithm failed
    if (connected.Size() > 2)
    {
      return false;
    }

    // If edge cannot be connected to an existing partial contour
    if(connected.Empty())
    {
      Array<Simplex> partial;
      partial.PushBack(edge);
      partials.PushBack(partial);
    }

    // If edge can be connected to only 1 partial contour
    if(connected.Size() == 1)
    {
      Array<Simplex>& partial = partials[connected[0]];
      if(EqualsExact(edge.p1, partial.Back().p2))
      {
        partial.PushBack(edge);
      }
      else
      {
        partial.Insert(partial.Data(), edge);
      }

      // If we have created a full contour
      // Add it to results
      if(EqualsExact(partial.Back().p2, partial.Front().p1))
      {
        Array<Vec2> contour;
        contour.Resize(partial.Size());
        for(size_t j = 0; j < partial.Size(); ++j)
        {
          contour[j].x = real(partial[j].p1.x);
          contour[j].y = real(partial[j].p1.y);
        }
        results->PushBack(contour);
        partials[connected[0]].Swap(partials.Back());
        partials.PopBack();
      }
    }

    // If edge can be connected to 2 partial contours
    if(connected.Size() == 2)
    {
      Array<Simplex>& partialA = partials[connected[0]];
      if(EqualsExact(edge.p1, partialA.Back().p2))
      {
        partialA.PushBack(edge);
      }
      else
      {
        partialA.Insert(partialA.Data(), edge);
      }     

      Array<Simplex>& partialB = partials[connected[1]];
      if(EqualsExact(partialB.Front().p1, partialA.Back().p2))
      {
        partialA.Append(partialB.All());
      }
      else
      {
        partialA.Insert(partialA.Data(), partialB.All());
      }        

      // If we have created a full contour add it to results
      bool removed = false;
      if(EqualsExact(partialA.Back().p2, partialA.Front().p1))
      {
        Array<Vec2> contour;
        contour.Resize(partialA.Size());
        for(size_t j = 0; j < partialA.Size(); ++j)
        {
          contour[j].x = real(partialA[j].p1.x);
          contour[j].y = real(partialA[j].p1.y);
        }
        results->PushBack(contour);
        partials[connected[0]].Swap(partials.Back());
        partials.PopBack();
        removed = true;
      }

      // In case element that we need to remove was swapped
      size_t indexB = connected[1];
      if(removed && indexB == partials.Size() - 1)
      {
        indexB = connected[0];
      }
      partials[indexB].Swap(partials.Back());
      partials.PopBack();
    }
  }

  // If there remains partials at this point, either the subdivision or chain
  // selection steps produced invalid results due to floating point error so
  // the clipper failed
  return partials.Empty();
}

void Convert(const ContourArray& input,
             HighPrecisionContourArray* output)
{
  size_t contourCount = input.Size();
  output->Resize(contourCount);
  for(size_t i = 0; i < contourCount; ++i)
  {
    Array<Vec2_t>& current = (*output)[i];
    const Array<Vec2>& old = input[i];

    current.Resize(old.Size());
    for(size_t j = 0; j < old.Size(); ++j)
    {
      current[j].x = float_t(old[j].x);
      current[j].y = float_t(old[j].y);
    }
  }
}

} // namespace Internal

bool Operate(const OperationInput& input,
             ContourArray* results)
{
  ContourArray contoursA = *input.ContoursA;
  ContourArray contoursB = *input.ContoursB;

  Internal::Clean(&contoursA, 
                  input.CollinearAngleTolerance, 
                  input.DistanceTolerance);
  Internal::Clean(&contoursB, 
                  input.CollinearAngleTolerance, 
                  input.DistanceTolerance);

  HighPrecisionContourArray subdividedA;
  HighPrecisionContourArray subdividedB;
  Internal::Convert(contoursA, &subdividedA);
  Internal::Convert(contoursB, &subdividedB);

  Internal::Subdivide(&subdividedA, &subdividedB);

  Array<Simplex> chainA;
  Array<Simplex> chainB;
  Internal::Build(subdividedA, &chainA);
  Internal::Build(subdividedB, &chainB);

  Array<Simplex> selectedA;
  Array<Simplex> selectedB;
  Internal::Select(input.Operation, chainA, chainB, &selectedA, &selectedB);

  bool success = Internal::Merge(selectedA, selectedB, results); 
  if (success == false)
  {
    return false;
  }

  Internal::Clean(results, 
                  input.CollinearAngleTolerance, 
                  input.DistanceTolerance);

  return true;
}

} // namespace Csg

} // namespace Zero

