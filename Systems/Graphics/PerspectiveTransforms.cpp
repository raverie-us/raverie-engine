#include "Precompiled.hpp"

namespace Zero
{

void BuildOrthographicTransformZero(Mat4& matrix, float verticalSize, float aspectRatio, float nearDistance, float farDistance)
{
  // Zero maps NDC [-1,-1,-1] to [1,1,1] ([l,b,n] to [r,t,f])
  BuildOrthographicTransformGl(matrix, verticalSize, aspectRatio, nearDistance, farDistance);
}

// Column major formula for NDC [-1,-1,-1] to [1,1,1] ([l,b,n] to [r,t,f])
// | 1/r   0     0             0      |
// |  0   1/t    0             0      |
// |  0    0   -2/(f-n)  -(f+n)/(f-n) |
// |  0    0     0             1      |
void BuildOrthographicTransformGl(Mat4& matrix, float verticalSize, float aspectRatio, float nearDistance, float farDistance)
{
  matrix = Mat4::cIdentity;

  // Near and far distances are expected to be positive
  float depth = farDistance - nearDistance;
  if (depth < Math::Epsilon() || aspectRatio < Math::Epsilon())
    return;

  // t = size/2
  float t = verticalSize * 0.5f;
  // r = t*aspect
  float r = t * aspectRatio;

  matrix.m00 = 1.0f / r;
  matrix.m11 = 1.0f / t;
  matrix.m22 = -2.0f / depth;
  matrix.m23 = -(farDistance + nearDistance) / depth;
}

// Column major formula for NDC [-1,-1,0] to [1,1,1] ([l,b,n] to [r,t,f])
// | 1/r   0     0             0      |
// |  0   1/t    0             0      |
// |  0    0   -1/(f-n)      -n/(f-n) |
// |  0    0     0             1      |
void BuildOrthographicTransformDx(Mat4& matrix, float verticalSize, float aspectRatio, float nearDistance, float farDistance)
{
  matrix = Mat4::cIdentity;

  // Near and far distances are expected to be positive
  float depth = farDistance - nearDistance;
  if (depth < Math::Epsilon() || aspectRatio < Math::Epsilon())
    return;

  // t = size/2
  float t = verticalSize * 0.5f;
  // r = t*aspect
  float r = t * aspectRatio;

  matrix.m00 = 1.0f / r;
  matrix.m11 = 1.0f / t;
  matrix.m22 = -1.0f / depth;
  matrix.m23 = -nearDistance / depth;
}

void BuildPerspectiveTransformZero(Mat4& matrix, float verticalFov, float aspectRatio, float nearDistance, float farDistance)
{
  // Zero maps NDC [-1,-1,-1] to [1,1,1] ([l,b,n] to [r,t,f])
  BuildPerspectiveTransformGl(matrix, verticalFov, aspectRatio, nearDistance, farDistance);
}

// Column major formula for NDC [-1,-1,-1] to [1,1,1] ([l,b,n] to [r,t,f])
// | n/r   0         0           0      |
// |  0   n/t        0           0      |
// |  0    0   -(f+n)/(f-n)  -2fn/(f-n) |
// |  0    0        -1           0      |
void BuildPerspectiveTransformGl(Mat4& matrix, float verticalFov, float aspectRatio, float nearDistance, float farDistance)
{
  matrix = Mat4::cIdentity;

  // Near and far distances are expected to be positive
  float depth = farDistance - nearDistance;
  if (depth < Math::Epsilon() || aspectRatio < Math::Epsilon())
    return;

  // horizontal+ (fixed vertical fov)
  //
  //            /|  |
  //          /  | top
  // fov/2__/    |  |
  //      /_|____|  |
  //      --near--

  // tan(fov/2) = t/n
  // n/t = cot(fov/2)
  float n_t = Math::Cot(verticalFov * 0.5f);

  // r = t*(r/t) = t*aspect
  // n/r = n/(t*aspect) = (n/t)/aspect
  float n_r = n_t / aspectRatio;

  matrix.m00 = n_r;
  matrix.m11 = n_t;
  matrix.m22 = -(farDistance + nearDistance) / depth;
  matrix.m33 = 0.0f;
  matrix.m23 = -2.0f * farDistance * nearDistance / depth;
  matrix.m32 = -1.0f;
}

// Column major formula for NDC [-1,-1,0] to [1,1,1] ([l,b,n] to [r,t,f])
// | n/r   0         0           0      |
// |  0   n/t        0           0      |
// |  0    0       -f/(f-n)   -fn/(f-n) |
// |  0    0        -1           0      |
void BuildPerspectiveTransformDx(Mat4& matrix, float verticalFov, float aspectRatio, float nearDistance, float farDistance)
{
  matrix = Mat4::cIdentity;

  // Near and far distances are expected to be positive
  float depth = farDistance - nearDistance;
  if (depth < Math::Epsilon() || aspectRatio < Math::Epsilon())
    return;

  // horizontal+ (fixed vertical fov)
  //
  //            /|  |
  //          /  | top
  // fov/2__/    |  |
  //      /_|____|  |
  //      --near--

  // tan(fov/2) = t/n
  // n/t = cot(fov/2)
  float n_t = Math::Cot(verticalFov * 0.5f);

  // r = t*(r/t) = t*aspect
  // n/r = n/(t*aspect) = (n/t)/aspect
  float n_r = n_t / aspectRatio;

  matrix.m00 = n_r;
  matrix.m11 = n_t;
  matrix.m22 = -farDistance / depth;
  matrix.m33 = 0.0f;
  matrix.m23 = -farDistance * nearDistance / depth;
  matrix.m32 = -1.0f;
}

} // namespace Zero
