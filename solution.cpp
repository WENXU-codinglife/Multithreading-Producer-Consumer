#include <assert.h>
#include "solution.h"
#include <unistd.h>

void *producerFunction(void *_arg)
{
  // Parse the _arg passed to the function.
  // Enqueue `n` items into the `production_buffer`. The items produced should
  // be 0, 1, 2,..., (n-1).
  // Keep track of the number of items produced and the value produced by the
  // thread.
  // The producer that was last active should ensure that all the consumers have
  // finished. NOTE: Each thread will enqueue `n` items.
  // Use mutex variables and conditional variables as necessary.
  // *********** MY CODE *********************
  // *****************************************
  // 1. _arg should contain pointers to:
  //    production_buffer,
  //    a mutex,
  //    two conditional variables,
  //    an id (source No.) to identify this producer thread,
  //    the total number of the producer threads
  // 2. this function is responsible for enqueuing items
  //    so, its code body should be wrapped by a lock
  // ******************************************
  // Reference:
  // - https://www.cse.iitd.ac.in/~sbansal/os/lec/l24.html
  // - https://www.cse.iitd.ac.in/~sbansal/os/lec/l25.html
  // ******************************************
  // char queue[MAX];  //global
  // int head = 0, tail = 0; //global
  // struct cv not_full, not_empty;
  // struct lock qlock;

  // void produce(char data) {
  //   acquire(&qlock);
  //   while ((head + 1) % MAX  ==  tail) {
  //     wait(&not_full, &qlock);
  //   }
  //   queue[head] = data;
  //   head = (head + 1) % MAX;
  //   notify(&not_full);
  //   release(&qlock);
  // }

  // char consume(void) {
  //   acquire(&qlock);
  //   while (tail == head) {
  //     wait(&not_empty, &qlock);
  //   }
  //   e = queue[tail];
  //   tail = (tail + 1) % MAX;
  //   notify(&not_empty);
  //   release(&qlock);
  //   return e;
  // }
  // ******************************************

  // parse arguments
  timer t;
  t.start();
  CircularQueue *production_buffer = ((params_producer *)_arg)->production_buffer;
  pthread_mutex_t *mt = ((params_producer *)_arg)->mt;
  pthread_cond_t *not_full = ((params_producer *)_arg)->not_full;
  pthread_cond_t *not_empty = ((params_producer *)_arg)->not_empty;
  long *n_items = ((params_producer *)_arg)->n_items;
  int producer_id = ((params_producer *)_arg)->producer_id; // used to mark source property of every entry (product)
  long product_value = producer_id;                         // incrementd by the total number of producer threads
  int producer_number = ((params_producer *)_arg)->producer_number;
  long *items_produced = ((params_producer *)_arg)->items_produced;
  long *value_produced = ((params_producer *)_arg)->value_produced;
  int *active_producer_count = ((params_producer *)_arg)->active_producer_count;
  int *active_consumer_count = ((params_producer *)_arg)->active_consumer_count;
  double *time_taken_by_producer = ((params_producer *)_arg)->time_taken_by_producer;

  long upper_bound = (*n_items) / producer_number;
  if (producer_id == 0)
    upper_bound += ((*n_items) % producer_number);

  while ((*items_produced) < upper_bound)
  {
    pthread_mutex_lock(mt);
    while ((*production_buffer).isFull())
      pthread_cond_wait(not_full, mt);
    assert((*production_buffer).enqueue(product_value, producer_id));
    (*value_produced) += product_value;
    (*items_produced)++;
    product_value += producer_number;
    // if (product_value + producer_number < *n_items)
    // {
    //   product_value += producer_number;
    // }
    // else
    // {
    //   product_value++;
    // }
    pthread_cond_signal(not_empty);
    pthread_mutex_unlock(mt);
  }
  if (--(*active_producer_count) == 0)
  {
    while (*active_consumer_count > 0)
    {
      pthread_mutex_lock(mt);
      pthread_cond_signal(not_empty);
      pthread_mutex_unlock(mt);
    }
  }
  *time_taken_by_producer = t.stop();
  pthread_exit(_arg);
  // *****************************************
}

