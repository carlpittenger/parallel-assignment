#include <math.h>
#include <stdlib.h>

typedef float (*fn_type)(float, int);

#ifdef __cplusplus
extern "C" {
#endif

float f1(float x, int intensity);
float f2(float x, int intensity);
float f3(float x, int intensity);
float f4(float x, int intensity);

#ifdef __cplusplus
}
#endif

float sequential_integrate(const int argc, const char *const *const argv) {
  const int function_id = atoi(argv[1]);
  const float a = atof(argv[2]);
  const float b = atof(argv[3]);
  const int n = atoi(argv[4]);
  const int intensity = atoi(argv[5]);

  fn_type f = NULL;

  switch (function_id) {
  case 1:
    f = f1;
    break;
  case 2:
    f = f2;
    break;
  case 3:
    f = f3;
    break;
  default:
    // case 4:
    f = f4;
  }

  double sum = 0.0;

  for (int i = 0; i < n; ++i) {
    const float x = a + (i + 0.5) * (b - a) / n;
    const float val = f(x, intensity);

    // std::cout << val << '\n';

    sum += val;
  }

  const float integrate = sum * (b - a) / n;

  return integrate;
}
