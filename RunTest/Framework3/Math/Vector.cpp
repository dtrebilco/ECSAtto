
/* * * * * * * * * * * * * Author's note * * * * * * * * * * * *\
*   _       _   _       _   _       _   _       _     _ _ _ _   *
*  |_|     |_| |_|     |_| |_|_   _|_| |_|     |_|  _|_|_|_|_|  *
*  |_|_ _ _|_| |_|     |_| |_|_|_|_|_| |_|     |_| |_|_ _ _     *
*  |_|_|_|_|_| |_|     |_| |_| |_| |_| |_|     |_|   |_|_|_|_   *
*  |_|     |_| |_|_ _ _|_| |_|     |_| |_|_ _ _|_|  _ _ _ _|_|  *
*  |_|     |_|   |_|_|_|   |_|     |_|   |_|_|_|   |_|_|_|_|    *
*                                                               *
*                     http://www.humus.name                     *
*                                                                *
* This file is a part of the work done by Humus. You are free to   *
* use the code in any way you like, modified, unmodified or copied   *
* into your own work. However, I expect you to respect these points:  *
*  - If you use this file and its contents unmodified, or use a major *
*    part of this file, please credit the author and leave this note. *
*  - For use in anything commercial, please request my approval.     *
*  - Share your work and ideas too as much as you can.             *
*                                                                *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "Vector.h"

half::half(const float x){
  union {
    float floatI;
    unsigned int i;
  };
  floatI = x;

//	unsigned int i = *((unsigned int *) &x);
  int e = ((i >> 23) & 0xFF) - 112;
  int m =  i & 0x007FFFFF;

  sh = (i >> 16) & 0x8000;
  if (e <= 0){
    // Denorm
    m = ((m | 0x00800000) >> (1 - e)) + 0x1000;
    sh |= (m >> 13);
  } else if (e == 143){
    sh |= 0x7C00;
    if (m != 0){
      // NAN
      m >>= 13;
      sh |= m | (m == 0);
    }
  } else {
    m += 0x1000;
    if (m & 0x00800000){
      // Mantissa overflow
      m = 0;
      e++;
    }
    if (e >= 31){
      // Exponent overflow
      sh |= 0x7C00;
    } else {
      sh |= (e << 10) | (m >> 13);
    }
  }
}

half::operator float () const {
  union {
    unsigned int s;
    float result;
  };

  s = (sh & 0x8000) << 16;
  unsigned int e = (sh >> 10) & 0x1F;
  unsigned int m = sh & 0x03FF;

  if (e == 0){
    // +/- 0
    if (m == 0) return result;

    // Denorm
    while ((m & 0x0400) == 0){
      m += m;
      e--;
    }
    e++;
    m &= ~0x0400;
  } else if (e == 31){
    // INF / NAN
    s |= 0x7F800000 | (m << 13);
    return result;
  }

  s |= ((e + 112) << 23) | (m << 13);

  return result;
}

float planeDistance(const vec3 &normal, const float offset, const vec3 &point){
    return point.x * normal.x + point.y * normal.y + point.z * normal.z + offset;
}

float planeDistance(const vec4 &plane, const vec3 &point){
    return point.x * plane.x + point.y * plane.y + point.z * plane.z + plane.w;
}

vec3 rgbeToRGB(unsigned char *rgbe){
  if (rgbe[3]){
    return vec3(rgbe[0], rgbe[1], rgbe[2]) * ldexpf(1.0f, rgbe[3] - (int) (128 + 8));
  } else return vec3(0, 0, 0);
}

unsigned int rgbToRGBE8(const vec3 &rgb){
  float v = max(rgb.x, rgb.y);
  v = max(v, rgb.z);

  if (v < 1e-32f){
    return 0;
  } else {
    int ex;
    float m = frexpf(v, &ex) * 256.0f / v;

    unsigned int r = (unsigned int) (m * rgb.x);
    unsigned int g = (unsigned int) (m * rgb.y);
    unsigned int b = (unsigned int) (m * rgb.z);
    unsigned int e = (unsigned int) (ex + 128);

    return r | (g << 8) | (b << 16) | (e << 24);
  }
}

unsigned int rgbToRGB9E5(const vec3 &rgb){
  float v = max(rgb.x, rgb.y);
  v = max(v, rgb.z);

  if (v < 1.52587890625e-5f){
    return 0;
  } else if (v < 65536){
    int ex;
    float m = frexpf(v, &ex) * 512.0f / v;

    unsigned int r = (unsigned int) (m * rgb.x);
    unsigned int g = (unsigned int) (m * rgb.y);
    unsigned int b = (unsigned int) (m * rgb.z);
    unsigned int e = (unsigned int) (ex + 15);

    return r | (g << 9) | (b << 18) | (e << 27);
  } else {
    unsigned int r = (rgb.x < 65536)? (unsigned int) (rgb.x * (1.0f / 128.0f)) : 0x1FF;
    unsigned int g = (rgb.y < 65536)? (unsigned int) (rgb.y * (1.0f / 128.0f)) : 0x1FF;
    unsigned int b = (rgb.z < 65536)? (unsigned int) (rgb.z * (1.0f / 128.0f)) : 0x1FF;
    unsigned int e = 31;

    return r | (g << 9) | (b << 18) | (e << 27);
  }
}


/* --------------------------------------------------------------------------------- */