void *consumerFunction(void *_arg)
{
  // Parse the _arg passed to the function.
  // The consumer thread will consume items by dequeueing the items from the
  // `production_buffer`.
  // Keep track of the number of items consumed and the value consumed by the
  // thread.
  // Once the productions is complete and the queue is also empty, the thread
  // will exit. NOTE: The number of items consumed by each thread need not be
  // same.
  // Use mutex variables and conditional variables as necessary.
  // *********** MY CODE *********************
  // parse arguments
  timer t;
  t.start();
  CircularQueue *production_buffer = ((params_consumer *)_arg)->production_buffer;
  pthread_mutex_t *mt = ((params_consumer *)_arg)->mt;
  pthread_cond_t *not_full = ((params_consumer *)_arg)->not_full;
  pthread_cond_t *not_empty = ((params_consumer *)_arg)->not_empty;
  //int consumer_id = ((params_consumer *)_arg)->consumer_id;
  long *items_consumed = ((params_consumer *)_arg)->items_consumed;
  long *value_consumed = ((params_consumer *)_arg)->value_consumed;
  long *value_from_producers = ((params_consumer *)_arg)->value_from_producers;

  // create two variables to recieve the entry info
  int producer_id;    // used to contain item producer id
  long product_value; // used to contain item value
  int *active_producer_count = ((params_consumer *)_arg)->active_producer_count;
  int *active_consumer_count = ((params_consumer *)_arg)->active_consumer_count;
  double *time_taken_by_consumer = ((params_consumer *)_arg)->time_taken_by_consumer;

  while (true)
  {
    pthread_mutex_lock(mt);
    while ((*production_buffer).isEmpty())
    {
      if (*active_producer_count == 0)
      {
        (*active_consumer_count)--;
        pthread_mutex_unlock(mt);
        *time_taken_by_consumer = t.stop();
        pthread_exit(_arg);
      }
      pthread_cond_wait(not_empty, mt);
      if (*active_producer_count == 0 && (*production_buffer).isEmpty())
      {
        (*active_consumer_count)--;
        pthread_mutex_unlock(mt);
        *time_taken_by_consumer = t.stop();
        pthread_exit(_arg);
      }
    }
    assert((*production_buffer).dequeue(&product_value, &producer_id));
    (*items_consumed)++;
    (*value_consumed) += product_value;
    value_from_producers[producer_id] += product_value;
    pthread_cond_signal(not_full);
    pthread_mutex_unlock(mt);
  }
  // std::cout << "CONSUMER" << consumer_id << " HAS BEEN OUT OF WORK!\n";
  // (*active_consumer_count)--;
  // *time_taken = t.stop();
  pthread_exit(_arg);
  // *****************************************
}

ProducerConsumerProblem::ProducerConsumerProblem(long _n_items,
                                                 int _n_producers,
                                                 int _n_consumers,
                                                 long _queue_size)
    : n_items(_n_items), n_producers(_n_producers), n_consumers(_n_consumers),
      production_buffer(_queue_size)
{
  std::cout << "Constructor\n";
  //std::cout << "Number of items: " << n_items << "\n";
  std::cout << "Number of producers: " << n_producers << "\n";
  std::cout << "Number of consumers: " << n_consumers << "\n";
  std::cout << "Queue size: " << _queue_size << "\n";

  producer_threads = new pthread_t[n_producers];
  consumer_threads = new pthread_t[n_consumers];
  // Initialize all mutex and conditional variables here.
  // *********** MY CODE *********************
  producer_params = new params_producer[n_producers];
  consumer_params = new params_consumer[n_consumers];
  mt = PTHREAD_MUTEX_INITIALIZER;
  not_full = PTHREAD_COND_INITIALIZER;
  not_empty = PTHREAD_COND_INITIALIZER;

  items_produced = new long[n_producers];
  value_produced = new long[n_producers];
  time_taken_by_producers = new double[n_producers];

  items_consumed = new long[n_consumers];
  value_consumed = new long[n_consumers];
  value_from_producers = new long *[n_consumers];
  for (int i = 0; i < n_consumers; i++)
  {
    value_from_producers[i] = new long[n_producers];
  }
  time_taken_by_consumers = new double[n_consumers];
  // *****************************************
}

