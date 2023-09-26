#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>

#include <pthread.h>

int nb_threads;

[[nodiscard]] auto f(void *const arg) noexcept -> void * {
  const auto thread_id = *static_cast<int *>(arg);

  std::cout << "I am thread " << thread_id << " in " << nb_threads
            << " threads\n";

  return nullptr;
}

[[nodiscard]] auto main(const int argc, const char *const *const argv) noexcept
    -> int {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <nb_threads>\n";
    return EXIT_FAILURE;
  }

  nb_threads = std::atoi(argv[1]);

  if (nb_threads <= 0) {
    std::cerr << "Number of threads must be greater than 0\n";
    return EXIT_FAILURE;
  }

  const auto threads = std::make_unique<pthread_t[]>(nb_threads);
  const auto ids = std::make_unique<int[]>(nb_threads);

  // create and run threads
  for (auto i = 0; i < nb_threads; ++i) {
    ids[i] = i;
    if (pthread_create(&threads[i], nullptr, f, &ids[i]) != 0) {
      std::cerr << "Error creating thread " << i << '\n';
      return EXIT_FAILURE;
    }
  }

  // wait for all threads to finish
  for (auto i = 0; i < nb_threads; ++i) {
    if (pthread_join(threads[i], nullptr) != 0) {
      std::cerr << "Error joining thread " << i << '\n';
      return EXIT_FAILURE;
    }
  }
}
