#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <pthread.h>

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

using fn_type = float (*)(float x, int intensity);

// structure to pass thread-specific data
struct ThreadData {
  fn_type fn;
  double a;
  double b;
  int n;
  int intensity;
  int nb_threads;
  std::string sync;
  int thread_id;
  double *local_result;
  // const int start;
  // const int end;
};

// global variables for result and mutual exclusion
auto result = 0.0;
pthread_mutex_t result_mutex;

// numerical integration function
[[nodiscard]] auto thread_fn(void *const arg) noexcept -> void * {
  const auto data = static_cast<const ThreadData *>(arg);
  const auto fn = data->fn;
  const auto a = data->a;
  const auto b = data->b;
  const auto n = data->n;
  const auto intensity = data->intensity;
  const auto nb_threads = data->nb_threads;
  const auto sync = data->sync;
  const auto thread_id = data->thread_id;
  auto &local_result = *(data->local_result);

  // calculate the range of iterations for this thread
  const auto iterations_per_thread = n / nb_threads;
  const auto start = thread_id * iterations_per_thread;
  const auto end = start + iterations_per_thread;

  // perform numerical integration
  for (auto i = start; i < end; ++i) {
    const auto x =
        static_cast<double>(fn(a + (((i + 0.5) * (b - a)) / (n)), intensity));
    if (sync == "iteration") {
      pthread_mutex_lock(&result_mutex);
      result += x;
      pthread_mutex_unlock(&result_mutex);
    }

    // sync == "thread"
    local_result += x;
  }

  return nullptr;
}

[[nodiscard]] auto main(const int argc, const char *const *const argv) noexcept
    -> int {
  if (argc < 8) {
    std::cerr << "Usage: " << argv[0]
              << " <function_id> <a> <b> <n> <intensity> <nb_threads> <sync>\n";
    return EXIT_FAILURE;
  }

  const auto function_id = std::atoi(argv[1]);
  const auto a = std::atof(argv[2]);
  const auto b = std::atof(argv[3]);
  const auto n = std::atoi(argv[4]);
  const auto intensity = std::atoi(argv[5]);
  const auto nb_threads = std::atoi(argv[6]);
  const auto sync = std::string{argv[7]};

  if (nb_threads <= 0) {
    std::cerr << "Number of threads must be greater than 0\n";
    return EXIT_FAILURE;
  }

  const auto start = std::chrono::system_clock::now();

  // do your calculation here
  const auto fn = [function_id]() {
    switch (function_id) {
    case 1:
      return f1;
    case 2:
      return f2;
    case 3:
      return f3;
    default:
      // case 4:
      return f4;
    }
  }();

  // initialize pthread structures
  const auto threads = std::make_unique<pthread_t[]>(nb_threads);
  const auto local_results = std::make_unique<double[]>(nb_threads);
  const auto thread_data = std::make_unique<ThreadData[]>(nb_threads);

  // create and run threads
  for (auto i = 0; i < nb_threads; ++i) {
    local_results[i] = 0;
    thread_data[i] = ThreadData{
        fn, a, b, n, intensity, nb_threads, sync, i, &local_results[i],
    };
    if (pthread_create(&threads[i], nullptr, thread_fn, &thread_data[i]) != 0) {
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

  if (sync == "thread") {
    for (auto i = 0; i < nb_threads; ++i) {
      result += local_results[i];
    }
  }

  result = (static_cast<double>(b - a) * result) / static_cast<double>(n);

  const auto end = std::chrono::system_clock::now();
  const auto elapsed_seconds = end - start;

  // report result and time
  std::cout << result << '\n';
  std::cerr << elapsed_seconds.count() << '\n';
}
