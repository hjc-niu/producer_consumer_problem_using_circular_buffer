#ifndef _QUEUE_USING_LINKED_LIST_H_INCLUDED
    #define _QUEUE_USING_LINKED_LIST_H_INCLUDED

    #include "plibsys.h"

    /**
     * Structure to store a sample and the respective sensor id that the sample was
     * collected from.
     */
    typedef struct sens_sample_t
    {
        puint8  sens_id; // The sensor ID
        puint32 val;     // The sample value
        psize   num;     // The sample number
    }sens_sample;

    struct queue_t;

    /**
     * Concurrent queue constructor.
     * @param len: The capacity of the queue.
     * @returns: A pointer to the queue if successful, NULL otherwise.
     */
    struct queue_t*
    queue_create(const psize len);

    /**
     * Concurrent queue destructor.
     * @param self: A pointer to the queue instance.
     */
    void
    queue_destroy(struct queue_t* const self);

    /**
     * Returns true if the queue is full and cannot accept any more items.
     * @param self: A pointer to the queue instance.
     */
    pboolean
    queue_full(struct queue_t* const self);

    /**
     * Returns true if the queue is empty.
     * @param self: A pointer to the queue instance.
     */
    pboolean
    queue_empty(struct queue_t* const self);

    /**
     * Push an item onto the queue.
     * If the queue is full, this function blocks until space is available.
     * @param self: A pointer to the queue instance.
     * @param sample: The sample to push to the queue.
     */
    void
    queue_push(      struct queue_t* const self,
               const struct sens_sample_t  sample);

    /**
     * Pop an item from the queue.
     * If the queue is empty, this function blocks until an item is available.
     * @param self: A pointer to the queue instance.
     * @returns: A sample from the queue.
     */
    struct sens_sample_t
    queue_pop(struct queue_t* const self);

#endif // _QUEUE_USING_LINKED_LIST_H_INCLUDED
