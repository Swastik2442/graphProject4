#ifndef _PRIORITY_QUEUE_H_
#define _PRIORITY_QUEUE_H_

#define MAX_HEAP_SIZE 128

typedef struct priorityQueueElement_t {
    int key;      // Value of the Element
    int priority; // Priority of the Element
} priorityQueueElement;

typedef struct pQueue_t {
    int filled;                               // Number of Elements in the Heap
    priorityQueueElement heap[MAX_HEAP_SIZE]; // Array of Elements
} priorityQueue;

int _parent(int i);
int _leftChild(int i);
int _rightChild(int i);
void _swapElements(priorityQueueElement *array, int indexA, int indexB);

void pQueueInit(priorityQueue *pQueue);
void pQueueDeinit(priorityQueue *pQueue);
void pQueueInsert(priorityQueue *pQueue, int key, int priority);
int pQueueExtractMin(priorityQueue *pQueue);
void pQueueDecreaseKey(priorityQueue *pQueue, int key, int priority);

#endif
