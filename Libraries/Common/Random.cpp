// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

using Zero::String;

namespace Math
{

uint Random::mGlobalSeed = 1299827;

Random::Random(void)
{
  // we need something with higher precision than seconds (such as time(0)),
  // so use clock which is the number of clock ticks since the program was
  // executed.
  mSeed = (uint)Zero::Time::GenerateSeed();
  mSeed ^= mGlobalSeed;

  // Call next once just to jumble it up a bit
  Next();
}

Random::Random(int initialSeed) : mSeed(initialSeed)
{
  //
}

uint Random::GetSeed()
{
  return mSeed;
}

void Random::SetSeed(uint seed)
{
  mSeed = seed;
}

uint Random::Next(void)
{
  mSeed = 214013 * mSeed + 2531011;
  mGlobalSeed = mSeed;
  return (mSeed >> 16) & cRandMax;
}

u32 Random::Uint32(void)
{
  return (Next() << 16) | Next();
}

u64 Random::Uint64(void)
{
  u64 a = Uint32();
  a = a << 32;
  a |= Uint32();
  return a;
}

float Random::Float(void)
{
  return Next() / float(cRandMax);
}

double Random::Double(void)
{
  return Uint64() / double(cRandMax64);
}

bool Random::Bool(void)
{
  return IntRangeInIn(0, 1) == 1;
}

int Random::IntRangeInIn(int min, int max)
{
  if (min > max)
  {
    String msg = String::Format("The min value '%d' must be less than or equal to the max value '%d'", min, max);
    // DoNotifyException("Invalid range", msg);
    return min;
  }

  ErrorIf(min > max, "Invalid range.");
  int range = max - min;
  return int(Uint32() % (range + 1)) + min;
}

int Random::IntRangeInEx(int min, int max)
{
  if (min >= max)
  {
    String msg = String::Format("The min value '%d' must be less than the max value '%d'", min, max);
    // DoNotifyException("Invalid range", msg);
    return min;
  }

  ErrorIf(min > max, "Invalid range.");
  int range = max - min;
  return (Next() % range) + min;
}

int Random::IntVariance(int base, int variance)
{
  if (variance < 0)
  {
    String msg = String::Format("The variance value '%d' cannot be negative.", variance);
    // DoNotifyException("Invalid variance", msg);
    return base;
  }

  return IntRangeInIn(base - variance, base + variance);
}

float Random::FloatRange(float min, float max)
{
  ErrorIf(min > max, "Invalid range.");
  float range = max - min;
  return (Float() * range) + min;
}

double Random::DoubleRange(double min, double max)
{
  ErrorIf(min > max, "Invalid range.");
  double range = max - min;
  return (Double() * range) + min;
}

float Random::FloatVariance(float base, float variance)
{
  // Map from [0.0f, 1.0f] to [-0.5f, 0.5f]
  float val = Float() - 0.5f;

  // Map from [-0.5f, 0.5f] to [-variance, variance)
  val *= variance * 2.0f;

  // Map from [base - variance, base + variance]
  val += base;

  return val;
}

double Random::DoubleVariance(double base, double variance)
{
  // Map from [0.0, 1.0] to [-0.5, 0.5]
  double val = Double() - 0.5;

  // Map from [-0.5, 0.5] to [-variance, variance)
  val *= variance * 2.0;

  // Map from [base - variance, base + variance]
  val += base;

  return val;
}

Vector2 Random::PointOnUnitCircle(void)
{
  real angle = real(float(Math::cTwoPi) * Float());
  return Vector2(Math::Cos(angle), Math::Sin(angle));
}

// Returns a point on a unit circle with the x-axis going through the center
// of the circle.
Vector3 Random::PointOnUnitCircleX(void)
{
  Vector2 point = PointOnUnitCircle();
  return Vector3(real(0.0), point.y, point.x);
}

// Returns a point on a unit circle with the y-axis going through the center
// of the circle.
Vector3 Random::PointOnUnitCircleY(void)
{
  Vector2 point = PointOnUnitCircle();
  return Vector3(point.x, real(0.0), point.y);
}

// Returns a point on a unit circle with the z-axis going through the center
// of the circle.
Vector3 Random::PointOnUnitCircleZ(void)
{
  Vector2 point = PointOnUnitCircle();
  return Vector3(point.x, point.y, real(0.0));
}

Vector3 Random::PointOnUnitSphere(void)
{
  Vector3 v = Vector3(real(0.0), real(0.0), FloatRange(real(-1.0), real(1.0)));
  const real t = FloatRange(-cPi, cPi);
  const real radius = Sqrt(real(1.0) - (v.z * v.z));
  v.x = Cos(t) * radius;
  v.y = Sin(t) * radius;
  return Normalized(v);
}

Vector3 Random::PointInUnitSphere(void)
{
  real cubeRoot = real(1.0 / 3.0);
  real scalar = Float();
  return PointOnUnitSphere() * Math::Pow(scalar, cubeRoot);
}

// Generate uniform random quaternion
Quaternion Random::RotationQuaternion(void)
{
  // This algorithm generates a Gaussian deviate for each coordinate, so the
  // total effect is to generate a symmetric 4-D Gaussian distribution, by
  // separability. Projecting onto the surface of the hypersphere gives a
  // uniform distribution
  float x = FloatRange(-1.0f, 1.0f);
  float y = FloatRange(-1.0f, 1.0f);
  float z = FloatRange(-1.0f, 1.0f);
  float w = FloatRange(-1.0f, 1.0f);

  float s1;
  while ((s1 = x * x + y * y) > 1.0f)
  {
    x = FloatRange(-1.0f, 1.0f);
    y = FloatRange(-1.0f, 1.0f);
  }

  float s2;
  while ((s2 = z * z + w * w) > 1.0f)
  {
    z = FloatRange(-1.0f, 1.0f);
    w = FloatRange(-1.0f, 1.0f);
  }

  // Now the point (x, y) is uniformly distributed in the unit disk, so is the
  // point (z, w), independently
  float num1 = -2.0f * float(Math::Log(s1));
  float num2 = -2.0f * float(Math::Log(s2));

  // Now "x * sqrt(num1 / s2)" is Gaussian distributed, using polar method.
  // Similarly for y, z, and w, and all are independent
  float r = num1 + num2; // Sum of squares of four Gaussians
  float root1 = float(Math::Sqrt(num1 / (s1 * r)));
  float root2 = float(Math::Sqrt(num2 / (s2 * r)));

  // Normalizing onto unit sphere gives uniform unit quaternion
  return Quaternion(x * root1, y * root1, z * root2, w * root2);
}

Vector2 Random::ScaledVector2(float minLength, float maxLength)
{
  minLength = Math::Abs(minLength);
  maxLength = Math::Abs(maxLength);

  real size = maxLength - minLength;
  real scalar = Float();
  return PointOnUnitCircle() * (minLength + size * Math::Sqrt(scalar));
}

Vector3 Random::ScaledVector3(float minLength, float maxLength)
{
  minLength = Math::Abs(minLength);
  maxLength = Math::Abs(maxLength);

  real cubeRoot = 1.0f / 3.0f;
  real size = maxLength - minLength;
  real scalar = Float();
  return PointOnUnitSphere() * (minLength + size * Math::Pow(scalar, cubeRoot));
}

// Generate uniform random matrix
Matrix3 Random::RotationMatrix(void)
{
  Matrix3 matrix;
  RotationMatrix(&matrix);
  return matrix;
}

void Random::RotationMatrix(Mat3Ptr matrix)
{
  ErrorIf(matrix == nullptr, "Invalid matrix provided.");
  Quaternion quat = RotationQuaternion();
  ToMatrix3(quat, matrix);
}

int Random::DieRoll(uint sides)
{
  if (sides == 0)
  {
    // DoNotifyException("Invalid die roll", "Cannot roll a zero sided die");
    return 0;
  }
  return IntRangeInEx(0, sides) + 1;
}

float Random::BellCurve(float center, float range, float standardDeviation)
{
  float randVal = 0.0f;
  do
  {
    // This is the box-muller normal distribution algorithm.
    float u1, u2, s;
    do
    {
      // get two random floats in the range [-1,1]
      u1 = (Float() - .5f) * 2.0f;
      u2 = (Float() - .5f) * 2.0f;
      s = u1 * u1 + u2 * u2;
    } while (s >= 1.0f || s == 0.0f);

    // this technically generates 2 random for every call but we're only saving
    // one now. the other one generated is u2 * multiplier
    float multiplier = Math::Sqrt(-2.0f * Math::Log(s) / s);
    randVal = u1 * multiplier;

    // alter the standard deviation
    randVal *= standardDeviation;
    // Since the user gave us a range, restart if we don't have a value within
    // the valid range. a normal distribution can return a value at an infinite
    // range technically, and clamping the value will alter our probability.
    // Restarting is the only way to properly deal with this.
  } while (randVal < -3.0f || randVal > 3.0f);

  // shift over to the center and remap the [-3,3] range to [center - range,
  // center + range]
  randVal = randVal * (range / 3.0f) + center;

  return randVal;
}

} // namespace Math
