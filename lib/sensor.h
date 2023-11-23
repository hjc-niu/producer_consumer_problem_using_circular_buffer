#ifndef _SENSOR_H_INCLUDED
    #define _SENSOR_H_INCLUDED

    #include "plibsys.h"

    struct sensor_t;

    /**
     * Sensor constructor.
     * @param id: The ID of the sensor.
     * @returns: A pointer to the sensor if successful, NULL otherwise.
     */
    struct sensor_t*
    sensor_create(const puint8 id);

    /**
     * Sensor destructor.
     * @param self: A pointer to the sensor instance.
     */
    void
    sensor_destroy(struct sensor_t* self);

    /**
     * Stop emitting readings. Readings may continue to emit from the sensor for up to 1000ms after this function is called.
     * @param self: A pointer to the sensor instance.
     */
    void
    sensor_stop(struct sensor_t* const self);

    /**
     * Read the latest emitted sample from the sensor.
     * @param self: A pointer to the sensor instance.
     * @returns: The sample.
     */
    puint32
    sensor_read(struct sensor_t* const self);

    /**
     * Returns true if a new sample has been emitted from the sensor since the last read.
     * @param self: A pointer to the sensor instance.
     */
    pboolean
    sensor_sample_rdy(const struct sensor_t* const self);

    /**
     * Get the number of samples emitted from the sensor.
     * @param self: A pointer to the sensor instance.
     */
    psize
    sensor_get_num_samples(const struct sensor_t* const self);

#endif // _SENSOR_H_INCLUDED
