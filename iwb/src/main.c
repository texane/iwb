#include <stdio.h>
#include "iwb.h"


/* unit testing
 */

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
