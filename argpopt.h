/**
 * ally-motion-evdev
 * Copyright (C) 2023 Kenny Wottrich
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 3 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#pragma once
#include<argp.h>
#include<string>

struct Config
{
    int rate = -1;
	std::string device;
    bool verbose = true;
};

const char *argp_program_version = "1.0";
const char *argp_program_bug_address = "<kenny.wottrich@gmail.com>";
static char doc[] = "Tool to redirect ROG Ally accelerometer and gyro data from iio to evdev ";
static char args_doc[] = "";

static struct argp_option options[] = 
{
  {"rate",   'r', "rate",       0, "Rate in Hz at which to run the acclerometer and gyroscope" },
  {"device", 'd', "iio device", 0, "The name of the iio device corresponding to the Bosch BMI323" },
  {"silent", 's', 0,            0, "Suppress all non-error output" },
  { 0 }
};


error_t parse_opt (int key, char *arg, struct argp_state *state)
{
    Config* config = reinterpret_cast<Config*>(state->input);
    switch (key)
    {
        case 'r':
            config->rate = atol(arg);
            break;
        case 'd':
            config->device.assign(arg);
            break;
        case 's':
            config->verbose = false;
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };


