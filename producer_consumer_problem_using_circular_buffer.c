#include <assert.h>
#include <stdio.h>

#include "plibsys.h"

#include "queue.h"
#include "sensor.h"

psize sens1_num_samples_proc = 0;
psize sens2_num_samples_proc = 0;
psize sens3_num_samples_proc = 0;

/**
 * Ensure to pass each sample from sensor 1 to process here
 * @param val: The sample of sensor 1
 */
static void
sens1_hdlr(puint32 val)
{
    //assert(p_uthread_sleep(100) == 0);
    // Let the sample processing speed be slower than the sample generation speed
    assert(p_uthread_sleep(300) == 0);

    sens1_num_samples_proc++;

    printf("Processing sensor 1 sample number %lu: %d\n",
           sens1_num_samples_proc,
           val);
}

/**
 * Ensure to pass each sample from sensor 2 to process here
 * @param val: The sample of sensor 2
 */
static void
sens2_hdlr(puint32 val)
{
  //assert(p_uthread_sleep(200) == 0);
  // Let the sample processing speed be slower than the sample generation speed
  assert(p_uthread_sleep(300) == 0);

  sens2_num_samples_proc++;

  printf("Processing sensor 2 sample number %lu: %d\n",
         sens2_num_samples_proc,
         val);
}

/**
 * Ensure to pass each sample from sensor 3 to process here
 * @param val: The sample of sensor 3
 */
static void sens3_hdlr(puint32 val)
{
  assert(p_uthread_sleep(300) == 0);

  sens3_num_samples_proc++;

  printf("Processing sensor 3 sample number %lu: %d\n",
         sens3_num_samples_proc,
         val);
}

static pboolean done = FALSE;

// --- START EDITING HERE ---
// Handler for the thread of handle samples from sensors
PUThread* collect_th = NULL;
PUThread* process_th = NULL;

struct queue_t* sensor_sample_queue = NULL;

struct sensorset_t {
    struct sensor_t* sens1;
    struct sensor_t* sens2;
    struct sensor_t* sens3;
};

static ppointer
collect_task(ppointer arg)
{
    struct sensorset_t* sensorset = (struct sensorset_t*)arg;

    struct sens_sample_t sens_sample_var;

    while (TRUE != done)
    {
        if (sensor_sample_rdy(sensorset->sens1))
        {
            sens_sample_var.sens_id = 1;
            sens_sample_var.val     = sensor_read(sensorset->sens1);
            sens_sample_var.num     = sensor_get_num_samples(sensorset->sens1);

            queue_push(sensor_sample_queue,
                       sens_sample_var);
        }

        if (sensor_sample_rdy(sensorset->sens2))
        {
            sens_sample_var.sens_id = 2;
            sens_sample_var.val     = sensor_read(sensorset->sens2);
            sens_sample_var.num     = sensor_get_num_samples(sensorset->sens2);

            queue_push(sensor_sample_queue,
                       sens_sample_var);
        }

        if (sensor_sample_rdy(sensorset->sens3))
        {
            sens_sample_var.sens_id = 3;
            sens_sample_var.val     = sensor_read(sensorset->sens3);
            sens_sample_var.num     = sensor_get_num_samples(sensorset->sens3);

            queue_push(sensor_sample_queue,
                       sens_sample_var);
        }
    }

    printf("### collect_task thread quit ###\n");

    return NULL;
}

static ppointer
process_task(ppointer arg)
{
    pboolean is_continue_running = TRUE;

    while (is_continue_running)
    {
        if ((TRUE == done) &&
            // must save all sample before exit
            (TRUE == queue_empty(sensor_sample_queue)))
        {
            is_continue_running = FALSE;
            continue;
        }

        if (queue_empty(sensor_sample_queue)) {
            continue;
        }

        struct sens_sample_t sens_sample_var = queue_pop(sensor_sample_queue);

        printf("### Handling sensor %d sample %d number %ld ###\n",
               sens_sample_var.sens_id,
               sens_sample_var.val,
               sens_sample_var.num);

        switch(sens_sample_var.sens_id) {
        case 1:
            sens1_hdlr(sens_sample_var.val);
            break;

        case 2:
            sens2_hdlr(sens_sample_var.val);
            break;

        case 3:
            sens3_hdlr(sens_sample_var.val);
            break;
        }
    }

    printf("### process_task thread quit ###\n");

    return NULL;
}
// --- STOP EDITING HERE ---

int
main(void)
{
    // Init plibsys.
    p_libsys_init();
    const char *const plib_ver = p_libsys_version();
    printf("PLIBSYS VERSION: %s\n", plib_ver);

    // Init sensors.
    struct sensor_t *const sens1 = sensor_create(1);
    assert(sens1 != NULL);
    struct sensor_t *const sens2 = sensor_create(2);
    assert(sens2 != NULL);
    struct sensor_t *const sens3 = sensor_create(3);
    assert(sens3 != NULL);

    // --- START EDITING HERE ---
    // Considering that memory addressing alignment affects the efficiency and correctness of processor access to data.
    // I chose 32 as the size of each sensor sample array because it is divisible by 8 with a margin.
    // Assume that the sample processing time of all three sensors takes 300 milliseconds,
    // and the fastest time for every sensor to generate a new sample is 200 milliseconds.
    // 1800ms = (300ms sensor1 sample + 300ms sensor2 sample + 300ms sensor3 sample) * 2 twice process
    // 27 samples = (1800ms / 200ms new sample) * 3 sensors
    // 32 = 27 + (8 - (27 % 8))
    sensor_sample_queue = queue_create(32);
    assert(sensor_sample_queue != NULL);

    struct sensorset_t* sensorset = p_malloc0(sizeof(struct sensorset_t));
    sensorset->sens1 = sens1;
    sensorset->sens2 = sens2;
    sensorset->sens3 = sens3;

    collect_th = p_uthread_create(collect_task, sensorset, TRUE);
    process_th = p_uthread_create(process_task, NULL, TRUE);
    // --- STOP EDITING HERE ---

    // Collect samples for 10 seconds.
    p_uthread_sleep(10000);

    // Stop collectin samples.
    printf("Stopping sensors...\n");
    sensor_stop(sens1);
    sensor_stop(sens2);
    sensor_stop(sens3);

    // Cleanup threads.
    printf("Stopping threads...\n");
    done = TRUE;

    // --- START EDITING HERE ---
    p_uthread_join(collect_th);
    p_uthread_join(process_th);

    if (NULL != sensorset) {
        p_free(sensorset);
    }

    if (NULL != sensor_sample_queue) {
        queue_destroy(sensor_sample_queue);
    }
    // --- STOP EDITING HERE ---

    // Calculate number of dropped samples.
    const psize sens1_num_dropped = sensor_get_num_samples(sens1) - sens1_num_samples_proc;
    const psize sens2_num_dropped = sensor_get_num_samples(sens2) - sens2_num_samples_proc;
    const psize sens3_num_dropped = sensor_get_num_samples(sens3) - sens3_num_samples_proc;

    printf("Number of samples from sensor 1 dropped: %lu\n", sens1_num_dropped);
    printf("Number of samples from sensor 2 dropped: %lu\n", sens2_num_dropped);
    printf("Number of samples from sensor 3 dropped: %lu\n", sens3_num_dropped);
}
