#include <assert.h>

#include "queue.h"

// a single link list
typedef struct sens_sample_node_t sens_sample_node;
struct sens_sample_node_t
{
    sens_sample       sample_data;
    sens_sample_node* next_node;
};

struct queue_t
{
    sens_sample_node* head_data;
    sens_sample_node* tail_data;

    sens_sample_node* next_in;  // push
    sens_sample_node* next_out; // pop

    psize             len;

    PMutex*           mutex;
};

struct queue_t*
queue_create(const psize len)
{
    struct queue_t* self = NULL;

    do
    {
        self = p_malloc0(sizeof(struct queue_t));

        if (NULL == self)
        {
            printf("!!! not enough memory to create a new queue !!!\n");
            break;
        }

        sens_sample_node* last_node = NULL;

        for (psize idx = 0;
                   idx < len;
                   idx++)
        {
            sens_sample_node *new_node = p_malloc0(sizeof(sens_sample_node));

            if (NULL == new_node)
            {
                printf("!!! not enough memory to create a new sample node !!!\n");
                queue_destroy(self);

                break;
            }

            if (0 == idx)
            {
                self->head_data = new_node;
                last_node = new_node;

                continue;
            }

            if ((len - 1) == idx)
            {
                last_node->next_node = new_node;
                self->tail_data = new_node;

                break;
            }

            last_node->next_node = new_node;
            last_node = new_node;
        }

        if (NULL == self->head_data) {
            break;
        }

        self->mutex = p_mutex_new();

        if (NULL == self->mutex)
        {
            printf("!!! not enough memory to create a mutex !!!\n");
            queue_destroy(self);
            break;
        }

        self->len = len;
        self->next_in  = self->head_data;
        self->next_out = self->head_data;

    } while (0);

    return self;
}

void
queue_destroy(struct queue_t* const self)
{
    if (NULL == self) {
        return;
    }

    sens_sample_node *curr_node = self->head_data;

    printf("### %ld nodes of the queue will be released ###\n",
           self->len);

    while (NULL != curr_node)
    {
        sens_sample_node *last_node = curr_node;
        curr_node = last_node->next_node;
        p_free(last_node);
    }

    self->head_data = NULL;
    self->tail_data = NULL;
    self->next_in   = NULL;
    self->next_out  = NULL;

    if (self->mutex != NULL)
    {
        p_mutex_free(self->mutex);
        self->mutex = NULL;
    }

    p_free(self);
}

pboolean
queue_empty(struct queue_t* const self)
{
    return self->next_in == self->next_out;
}

void
queue_push(      struct queue_t* const self,
           const struct sens_sample_t  sample)
{
    if (NULL == self->next_in)
    {
        printf("!!! the %ld nodes of queue needs to add a new node !!!\n",
               self->len);

        sens_sample_node* new_node = NULL;

        do
        {
            new_node = p_malloc0(sizeof(sens_sample_node));

            if (NULL == new_node) {
                printf("!!! not enough memory to create a new node !!!\n");
            }

        } while ((NULL == new_node) &&
                 (NULL == self->next_in));

        if (NULL != new_node)
        {
            assert(TRUE == p_mutex_lock(self->mutex));

            if (NULL == self->next_in)
            {
                self->next_in              = new_node;
                self->next_in->sample_data = sample;
                self->next_in              = self->next_in->next_node;
            }

            self->tail_data->next_node = new_node;
            self->tail_data            = new_node;

            self->len++;

            p_mutex_unlock(self->mutex);
        }
    }
    else
    {
        assert(TRUE == p_mutex_lock(self->mutex));

        self->next_in->sample_data = sample;
        self->next_in              = self->next_in->next_node;

        p_mutex_unlock(self->mutex);
    }

    printf("### Saving sensor %d sample %d number %ld ###\n",
           sample.sens_id,
           sample.val,
           sample.num);
}

struct sens_sample_t
queue_pop(struct queue_t* const self)
{
    sens_sample sample;

    // the sample is initialized to invalid data
    memset(&sample,
           0,
           sizeof(sens_sample));

    do
    {
        if (self->next_in == self->next_out)
        {
            break;
        }

        assert(TRUE == p_mutex_lock(self->mutex));

        sample = self->next_out->sample_data;

        self->head_data = self->next_out->next_node;

        // recycle to reuse the handled node
        self->tail_data->next_node = self->next_out;
        self->tail_data            = self->next_out;
        self->tail_data->next_node = NULL;

        if (NULL == self->next_in) {
            self->next_in = self->tail_data;
        }

        self->next_out = self->head_data;

        p_mutex_unlock(self->mutex);

    } while (0);

    return sample;
}
