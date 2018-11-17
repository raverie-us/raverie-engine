///////////////////////////////////////////////////////////////////////////////
///
///  \file Solids.hpp
///  Contains all the hardcoded solids used in the engine.
///
///  Authors: Benjamin Strukus
///  Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Geometry
{

#define zero real(0.0)
#define one real(1.0)
#define two real(2.0)
#define three real(3.0)


//------------------------------------------------------------------------------
namespace Box
{
/*

          y = 1
          |
          |
          |_______x = 0
         /
        /
      z = 2

           POINTS                      EDGES                        FACES

      4 _____________ 5             _____________               _____________
       /|           /|             /|     8     /|             /|   2       /|
      / |          / |            / |          / |            / |          / |
     /  |         /  |         11/  |4   10  9/  |5          /  |   5     /  |
  7 /___|________/ 6 |          /___|________/   |          /___|________/ 0 |
    |   |________|___|          |   |________|___|          | 1 |________|___|
    |   / 0      |   / 1        |   /     0  |   /          |   /        |   /
    |  /         |  /          7|  /3       6|  /1          |  /    4    |  /
    | /          | /            | /          | /            | /          | /
    |/___________|/             |/___________|/             |/___________|/
    3            2                     2                            3

    *For EDGES, the 10th edge is the one that's right above 7 and 6, top of the
     box. I note this because it's a little hard to tell.

    *For FACES, 0 = +x, 1 = -x, 2 = +y, 3 = -y, 4 = +z, 5 = -z
     A little hard to see in the picture but if you use your imagination, you
     can make it out.
*/


///Permutation of positives and negatives for box's points
const Vec3 cPoint[8] = { Vec3(-one, -one, -one),
                         Vec3(one, -one, -one),
                         Vec3(one, -one,  one),
                         Vec3(-one, -one,  one),
                         Vec3(-one,  one, -one),
                         Vec3(one,  one, -one),
                         Vec3(one,  one,  one),
                         Vec3(-one,  one,  one) };

///For use in mapping the edge index of an oriented bounding box to the points
///on that edge.
const uint cEdge[12][2] = { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 },
                            { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 },
                            { 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 4 } };

///For use in determining which points are connected ("on the same edge as") to
///the given point. Inputs are the index of the currently known point and the
///axis corresponding to the edge in the box's local space (x = 0, y = 1, z = 2)
const uint cPointPointEdge[8][3] = { { 1, 4, 3 }, { 0, 5, 2 },
                                     { 3, 6, 1 }, { 2, 7, 0 },
                                     { 5, 0, 7 }, { 4, 1, 6 },
                                     { 7, 2, 5 }, { 6, 3, 4 } };

///For use in determining an edge from the two point indices that lie on it.
#define n static_cast<uint>(-1)
//                                      0   1   2   3   4   5   6   7
const uint cEdgePointPoint[8][8] = { {  n,  0,  n,  3,  4,  n,  n,  n },   // 0
                                     {  0,  n,  1,  n,  n,  5,  n,  n },   // 1
                                     {  n,  1,  n,  2,  n,  n,  6,  n },   // 2
                                     {  3,  n,  2,  n,  n,  n,  n,  7 },   // 3
                                     {  4,  n,  n,  n,  n,  8,  n, 11 },   // 4
                                     {  n,  5,  n,  n,  8,  n,  9,  n },   // 5
                                     {  n,  n,  6,  n,  n,  9,  n, 10 },   // 6
                                     {  n,  n,  n,  7, 11,  n, 10,  n } }; // 7

///For use in determining the index of the major axis that the two points lie on
///when the box is represented in local space (axis-aligned).
//                                     0  1  2  3  4  5  6  7
const uint cAxisPointPoint[8][8] = { { n, 0, n, 2, 1, n, n, n },   // 0
                                     { 0, n, 2, n, n, 1, n, n },   // 1
                                     { n, 2, n, 0, n, n, 1, n },   // 2
                                     { 2, n, 0, n, n, n, n, 1 },   // 3
                                     { 1, n, n, n, n, 0, n, 2 },   // 4
                                     { n, 1, n, n, 0, n, 2, n },   // 5
                                     { n, n, 1, n, n, 2, n, 0 },   // 6
                                     { n, n, n, 1, 2, n, 0, n } }; // 7
#undef n

///For use in determining an edge from the face and the permutation of points
///negative-negative, negative-positive, positive-negative, positive-positive
const uint cEdgeFaceBase[6][4] = { {  5,  9,  6,  1 }, {  4, 11,  7,  3 },
                                   {  8,  9, 10, 11 }, {  0,  1,  2,  3 },
                                   {  2,  6, 10,  7 }, {  0,  5,  8,  4 } };

///For use in determining the edges on a face.
const uint cEdgeFace[6][4] = { {  9,  6,  1,  5 }, {  4,  3,  7,  11 },
                               { 11, 10,  9,  8 }, {  0,  1,  2,   3 },
                               {  7,  2,  6, 10 }, {  5,  0,  4,   8 } } ;

///For use in conjunction with the ObbPoint array to produce the points of a
///face in counter-clockwise order.
//                            Positive        Negative
const uint cFace[6][4] = { { 5, 6, 2, 1 }, { 4, 0, 3, 7 },   // X-Axis
                           { 4, 7, 6, 5 }, { 0, 1, 2, 3 },   // Y-Axis
                           { 7, 3, 2, 6 }, { 5, 1, 0, 4 } }; // Z-Axis

///For use in determining the points on a face given the face.
const uint cPointFaceSign[6][4] = { { 1, 2, 5, 6 }, { 0, 3, 4, 7 },   // X-Axis
                                    { 4, 7, 5, 6 }, { 0, 3, 1, 2 },   // Y-Axis
                                    { 3, 7, 2, 6 }, { 0, 4, 1, 5 } }; // Z-Axis

///0 is positive, 1 is negative.
const uint cFaceAxisSign[3][2] = { { 0, 1 }, { 2, 3 }, { 4, 5 } };

}// namespace Box