mat4 rotateX(const float angle){
  float cosA = cosf(angle), sinA = sinf(angle);
  
  return mat4(
    1, 0,     0,    0,
    0, cosA,  sinA, 0,
    0, -sinA, cosA, 0,
    0, 0,     0,    1);
}

mat4 rotateY(const float angle){
  float cosA = cosf(angle), sinA = sinf(angle);

  return mat4(
    cosA,  0,  sinA, 0,
    0,     1,  0,    0,
    -sinA, 0,  cosA, 0,
    0,     0,  0,    1);
}

mat4 rotateZ(const float angle){
  float cosA = cosf(angle), sinA = sinf(angle);

  return mat4(
    cosA,  sinA, 0, 0,
    -sinA, cosA, 0, 0,
    0,     0,    1, 0,
    0,     0,    0, 1);
}

mat4 rotateXY(const float angleX, const float angleY){
  float cosX = cosf(angleX), sinX = sinf(angleX), 
        cosY = cosf(angleY), sinY = sinf(angleY);

  return mat4(
     cosY,       -sinX * sinY,    cosX * sinY,  0,
     0,           cosX,           sinX,         0,
    -sinY,        -sinX * cosY,   cosX * cosY,  0,
     0,           0,               0,           1);
}

mat4 translate(const vec3 &v){
  return translate(v.x, v.y, v.z);
}

mat4 translate(const float x, const float y, const float z){
  return mat4(1,0,0,0, 0,1,0,0, 0,0,1,0, x,y,z,1);
}

mat4 scale(const float x, const float y, const float z){
  return mat4(x,0,0,0, 0,y,0,0, 0,0,z,0, 0,0,0,1);
}

mat4 perspectiveMatrixX(const float fov, const int width, const int height, const float zNear, const float zFar){
  float w = cosf(0.5f * fov) / sinf(0.5f * fov);
  float h = (w * width) / height;

  return mat4(
    w, 0, 0, 0,
    0, h, 0, 0,
    0, 0, (zFar + zNear) / (zFar - zNear), 1,
    0, 0, -(2 * zFar * zNear) / (zFar - zNear), 0);
}


void getProjectionPlanes(const mat4 & a_proj, vec4 a_retPlanes[6])
{
  // Get the projection planes in view space (not normalized yet)
  mat4 trans = transpose(a_proj);
  a_retPlanes[0] = (trans[3] + trans[0]); // Left
  a_retPlanes[1] = (trans[3] - trans[0]); // Right

  a_retPlanes[2] = (trans[3] + trans[1]); // Bottom
  a_retPlanes[3] = (trans[3] - trans[1]); // Top

  a_retPlanes[4] = (trans[3] + trans[2]); // Near
  a_retPlanes[5] = (trans[3] - trans[2]); // Far
}


void planeInvTransform(const mat4 & a_modelView, vec4 * a_retPlanes, int a_planeCount)
{
  // Transform the plane by the model view transpose
  // NOTE: Planes have to be transformed by the inverse transpose of a matrix
  //       Nice reference here: http://www.opengl.org/discussion_boards/showthread.php/159564-Clever-way-to-transform-plane-by-matrix
  //
  //       So for a transform to model space we need to do:
  //            inverse(transpose(inverse(MV)))
  //       This equals : transpose(MV) - see Lemma 5 in http://mathrefresher.blogspot.com.au/2007/06/transpose-of-matrix.html
  mat4 modelViewT = transpose(a_modelView);
  for(int i = 0; i < a_planeCount; i++)
  {
    a_retPlanes[i] = modelViewT * a_retPlanes[i];
  }
}


