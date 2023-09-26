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

// Structure to pass thread-specific data
struct ThreadData {
  int thread_id;
  int num_threads;
  double a;
  double b;
  int intensity;
  float *result;
  std::string sync;
  int granularity;
  int num_iterations;
  int *current_iteration;
  pthread_mutex_t *mutex;
};

// Global variables for result and mutual exclusion
auto global_result = 0.0;
pthread_mutex_t mutex;

// Numerical integration function
void *integrate(void *arg) {
  const auto data = static_cast<const ThreadData *>(arg);

  auto local_result = 0.0;

  while (true) {
    // get the next range of iterations
    pthread_mutex_lock(data->mutex);
    if (*(data->current_iteration) >= data->num_iterations) {
      pthread_mutex_unlock(data->mutex);
      // all iterations are done
      break;
    }
    const auto begin = *(data->current_iteration);
    const auto end = std::min(begin + data->granularity, data->num_iterations);
    *(data->current_iteration) = end;
    pthread_mutex_unlock(data->mutex);

    // Perform numerical integration for the current range
    for (auto i = begin; i < end; ++i) {
      const auto x =
          data->a + (i + 0.5) * ((data->b - data->a) / data->num_iterations);
      local_result += data->sync == "iteration" ? f1(x, data->intensity)
                                                : f2(x, data->intensity);
    }
  }

  // Update the global result with appropriate synchronization
  if (data->sync == "thread") {
    pthread_mutex_lock(&mutex);
    *(data->result) += local_result;
    pthread_mutex_unlock(&mutex);
  } else {
    *(data->result) += local_result;
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
  const auto sync_type = std::string{argv[7]};
  const auto granularity = std::atoi(argv[8]);

  if (nb_threads <= 0 || granularity <= 0) {
    std::cerr << "Number of threads and granularity must be greater than 0\n";
    return EXIT_FAILURE;
  }

  const auto start = std::chrono::system_clock::now();

  // do your calculation here
  // auto result = 0.0;
  // initialize pthread structures
  const auto threads = std::make_unique<pthread_t[]>(nb_threads);
  const auto thread_data = std::make_unique<ThreadData[]>(nb_threads);

  int currentIteration = 0;
  pthread_mutex_t iterationMutex;
  pthread_mutex_init(&iterationMutex, nullptr);

  // Calculate the number of iterations
  const int numIterations = n;

  // Create and run the threads
  for (int i = 0; i < nb_threads; ++i) {
    thread_data[i] = ThreadData{i,
                                nb_threads,
                                a,
                                b,
                                intensity,
                                &global_result,
                                sync_type,
                                granularity,
                                numIterations,
                                &currentIteration,
                                &iterationMutex};
    if (pthread_create(&threads[i], nullptr, integrate, &thread_data[i]) != 0) {
      std::cerr << "Error creating thread " << i << '\n';
      return EXIT_FAILURE;
    }
  }

  // Wait for all threads to finish
  for (int i = 0; i < nb_threads; ++i) {
    if (pthread_join(threads[i], nullptr) != 0) {
      std::cerr << "Error joining thread " << i << '\n';
      return EXIT_FAILURE;
    }
  }

  // clean up
  pthread_mutex_destroy(&iterationMutex);

  const auto end = std::chrono::system_clock::now();
  const auto elapsed_seconds = end - start;

  // report result and time
  std::cout << global_result << '\n';
  std::cerr << elapsed_seconds.count() << '\n';
}