ProducerConsumerProblem::~ProducerConsumerProblem()
{
  std::cout << "Destructor\n";
  delete[] producer_threads;
  delete[] consumer_threads;
  // Destroy all mutex and conditional variables here.
  // *********** MY CODE *********************
  delete[] producer_params;
  delete[] consumer_params;
  pthread_mutex_destroy(&mt);
  pthread_cond_destroy(&not_full);
  pthread_cond_destroy(&not_empty);
  delete[] items_produced;
  delete[] value_produced;
  delete[] time_taken_by_producers;

  delete[] items_consumed;
  delete[] value_consumed;
  for (int i = 0; i < n_consumers; i++)
  {
    delete[] value_from_producers[i];
  }
  delete[] value_from_producers;
  delete[] time_taken_by_consumers;

  // *****************************************
}

void ProducerConsumerProblem::startProducers()
{
  std::cout << "Starting Producers\n";
  active_producer_count = n_producers;
  // Create producer threads P1, P2, P3,.. using pthread_create.
  // *********** MY CODE *********************

  for (int i = 0; i < n_producers; i++)
  {
    (&producer_params[i])->production_buffer = &production_buffer;
    (&producer_params[i])->mt = &mt;
    (&producer_params[i])->not_full = &not_full;
    (&producer_params[i])->not_empty = &not_empty;
    (&producer_params[i])->n_items = &n_items;
    (&producer_params[i])->producer_id = i;
    (&producer_params[i])->producer_number = n_producers;
    (&producer_params[i])->active_producer_count = &active_producer_count;
    (&producer_params[i])->active_consumer_count = &active_consumer_count;
    (&producer_params[i])->time_taken_by_producer = &time_taken_by_producers[i];
    items_produced[i] = 0;
    (&producer_params[i])->items_produced = &items_produced[i];
    value_produced[i] = 0;
    (&producer_params[i])->value_produced = &value_produced[i];
    int err = pthread_create(&producer_threads[i], NULL, producerFunction, &producer_params[i]);
    if (err)
      std::cout << "Producer thread creation failed : " << strerror(err);
    // else
    //   std::cout << "Producer thread Created with ID : " << i << std::endl;
  }
  // *****************************************
}

void ProducerConsumerProblem::startConsumers()
{
  std::cout << "Starting Consumers\n";
  active_consumer_count = n_consumers;
  // Create consumer threads C1, C2, C3,.. using pthread_create.
  // *********** MY CODE *********************

  for (int i = 0; i < n_consumers; i++)
  {
    (&consumer_params[i])->production_buffer = &production_buffer;
    (&consumer_params[i])->mt = &mt;
    (&consumer_params[i])->not_full = &not_full;
    (&consumer_params[i])->not_empty = &not_empty;
    (&consumer_params[i])->consumer_id = i;
    (&consumer_params[i])->active_producer_count = &active_producer_count;
    (&consumer_params[i])->active_consumer_count = &active_consumer_count;
    items_consumed[i] = 0;
    (&consumer_params[i])->items_consumed = &items_consumed[i];
    value_consumed[i] = 0;
    (&consumer_params[i])->value_consumed = &value_consumed[i];
    (&consumer_params[i])->value_from_producers = value_from_producers[i];
    (&consumer_params[i])->time_taken_by_consumer = &time_taken_by_consumers[i];

    for (int j = 0; j < n_producers; j++)
    {
      ((&consumer_params[i])->value_from_producers)[j] = 0;
    }
    int err = pthread_create(&consumer_threads[i], NULL, consumerFunction, &consumer_params[i]);
    if (err)
      std::cout << "Consumer thread creation failed : " << strerror(err);
    // else
    //   std::cout << "Consumer thread Created with ID : " << i << std::endl;
  }
  // *****************************************
}

