#include "iwb.h"


iwb_error_t iwb_conf_load_default
(
 iwb_state_t* state,
 iwb_conf_t* conf
)
{
  return IWB_ERR_UNIMPL;
}

iwb_error_t iwb_conf_load_file
(
 iwb_state_t* state,
 const char* path,
 iwb_conf_t* conf
)
{
  return IWB_ERR_UNIMPL;
}

iwb_error_t iwb_conf_load_av
(
 iwb_state_t* state,
 int ac,
 const char** av
)
{
  return IWB_ERR_UNIMPL;
}

iwb_error_t iwb_state_init
(
 iwb_state_t* state,
 iwb_conf_t* conf
)
{
  return IWB_ERR_UNIMPL;
}

iwb_error_t iwb_state_fini
(
 iwb_state_t* state
)
{
  return IWB_ERR_UNIMPL;
}

iwb_error_t iwb_get_coords
(
 iwb_state_t* state,
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
 iwb_state_t* state,
 int origin[2],
 int size_in_pixels[2],
 double size_in_user_units[2]
)
{
  /* set the window geometry for get_coords translation
   */

  return IWB_ERR_UNIMPL;
}

iwb_error_t iwb_calib_reset
(
 iwb_state_t* state
)
{
  /* delete the calibration points */
  /* set the calibration to identity */

  return IWB_ERR_UNIMPL;
}

iwb_error_t iwb_calib_add_point
(
 iwb_state_t* state,
 int cam_coords[2],
 int screen_coords[2]
)
{
  return IWB_ERR_UNIMPL;
}

iwb_error_t iwb_calib_update
(
 iwb_state_t* state
)
{
  return IWB_ERR_UNIMPL;
}
