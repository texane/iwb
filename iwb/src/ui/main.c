/* http://www.vision.caltech.edu/bouguetj/calib_doc/ */
/* opencv2 documentation */

#include <stdio.h>
#include <math.h>
#include <opencv2/core/core_c.h>
#include <opencv2/core/types_c.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/calib3d/calib3d.hpp>


/* calibration */

typedef struct calib_data
{
  CvMat* cam_mat;
  CvMat* dist_mat;
  CvMat* rot_vec;
  CvMat* rot_mat;
  CvMat* trans_vec;
} calib_data_t;

static int calibrate
(
 const double* ob_points_arr, /* 3 * npoints */
 const double* im_points_arr, /* 2 * npoints */
 unsigned int npoints,
 unsigned int im_width,
 unsigned int im_height,
 calib_data_t* calib
)
{
  CvMat* ob_points_mat;
  CvMat* im_points_mat;
  CvMat* counts_mat;
  CvSize im_size;
  unsigned int i;
  unsigned int j;

  ob_points_mat = cvCreateMat(npoints, 3, CV_32F);
  for (i = 0; i < npoints; ++i)
  {
    for (j = 0; j < 3; ++j)
    {
      const unsigned int k = i * 3 + j;
      cvSetReal2D(ob_points_mat, i, j, ob_points_arr[k]);
    }
  }

  im_points_mat = cvCreateMat(npoints, 2, CV_32F);
  for (i = 0; i < npoints; ++i)
  {
    for (j = 0; j < 2; ++j)
    {
      const unsigned int k = i * 2 + j;
      cvSetReal2D(im_points_mat, i, j, im_points_arr[k]);
    }
  }

  counts_mat = cvCreateMat(1, 1, CV_32S);
  cvSetReal2D(counts_mat, 0, 0, (double)npoints);

  im_size = cvSize((int)im_width, (int)im_height);

  calib->cam_mat = cvCreateMat(3, 3, CV_32F);
  calib->dist_mat = cvCreateMat(1, 5, CV_32F);
  calib->rot_vec = cvCreateMat(1, 3, CV_32F);
  calib->trans_vec = cvCreateMat(1, 3, CV_32F);

  /* initialize intrinsic so focal length have 1.0 ratio */
  cvSetReal2D(calib->cam_mat, 0, 0, 1.0);
  cvSetReal2D(calib->cam_mat, 1, 1, 1.0);

  cvCalibrateCamera2
  (
   ob_points_mat,
   im_points_mat,
   counts_mat,
   im_size,
   calib->cam_mat,
   calib->dist_mat,
   calib->rot_vec,
   calib->trans_vec,
   CV_CALIB_FIX_ASPECT_RATIO
  );

  /* turn into matrix */
  calib->rot_mat = cvCreateMat(3, 3, CV_32F);
  cvRodrigues2(calib->rot_vec, calib->rot_mat, NULL);
  
  cvReleaseMat(&ob_points_mat);
  cvReleaseMat(&im_points_mat);
  cvReleaseMat(&counts_mat);

  return 0;
}


/* user interface related routines
   the interface consists of 2 frames:
   the left frame is the webcam view,
   the right frame is virtual panel view.
   the stylus is simulated by by moving the mouse
   over the left view. the right view gets updated
   with the translated coordinates.
   in the real setup, the stylus position is got via
   via the blob detection algorithm.
*/

typedef struct ui_state
{
  /* main window and corresponding image */
  const char* name;
  IplImage* image;

  /* video capture */
  CvCapture* capture;

  /* frame dimensions */
  CvSize lsize;
  CvSize rsize;

  /* current ui mode */
  unsigned int is_calib;
  unsigned int is_done;

  /* calibration points */
  double* points;
  unsigned int cur_npoints;
  unsigned int max_npoints;

  /* wb pointer */
  unsigned int wb_x;
  unsigned int wb_y;

} ui_state_t;


static void on_mouse(int event, int x, int y, int flags, void* param)
{
  ui_state_t* const ui = (ui_state_t*)param;

  /* ui in calibration mode */
  if (ui->is_calib)
  {
    if (ui->cur_npoints == ui->max_npoints) return ;
    if (event != CV_EVENT_LBUTTONDOWN) return ;
    if ( ! ((x >= 0) && (x < (int)ui->lsize.width))) return ;
    if ( ! ((y >= 0) || (y < (int)ui->lsize.height))) return ;

    ui->points[ui->cur_npoints * 2 + 0] = (double)x;
    ui->points[ui->cur_npoints * 2 + 1] = (double)y;
    if ((++ui->cur_npoints) == ui->max_npoints) ui->is_done = 1;
  }
  else /* normal mode */
  {
    if (event != CV_EVENT_MOUSEMOVE) return ;
    if ((flags & CV_EVENT_FLAG_LBUTTON) == 0) return ;
    if ((x < 0) || (x >= (int)ui->lsize.width)) return ;
    if ((y < 0) || (y >= (int)ui->lsize.height)) return ;

    /* TODO: compute pointer coords */
    /* TODO: refresh right ui frame */
  }
}