void ProducerConsumerProblem::joinProducers()
{
  std::cout << "Joining Producers\n";
  // Join the producer threads with the main thread using pthread_join
  // *********** MY CODE *********************
  for (int i = 0; i < n_producers; i++)
  {
    // std::cout << "Joining Producer Thread " << i << "\n";
    void *anything;
    int err = pthread_join(producer_threads[i], &anything);
    if (err)
      std::cout << "Producer thread join failed : " << strerror(err);
    // else
    // {
    //   std::cout << "Producer thread joined : " << i << std::endl;
    // }
  }
  // *****************************************
}

void ProducerConsumerProblem::joinConsumers()
{
  std::cout << "Joining Consumers\n";
  // Join the consumer threads with the main thread using pthread_join
  // *********** MY CODE *********************
  for (int i = 0; i < n_consumers; i++)
  {
    // std::cout << "Joining Consumer Thread " << i << "\n";
    void *anything;
    int err = pthread_join(consumer_threads[i], &anything);
    if (err)
      std::cout << "Consumer thread join failed : " << strerror(err);
    // else
    // {
    //   std::cout << "Consumer thread joined : " << i << std::endl;
    // }
  }
  // *****************************************
}

void ProducerConsumerProblem::printStats()
{
  std::cout << "Producer stats\n";
  std::cout << "producer_id, items_produced, value_produced, time_taken \n";
  // Make sure you print the producer stats in the following manner
  // 0, 1000, 499500, 0.00123596
  // 1, 1000, 499500, 0.00154686
  // 2, 1000, 499500, 0.00122881
  long total_produced = 0;
  long total_value_produced = 0;
  for (int i = 0; i < n_producers; i++)
  {
    // *********** MY CODE *********************
    std::cout << i << ", " << items_produced[i] << ", " << value_produced[i] << ", "
              << time_taken_by_producers[i]
              << "\n";
    total_produced += items_produced[i];
    total_value_produced += value_produced[i];
    // *****************************************
  }
  std::cout << "Total produced = " << total_produced << "\n";
  std::cout << "Total value produced = " << total_value_produced << "\n";
  std::cout << "Consumer stats\n";
  std::cout << "consumer_id, items_consumed, value_consumed, time_taken, ";
  for (int i = 0; i < n_producers; i++)
  {
    std::cout << "value_from_producer_" << i;
    if (i < n_producers - 1)
      std::cout << ", ";
  }
  std::cout << "\n";
  // Make sure you print the consumer stats in the following manner
  // 0, 677, 302674, 0.00147414, 120000, 130000, 140000
  // 1, 648, 323301, 0.00142694, 130000, 140000, 120000
  // 2, 866, 493382, 0.00139689, 140000, 120000, 130000
  // 3, 809, 379143, 0.00134516, 109500, 109500, 109500
  long total_consumed = 0;
  long total_value_consumed = 0;
  for (int i = 0; i < n_consumers; i++)
  {
    // *********** MY CODE *********************
    std::cout << i << ", " << items_consumed[i] << ", " << value_consumed[i]
              << ", "
              << time_taken_by_consumers[i];
    for (int j = 0; j < n_producers; j++)
    {
      std::cout << ", " << value_from_producers[i][j];
    }
    std::cout << "\n";
    total_consumed += items_consumed[i];
    total_value_consumed += value_consumed[i];
    // *****************************************
  }
  std::cout << "Total consumed = " << total_consumed << "\n";
  std::cout << "Total value consumed = " << total_value_consumed << "\n";
}
