#ifndef RTRACE_H
#define RTRACE_H

#include <cmath>
#include <iomanip>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#define TIME 0.025f

#define EPSILON 0.001f
#define PI 3.141592f
#define TAU PI * 2.0f

#define GRAD_STEP 0.02f
#define MARCH_STEPS 128

#define CLIP_NEAR 0.1f
#define CLIP_FAR 1000.0f

#define SHADES "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. "

using namespace glm;

namespace sdf {
  float sphere(vec3 p, float r) {
    return length(p) - r;
  }

  float cube(vec3 p, vec3 b) {
    vec3 d = abs(p) - b;
    return std::min(std::max(d.x, std::max(d.y, d.z)), 0.0f) + length(std::max(d.x, std::max(d.y, d.z)));
  }

  float intersect(float d0, float d1) {
    return std::max(d0, d1);
  }

  float distance(vec3 p) {
    float ds = sphere(p + vec3(0.0f, 0.0f, 0.0f), 0.8f);
    float db = cube(p + vec3(0.0f, 0.0f, 0.0f), { 0.7f, 0.7f, 0.7f });

    float d = intersect(ds, db);

    return d;
  }
}

vec3 gradient(vec3 p) {
  vec3 dx = { GRAD_STEP, 0.0f, 0.0f };
  vec3 dy = { 0.0f, GRAD_STEP, 0.0f };
  vec3 dz = { 0.0f, 0.0f, GRAD_STEP };

  vec3 g = {
    sdf::distance(p + dx) - sdf::distance(p - dx),
    sdf::distance(p + dy) - sdf::distance(p - dy),
    sdf::distance(p + dz) - sdf::distance(p - dz),
  };

  return normalize(g);
}

bool trace(vec3 o, vec3 d, vec3 & n) {
  float depth = 1.0f;

  for (int i = 0; i < MARCH_STEPS; i++) {
    vec3 odd = o + d * depth;

    float dist = sdf::distance(odd);

    if (dist < EPSILON) {
      n = gradient(odd);

      return true;
    };

    depth += dist;

    if (depth >= CLIP_FAR) return false;
  }

  return false;
}

vec3 r = { 0.0f, 0.0f, 0.0f };

bool start() { return true; }

bool update() {
  if (rtt::ctx->event->value == EV_KEY)
    if (rtt::ctx->event->code == KEY_Q)
      return false; 

  if (r.x >= TAU) r.x = 0.0f;
  if (r.y >= TAU) r.y = 0.0f;
  if (r.z >= TAU) r.z = 0.0f;

  r.x += TIME * 0.6f;
  r.y += TIME * 0.4f;
  r.z += TIME * 0.2f;

  mat4 xm = rotate(mat4(1.0f), r.x, { 1.0f, 0.0f, 0.0f });
  mat4 ym = rotate(xm, r.y, { 0.0f, 1.0f, 0.0f });
  mat4 zm = rotate(ym, r.z, { 0.0f, 0.0f, 1.0f });

  for (int i = 0; i < rtt::w; i++)
    for (int j = 0; j < rtt::h; j++) {
      float jj = j * 2.0f - rtt::h / 2;

      float a = static_cast<float>(rtt::w) / rtt::h;

      float x = (i / (rtt::w / 2.0f) - 1.0f) * a * 0.5f;
      float y = (jj / (rtt::h / 2.0f) - 1.0f) * 0.5f;

      vec3 o = zm * vec4(x, y, -10.0f, 1.0f);
      vec3 d = zm * vec4(0.0f, 0.0f, 1.0f, 1.0f);
      vec3 n;

      rtt::ctx->buffer[i + j * rtt::w] = ' ';

      if (!trace(o, d, n)) continue;

      vec3 nn = (n + 1.0f) / 2.0f; 

      float c = (nn.x * 0.3f) + (nn.y * 0.59f) + (nn.z * 0.11f);

      char ch = SHADES[static_cast<int>(std::floor(c * 70))];

      rtt::ctx->buffer[i + j * rtt::w] = ch;
    }

  return true;
}

#endif
