
#ifndef LOCKER_H
#define LOCKER_H

#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>

void init_sem(sem_t *m_sem);
void del_sem(sem_t * m_sem);

bool wait_get(sem_t *m_sem);
bool post(sem_t *m_sem);


void init_locker(pthread_mutex_t *m_mutex);
void del_locker(pthread_mutex_t *m_mutex);
bool lock(pthread_mutex_t *m_mutex);
bool unlock(pthread_mutex_t *m_mutex);
#endif