static int ui_init
(
 ui_state_t* ui,
 unsigned int wb_width, unsigned int wb_height,
 unsigned int cam_index
)
{
  CvSize im_size;

  ui->capture = cvCreateCameraCapture(cam_index);
  if (ui->capture == NULL) return -1;

  ui->lsize.width = (int)cvGetCaptureProperty
    (ui->capture, CV_CAP_PROP_FRAME_WIDTH);
  ui->lsize.height = (int)cvGetCaptureProperty
    (ui->capture, CV_CAP_PROP_FRAME_HEIGHT);
  ui->rsize.width = wb_width;
  ui->rsize.height = wb_height;

  im_size.width = ui->lsize.width + ui->rsize.width;
  im_size.height = ui->lsize.height;
  if (ui->lsize.height < ui->rsize.height)
    im_size.height = ui->rsize.height;

  /* TODO: use CV_CAP_PROP_FORMAT */
  ui->image = cvCreateImage(im_size, IPL_DEPTH_8U, 3);
  ui->name = "ui";
  cvNamedWindow(ui->name, CV_WINDOW_AUTOSIZE);
  cvShowImage(ui->name, ui->image);

  cvSetMouseCallback(ui->name, on_mouse, (void*)ui);

  /* reset calibration points and whiteboard coords */
  ui->cur_npoints = 0;
  ui->wb_x = ui->rsize.width / 2;
  ui->wb_y = ui->rsize.height / 2;

  return 0;
}

static void ui_fini(ui_state_t* ui)
{
  cvDestroyWindow(ui->name);
  cvReleaseImage(&ui->image);
  cvReleaseCapture(&ui->capture);
}

static int ui_run_common(ui_state_t* ui)
{
  const CvScalar blue = cvScalar(0xff, 0, 0, 0);
  const CvScalar white = cvScalar(0xff, 0xff, 0xff, 0);

  IplImage* cap_image;
  CvRect left_roi;
  unsigned int x;
  unsigned int y;
  unsigned int i;
  CvPoint points[2];

  left_roi.x = 0;
  left_roi.y = 0;
  left_roi.width = ui->lsize.width;
  left_roi.height = ui->lsize.height;

  ui->is_done = 0;
  while (ui->is_done == 0)
  {
    /* refresh left frame */
    cap_image = cvQueryFrame(ui->capture);
    cvSetImageROI(ui->image, left_roi);
    cvCopy(cap_image, ui->image, NULL);
    cvResetImageROI(ui->image);

    /* poll for event */
    if ((cvWaitKey(1) & 0xff) == 27) return -1;

    /* redraw calibration points */
    for (i = 0; i < ui->cur_npoints; ++i)
    {
      x = ui->points[i * 2 + 0];
      y = ui->points[i * 2 + 1];
      if ((x - 3) < 0) x = 3;
      else if ((x + 3) > ui->lsize.width) x = ui->lsize.width - 3;
      if ((y - 3) < 0) y = 3;
      else if ((y + 3) > ui->lsize.height) y = ui->lsize.height - 3;
      points[0] = cvPoint(x - 3, y - 3);
      points[1] = cvPoint(x + 3, y + 3);
      cvRectangle(ui->image, points[0], points[1], blue, CV_FILLED, 8, 0);
    }

    /* redraw whitebaord and pointer */
    points[0] = cvPoint(ui->lsize.width, 0);
    points[1] = cvPoint(ui->lsize.width + ui->rsize.width, ui->rsize.height);
    cvRectangle(ui->image, points[0], points[1], white, CV_FILLED, 8, 0);
    points[0] = cvPoint(ui->lsize.width + ui->wb_x - 2, ui->wb_y - 2);
    points[1] = cvPoint(points[0].x + 2, points[0].y + 2);
    cvRectangle(ui->image, points[0], points[1], blue, CV_FILLED, 8, 0);

    /* refresh image */
    cvShowImage(ui->name, ui->image);
  }

  return 0;
}

static int ui_run_calibration(ui_state_t* ui)
{
  /* acquire image points. enter clockwise, topleft first. */

  static const unsigned int npoints = 4;

  const unsigned int im_width = (unsigned int)ui->lsize.width;
  const unsigned int im_height = (unsigned int)ui->lsize.height;
  const unsigned int wb_width = (unsigned int)ui->rsize.width;
  const unsigned int wb_height = (unsigned int)ui->rsize.height;

  double ob_points[npoints * 3];
  double im_points[npoints * 2];
  calib_data_t calib;

  ui->is_calib = 1;

  ui->points = im_points;
  ui->cur_npoints = 0;
  ui->max_npoints = npoints;

  if (ui_run_common(ui) == -1) return -1;

#if 0
  {
    unsigned int i;
    printf("im_points:\n");
    for (i = 0; i < npoints; ++i)
    {
      const unsigned int k = i * 2;
      printf("%lf, %lf\n", im_points[k + 0], im_points[k + 1]);
    }
  }
#endif

  /* object points, clockwise, topleft first */
  ob_points[0 * 3 + 0] = 0;
  ob_points[0 * 3 + 1] = wb_height;
  ob_points[0 * 3 + 2] = 0;
  ob_points[1 * 3 + 0] = wb_width;
  ob_points[1 * 3 + 1] = wb_height;
  ob_points[1 * 3 + 2] = 0;
  ob_points[2 * 3 + 0] = wb_width;
  ob_points[2 * 3 + 1] = 0;
  ob_points[2 * 3 + 2] = 0;
  ob_points[3 * 3 + 0] = 0;
  ob_points[3 * 3 + 1] = 0;
  ob_points[3 * 3 + 2] = 0;

  /* calibrate */
  calibrate(ob_points, im_points, npoints, im_width, im_height, &calib);

  return 0;
}

static int ui_run_whiteboard(ui_state_t* ui)
{
  ui->is_calib = 0;
  return ui_run_common(ui);
}

int main(int ac, char** av)
{
  static const unsigned int wb_width = 400;
  static const unsigned int wb_height = 400;
  static const unsigned int cam_index = 1; /* second webcam */

  ui_state_t ui;

  if (ui_init(&ui, wb_width, wb_height, cam_index))
  {
    printf("ui_init failed\n");
    return -1;
  }

  if (ui_run_calibration(&ui) != -1)
    ui_run_whiteboard(&ui);

  ui_fini(&ui);

  return 0;
}
