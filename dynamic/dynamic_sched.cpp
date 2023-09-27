#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
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

class DynamicLoopScheduler {
private:
  const int n;
  const int granularity;
  int current = 0;
  pthread_mutex_t lock;

public:
  [[nodiscard]] DynamicLoopScheduler(const int n,
                                     const int granularity) noexcept
      : n{n}, granularity{granularity} {
    pthread_mutex_init(&lock, nullptr);
  }

  [[nodiscard]] auto done() noexcept -> bool {
    pthread_mutex_lock(&lock);
    const auto isDone = current >= n;
    pthread_mutex_unlock(&lock);
    return isDone;
  }

  auto get_next(int &begin, int &end) noexcept -> void {
    pthread_mutex_lock(&lock);
    begin = current;
    end = std::min(current + granularity, n);
    current += granularity;
    pthread_mutex_unlock(&lock);
  }
};

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
  int granularity;
  int thread_id;
  double *local_result;
  DynamicLoopScheduler *loop;
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
  const auto granularity = data->granularity;
  const auto thread_id = data->thread_id;
  auto &local_result = *(data->local_result);
  auto &loop = *(data->loop);

  // for chunk
  // perform numerical integration
  while (!loop.done()) {
    int begin;
    int end;
    loop.get_next(begin, end);

    auto local_sum = 0.0;
    for (auto i = begin; i < end; ++i) {
      const auto x =
          static_cast<double>(fn(a + (((i + 0.5) * (b - a)) / (n)), intensity));

      if (sync == "iteration") {
        pthread_mutex_lock(&result_mutex);
        result += x;
        pthread_mutex_unlock(&result_mutex);
      }

      // chunk
      local_sum += x;

      // thread
      local_result += x;
    }

    if (sync == "chunk") {
      pthread_mutex_lock(&result_mutex);
      result += local_sum;
      pthread_mutex_unlock(&result_mutex);
    }
  }
  return nullptr;
}

[[nodiscard]] auto main(const int argc, const char *const *const argv) noexcept
    -> int {
  if (argc < 9) {
    std::cerr << "Usage: " << argv[0]
              << " <function_id> <a> <b> <n> <intensity> <nb_threads> <sync> "
                 "<granularity>\n";
    return EXIT_FAILURE;
  }

  const auto function_id = std::atoi(argv[1]);
  const auto a = std::atof(argv[2]);
  const auto b = std::atof(argv[3]);
  const auto n = std::atoi(argv[4]);
  const auto intensity = std::atoi(argv[5]);
  const auto nb_threads = std::atoi(argv[6]);
  const auto sync = std::string{argv[7]};
  const auto granularity = std::atoi(argv[8]);

  if (nb_threads <= 0 || granularity <= 0) {
    std::cerr << "Number of threads and granularity must be greater than 0\n";
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
  auto loop = DynamicLoopScheduler{n, granularity};

  // create and run the threads
  for (auto i = 0; i < nb_threads; ++i) {
    local_results[i] = 0;
    thread_data[i] =
        ThreadData{fn,         a,    b,           n, intensity,
                   nb_threads, sync, granularity, i, &local_results[i],
                   &loop};
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
