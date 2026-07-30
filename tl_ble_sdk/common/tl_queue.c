/********************************************************************************************************
 * @file    tl_queue.c
 *
 * @brief   This is the source file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    06,2022
 *
 * @par     Copyright (c) 2022, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *
 *          Licensed under the Apache License, Version 2.0 (the "License");
 *          you may not use this file except in compliance with the License.
 *          You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *          Unless required by applicable law or agreed to in writing, software
 *          distributed under the License is distributed on an "AS IS" BASIS,
 *          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *          See the License for the specific language governing permissions and
 *          limitations under the License.
 *
 *******************************************************************************************************/
#include "tl_queue.h"
#include "drivers.h"
#include "common/compiler.h"

/*********************************************************************
 * @fn          queue_init
 * @brief       Initialize the queue
 * @param[in]   pQueue  - The queue need to use
 * @param[in]   priFunc - Pointer to the function that calculates a priority
 *                    NULL means not use the priority feature
 *              Note: a small priority value has a higher priority
 * @return  Status
 */
queue_sts_t queue_init(queue_t *pQueue, priCmpCbFunc_t priFunc)
{
#if (1) //Can be optimized
    if (pQueue == NULL) {
        return QUEUE_PARAM_INVALID;
    }
#else
    assert(pQueue != NULL);
#endif

    pQueue->head = pQueue->tail = NULL;
    pQueue->curNum              = 0;
    pQueue->priCmpCb            = priFunc;
    return QUEUE_SUCCESS;
}

/*********************************************************************
  * @fn          queue_enq
  * @brief       Enqueue an element to the tail of the specified queue
  * @param[in]   pQueue - The queue that a new element need to push to
  * @param[in]   pItem  - The payload of the new element
  * @return      Status
  */
_attribute_ram_code_
    queue_sts_t
    queue_enq(queue_t *pQueue, queue_item_t *pItem)
{
#if (1) //Can be optimized
    if (pQueue == NULL || pItem == NULL) {
        return QUEUE_PARAM_INVALID;
    }
#else
    assert(pQueue != NULL);
    assert(pItem != NULL);
#endif

    u32 r = irq_disable();

    /* if the Queue was empty, then update the head */
    if (pQueue->head == NULL) {
        pQueue->head = pItem;
        pQueue->tail = pItem;
        pItem->next  = NULL;
        pQueue->curNum++;
        irq_restore(r);
        return QUEUE_SUCCESS;
    }

    /* if priority is not used, insert at the end as in a FIFO */
    if (pQueue->priCmpCb == NULL) {
        pQueue->tail->next = pItem;
        pItem->next        = NULL;
        pQueue->tail       = pItem;
    }
    /* if priority callback function is used, insert at the end */
    else {
        queue_item_t *previous = NULL;
        queue_item_t *current  = pQueue->head;

        while (current != NULL) {
            /* Here, a small priority value has a higher priority */
            if (pQueue->priCmpCb((arg_t)(pItem)) < pQueue->priCmpCb((arg_t)(current))) {
                break;
            } else {
                previous = current;
                current  = current->next;
            }
        }

        /* Insert between previous and current */
        if (previous == NULL) { /* insert at the head */
            pQueue->head = pItem;
        } else {
            previous->next = pItem;
        }

        pItem->next = current;

        if (current == NULL) {
            pQueue->tail = pItem;
        }
    }

    pQueue->curNum++;
    irq_restore(r);
    return QUEUE_SUCCESS;
}

/*********************************************************************
 * @fn          queue_deq
 * @brief       Dequeue and element from the head of the specified queue.
 * @param[in]   pQueue - The specified queue
 * @return      Pointer to first element in the queue
 */
_attribute_ram_code_
queue_item_t *queue_deq(queue_t *pQueue)
{
    queue_item_t *pItem = NULL;
    u32           r     = irq_disable();

    pItem = pQueue->head;

    if (pItem != NULL) {
        /* update the head pointer in queue */
        pQueue->head = pItem->next;

        /* check for empty queue */
        if (pQueue->head == NULL) {
            pQueue->tail = NULL;
        }
        pQueue->curNum--;
    }

    irq_restore(r);
    return pItem;
}

/*********************************************************************
  * @fn          queue_insert
  * @brief       Insert an element into the specified queue, usually
  *              used when iterating over elements in a Queue
  * @param[in]   pQueue - The queue that a new element need to push to
  * @param[in]   pItem  - The new element to be inserted
  * @param[in]   pPrev  - The element in the queue before the new element to be inserted.
  *                       Note: if the pPrev is NULL, the pItem is the 1st element in queue.
  * @return      Status
  */