//------------------------------------------------------------------------------
namespace PlatonicSolids
{

//------------------------------------------------------------------- Octahedron
const Vec3 cOctahedronPoints[6] = { Vec3(one, zero, zero),
                                    Vec3(-one, zero, zero),
                                    Vec3(zero,  one, zero),
                                    Vec3(zero, -one, zero),
                                    Vec3(zero, zero,  one),
                                    Vec3(zero, zero, -one) };

const uint cOctahedronFaces[8][3] = { { 0, 2, 4 }, { 0, 4, 3 },
                                      { 0, 5, 2 }, { 0, 3, 5 },
                                      { 1, 4, 2 }, { 1, 3, 4 },
                                      { 1, 2, 5 }, { 1, 5, 3 } };

#define tau Math::cGoldenRatio
#define oot (one / tau) //oot = One over Tau

//----------------------------------------------------------------- Dodecahedron
const Vec3 cDodecahedronPoints[20] = { Vec3(one,  one,  one),   // 0
                                       Vec3(one,  one, -one),   // 1
                                       Vec3(one, -one,  one),   // 2
                                       Vec3(one, -one, -one),   // 3
                                       Vec3(-one,  one,  one),   // 4
                                       Vec3(-one,  one, -one),   // 5
                                       Vec3(-one, -one,  one),   // 6
                                       Vec3(-one, -one, -one),   // 7
                                       Vec3(zero,  oot,  tau),   // 8
                                       Vec3(zero,  oot, -tau),   // 9
                                       Vec3(zero, -oot,  tau),   // 10
                                       Vec3(zero, -oot, -tau),   // 11
                                       Vec3(oot,  tau, zero),   // 12
                                       Vec3(oot, -tau, zero),   // 13
                                       Vec3(-oot,  tau, zero),   // 14
                                       Vec3(-oot, -tau, zero),   // 15
                                       Vec3(tau, zero,  oot),   // 16
                                       Vec3(tau, zero, -oot),   // 17
                                       Vec3(-tau, zero,  oot),   // 18
                                       Vec3(-tau, zero, -oot) }; // 19
#undef oot

const uint cDodecahedronFaces[12][5] = { {  0, 12, 14,  4,  8 },
                                         {  0,  8, 10,  2, 16 },
                                         {  0, 16, 17,  1, 12 },
                                         {  9,  5, 14, 12,  1 },
                                         {  9,  1, 17,  3, 11 },
                                         {  9, 11,  7, 19,  5 },
                                         { 13, 15,  7, 11,  3 },
                                         { 13,  3, 17, 16,  2 },
                                         { 13,  2, 10,  6, 15 },
                                         { 18,  6,  2,  8,  4 },
                                         { 18,  4, 14,  5, 19 },
                                         { 18, 19,  7, 15,  6 } };

//------------------------------------------------------------------ Icosahedron
const Vec3 cIcosahedronPoints[12] = { Vec3(one,  tau, zero),    // 0
                                      Vec3(-one,  tau, zero),    // 1
                                      Vec3(one, -tau, zero),    // 2
                                      Vec3(-one, -tau, zero),    // 3
                                      Vec3(zero,  one,  tau),    // 4
                                      Vec3(zero, -one,  tau),    // 5
                                      Vec3(zero,  one, -tau),    // 6
                                      Vec3(zero, -one, -tau),    // 7
                                      Vec3(tau,  zero,  one),    // 8
                                      Vec3(-tau,  zero,  one),    // 9
                                      Vec3(tau,  zero, -one),    // 10
                                      Vec3(-tau,  zero, -one) };  // 11

const uint cIcosahedronFaces[20][3] = { {  0,  1,  4 }, { 1,  0,  6 },
                                        {  0,  4,  8 }, { 5,  2,  8 },
                                        {  4,  5,  8 }, { 4,  1,  9 },
                                        {  5,  4,  9 }, { 6,  0, 10 },
                                        {  2,  7, 10 }, { 7,  6, 10 },
                                        {  0,  8, 10 }, { 8,  2, 10 },
                                        {  1,  6, 11 }, { 6,  7, 11 },
                                        {  9,  1, 11 }, { 2,  5,  3 },
                                        {  7,  2,  3 }, { 5,  9,  3 },
                                        { 11,  7,  3 }, { 9, 11,  3 } };
#undef tau
}// namespace PlatonicSolids

