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

// structure to pass thread-specific data
struct ThreadData {
  int function_id;
  float a;
  float b;
  float n;
  int intensity;
  int num_threads;
  std::string sync;
  int thread_id;
  int num_iterations;
  float *result;
  // used for thread-level synchronization
  // pthread_barrier_t *barrier;
};

// global variables for result and mutual exclusion
auto global_result = 0.0;
pthread_mutex_t mutex;


// numerical integration function
void *thread_fn(void *arg) {
  const auto data = static_cast<ThreadData *>(arg);

  auto local_result = 0.0;

  // Calculate the range of iterations for this thread
  const auto iterations_per_thread = data->num_iterations / data->num_threads;
  const auto startIteration = data->thread_id * iterations_per_thread;
  const auto endIteration = startIteration + iterations_per_thread;

  // Perform numerical integration
  for (auto i = startIteration; i < endIteration; ++i) {
    const auto x =
        data->a + (i + 0.5) * ((data->b - data->a) / data->num_iterations);
    local_result += data->sync == "iteration" ? f1(x, data->intensity)
                                                  : f2(x, data->intensity);
  }

  if (data->sync == "thread") {
    // Thread-level synchronization
    pthread_mutex_lock(&mutex);
    *(data->result) += local_result;
    pthread_mutex_unlock(&mutex);
    // wait for all threads to finish
    // pthread_barrier_wait(data->barrier);
  } else {
    // Iteration-level synchronization
    pthread_mutex_lock(&mutex);
    global_result += local_result;
    pthread_mutex_unlock(&mutex);
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
  const auto a = std::atoi(argv[2]);
  const auto b = std::atoi(argv[3]);
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

  // initialize pthread structures
  const auto threads = std::make_unique<pthread_t[]>(nb_threads);
  const auto thread_data = std::make_unique<ThreadData[]>(nb_threads);

  // pthread_barrier_t barrier;
  // if (sync_type == "thread") {
  //   pthread_barrier_init(&barrier, nullptr, nb_threads);
  // }

  // Calculate the number of iterations per thread
  const auto iterations_per_thread = n / nb_threads;

  // create and run threads
  for (auto i = 0; i < nb_threads; ++i) {
    thread_data[i] =
        ThreadData{i,       numThreads, iterations_per_thread, a,
                   b,       intensity,  &globalResult,         sync,
                   &barrier};
    if (pthread_create(&threads[i], nullptr, integrate, &thread_data[i]) != 0) {
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

  // clean up
  // delete[] threads;
  // delete[] thread_data;
  // if (sync_type == "thread") {
  //   pthread_barrier_destroy(&barrier);
  // }

  const auto end = std::chrono::system_clock::now();
  const auto elapsed_seconds = end - start;

  // report result and time
  std::cout << global_result << '\n';
  std::cerr << elapsed_seconds.count() << '\n';
}