//_attribute_ram_code_
queue_sts_t queue_insert(queue_t *pQueue, void *item, void *prev)
{
    queue_item_t *pItem = (queue_item_t *)item;
    queue_item_t *pPrev = (queue_item_t *)prev;
#if (1) //Can be optimized
    if (pQueue == NULL || pItem == NULL) {
        return QUEUE_PARAM_INVALID;
    }
#else
    assert(pQueue != NULL);
    assert(pItem != NULL);
#endif

    u32 r = irq_disable();

    /* if the queue was empty or pPrev was at tail */
    if (pQueue->head == NULL || pPrev == pQueue->tail) {
        queue_enq(pQueue, pItem);
        /* in the API: queue_enq, pQueue->curNum has already ++. */
    }
    /* if pPrev was empty, inserting pItem at head */
    else if (pPrev == NULL) {
        /* inserting element to head of the queue */
        pItem->next = pQueue->head;
        /* if the queue was empty, inserting new element at tail */
        if (pQueue->head == NULL) {
            pQueue->tail = pItem;
        }
        /* update queue head */
        pQueue->head = pItem;
        pQueue->curNum++;
    }
    /* inserting new element in middle of the queue */
    else {
        pItem->next = pPrev->next;
        pPrev->next = pItem;
        pQueue->curNum++;
    }

    irq_restore(r);
    return QUEUE_SUCCESS;
}

/*********************************************************************
  * @fn          queue_insert
  * @brief       Remove an element from the specified queue, usually
  *              used when iterating over elements in a Queue
  * @param[in]   pQueue - The queue that a target element need to be removed
  * @param[in]   pItem  - The target element to be removed
  * @param[in]   pPrev  - The element in the queue before the target element
  *                       to be removed. Note: if the pPrev is NULL, which
  *                       means the pItem is the 1st element in queue
  * @return      Status
  */
//_attribute_ram_code_
queue_sts_t queue_remove(queue_t *pQueue, void *item, void *prev)
{
    queue_item_t *pItem = (queue_item_t *)item;
    queue_item_t *pPrev = (queue_item_t *)prev;
#if (1) //Can be optimized
    if (pQueue == NULL || pQueue->head == NULL || pItem == NULL) {
        return QUEUE_PARAM_INVALID;
    }
#else
    assert(pQueue != NULL);
    assert(pQueue->head != NULL);
    assert(pItem != NULL);
#endif

    u32 r = irq_disable();

    /* if the 1st element, remove from head of queue*/
    if (pQueue->head == pItem) {
        pQueue->head = pItem->next;
    }
    /* removing the element in middle of the queue */
    else if (pPrev != NULL) {
        pPrev->next = pItem->next;
    }
    /* if the removing element is last element */
    if (pQueue->tail == pItem) {
        /* update queue tail */
        pQueue->tail = pPrev;
    }

    pQueue->curNum--;
    irq_restore(r);
    return QUEUE_SUCCESS;
}

/*********************************************************************
 * @fn          queue_delete
 * @brief       Delete an element from the queue (Do not use iterator)
 * @param[in]   pQueue - The specified queue
 * @param[in]   pItem  - Pointer to the target element to be removed
 * @return  Status
 */
//_attribute_ram_code_
queue_sts_t queue_delete(queue_t *pQueue, queue_item_t *pItem)
{
#if (1) //Can be optimized
    if (pQueue == NULL || pItem == NULL) {
        return QUEUE_PARAM_INVALID;
    }
#else
    assert(pQueue != NULL);
    assert(pItem != NULL);
#endif

    u32 r = irq_disable();

    /* the queue was empty */
    if (pQueue->head == NULL) {
        irq_restore(r);
        return QUEUE_EMPTY;
    }

    /* if the 1st element, remove from head of queue*/
    if (pQueue->head == pItem) {
        pQueue->head = pItem->next;

        /* if the removing element is last element */
        if (pQueue->tail == pItem) { //same as 'pQueue->head == NULL'
            /* update queue tail */
            pQueue->tail = NULL;
        }

        pQueue->curNum--;
        irq_restore(r);
        return QUEUE_SUCCESS;
    }

    /* find the to be delete element */
    queue_item_t *previous = NULL;
    queue_item_t *current  = pQueue->head;

    while (current && current != pItem) {
        previous = current;
        current  = current->next;
    }

    /* delete element was found in the queue */
    if (current == pItem) {
        previous->next = current->next;
        /* if the removing element is last element */
        if (pQueue->tail == current) { //same as 'current->next == NULL'
            /* update queue tail */
            pQueue->tail = previous;
        }

        pQueue->curNum--;
        irq_restore(r);
        return QUEUE_SUCCESS;
    }
    /* element not in the Queue */
    else {
        irq_restore(r);
        return QUEUE_NOT_FOUND;
    }
}

/*********************************************************************
 * @fn          queue_isempty
 * @brief       Check if the specified queue is empty
 * @param[in]   pQueue - The specified queue
 * @return      Return TRUE, if the specified queue is empty
 */
//_attribute_ram_code_
bool queue_isempty(queue_t *pQueue)
{
    assert(pQueue != NULL);

    u32  r     = irq_disable();
    bool empty = (pQueue->head == NULL);
    irq_restore(r);

    return empty;
}

/*********************************************************************
 * @fn          queue_count
 * @brief       Count the number of elements in the queue
 * @param[in]   pQueue - The specified queue
 * @return      Number of elements in queue
 */
//_attribute_ram_code_
u32 queue_count(queue_t *pQueue)
{
    assert(pQueue != NULL);

    u32 r       = irq_disable();
    u32 currNum = pQueue->curNum;
    irq_restore(r);

    return currNum;
}
