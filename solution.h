#include "core/circular_queue.h"
#include "core/utils.h"
#include <pthread.h>
#include <stdlib.h>

// ******************************************************************
typedef struct params_producer
{
  CircularQueue *production_buffer;
  pthread_mutex_t *mt;
  pthread_cond_t *not_full;
  pthread_cond_t *not_empty;
  long *n_items;
  int producer_id;
  int producer_number; // the total number of the producer threads
  long *items_produced;
  long *value_produced;
  int *active_producer_count;
  int *active_consumer_count;
  double *time_taken_by_producer;
} params_producer;

typedef struct params_consumer
{
  CircularQueue *production_buffer;
  pthread_mutex_t *mt;
  pthread_cond_t *not_full;
  pthread_cond_t *not_empty;
  int consumer_id;
  int *active_producer_count;
  int *active_consumer_count;
  long *items_consumed;
  long *value_consumed;
  long *value_from_producers;
  double *time_taken_by_consumer;
} params_consumer;
// ******************************************************************

class ProducerConsumerProblem
{
  long n_items;
  int n_producers;
  int n_consumers;
  CircularQueue production_buffer;

  // *********** MY CODE *********************
  pthread_mutex_t mt;
  pthread_cond_t not_full;
  pthread_cond_t not_empty;

  long *items_produced;
  long *value_produced;
  double *time_taken_by_producers;

  long *items_consumed;
  long *value_consumed;
  long **value_from_producers;
  double *time_taken_by_consumers;

  // *****************************************

  // Dynamic array of thread identifiers for producer and consumer threads.
  // Use these identifiers while creating the threads and joining the threads.
  pthread_t *producer_threads;
  pthread_t *consumer_threads;

  params_producer *producer_params;
  params_consumer *consumer_params;

  int active_producer_count;
  int active_consumer_count;

public:
  // The following 6 methods should be defined in the implementation file (solution.cpp)
  ProducerConsumerProblem(long _n_items, int _n_producers, int _n_consumers,
                          long _queue_size);
  ~ProducerConsumerProblem();
  void startProducers();
  void startConsumers();
  void joinProducers();
  void joinConsumers();
  void printStats();
};
