#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "sensor.h"

struct sensor_t
{
    puint32 val;
    pboolean val_hdld;
    psize num_samples;
    pboolean done;
    puint8 sensor_id;
    PMutex *mutex;
    PUThread *prod_th;
};

static ppointer
sensor_task(ppointer arg)
{
    struct sensor_t* self = (struct sensor_t*)arg;

    srand(self->sensor_id);

    puint32 prev_val = 1;

    while (TRUE)
    {
        // comment off these coding of the random-generating sample
        //    const puint32 rand_sleep_timeout = (rand() % 800) + 200;
        //    printf("### Sensor %d will sleep %d ###\n",
        //           self->sensor_id,
        //           rand_sleep_timeout);
        // fixed frequency is the sensor generating a sample every 200 milliseconds
        // to lead to the data accumulation
        const puint32 rand_sleep_timeout = 200;

        for (psize idx = 0;
                   idx < rand_sleep_timeout / 100;
                   idx++)
        {
            assert(p_uthread_sleep(100) == 0);

            if (self->done)
            {
                p_uthread_exit(0);
                return NULL;
            }
        }

        assert(p_mutex_lock(self->mutex) == TRUE);

        if (!self->val_hdld)
        {
            printf("[SENS %d] Dropped reading\n",
                   self->sensor_id);
        }

        self->val_hdld = FALSE;

        puint32 next_val = prev_val + self->val;
        prev_val = self->val;
        self->val = next_val;
        self->num_samples++;

        printf("[SENS %d] Sample num %ld ready: %d\n",
               self->sensor_id,
               self->num_samples,
               next_val);

        p_mutex_unlock(self->mutex);
    }

    return NULL;
}

struct sensor_t*
sensor_create(const puint8 id)
{
    struct sensor_t* self = p_malloc0(sizeof(struct sensor_t));

    if (self == NULL) {
        return NULL;
    }

    self->mutex = p_mutex_new();

    if (self->mutex == NULL) {
        sensor_destroy(self);
        return NULL;
    }

    self->prod_th = p_uthread_create(sensor_task,
                                     self,
                                     TRUE);

    if (self->prod_th == NULL) {
        sensor_destroy(self);
        return NULL;
    }

    self->sensor_id = id;
    self->val_hdld = TRUE;

    return self;
}

void
sensor_destroy(struct sensor_t* self)
{
    if (self == NULL) {
        return;
    }

    if (self->prod_th != NULL) {
        sensor_stop(self);
    }

    if (self->mutex != NULL)
    {
        p_mutex_free(self->mutex);
        self->mutex = NULL;
    }

    p_free(self);
}

void
sensor_stop(struct sensor_t *const self)
{
    self->done = TRUE;
    p_uthread_join(self->prod_th);
}

puint32
sensor_read(struct sensor_t *const self)
{
    assert(p_mutex_lock(self->mutex) == TRUE);

    self->val_hdld = TRUE;
    puint32 val = self->val;
    p_mutex_unlock(self->mutex);

    return val;
}

pboolean
sensor_sample_rdy(const struct sensor_t *const self) {
    return !self->val_hdld;
}

psize
sensor_get_num_samples(const struct sensor_t *const self) {
    return self->num_samples;
}
