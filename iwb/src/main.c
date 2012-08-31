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

  return IWB_ERR_UNIMPL;
}

iwb_error_t iwb_reset_calib(iwb_state_t* s)
{
  /* delete the calibration points */
  /* set the calibration to identity */

  return IWB_ERR_UNIMPL;
}

iwb_errort iwb_add_calib_point
(
 iwb_state_t* state,
 int cam_coords[2],
 int screen_coords[2]
)
{
  return IWB_ERR_UNIMPL;
}


static void do_iwb_calib(iwb_state_t* state)
{
  /* calibration procedure example */

  iwb_reset_matrix(state);

  /* Lop over 4 calibration points. */

  for (int i = 0; i < 4; i++)
  {
    int screen_coords[2] = {}; /* (i-ème coin de l'écran) */
    int cam_coords[2];
    int pointer_on;
    display_calibration_target(screen_coords);
    ask_user_to_click_on_target();
    /* Wait for the user to release the button... */
    do {
      iwb_get_next_frame(state);
      iwb_get_coords(state, &pointer_on, cam_coords);
    } while (pointer_on);
    /* ...and then to click again. */
    do {
      iwb_get_next_frame(state);
      iwb_get_coords(state, &pointer_on, cam_coords);
    } while (!pointer_on);
    iwb_add_calibration_point(state, cam_coords, screen_coords);
  }
  iwb_update_calib_matrix(state);
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

  do_iwb_calibration(&state);

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
