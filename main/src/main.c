#include "iwb.h"


/* iwb types */

typedef enum iwb_error
{
  IWB_ERR_SUCCESS = 0,
  IWB_ERR_UNIMPL,
  IWB_ERR_MAX
} iwb_error_t;


typedef struct iwb_state
{
  /* TODO: video input opencv context */
  /* TODO: calibration matrix */
} iwb_state_t;

typedef struct iwb_conf
{
  /* TODO: video input */
} iwb_conf_t;


iwb_error_t iwb_load_default_conf
(
 iwb_state_t* s,
 const char* path
)
{
  return IWB_ERR_UNIMPL;
}

iwb_error_t iwb_load_conf_from_file
(
 iwb_state_t* s,
 const char* path
)
{
  return IWB_ERR_UNIMPL;
}

iwb_error_t iwb_load_conf_from_av
(
 iwb_state_t* s,
 int ac,
 const char** av
)
{
  return IWB_ERR_UNIMPL;
}

iwb_error_t iwb_init_with_conf
(
 iwb_state_t* s,
 iwb_conf_t* c
)
{
  return IWB_ERR_UNIMPL;
}

iwb_error_t iwb_fini(iwb_state_t* s)
{
  return IWB_ERR_UNIMPL;
}

iwb_error_t iwb_get_coords
(
 iwb_state_t* s,
 int coords[2],
 int* is_pointer_on
)
{
  /* s is the initialized iwb state
   */

  /* coords (x, y) is the resulting pointer coordinate
     according to windows geometry (or screen if window
     geometry not present).
   */

  /* algorithm:
     . grab the current frame (possibly from cache)
     . check if calibration should be done
     .. either this is the first frame
     .. or the camera view changed
     . retrieve blob centroid
     . apply transformation matrix
     . map using dims
   */

  return IWB_ERR_UNIMPL;
}

iwb_error_t iwb_set_window_geometry
(
 iwb_state_t* s,
 int origin[2],
 int size_in_pixels[2],
 double size_in_user_units[2]
)
{
  /* set the window geometry for get_coords translation
   */
}

int main(int ac, char** av)
{
  iwb_state_t state;
  iwb_conf_t conf;
  iwb_error_t err;
  int is_on;
  int coords[2];
  int dims[2];

  iwb_load_conf(&conf, av[0]);

  iwb_init_with_conf(&state, &conf);

  /* TODO: initialize dims from X11 */
  dims[0] = 640;
  dims[1] = 480;

  while (1)
  {
    iwb_get_next_frame(&state);
    iwb_get_coords(&state, dims, coords, &is_on);
    printf("%d,%d,%d\n", is_on, coords[0], coords[1]);
  }

  iwb_fini(&state);

  return 0;
}