void planeNormalize(vec4 * a_retPlanes, int a_planeCount)
{
  for(int i = 0; i < a_planeCount; i++)
  {
    float invLength = 1.0f / glm::length(vec3(a_retPlanes[i]));
    a_retPlanes[i] = a_retPlanes[i] * invLength;
  }
}


bool testAABBFrustumPlanes(const vec4 *__restrict a_planes, const vec3& a_boxCenter, const vec3& a_boxExtents)
{
  // Do bit tests?
  // Perhaps update to do this?
  // http://www.gamedev.net/page/resources/_/technical/general-programming/useless-snippet-2-aabbfrustum-test-r3342
  // http://fgiesen.wordpress.com/2010/10/17/view-frustum-culling/
  // http://iquilezles.org/www/articles/frustumcorrect/frustumcorrect.htm
  
  for (uint32_t i = 0; i < 6; i++)
  {
    vec4 curPlane = a_planes[i];
    float distToCenter = planeDistance(curPlane, a_boxCenter);
    float radiusBoxAtPlane = dot(a_boxExtents, glm::abs(vec3(curPlane)));
    if (distToCenter < -radiusBoxAtPlane)
    {
      return false; // Box is entirely behind at least one plane
    }
    //else if (distToCenter <= radiusBoxAtPlane) // If spanned (not entirely infront)
    //{
    //}
  }

  return true;
}


bool testAABBPoint(const vec3& a_boxCenter, const vec3& a_boxExtents, const vec3 & a_point)
{
  return (fabsf(a_point.x - a_boxCenter.x) <= a_boxExtents.x &&
          fabsf(a_point.y - a_boxCenter.y) <= a_boxExtents.y &&
          fabsf(a_point.z - a_boxCenter.z) <= a_boxExtents.z);

  //vec3 min = a_boxCenter - a_boxExtents;
  //vec3 max = a_boxCenter + a_boxExtents;
  //
  //return (a_point.x >= min.x && a_point.x <= max.x && 
  //        a_point.y >= min.y && a_point.y <= max.y && 
  //        a_point.z >= min.z && a_point.z <= max.z);
}


vec3 findAABBClosestPoint(const vec3 & a_point, const vec3& a_boxCenter, const vec3& a_boxExtents)
{
  vec3 retPoint = a_point;

  retPoint = max(retPoint, a_boxCenter - a_boxExtents);
  retPoint = min(retPoint, a_boxCenter + a_boxExtents);

  return retPoint;
}


vec3 findPositionFromTransformMatrix(const mat4 & a_modelView)
{
  // Get the 3 basis vector planes at the camera origin and transform them into model space.
  //  
  // NOTE: Planes have to be transformed by the inverse transpose of a matrix
  //       Nice reference here: http://www.opengl.org/discussion_boards/showthread.php/159564-Clever-way-to-transform-plane-by-matrix
  //
  //       So for a transform to model space we need to do:
  //            inverse(transpose(inverse(MV)))
  //       This equals : transpose(MV) - see Lemma 5 in http://mathrefresher.blogspot.com.au/2007/06/transpose-of-matrix.html
  //
  // As each plane is simply (1,0,0,0), (0,1,0,0), (0,0,1,0) we can pull the data directly from the transpose matrix.

  const float * a_m = value_ptr(a_modelView);

  // Get plane normals 
  vec3 n1(a_m[0], a_m[4], a_m[8]);
  vec3 n2(a_m[1], a_m[5], a_m[9]);
  vec3 n3(a_m[2], a_m[6], a_m[10]);

  // Get plane distances
  float d1 = a_m[12];
  float d2 = a_m[13];
  float d3 = a_m[14];

  // Get the intersection of these 3 planes
  // http://paulbourke.net/geometry/3planes/
  vec3 n2n3 = cross(n2, n3);
  vec3 n3n1 = cross(n3, n1);
  vec3 n1n2 = cross(n1, n2);

  vec3 top = (n2n3 * d1) + (n3n1 * d2) + (n1n2 * d3);
  float denom = dot(n1, n2n3);
  //if (denom == 0)
  //{
  //  Bad matrix?
  //}

  float denomScale = -1.0f / denom;
  
  return top * denomScale;
}



