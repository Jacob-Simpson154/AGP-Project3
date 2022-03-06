#pragma once


#include "SimpleMath.h"
#include <algorithm>
DirectX::SimpleMath::Vector3 FloorV3(const DirectX::SimpleMath::Vector3& v)
{
  return  { floorf(v.x), floorf(v.y), floorf(v.z) };
}

// deterministic random function for noise using xz
DirectX::SimpleMath::Vector3 Hash(DirectX::SimpleMath::Vector3 p, float seed = 419.2)
{

  float x = sinf(p.Dot(DirectX::SimpleMath::Vector3(127.1f, 0.0f, 311.7f)));
  float y = sinf(p.Dot(DirectX::SimpleMath::Vector3(269.5f, 0.0f, 183.3f)));
  float z = sinf(p.Dot(DirectX::SimpleMath::Vector3(419.2f, 0.0f, 371.9f)));

  DirectX::SimpleMath::Vector3 a = DirectX::SimpleMath::Vector3(x, y, z) * seed;
  DirectX::SimpleMath::Vector3 b = FloorV3(a);

  // between 0-1
  return a - b;
}
// std::clamp available since C++17
float Clamp(float a, float low, float high)
{
  float r = a;
  if (a < low)
  {
    r = low;
  }
  else
  {
    if (high < a)
    {
      r = high;
    }
  }
}

float Smoothstep(float a, float low, float high)
{
  return Clamp(a * a * (3.0f-2.0f * a),0.0f,1.0f);
}

// noise function to grid y
float CalcTerrainHeight(const DirectX::SimpleMath::Vector3& pos, float noiseScale, float voronoi, float smooth)
{
  const DirectX::SimpleMath::Vector3 p = pos * noiseScale;
  const DirectX::SimpleMath::Vector3 f = FloorV3(p);

  const float k = 1.0f + 63.0f * pow(1.0 - smooth, 4.0);

  float va = 0.0f;
  float wt = 0.0f;
  for (int j = -2; j <= 2; j++)
  {
    for (int i = -2; i <= 2; i++)
    {
      
      // neighbour offset
      const DirectX::SimpleMath::Vector2 g = const DirectX::SimpleMath::Vector3((float)i, 1.0f,(float)j);
      const DirectX::SimpleMath::Vector3 o = Hash(p + g)* DirectX::SimpleMath::Vector3(voronoi, 1.0f, voronoi);
      const DirectX::SimpleMath::Vector2 r = g - DirectX::SimpleMath::Vector2(f.x,f.z) + DirectX::SimpleMath::Vector2(o.x, o.z);
      const float d = r.Dot(r);
      float ww = pow(1.0f - Smoothstep(sqrt(d), 0.0f, 1.414f), k);
      va += o.y * ww;
      wt += ww;
    }
  }

  // return height
  return va / wt;
}

// produce grid using Geom generator


// generate points on surface for obsticles

