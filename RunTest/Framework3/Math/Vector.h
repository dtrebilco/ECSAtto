
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

#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <cstdint>

#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp"
#include "../glm/gtc/type_ptr.hpp"

typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;

typedef glm::mat2 mat2;
typedef glm::mat3 mat3;
typedef glm::mat4 mat4;
typedef glm::mat4x3 mat4x3;

typedef glm::vec2 float2;
typedef glm::vec3 float3;
typedef glm::vec4 float4;

typedef glm::quat quat;

using glm::normalize;
using glm::dot;
using glm::cross;
using glm::clamp;
using glm::mix;
using glm::length;
using glm::transpose;
using glm::inverse;
using glm::value_ptr;
using glm::min;
using glm::max;

#define lerp(a,b,x) glm::mix(a,b,x)
#define saturate(x) clamp(x, 0.0f, 1.0f)
#define PI 3.14159265358979323846f

#include <limits.h>
#include <float.h>

struct half {
	unsigned short sh;

	half(){};
    half(const float x);
	operator float () const;
};

float planeDistance(const vec3 &normal, const float offset, const vec3 &point);
float planeDistance(const vec4 &plane, const vec3 &point);

// Extract the projection planes from the projection matrix
void getProjectionPlanes(const mat4 & a_proj, vec4 a_retPlanes[6]);

// Inverse transforn the passed planes (eg. view -> model space)
void planeInvTransform(const mat4 & a_modelView, vec4 * a_retPlanes, int a_planeCount);

// Normalize rge planes
void planeNormalize(vec4 * a_retPlanes, int a_planeCount);

// Test if the passed box is in (or intersecting) the given frustum planes (6 planes) 
bool testAABBFrustumPlanes(const vec4 *__restrict a_planes, const vec3& a_boxCenter, const vec3& a_boxExtents);

// Test if the point is in the AABB
bool testAABBPoint(const vec3& a_boxCenter, const vec3& a_boxExtents, const vec3 & a_point);

// Find the closest point to the AABB from the given point
vec3 findAABBClosestPoint(const vec3 & a_point, const vec3& a_boxCenter, const vec3& a_boxExtents);

// Find the camera position from modelView matrix (in modelspace)
vec3 findPositionFromTransformMatrix(const mat4 & a_modelView);

vec3 rgbeToRGB(unsigned char *rgbe);
unsigned int rgbToRGBE8(const vec3 &rgb);
unsigned int rgbToRGB9E5(const vec3 &rgb);

mat4 rotateX(const float angle);
mat4 rotateY(const float angle);
mat4 rotateZ(const float angle);
mat4 rotateXY(const float angleX, const float angleY);
mat4 translate(const vec3 &v);
mat4 translate(const float x, const float y, const float z);
mat4 scale(const float x, const float y, const float z);
mat4 perspectiveMatrixX(const float fov, const int width, const int height, const float zNear, const float zFar);

#endif
