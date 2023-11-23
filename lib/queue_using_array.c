#include <assert.h>

#include "queue.h"

struct queue_t
{
    struct sens_sample_t *data;
    psize len;
    psize next_in;
    psize next_out;

    PMutex *mutex;
    PCondVariable *non_empty_sig;
    PCondVariable *non_full_sig;
};

static inline psize
queue_incr(struct queue_t* const self,
                  psize          i)
{
    return (i + 1) % self->len;
}

struct queue_t*
queue_create(const psize len)
{
    struct queue_t* self = p_malloc0(sizeof(struct queue_t));

    if (self == NULL)
    {
        return NULL;
    }

    self->data = p_malloc0(sizeof(struct sens_sample_t) * len);

    if (self->data == NULL)
    {
        queue_destroy(self);
        return NULL;
    }

    self->mutex = p_mutex_new();

    if (self->mutex == NULL)
    {
        queue_destroy(self);
        return NULL;
    }

    self->non_empty_sig = p_cond_variable_new();

    if (self->non_empty_sig == NULL)
    {
        queue_destroy(self);
        return NULL;
    }

    self->non_full_sig = p_cond_variable_new();

    if (self->non_full_sig == NULL)
    {
        queue_destroy(self);
        return NULL;
    }

    self->len = len;
    self->next_in = 0;
    self->next_out = 0;

    return self;
}

void
queue_destroy(struct queue_t *const self)
{
    if (self == NULL) {
        return;
    }

    if (self->data != NULL)
    {
        p_free(self->data);
        self->data = NULL;
    }

    if (self->mutex != NULL)
    {
        p_mutex_free(self->mutex);
        self->mutex = NULL;
    }

    if (self->non_empty_sig != NULL)
    {
        p_cond_variable_free(self->non_empty_sig);
        self->non_empty_sig = NULL;
    }

    if (self->non_full_sig != NULL)
    {
        p_cond_variable_free(self->non_full_sig);
        self->non_full_sig = NULL;
    }

    p_free(self);
}

pboolean
queue_full(struct queue_t *const self)
{
    // Modified for print log
    //return queue_incr(self, self->next_in) == self->next_out;

    pboolean is_full = queue_incr(self, self->next_in) == self->next_out;

    if (is_full) {
        printf("!!! queue is full !!!");
    }

    return is_full;
}

pboolean
queue_empty(struct queue_t *const self)
{
    return self->next_in == self->next_out;
}

void
queue_push(      struct queue_t* const self,
           const struct sens_sample_t  sample)
{
    assert(p_mutex_lock(self->mutex) == TRUE);

    while (queue_full(self))
    {
        p_cond_variable_wait(self->non_full_sig, self->mutex);
    }

    self->data[self->next_in] = sample;
    self->next_in = queue_incr(self, self->next_in);
    p_mutex_unlock(self->mutex);
    p_cond_variable_signal(self->non_empty_sig);

    printf("### Saving sensor %d sample %d number %ld ###\n",
           sample.sens_id,
           sample.val,
           sample.num);
}

struct sens_sample_t
queue_pop(struct queue_t* const self)
{
    assert(p_mutex_lock(self->mutex) == TRUE);

    while (queue_empty(self))
    {
        p_cond_variable_wait(self->non_empty_sig,
                             self->mutex);
    }

    struct sens_sample_t sample = self->data[self->next_out];
    self->next_out = queue_incr(self, self->next_out);

    p_mutex_unlock(self->mutex);
    p_cond_variable_signal(self->non_full_sig);

    return sample;
}
