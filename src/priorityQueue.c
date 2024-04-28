#include "priorityQueue.h"

#include <limits.h>

// Return the Index of the Parent Node
int _parent(int i)
{
    return ((i - 1) / 2);
}

// Return the Index of the Left Child Node
int _leftChild(int i)
{
    return ((2 * i) + 1);
}

// Return the Index of the Right Child Node
int _rightChild(int i)
{
    return ((2 * i) + 2);
}

// Swaps the Elements at the provided Indices
void _swapElements(priorityQueueElement *array, int indexA, int indexB) {
    int tempData = array[indexA].key;
    int tempPriority = array[indexA].priority;
    array[indexA].key = array[indexB].key;
    array[indexA].priority = array[indexB].priority;
    array[indexB].key = tempData;
    array[indexB].priority = tempPriority;
}

// Initialize a Priority Queue
void pQueueInit(priorityQueue *pQueue)
{
    pQueue->filled = 0;
}

// Deinitialize a Priority Queue
void pQueueDeinit(priorityQueue *pQueue)
{
    pQueueInit(pQueue);
}

// Insert a Key with Priority into the Priority Queue
void pQueueInsert(priorityQueue *pQueue, int key, int priority)
{
    pQueue->heap[(pQueue->filled)++] = (priorityQueueElement){key, priority};
    int pos = pQueue->filled - 1;
    int parentPos = _parent(pos);

    // Sift Up Process
    while (pos > 0 && pQueue->heap[parentPos].priority > pQueue->heap[pos].priority) {
        _swapElements(pQueue->heap, pos, parentPos);
        pos = parentPos;
        parentPos = _parent(pos);
    }
}

// Extract the Key with the Minimum Priority from the Priority Queue
int pQueueExtractMin(priorityQueue *pQueue)
{
    if (pQueue->filled == 0)
        return INT_MIN;

    int ret = pQueue->heap[0].key;
    _swapElements(pQueue->heap, 0, pQueue->filled - 1);
    (pQueue->filled)--;

    int pos = 0, arrSize = pQueue->filled;
    int left = _leftChild(pos), right = _rightChild(pos);

    // Sift Down Process
    while (1) {
        if (left < arrSize && right < arrSize)
        {
            if (pQueue->heap[right].priority < pQueue->heap[pos].priority && pQueue->heap[right].priority <= pQueue->heap[left].priority)
            {
                _swapElements(pQueue->heap, pos, right);
                pos = right;
            }
            else if (pQueue->heap[left].priority < pQueue->heap[pos].priority && pQueue->heap[left].priority <= pQueue->heap[right].priority)
            {
                _swapElements(pQueue->heap, pos, left);
                pos = left;
            }
            else
                break;
        }
        else if (left >= arrSize && right < arrSize && pQueue->heap[right].priority < pQueue->heap[pos].priority)
        {
            _swapElements(pQueue->heap, pos, right);
            pos = right;
        }
        else if (left < arrSize && right >= arrSize && pQueue->heap[left].priority < pQueue->heap[pos].priority)
        {
            _swapElements(pQueue->heap, pos, left);
            pos = left;
        }
        else
            break;
        left = _leftChild(pos);
        right = _rightChild(pos);
    }
    return ret;
}

// Decrease the Priority of a Key in the Priority Queue
void pQueueDecreaseKey(priorityQueue *pQueue, int key, int priority)
{
    if (pQueue->filled == 0)
        return;

    int pos = 0;
    while (pos < pQueue->filled && pQueue->heap[pos].key != key)
        pos++;
    if (pos == pQueue->filled)
        return;
    if (pQueue->heap[pos].priority < priority)
        return;
    pQueue->heap[pos].priority = priority;

    int parentPos = _parent(pos);
    while (pos > 0 && pQueue->heap[parentPos].priority > pQueue->heap[pos].priority) {
        _swapElements(pQueue->heap, pos, parentPos);
        pos = parentPos;
        parentPos = _parent(pos);
    }
}
