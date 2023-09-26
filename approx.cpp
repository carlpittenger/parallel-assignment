#include <cmath>
#include <cstdlib>

[[nodiscard]] auto main(const int argc, const char *const *const argv) noexcept
    -> int {
  if (argc != 3) {
    return EXIT_FAILURE;
  }

  // atof is an unsafe function
  // really should use strtof and check error
  const auto a = std::atof(argv[1]);
  const auto b = std::atof(argv[2]);

  return std::fabs(a - b) > 0.1;
}
