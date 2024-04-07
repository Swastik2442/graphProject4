#include <stdio.h>
#include <assert.h>

#include "../src/priorityQueue.h"

priorityQueue pQueue;

// Priority Queue - Test Case 1
int test1()
{
    pQueueInit(&pQueue);

    pQueueInsert(&pQueue, 1, 5);
    pQueueInsert(&pQueue, 2, 3);
    pQueueInsert(&pQueue, 3, 2);
    pQueueInsert(&pQueue, 4, 1);
    pQueueInsert(&pQueue, 5, 4);

    assert(pQueue.filled == 5);
    
    assert(pQueue.heap[0].key == 4);
    assert(pQueue.heap[1].key == 3);
    assert(pQueue.heap[2].key == 2);
    assert(pQueue.heap[3].key == 1);
    assert(pQueue.heap[4].key == 5);

    assert(pQueueExtractMin(&pQueue) == 4);
    assert(pQueueExtractMin(&pQueue) == 3);
    assert(pQueueExtractMin(&pQueue) == 2);
    assert(pQueueExtractMin(&pQueue) == 5);
    assert(pQueueExtractMin(&pQueue) == 1);

    assert(pQueue.filled == 0);

    pQueueDeinit(&pQueue);
    return 1;
}

// Priority Queue - Test Case 2
int test2() {
    pQueueInit(&pQueue);

    pQueueInsert(&pQueue, 1, 5);
    pQueueInsert(&pQueue, 2, 3);
    pQueueInsert(&pQueue, 3, 2);
    pQueueInsert(&pQueue, 4, 1);
    pQueueInsert(&pQueue, 5, 4);

    assert(pQueue.filled == 5);

    pQueueDecreaseKey(&pQueue, 1, 1);
    pQueueDecreaseKey(&pQueue, 2, 2);
    pQueueDecreaseKey(&pQueue, 3, 3);
    pQueueDecreaseKey(&pQueue, 4, 1);
    pQueueDecreaseKey(&pQueue, 5, 0);

    assert(pQueue.heap[0].key == 5);
    assert(pQueue.heap[1].key == 4);
    assert(pQueue.heap[2].key == 2);
    assert(pQueue.heap[3].key == 3);
    assert(pQueue.heap[4].key == 1);

    pQueueDeinit(&pQueue);
    return 1;
}

int main()
{
    printf("Priority Queue - Test 1 %s\n", test1() ? "PASSED" : "FAILED");
    printf("Priority Queue - Test 2 %s\n", test2() ? "PASSED" : "FAILED");
}
