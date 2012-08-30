#include "tni.h"


/* tni types */

typedef enum tni_error
{
  TNI_ERR_SUCCESS = 0,
  TNI_ERR_UNIMPL,
  TNI_ERR_MAX
} tni_error_t;


typedef struct tni_state
{
  /* TODO: video input opencv context */
  /* TODO: calibration matrix */
} tni_state_t;

typedef struct tni_conf
{
  /* TODO: video input */
} tni_conf_t;


tni_error_t tni_load_default_conf
(
 tni_state_t* s,
 const char* path
)
{
  return TNI_ERR_UNIMPL;
}

tni_error_t tni_load_conf_from_file
(
 tni_state_t* s,
 const char* path
)
{
  return TNI_ERR_UNIMPL;
}

tni_error_t tni_load_conf_from_av
(
 tni_state_t* s,
 int ac,
 const char** av
)
{
  return TNI_ERR_UNIMPL;
}

tni_error_t tni_init_with_conf
(
 tni_state_t* s,
 tni_conf_t* c
)
{
  return TNI_ERR_UNIMPL;
}

tni_error_t tni_fini(tni_state_t* s)
{
  return TNI_ERR_UNIMPL;
}

tni_error_t tni_get_coords
(
 tni_state_t* s,
 const unsigned int dims[2],
 unsigned int coords[2]
)
{
  /* s is the initialized tni state
   */

  /* dims (width, height) is the current UI surface
     dimensions it is needed to map from the physical
     space to the virtual panel space.
   */

  /* coords (x, y) is the resulting pointer coordinate
     in the virtual panel.
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

  return TNI_ERR_UNIMPL;
}


int main(int ac, char** av)
{
  tni_state_t state;
  tni_conf_t conf;
  tni_error_t err;
  unsigned int coords[2];
  unsigned int dims[2];

  tni_load_conf(&conf, av[0]);

  tni_init_with_conf(&state, &conf);

  /* TODO: initialize dims from X11 */
  dims[0] = 640;
  dims[1] = 480;

  while (1)
  {
    /* TODO: sleep */
    tni_get_coords(&state, dims, coords);
    printf("%u,%u\n", coords[0], coords[1]);
  }

  tni_fini(&state);

  return 0;
}
