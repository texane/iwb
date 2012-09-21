#ifndef IWB_H_INCLUDED
# define IWB_H_INCLUDED



/* iwb internal state, opaque to client
 */

typedef struct iwb_state
{
  /* TODO: video input opencv context */
  /* TODO: calibration matrix */
} iwb_state_t;


/* configuration, opaque to client
 */

typedef struct iwb_conf
{
  /* TODO: video input */
} iwb_conf_t;


/* error type
 */

typedef enum iwb_error
{
  IWB_ERR_SUCCESS = 0,
  IWB_ERR_UNIMPL,
  IWB_ERR_MAX
} iwb_error_t;


/* exported api
 */

/* internal state initialization
 */

iwb_error_t iwb_state_init
(
 iwb_state_t* state,
 iwb_conf_t* conf
);

iwb_error_t iwb_state_fini
(
 iwb_state_t* state
);


/* configuration loading
 */

iwb_error_t iwb_conf_load_default
(
 iwb_state_t* state,
 iwb_conf_t* conf
);

iwb_error_t iwb_conf_load_file
(
 iwb_state_t* state,
 const char* path,
 iwb_conf_t* conf
);

iwb_error_t iwb_conf_load_av
(
 iwb_state_t* state,
 int ac,
 const char** av
);


/* video frame grabbing
 */

iwb_error_t iwb_get_next_frame
(
 iwb_state_t* state
);


/* get the pointer coordinates
 */

iwb_error_t iwb_get_coords
(
 iwb_state_t* state,
 int coords[2],
 int* is_pointer_on
);


iwb_error_t iwb_set_window_geometry
(
 iwb_state_t* s,
 int origin[2],
 int size_in_pixels[2],
 double size_in_user_units[2]
);


iwb_error_t iwb_calib_reset
(
 iwb_state_t* state
);


iwb_error_t iwb_calib_add_point
(
 iwb_state_t* state,
 int cam_coords[2],
 int screen_coords[2]
);


iwb_error_t iwb_calib_update
(
 iwb_state_t* state
);



#endif /* ! IWB_H_INCLUDED */
