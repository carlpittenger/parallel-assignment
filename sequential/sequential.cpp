#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>

extern "C" {
float sequential_integrate(int argc, const char *const *argv);
}

[[nodiscard]] auto main(const int argc, const char *const *const argv) noexcept
    -> int {
  if (argc < 6) {
    std::cerr << "Usage: " << argv[0]
              << " <function_id> <a> <b> <n> <intensity>\n";
    return EXIT_FAILURE;
  }

  const auto start = std::chrono::system_clock::now();

  const auto integrate = sequential_integrate(argc, argv);

  const auto end = std::chrono::system_clock::now();

  const auto elapsed_seconds = end - start;

  std::cout << integrate << '\n';

  std::cerr << elapsed_seconds.count() << '\n';
}
