#ifndef __batyr_config_h__
#define __batyr_config_h__

#include "macros.h"


// version number ---------------------------------------------

#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_PATCH 0
#define VERSION_FULL STRINGIFY(VERSION_MAJOR) "." STRINGIFY(VERSION_MINOR) "." STRINGIFY(VERSION_PATCH)



// names -------------------------------------------------------
#define APP_NAME_SERVER         "batyrd"
#define APP_NAME_SERVER_FULL    APP_NAME_SERVER " v" VERSION_FULL



// internal server configuration -------------------------------

/**
 * the interval (in seconds) in which the server checks for finished
 * and failed jobs. This is not the time how long these jobs are kept
 * in memory, it just the interval which controlls how often the job
 * time-to-live is enforced.
 *
 * unit: seconds
 */
#define SERVER_JOB_CLEANUP_INTERVAL 20


/**
 * how many threads the internal http server should use.
 * The threads server both, the HTTP API as well as the graphical
 * web interface.
 *
 * unit: number of threads
 */
#define SERVER_HTTP_THREADS 10


#endif // __batyr_config_h__
