#pragma once

#include "../Framework3/glm/glm.hpp"
#include "../Framework3/glm/gtc/quaternion.hpp"

typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;
typedef glm::quat quat;

typedef glm::mat3 mat3;
typedef glm::mat4 mat4;
typedef glm::mat4x3 mat4x3;

inline mat4 CalculateTransform4x4(const vec3& i_pos, const quat& i_rot, const vec3& i_scale)
{
  //mat4 modelWorld = mat4(1.0f);
  //modelWorld = glm::translate(modelWorld, i_pos);
  //modelWorld *= glm::mat4_cast(i_rot);
  //modelWorld = glm::scale(modelWorld, i_scale);

  mat4 modelWorld;
  const vec3& scale = i_scale;
  const quat& q = i_rot;

  vec3 scale2 = scale * 2.0f;

  float qxx(q.x * q.x);
  float qyy(q.y * q.y);
  float qzz(q.z * q.z);
  float qxz(q.x * q.z);
  float qxy(q.x * q.y);
  float qyz(q.y * q.z);
  float qwx(q.w * q.x);
  float qwy(q.w * q.y);
  float qwz(q.w * q.z);

  modelWorld[0][0] = scale.x - scale2.x * (qyy + qzz);
  modelWorld[0][1] = scale2.x * (qxy + qwz);
  modelWorld[0][2] = scale2.x * (qxz - qwy);
  modelWorld[0][3] = 0.0f;

  modelWorld[1][0] = scale2.y * (qxy - qwz);
  modelWorld[1][1] = scale.y - scale2.y * (qxx + qzz);
  modelWorld[1][2] = scale2.y * (qyz + qwx);
  modelWorld[1][3] = 0.0f;

  modelWorld[2][0] = scale2.z * (qxz + qwy);
  modelWorld[2][1] = scale2.z * (qyz - qwx);
  modelWorld[2][2] = scale.z - scale2.z * (qxx + qyy);
  modelWorld[2][3] = 0.0f;

  modelWorld[3] = vec4(i_pos, 1.0f);
  return modelWorld;
}

inline mat4x3 CalculateTransform4x3(const vec3& i_pos, const quat& i_rot, const vec3& i_scale)
{
  mat4x3 modelWorld;

  const vec3& scale = i_scale;
  const quat& q = i_rot;

  vec3 scale2 = scale * 2.0f;

  float qxx(q.x * q.x);
  float qyy(q.y * q.y);
  float qzz(q.z * q.z);
  float qxz(q.x * q.z);
  float qxy(q.x * q.y);
  float qyz(q.y * q.z);
  float qwx(q.w * q.x);
  float qwy(q.w * q.y);
  float qwz(q.w * q.z);

  modelWorld[0][0] = scale.x - scale2.x * (qyy + qzz);
  modelWorld[0][1] = scale2.x * (qxy + qwz);
  modelWorld[0][2] = scale2.x * (qxz - qwy);

  modelWorld[1][0] = scale2.y * (qxy - qwz);
  modelWorld[1][1] = scale.y - scale2.y * (qxx + qzz);
  modelWorld[1][2] = scale2.y * (qyz + qwx);

  modelWorld[2][0] = scale2.z * (qxz + qwy);
  modelWorld[2][1] = scale2.z * (qyz - qwx);
  modelWorld[2][2] = scale.z - scale2.z * (qxx + qyy);

  modelWorld[3] = i_pos;

  return modelWorld;
}

inline mat4x3 CalculateTransform4x3(const vec3& i_pos, const quat& i_rot)
{
  mat4x3 modelWorld;

  const quat& q = i_rot;

  float qxx(q.x * q.x);
  float qyy(q.y * q.y);
  float qzz(q.z * q.z);
  float qxz(q.x * q.z);
  float qxy(q.x * q.y);
  float qyz(q.y * q.z);
  float qwx(q.w * q.x);
  float qwy(q.w * q.y);
  float qwz(q.w * q.z);

  modelWorld[0][0] = 1.0f - 2.0f * (qyy + qzz);
  modelWorld[0][1] = 2.0f * (qxy + qwz);
  modelWorld[0][2] = 2.0f * (qxz - qwy);

  modelWorld[1][0] = 2.0f * (qxy - qwz);
  modelWorld[1][1] = 1.0f - 2.0f * (qxx + qzz);
  modelWorld[1][2] = 2.0f * (qyz + qwx);

  modelWorld[2][0] = 2.0f * (qxz + qwy);
  modelWorld[2][1] = 2.0f * (qyz - qwx);
  modelWorld[2][2] = 1.0f - 2.0f * (qxx + qyy);

  modelWorld[3] = i_pos;

  return modelWorld;
}

inline mat4x3 ApplyScale(const mat4x3& i_mat, const vec3& i_scale)
{
  return mat4x3(i_mat[0] * i_scale[0],
                i_mat[1] * i_scale[1],
                i_mat[2] * i_scale[2],
                i_mat[3]);
}
