/*****************************************************************************/
/*!
    @file     orientation.c
    @author   ktownsend (Adafruit Industries)

    @section LICENSE

    Software License Agreement (BSD License)

    Copyright (c) 2017, Adafruit Industries (adafruit.com)
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
    THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/*****************************************************************************/
#include <math.h>
#include "orientation/orientation.h"

/**
 * Populates the .pitch/.roll fields in the or_orientation_vec struct
 * with the right angular data (in degree)
 *
 * @param  The sensors_event_t variable containing the data from the
 *         accelerometer
 * @param  The or_orientation_vec object that will have it's .pitch and
 *         .roll fields populated
 *
 * @return 0 on success, non-zero on failure
 */
int
or_from_accel(struct sensor_accel_data *accel, struct or_orientation_vec *orv)
{
  float t_pitch, t_roll;
  float const PI = 3.14159265F;
  float signOfZ = accel->sad_z >= 0 ? 1.0F : -1.0F;

  /* roll: Rotation around the longitudinal axis (the plane body, 'X axis'). */
  /* -90<=roll<=90                                                           */
  /* roll is positive and increasing when moving downward                    */
  /*                                                                         */
  /*                                 y                                       */
  /*             roll = atan(-----------------)                              */
  /*                          sqrt(x^2 + z^2)                                */
  /* where:  x, y, z are returned value from accelerometer sensor            */

  t_roll = accel->sad_x * accel->sad_x + accel->sad_z * accel->sad_z;
  orv->roll = (float)atan2(accel->sad_y, sqrt(t_roll)) * 180 / PI;

  /* pitch: Rotation around the lateral axis (the wing span, 'Y axis').      */
  /* -180<=pitch<=180)                                                       */
  /* pitch is positive and increasing when moving upwards                    */
  /*                                                                         */
  /*                                 x                                       */
  /*             pitch = atan(-----------------)                             */
  /*                          sqrt(y^2 + z^2)                                */
  /* where:  x, y, z are returned value from accelerometer sensor            */

  t_pitch = accel->sad_y * accel->sad_y + accel->sad_z * accel->sad_z;
  orv->pitch = (float)atan2(accel->sad_x, signOfZ * sqrt(t_pitch)) * 180 / PI;

  orv->heading = 0.0F;

  return (0);
}

/**
 * Populates the .roll/.pitch/.heading fields in the or_orientation_vec
 * struct with the right angular data (in degree).
 *
 * The starting position is set by placing the object flat and pointing
 * northwards (Z-axis pointing upward and X-axis pointing northwards).
 *
 * The orientation of the object can be modeled as resulting from 3
 * consecutive rotations in turn: heading (Z-axis), pitch (Y-axis), and roll
 * (X-axis) applied to the starting position.
 *
 * @param  The sensors_accel_data variable containing the data from the
 *         accelerometer
 * @param  The sensors_mag_data variable containing the data from the
 *         magnetometer
 * @param  The or_orientation_vec field that will have it's .roll, .pitch and
 *         .heading values populated
 *
 * @return 0 on success, non-zero on failure
 */
int
or_from_accel_mag(struct sensor_accel_data *accel, struct sensor_mag_data *mag,
    struct or_orientation_vec *orv)
{
    float const PI = 3.14159265F;

    /* roll: Rotation around the X-axis. -180 <= roll <= 180                */
    /* a positive roll angle is defined to be a clockwise rotation about    */
    /* the positive X-axis                                                  */
    /*                                                                      */
    /*                    y                                                 */
    /*      roll = atan2(---)                                               */
    /*                    z                                                 */
    /*                                                                      */
    /* where:  y, z are returned value from accelerometer sensor            */
    orv->roll = (float)atan2(accel->sad_y, accel->sad_z);

    /* pitch: Rotation around the Y-axis. -180 <= roll <= 180               */
    /* a positive pitch angle is defined to be a clockwise rotation about   */
    /* the positive Y-axis                                                  */
    /*                                                                      */
    /*                                 -x                                   */
    /*      pitch = atan(-------------------------------)                   */
    /*                    y * sin(roll) + z * cos(roll)                     */
    /*                                                                      */
    /* where:  x, y, z are returned value from accelerometer sensor         */
    if (accel->sad_y * sin(orv->roll) + accel->sad_z * cos(orv->roll) == 0) {
        orv->pitch = accel->sad_x > 0 ? (PI / 2) : (-PI / 2);
    } else {
        orv->pitch = (float)atan(-accel->sad_x / (accel->sad_y *
            sin(orv->roll) +
            accel->sad_z * cos(orv->roll)));
    }

    /* heading: Rotation around the Z-axis. -180 <= roll <= 180             */
    /* a positive heading angle is defined to be a clockwise rotation about */
    /* the positive Z-axis                                                  */
    /*                                                                      */
    /*                            z * sin(roll) - y * cos(roll)             */
    /*   heading = atan2(-------------------------------------------------) */
    /*                    x * cos(pitch) + y * sin(pitch) * sin(roll) +     */
    /*                             z * sin(pitch) * cos(roll))              */
    /*                                                                      */
    /* where:  x, y, z are returned value from magnetometer sensor          */
    orv->heading = (float)atan2(mag->smd_z * sin(orv->roll) -
        mag->smd_y * cos(orv->roll),
        mag->smd_x * cos(orv->pitch) +
        mag->smd_y * sin(orv->pitch) * sin(orv->roll) +
        mag->smd_z * sin(orv->pitch) * cos(orv->roll));

    /* Convert angular data to degree */
    orv->roll = orv->roll * 180 / PI;
    orv->pitch = orv->pitch * 180 / PI;
    orv->heading = orv->heading * 180 / PI;

    return (0);
}
