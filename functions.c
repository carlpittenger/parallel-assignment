#include <math.h>
#include <stdbool.h>

float f1(float x, const int intensity) {
  const bool sign = x > 0;

  for (int i = 0; i < intensity; ++i) {
    x = sqrt(x * x);
  }
  return sign ? x : -x;
}

float f2(const float x, const int intensity) {
  const float real_x = f1(x, intensity);
  return real_x * real_x;
}

float f3(const float x, const int intensity) { return sin(f1(x, intensity)); }

float f4(const float x, const int intensity) {
  return exp(cos(f1(x, intensity)));
}