//----------------------------------------------------------- Archimedean Solids
namespace ArchimedeanSolids
{

//-------------------------------------------------------- Truncated Tetrahedron
const Vec3 cTruncatedTetrahedron[12] = { Vec3(three,    one,    one),
                                         Vec3( one,  three,    one),
                                         Vec3( one,    one,  three),
                                         Vec3(-three,   -one,    one),
                                         Vec3(-one, -three,    one),
                                         Vec3(-one,   -one,  three),
                                         Vec3(-three,    one,   -one),
                                         Vec3(-one,  three,   -one),
                                         Vec3(-one,    one, -three),
                                         Vec3(three,   -one,   -one),
                                         Vec3( one, -three,   -one),
                                         Vec3( one,   -one, -three) };

//---------------------------------------------------------------- Cuboctahedron
const Vec3 cCuboctahedron[12] = { Vec3(one,  one, zero),
                                  Vec3(one, -one, zero),
                                  Vec3(-one,  one, zero),
                                  Vec3(-one, -one, zero),
                                  Vec3(one, zero,  one),
                                  Vec3(one, zero, -one),
                                  Vec3(-one, zero,  one),
                                  Vec3(-one, zero, -one),
                                  Vec3(zero,  one,  one),
                                  Vec3(zero,  one, -one),
                                  Vec3(zero, -one,  one),
                                  Vec3(zero, -one, -one) };

//--------------------------------------------------------------- Truncated Cube
//Constant used for the truncated cube.
#define sto real(0.4142135623730950488016887242097) /* sqrt(2) - 1 */
const Vec3 cTruncatedCube[24] = { Vec3(sto,  one,  one),
                                  Vec3(sto,  one, -one),
                                  Vec3(sto, -one,  one),
                                  Vec3(sto, -one, -one),
                                  Vec3(-sto,  one,  one),
                                  Vec3(-sto,  one, -one),
                                  Vec3(-sto, -one,  one),
                                  Vec3(-sto, -one, -one),
                                  Vec3(one,  sto,  one),
                                  Vec3(one,  sto, -one),
                                  Vec3(one, -sto,  one),
                                  Vec3(one, -sto, -one),
                                  Vec3(-one,  sto,  one),
                                  Vec3(-one,  sto, -one),
                                  Vec3(-one, -sto,  one),
                                  Vec3(-one, -sto, -one),
                                  Vec3(one,  one,  sto),
                                  Vec3(one,  one, -sto),
                                  Vec3(one, -one,  sto),
                                  Vec3(one, -one, -sto),
                                  Vec3(-one,  one,  sto),
                                  Vec3(-one,  one, -sto),
                                  Vec3(-one, -one,  sto),
                                  Vec3(-one, -one, -sto) };
#undef sto

//--------------------------------------------------------- Truncated Octahedron
const Vec3 cTruncatedOctahedron[24] = { Vec3(zero,  one,  two),
                                        Vec3(zero,  one, -two),
                                        Vec3(zero, -one,  two),
                                        Vec3(zero, -one, -two),
                                        Vec3(zero,  two,  one),
                                        Vec3(zero,  two, -one),
                                        Vec3(zero, -two,  one),
                                        Vec3(zero, -two, -one),
                                        Vec3(one, zero,  two),
                                        Vec3(one, zero, -two),
                                        Vec3(-one, zero,  two),
                                        Vec3(-one, zero, -two),
                                        Vec3(one,  two, zero),
                                        Vec3(one, -two, zero),
                                        Vec3(-one,  two, zero),
                                        Vec3(-one, -two, zero),
                                        Vec3(two, zero,  one),
                                        Vec3(two, zero, -one),
                                        Vec3(-two, zero,  one),
                                        Vec3(-two, zero, -one),
                                        Vec3(two,  one, zero),
                                        Vec3(two, -one, zero),
                                        Vec3(-two,  one, zero),
                                        Vec3(-two, -one, zero) };

//---------------------------------------------------------- Rhombicuboctahedron
//Constant used for the rhombicuboctahedron and the truncated cuboctahedron.
#define ost real(2.4142135623730950488016887242097) /* 1 + sqrt(2) */
const Vec3 cRhombicuboctahedron[24] = { Vec3(one,  one,  ost),
                                        Vec3(one,  one, -ost),
                                        Vec3(one, -one,  ost),
                                        Vec3(one, -one, -ost),
                                        Vec3(-one,  one,  ost),
                                        Vec3(-one,  one, -ost),
                                        Vec3(-one, -one,  ost),
                                        Vec3(-one, -one, -ost),
                                        Vec3(one,  ost,  one), //
                                        Vec3(one,  ost, -one),
                                        Vec3(one, -ost,  one),
                                        Vec3(one, -ost, -one),
                                        Vec3(-one,  ost,  one),
                                        Vec3(-one,  ost, -one),
                                        Vec3(-one, -ost,  one),
                                        Vec3(-one, -ost, -one),
                                        Vec3(ost,  one,  one), //
                                        Vec3(ost,  one, -one),
                                        Vec3(ost, -one,  one),
                                        Vec3(ost, -one, -one),
                                        Vec3(-ost,  one,  one),
                                        Vec3(-ost,  one, -one),
                                        Vec3(-ost, -one,  one),
                                        Vec3(-ost, -one, -one) };

//------------------------------------------------------ Truncated Cuboctahedron
//Constant used for the truncated cuboctahedron.
#define ott real(3.8284271247461900976033774484194) /* 1 + (2 * sqrt(2)) */
const Vec3 cTruncatedCuboctahedron[48] = { Vec3(one,  ost,  ott),
                                           Vec3(one,  ost, -ott),
                                           Vec3(one, -ost,  ott),
                                           Vec3(one, -ost, -ott),
                                           Vec3(-one,  ost,  ott),
                                           Vec3(-one,  ost, -ott),
                                           Vec3(-one, -ost,  ott),
                                           Vec3(-one, -ost, -ott),
                                           Vec3(one,  ott,  ost),
                                           Vec3(one,  ott, -ost),
                                           Vec3(one, -ott,  ost),
                                           Vec3(one, -ott, -ost),
                                           Vec3(-one,  ott,  ost),
                                           Vec3(-one,  ott, -ost),
                                           Vec3(-one, -ott,  ost),
                                           Vec3(-one, -ott, -ost),
                                           Vec3(ost,  one,  ott),
                                           Vec3(ost,  one, -ott),
                                           Vec3(ost, -one,  ott),
                                           Vec3(ost, -one, -ott),
                                           Vec3(-ost,  one,  ott),
                                           Vec3(-ost,  one, -ott),
                                           Vec3(-ost, -one,  ott),
                                           Vec3(-ost, -one, -ott),
                                           Vec3(ost,  ott,  one),
                                           Vec3(ost,  ott, -one),
                                           Vec3(ost, -ott,  one),
                                           Vec3(ost, -ott, -one),
                                           Vec3(-ost,  ott,  one),
                                           Vec3(-ost,  ott, -one),
                                           Vec3(-ost, -ott,  one),
                                           Vec3(-ost, -ott, -one),
                                           Vec3(ott,  one,  ost),
                                           Vec3(ott,  one, -ost),
                                           Vec3(ott, -one,  ost),
                                           Vec3(ott, -one, -ost),
                                           Vec3(-ott,  one,  ost),
                                           Vec3(-ott,  one, -ost),
                                           Vec3(-ott, -one,  ost),
                                           Vec3(-ott, -one, -ost),
                                           Vec3(ott,  ost,  one),
                                           Vec3(ott,  ost, -one),
                                           Vec3(ott, -ost,  one),
                                           Vec3(ott, -ost, -one),
                                           Vec3(-ott,  ost,  one),
                                           Vec3(-ott,  ost, -one),
                                           Vec3(-ott, -ost,  one),
                                           Vec3(-ott, -ost, -one) };
#undef ott
#undef ost

//-------------------------------------------------------------------- Snub Cube
//Constants used for the snub cube. Second one is the reciprocal of the first.
/* (1/3) * (cbrt(17 + 3 * sqrt(33)) - cbrt(-17 + 3 * sqrt(33)) - 1) */
#define trm real(0.54368901269207636157085597180175) /* TeRM */
#define rot real(1.0) / trm                          /* Reciprocal Of Term */
const Vec3 cSnubCube[24] = { //Even permutations
                             Vec3(-one,  trm,  rot),  //a b c
                             Vec3(one, -trm,  rot),
                             Vec3(one,  trm, -rot),
                             Vec3(-one, -trm, -rot),

                             Vec3(-trm,  rot,  one),  //b c a
                             Vec3(trm, -rot,  one),
                             Vec3(trm,  rot, -one),
                             Vec3(-trm, -rot, -one),

                             Vec3(-rot,  one,  trm),  //c a b
                             Vec3(rot, -one,  trm),
                             Vec3(rot,  one, -trm),
                             Vec3(-rot, -one, -trm),

                             //Odd permutations
                             Vec3(one,  rot,  trm),  //a c b
                             Vec3(one, -rot, -trm),
                             Vec3(-one,  rot, -trm),
                             Vec3(-one, -rot,  trm),

                             Vec3(trm,  one,  rot),  //b a c
                             Vec3(trm, -one, -rot),
                             Vec3(-trm,  one, -rot),
                             Vec3(-trm, -one,  rot),

                             Vec3(rot,  trm,  one),  //c b a
                             Vec3(rot, -trm, -one),
                             Vec3(-rot,  trm, -one),
                             Vec3(-rot, -trm,  one) };
#undef rot
#undef trm

//------------------------------------------------------------ Icosidodecahedron
#define tau Math::cGoldenRatio
#define hlf real(1.0 / 2.0)               /* one HaLF */
#define tot tau / real(2.0)               /* Tau Over Two */
#define hot (real(1.0) + tau) / real(2.0) /* Half of One plus Tau */
const Vec3 cIcosidodecahedron[30] = { Vec3(zero, zero,  tau),
                                      Vec3(zero, zero, -tau),
                                      Vec3(zero,  tau, zero),
                                      Vec3(zero, -tau, zero),
                                      Vec3(tau, zero, zero),
                                      Vec3(-tau, zero, zero),

                                      Vec3(hlf,  tot,  hot),
                                      Vec3(hlf,  tot, -hot),
                                      Vec3(hlf, -tot,  hot),
                                      Vec3(hlf, -tot, -hot),
                                      Vec3(-hlf,  tot,  hot),
                                      Vec3(-hlf,  tot, -hot),
                                      Vec3(-hlf, -tot,  hot),
                                      Vec3(-hlf, -tot, -hot),

                                      Vec3(tot,  hot,  hlf),
                                      Vec3(tot,  hot, -hlf),
                                      Vec3(tot, -hot,  hlf),
                                      Vec3(tot, -hot, -hlf),
                                      Vec3(-tot,  hot,  hlf),
                                      Vec3(-tot,  hot, -hlf),
                                      Vec3(-tot, -hot,  hlf),
                                      Vec3(-tot, -hot, -hlf),

                                      Vec3(hot,  hlf,  tot),
                                      Vec3(hot,  hlf, -tot),
                                      Vec3(hot, -hlf,  tot),
                                      Vec3(hot, -hlf, -tot),
                                      Vec3(-hot,  hlf,  tot),
                                      Vec3(-hot,  hlf, -tot),
                                      Vec3(-hot, -hlf,  tot),
                                      Vec3(-hot, -hlf, -tot) };
#undef hot
#undef tot
#undef hlf

//------------------------------------------------------- Truncated Dodecahedron
#define oot real(1.0) / tau /* One over Tau */
#define TpT real(2.0) + tau /* Two Plus Tau */
#define ttt real(2.0) * tau /* Two Times Tau */
#define tsq tau * tau       /* Tau SQuared */
const Vec3 cTruncatedDodecahedron[60] = { Vec3(zero,  oot,  TpT),
                                          Vec3(zero,  oot, -TpT),
                                          Vec3(zero, -oot,  TpT),
                                          Vec3(zero, -oot, -TpT),

                                          Vec3(TpT, zero,  oot),
                                          Vec3(TpT, zero, -oot),
                                          Vec3(-TpT, zero,  oot),
                                          Vec3(-TpT, zero, -oot),

                                          Vec3(oot,  TpT, zero),
                                          Vec3(oot, -TpT, zero),
                                          Vec3(-oot,  TpT, zero),
                                          Vec3(-oot, -TpT, zero),

                                          Vec3(oot,  tau,  ttt),
                                          Vec3(oot,  tau, -ttt),
                                          Vec3(oot, -tau,  ttt),
                                          Vec3(oot, -tau, -ttt),
                                          Vec3(-oot,  tau,  ttt),
                                          Vec3(-oot,  tau, -ttt),
                                          Vec3(-oot, -tau,  ttt),
                                          Vec3(-oot, -tau, -ttt),

                                          Vec3(ttt,  oot,  tau),
                                          Vec3(ttt,  oot, -tau),
                                          Vec3(ttt, -oot,  tau),
                                          Vec3(ttt, -oot, -tau),
                                          Vec3(-ttt,  oot,  tau),
                                          Vec3(-ttt,  oot, -tau),
                                          Vec3(-ttt, -oot,  tau),
                                          Vec3(-ttt, -oot, -tau),

                                          Vec3(tau,  ttt,  oot),
                                          Vec3(tau,  ttt, -oot),
                                          Vec3(tau, -ttt,  oot),
                                          Vec3(tau, -ttt, -oot),
                                          Vec3(-tau,  ttt,  oot),
                                          Vec3(-tau,  ttt, -oot),
                                          Vec3(-tau, -ttt,  oot),
                                          Vec3(-tau, -ttt, -oot),

                                          Vec3(tau,  two,  tsq),
                                          Vec3(tau,  two, -tsq),
                                          Vec3(tau, -two,  tsq),
                                          Vec3(tau, -two, -tsq),
                                          Vec3(-tau,  two,  tsq),
                                          Vec3(-tau,  two, -tsq),
                                          Vec3(-tau, -two,  tsq),
                                          Vec3(-tau, -two, -tsq),

                                          Vec3(tsq,  tau,  two),
                                          Vec3(tsq,  tau, -two),
                                          Vec3(tsq, -tau,  two),
                                          Vec3(tsq, -tau, -two),
                                          Vec3(-tsq,  tau,  two),
                                          Vec3(-tsq,  tau, -two),
                                          Vec3(-tsq, -tau,  two),
                                          Vec3(-tsq, -tau, -two),

                                          Vec3(two,  tsq,  tau),
                                          Vec3(two,  tsq, -tau),
                                          Vec3(two, -tsq,  tau),
                                          Vec3(two, -tsq, -tau),
                                          Vec3(-two,  tsq,  tau),
                                          Vec3(-two,  tsq, -tau),
                                          Vec3(-two, -tsq,  tau),
                                          Vec3(-two, -tsq, -tau) };
#undef tsq
#undef ttt
#undef TpT
#undef oot

//-------------------------------------------------------- Truncated Icosahedron
#define tht (real(3.0) * tau) /* THree Tau */
#define twt (real(2.0) * tau) /* TWo Tau */
#define ott (real(1.0) + twt) /* One plus Two Tau */
#define tpt (real(2.0) + tau) /* Two Plus Tau */
const Vec3 cTruncatedIcosahedron[60] = { Vec3(zero,  one,  tht),  //abc
                                         Vec3(zero,  one, -tht),
                                         Vec3(zero, -one,  tht),
                                         Vec3(zero, -one, -tht),

                                         Vec3(one,  tht, zero),  //bca
                                         Vec3(one, -tht, zero),
                                         Vec3(-one,  tht, zero),
                                         Vec3(-one, -tht, zero),

                                         Vec3(tht, zero,  one),  //cab
                                         Vec3(tht, zero, -one),
                                         Vec3(-tht, zero,  one),
                                         Vec3(-tht, zero, -one),

                                         Vec3(two,  ott,  tau),  //abc
                                         Vec3(two,  ott, -tau),
                                         Vec3(two, -ott,  tau),
                                         Vec3(two, -ott, -tau),
                                         Vec3(-two,  ott,  tau),
                                         Vec3(-two,  ott, -tau),
                                         Vec3(-two, -ott,  tau),
                                         Vec3(-two, -ott, -tau),

                                         Vec3(ott,  tau,  two),  //bca
                                         Vec3(ott,  tau, -two),
                                         Vec3(ott, -tau,  two),
                                         Vec3(ott, -tau, -two),
                                         Vec3(-ott,  tau,  two),
                                         Vec3(-ott,  tau, -two),
                                         Vec3(-ott, -tau,  two),
                                         Vec3(-ott, -tau, -two),

                                         Vec3(tau,  two,  ott),  //cab
                                         Vec3(tau,  two, -ott),
                                         Vec3(tau, -two,  ott),
                                         Vec3(tau, -two, -ott),
                                         Vec3(-tau,  two,  ott),
                                         Vec3(-tau,  two, -ott),
                                         Vec3(-tau, -two,  ott),
                                         Vec3(-tau, -two, -ott),

                                         Vec3(one,  tpt,  twt),  //abc
                                         Vec3(one,  tpt, -twt),
                                         Vec3(one, -tpt,  twt),
                                         Vec3(one, -tpt, -twt),
                                         Vec3(-one,  tpt,  twt),
                                         Vec3(-one,  tpt, -twt),
                                         Vec3(-one, -tpt,  twt),
                                         Vec3(-one, -tpt, -twt),

                                         Vec3(tpt,  twt,  one),  //bca
                                         Vec3(tpt,  twt, -one),
                                         Vec3(tpt, -twt,  one),
                                         Vec3(tpt, -twt, -one),
                                         Vec3(-tpt,  twt,  one),
                                         Vec3(-tpt,  twt, -one),
                                         Vec3(-tpt, -twt,  one),
                                         Vec3(-tpt, -twt, -one),

                                         Vec3(twt,  one,  tpt),  //cab
                                         Vec3(twt,  one, -tpt),
                                         Vec3(twt, -one,  tpt),
                                         Vec3(twt, -one, -tpt),
                                         Vec3(-twt,  one,  tpt),
                                         Vec3(-twt,  one, -tpt),
                                         Vec3(-twt, -one,  tpt),
                                         Vec3(-twt, -one, -tpt) };
#undef tpt
#undef ott
#undef twt
#undef tht

//------------------------------------------------------- Rhombicosidodecahedron
#define tcu (tau * tau * tau) /* Tau CUbed */
#define tsq (tau * tau)       /* Tau SQuared */
#define ttt (real(2.0) * tau) /* Two Times Tau */
#define tpt (real(2.0) + tau) /* Two Plus Tau */
const Vec3 cRhombicosidodecahedron[60] = { Vec3(one,  one,  tcu),
                                           Vec3(one,  one, -tcu),
                                           Vec3(one, -one,  tcu),
                                           Vec3(one, -one, -tcu),
                                           Vec3(-one,  one,  tcu),
                                           Vec3(-one,  one, -tcu),
                                           Vec3(-one, -one,  tcu),
                                           Vec3(-one, -one, -tcu),

                                           Vec3(tcu,  one,  one),
                                           Vec3(tcu,  one, -one),
                                           Vec3(tcu, -one,  one),
                                           Vec3(tcu, -one, -one),
                                           Vec3(-tcu,  one,  one),
                                           Vec3(-tcu,  one, -one),
                                           Vec3(-tcu, -one,  one),
                                           Vec3(-tcu, -one, -one),

                                           Vec3(one,  tcu,  one),
                                           Vec3(one,  tcu, -one),
                                           Vec3(one, -tcu,  one),
                                           Vec3(one, -tcu, -one),
                                           Vec3(-one,  tcu,  one),
                                           Vec3(-one,  tcu, -one),
                                           Vec3(-one, -tcu,  one),
                                           Vec3(-one, -tcu, -one),

                                           Vec3(tsq,  tau,  ttt),
                                           Vec3(tsq,  tau, -ttt),
                                           Vec3(tsq, -tau,  ttt),
                                           Vec3(tsq, -tau, -ttt),
                                           Vec3(-tsq,  tau,  ttt),
                                           Vec3(-tsq,  tau, -ttt),
                                           Vec3(-tsq, -tau,  ttt),
                                           Vec3(-tsq, -tau, -ttt),

                                           Vec3(ttt,  tsq,  tau),
                                           Vec3(ttt,  tsq, -tau),
                                           Vec3(ttt, -tsq,  tau),
                                           Vec3(ttt, -tsq, -tau),
                                           Vec3(-ttt,  tsq,  tau),
                                           Vec3(-ttt,  tsq, -tau),
                                           Vec3(-ttt, -tsq,  tau),
                                           Vec3(-ttt, -tsq, -tau),

                                           Vec3(tau,  ttt,  tsq),
                                           Vec3(tau,  ttt, -tsq),
                                           Vec3(tau, -ttt,  tsq),
                                           Vec3(tau, -ttt, -tsq),
                                           Vec3(-tau,  ttt,  tsq),
                                           Vec3(-tau,  ttt, -tsq),
                                           Vec3(-tau, -ttt,  tsq),
                                           Vec3(-tau, -ttt, -tsq),

                                           Vec3(tpt, zero,  tsq),
                                           Vec3(tpt, zero, -tsq),
                                           Vec3(-tpt, zero,  tsq),
                                           Vec3(-tpt, zero, -tsq),

                                           Vec3(tsq,  tpt, zero),
                                           Vec3(tsq, -tpt, zero),
                                           Vec3(-tsq,  tpt, zero),
                                           Vec3(-tsq, -tpt, zero),

                                           Vec3(zero,  tsq,  tpt),
                                           Vec3(zero,  tsq, -tpt),
                                           Vec3(zero, -tsq,  tpt),
                                           Vec3(zero, -tsq, -tpt) };

#undef tsq
#undef ttt
#undef tpt
#undef tcu

//-------------------------------------------------- Truncated Icosidodecahedron
#define oot (real(1.0) / tau)               /* One Over Tau */
#define tot (real(2.0) / tau)               /* Two Over Tau */
#define tht (real(3.0) + tau)               /* THree plus Tau */
#define ott (real(1.0) + (real(2.0) * tau)) /* One plus Two Tau */
#define tsq (tau * tau)                     /* Tau Squared */
#define thm ((real(3.0) * tau) - real(1.0)) /* THree tau Minus one */
#define twm ((real(2.0) * tau) - real(1.0)) /* TWo tau Minus one */
#define tpt (real(2.0) + tau)               /* Two Plus Tau */
#define ttt (real(2.0) * tau)               /* Two Times Tau */
#define thr three                           /* 3 characters ftw */
const Vec3 cTruncatedIcosidodecahedron[120] = { Vec3(oot,  oot,  tht), //abc
                                                Vec3(oot,  oot, -tht),
                                                Vec3(oot, -oot,  tht),
                                                Vec3(oot, -oot, -tht),
                                                Vec3(-oot,  oot,  tht),
                                                Vec3(-oot,  oot, -tht),
                                                Vec3(-oot, -oot,  tht),
                                                Vec3(-oot, -oot, -tht),

                                                Vec3(oot,  tht,  oot), //bca
                                                Vec3(oot,  tht, -oot),
                                                Vec3(oot, -tht,  oot),
                                                Vec3(oot, -tht, -oot),
                                                Vec3(-oot,  tht,  oot),
                                                Vec3(-oot,  tht, -oot),
                                                Vec3(-oot, -tht,  oot),
                                                Vec3(-oot, -tht, -oot),

                                                Vec3(tht,  oot,  oot), //cab
                                                Vec3(tht,  oot, -oot),
                                                Vec3(tht, -oot,  oot),
                                                Vec3(tht, -oot, -oot),
                                                Vec3(-tht,  oot,  oot),
                                                Vec3(-tht,  oot, -oot),
                                                Vec3(-tht, -oot,  oot),
                                                Vec3(-tht, -oot, -oot),
                                                //---------------------------
                                                Vec3(tot,  tau,  ott), //abc
                                                Vec3(tot,  tau, -ott),
                                                Vec3(tot, -tau,  ott),
                                                Vec3(tot, -tau, -ott),
                                                Vec3(-tot,  tau,  ott),
                                                Vec3(-tot,  tau, -ott),
                                                Vec3(-tot, -tau,  ott),
                                                Vec3(-tot, -tau, -ott),

                                                Vec3(tau,  ott,  tot), //bca
                                                Vec3(tau,  ott, -tot),
                                                Vec3(tau, -ott,  tot),
                                                Vec3(tau, -ott, -tot),
                                                Vec3(-tau,  ott,  tot),
                                                Vec3(-tau,  ott, -tot),
                                                Vec3(-tau, -ott,  tot),
                                                Vec3(-tau, -ott, -tot),

                                                Vec3(ott,  tot,  tau), //cab
                                                Vec3(ott,  tot, -tau),
                                                Vec3(ott, -tot,  tau),
                                                Vec3(ott, -tot, -tau),
                                                Vec3(-ott,  tot,  tau),
                                                Vec3(-ott,  tot, -tau),
                                                Vec3(-ott, -tot,  tau),
                                                Vec3(-ott, -tot, -tau),
                                                //---------------------------
                                                Vec3(oot,  tsq,  thm), //abc
                                                Vec3(oot,  tsq, -thm),
                                                Vec3(oot, -tsq,  thm),
                                                Vec3(oot, -tsq, -thm),
                                                Vec3(-oot,  tsq,  thm),
                                                Vec3(-oot,  tsq, -thm),
                                                Vec3(-oot, -tsq,  thm),
                                                Vec3(-oot, -tsq, -thm),

                                                Vec3(tsq,  thm,  oot), //bca
                                                Vec3(tsq,  thm, -oot),
                                                Vec3(tsq, -thm,  oot),
                                                Vec3(tsq, -thm, -oot),
                                                Vec3(-tsq,  thm,  oot),
                                                Vec3(-tsq,  thm, -oot),
                                                Vec3(-tsq, -thm,  oot),
                                                Vec3(-tsq, -thm, -oot),

                                                Vec3(thm,  oot,  tsq), //cab
                                                Vec3(thm,  oot, -tsq),
                                                Vec3(thm, -oot,  tsq),
                                                Vec3(thm, -oot, -tsq),
                                                Vec3(-thm,  oot,  tsq),
                                                Vec3(-thm,  oot, -tsq),
                                                Vec3(-thm, -oot,  tsq),
                                                Vec3(-thm, -oot, -tsq),
                                                //---------------------------
                                                Vec3(twm,  two,  tpt), //abc
                                                Vec3(twm,  two, -tpt),
                                                Vec3(twm, -two,  tpt),
                                                Vec3(twm, -two, -tpt),
                                                Vec3(-twm,  two,  tpt),
                                                Vec3(-twm,  two, -tpt),
                                                Vec3(-twm, -two,  tpt),
                                                Vec3(-twm, -two, -tpt),

                                                Vec3(two,  tpt,  twm), //bca
                                                Vec3(two,  tpt, -twm),
                                                Vec3(two, -tpt,  twm),
                                                Vec3(two, -tpt, -twm),
                                                Vec3(-two,  tpt,  twm),
                                                Vec3(-two,  tpt, -twm),
                                                Vec3(-two, -tpt,  twm),
                                                Vec3(-two, -tpt, -twm),

                                                Vec3(tpt,  twm,  two), //cab
                                                Vec3(tpt,  twm, -two),
                                                Vec3(tpt, -twm,  two),
                                                Vec3(tpt, -twm, -two),
                                                Vec3(-tpt,  twm,  two),
                                                Vec3(-tpt,  twm, -two),
                                                Vec3(-tpt, -twm,  two),
                                                Vec3(-tpt, -twm, -two),
                                                //---------------------------
                                                Vec3(tau,  thr,  ttt), //abc
                                                Vec3(tau,  thr, -ttt),
                                                Vec3(tau, -thr,  ttt),
                                                Vec3(tau, -thr, -ttt),
                                                Vec3(-tau,  thr,  ttt),
                                                Vec3(-tau,  thr, -ttt),
                                                Vec3(-tau, -thr,  ttt),
                                                Vec3(-tau, -thr, -ttt),

                                                Vec3(thr,  ttt,  tau), //bca
                                                Vec3(thr,  ttt, -tau),
                                                Vec3(thr, -ttt,  tau),
                                                Vec3(thr, -ttt, -tau),
                                                Vec3(-thr,  ttt,  tau),
                                                Vec3(-thr,  ttt, -tau),
                                                Vec3(-thr, -ttt,  tau),
                                                Vec3(-thr, -ttt, -tau),

                                                Vec3(ttt,  tau,  thr), //cab
                                                Vec3(ttt,  tau, -thr),
                                                Vec3(ttt, -tau,  thr),
                                                Vec3(ttt, -tau, -thr),
                                                Vec3(-ttt,  tau,  thr),
                                                Vec3(-ttt,  tau, -thr),
                                                Vec3(-ttt, -tau,  thr),
                                                Vec3(-ttt, -tau, -thr) };
#undef ttt
#undef tpt
#undef twm
#undef thm
#undef tsq
#undef ott
#undef tht
#undef tot
#undef oot

//------------------------------------------------------------ Snub Dodecahedron
#define sqg real(1.7155614996973)     /* Wikipedia: Snub Dodecahedron */
#define alp (sqg - (real(1.0) / sqg)) /* Alpha = squiggly - (1 / squiggly) */
#define bet ((sqg * tau) + (tau * tau) + (tau / sqg)) /* Beta */
#define oot (real(1.0) / tau)
#define tal (real(2.0) * alp)         /* Two times ALpha */
#define tbe (real(2.0) * bet)         /* Two times BEta */

#define gam (alp + (bet / tau) + tau)
#define del (-(alp * tau) + bet + oot)
#define eps ((alp / tau) + (bet * tau) - one)

#define zet ((-alp / tau) + (bet * tau) + one)
#define eta (-alp + (bet / tau) - tau)
#define the ((alp * tau) + bet - oot)

#define iot ((-alp / tau) + (bet * tau) - one)
#define kap (alp - (bet / tau) - tau)
#define lam ((alp * tau) + bet + oot)

#define muu (alp + (bet / tau) - tau)
#define nuu ((alp * tau) - bet + oot)
#define xii ((alp / tau) + (bet * tau) + one)
const Vec3 cSnubDodecahedron[60] = { Vec3(-tal,  two,  tbe),  //abc
                                     Vec3(tal, -two,  tbe),
                                     Vec3(tal,  two, -tbe),
                                     Vec3(-tal, -two, -tbe),
                                     Vec3(-two,  tbe,  tal),  //bca
                                     Vec3(two, -tbe,  tal),
                                     Vec3(two,  tbe, -tal),
                                     Vec3(-two, -tbe, -tal),
                                     Vec3(-tbe,  tal,  two),  //cab
                                     Vec3(tbe, -tal,  two),
                                     Vec3(tbe,  tal, -two),
                                     Vec3(-tbe, -tal, -two),
                                     //----------------------------
                                     Vec3(-gam,  del,  eps),  //abc
                                     Vec3(gam, -del,  eps),
                                     Vec3(gam,  del, -eps),
                                     Vec3(-gam, -del, -eps),
                                     Vec3(-del,  eps,  gam),  //bca
                                     Vec3(del, -eps,  gam),
                                     Vec3(del,  eps, -gam),
                                     Vec3(-del, -eps, -gam),
                                     Vec3(-eps,  gam,  del),  //cab
                                     Vec3(eps, -gam,  del),
                                     Vec3(eps,  gam, -del),
                                     Vec3(-eps, -gam, -del),
                                     //----------------------------
                                     Vec3(-zet,  eta,  the),  //abc
                                     Vec3(zet, -eta,  the),
                                     Vec3(zet,  eta, -the),
                                     Vec3(-zet, -eta, -the),
                                     Vec3(-eta,  the,  zet),  //bca
                                     Vec3(eta, -the,  zet),
                                     Vec3(eta,  the, -zet),
                                     Vec3(-eta, -the, -zet),
                                     Vec3(-the,  zet,  eta),  //cab
                                     Vec3(the, -zet,  eta),
                                     Vec3(the,  zet, -eta),
                                     Vec3(-the, -zet, -eta),
                                     //----------------------------
                                     Vec3(-iot,  kap,  lam),  //abc
                                     Vec3(iot, -kap,  lam),
                                     Vec3(iot,  kap, -lam),
                                     Vec3(-iot, -kap, -lam),
                                     Vec3(-kap,  lam,  iot),  //bca
                                     Vec3(kap, -lam,  iot),
                                     Vec3(kap,  lam, -iot),
                                     Vec3(-kap, -lam, -iot),
                                     Vec3(-lam,  iot,  kap),  //cab
                                     Vec3(lam, -iot,  kap),
                                     Vec3(lam,  iot, -kap),
                                     Vec3(-lam, -iot, -kap),
                                     //----------------------------
                                     Vec3(-muu,  nuu,  xii),
                                     Vec3(muu, -nuu,  xii),
                                     Vec3(muu,  nuu, -xii),
                                     Vec3(-muu, -nuu, -xii),
                                     Vec3(-nuu,  xii,  muu),  //bca
                                     Vec3(nuu, -xii,  muu),
                                     Vec3(nuu,  xii, -muu),
                                     Vec3(-nuu, -xii, -muu),
                                     Vec3(-xii,  muu,  nuu),  //cab
                                     Vec3(xii, -muu,  nuu),
                                     Vec3(xii,  muu, -nuu),
                                     Vec3(-xii, -muu, -nuu) /**/};
#undef xii
#undef nuu
#undef muu
#undef lam
#undef kap
#undef iot
#undef the
#undef eta
#undef zet
#undef eps
#undef del
#undef gam
#undef tbe
#undef tal
#undef oot
#undef bet
#undef alp
#undef sqg
#undef tau
}// namespace ArchimedeanSolids

//---------------------------------------------------------------- Catlan Solids
namespace CatlanSolids
{
//--------------------------------------------------------- Rhombic Dodecahedron
const Vec3 cRhombicDodecahedron[14] = { Vec3(one,  one,  one),
                                        Vec3(one,  one, -one),
                                        Vec3(one, -one,  one),
                                        Vec3(one, -one, -one),
                                        Vec3(-one,  one,  one),
                                        Vec3(-one,  one, -one),
                                        Vec3(-one, -one,  one),
                                        Vec3(-one, -one, -one),
                                        Vec3(zero, zero,  two),
                                        Vec3(zero, zero, -two),
                                        Vec3(zero,  two, zero),
                                        Vec3(zero, -two, zero),
                                        Vec3(two, zero, zero),
                                        Vec3(-two, zero, zero) };
}// namespace CatlanSolids

#undef three
#undef two
#undef one
#undef zero

}// namespace Geometry
