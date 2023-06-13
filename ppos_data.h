// GRR20205646 Alexandre de Oliveira Plugge Freitas

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h> // biblioteca POSIX de trocas de contexto

// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
  struct task_t *prev, *next;
  int id;
  ucontext_t context;
  short status; // ready, suspended, terminated, running...
  int static_priority;
  int priority;
  short is_usertask;
  unsigned int creation_time;
  int activations;
  unsigned int processor_time;
  int exit_code;
  struct task_t *suspended_queue;
  unsigned int wakeup_time;
} task_t;

// estrutura que define um semáforo
typedef struct
{
  int id;
  int value;
  task_t *queue;
  int active;
} semaphore_t;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t;

typedef struct
{
  struct msgq_node_t *prev;
  struct msgq_node_t *next;
  void *msg;
} msgq_node_t;

// estrutura que define uma fila de mensagens
typedef struct
{
  msgq_node_t *buffer;
  int num_msgs;
  int max_msgs;
  int msg_size;
  semaphore_t s_buffer;
  semaphore_t s_vaga;
  semaphore_t s_elem;
} mqueue_t;

#endif
