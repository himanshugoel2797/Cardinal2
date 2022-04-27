/**
 * Copyright (c) 2022 Himanshu Goel
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include "containers/queue.h"
#include "local_spinlock.h"

int queue_init(queue_t *q, int32_t sz, void*(*alloc_fn)(size_t), void(*free_fn)(void*))
{
    if (q == NULL)
        return -1;
    if (alloc_fn == NULL)
        return -1;

    q->alloc = alloc_fn;
    q->free = free_fn;
    q->queue = alloc_fn(sz * sizeof(uint64_t));

    if (q->queue == NULL)
        return -1;

    q->size = sz;
    q->ent_cnt = 0;
    q->head = 0;
    q->tail = 0;

    return 0;
}

void queue_fini(queue_t *q)
{
    if (q == NULL)
        return;

    if (q->free != NULL)
        q->free(q->queue);
    q->size = 0;
}

int32_t queue_size(queue_t *q)
{
    if (q == NULL)
        return 0;

    return q->size;
}

int32_t queue_entcnt(queue_t *q)
{
    if (q == NULL)
        return 0;

    return q->ent_cnt;
}

bool queue_full(queue_t *q)
{
    return queue_entcnt(q) == queue_size(q);
}

bool queue_tryenqueue(queue_t *q, uint64_t val)
{
    if (q == NULL)
        return false;

    int32_t curTail = q->tail;
    if (q->head == (curTail + 1) % q->size)
        return false;

    q->queue[curTail] = val;
    q->tail = (curTail + 1) % q->size;
    q->ent_cnt++;

    return true;
}

bool queue_tryenqueue_front(queue_t *q, uint64_t val)
{
    if (q == NULL)
        return false;

    int32_t curTail = q->tail;
    int32_t curHead = q->head;
    if (curHead == (curTail + 1) % q->size)
        return false;

    if (curHead > 0)
        q->head = (curHead - 1) % q->size;
    else
        q->head = (q->size - 1);

    q->queue[q->head] = val;

    q->ent_cnt++;

    return true;
}

bool queue_trydequeue(queue_t *q, uint64_t *val)
{
    if (q == NULL)
        return false;

    int32_t curHead = q->head;
    if (curHead == q->tail)
        return false;

    *val = q->queue[curHead];
    q->queue[curHead] = 0;

    q->head = (curHead + 1) % q->size;
    q->ent_cnt--;

    return true;
}

bool queue_peek(queue_t *q, uint64_t *val)
{
    if (q == NULL)
        return false;

    int32_t curHead = q->head;
    if (curHead == q->tail)
        return false;

    *val = q->queue[curHead];
    return true;
}