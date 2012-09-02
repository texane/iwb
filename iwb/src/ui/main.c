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
  double a1;
  double b1;
  double c1;
  double a2;
  double b2;
  double c2;
  double a3;
  double b3;

} calib_data_t;


static void get_corrected_coords
(
 calib_data_t* calib,
 int im_x, int im_y,
 int* corr_x, int* corr_y
)
{
  /* from equation at http://www.zaunert.de/jochenz/wii:
     corr_x = (a1 * x + b1 * y + c1) / (a3 * x + b3 * y + 1);
     corr_y = (a2 * x + b2 * y + c2) / (a3 * x + b3 * y + 1);
  */

  const double x = (double)im_x;
  const double y = (double)im_y;

  const double a1 = calib->a1;
  const double a2 = calib->a2;
  const double a3 = calib->a3;
  const double b1 = calib->b1;
  const double b2 = calib->b2;
  const double b3 = calib->b3;
  const double c1 = calib->c1;
  const double c2 = calib->c2;

  *corr_x = (int)((a1 * x + b1 * y + c1 ) / (a3 * x + b3 * y + 1));
  *corr_y = (int)((a2 * x + b2 * y + c2 ) / (a3 * x + b3 * y + 1));
}

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
  CvMat* a;
  CvMat* x;
  CvMat* b;

  unsigned int i;

  a = cvCreateMat(8, 8, CV_32FC1);
  x = cvCreateMat(8, 1, CV_32FC1);
  b = cvCreateMat(8, 1, CV_32FC1);

  for (i = 0; i < 4; ++i)
  {
    const double obx = ob_points_arr[i * 3 + 0];
    const double oby = ob_points_arr[i * 3 + 1];
    const double imx = im_points_arr[i * 2 + 0];
    const double imy = im_points_arr[i * 2 + 1];

    /* fill a, the coefficient matrix */

    /* first equation, xcor = ... */
    cvSetReal2D(a, i * 2 + 0, 0, imx);
    cvSetReal2D(a, i * 2 + 0, 1, imy);
    cvSetReal2D(a, i * 2 + 0, 2, 1);
    cvSetReal2D(a, i * 2 + 0, 3, 0);
    cvSetReal2D(a, i * 2 + 0, 4, 0);
    cvSetReal2D(a, i * 2 + 0, 5, 0);
    cvSetReal2D(a, i * 2 + 0, 6, -1 * obx * imx);
    cvSetReal2D(a, i * 2 + 0, 7, -1 * obx * imy);

    /* second equation, ycor = ... */
    cvSetReal2D(a, i * 2 + 1, 0, 0);
    cvSetReal2D(a, i * 2 + 1, 1, 0);
    cvSetReal2D(a, i * 2 + 1, 2, 0);
    cvSetReal2D(a, i * 2 + 1, 3, imx);
    cvSetReal2D(a, i * 2 + 1, 4, imy);
    cvSetReal2D(a, i * 2 + 1, 5, 1);
    cvSetReal2D(a, i * 2 + 1, 6, -1 * oby * imx);
    cvSetReal2D(a, i * 2 + 1, 7, -1 * oby * imy);

    /* fill b, the result matrix */
    cvSetReal2D(b, i * 2 + 0, 0, obx);
    cvSetReal2D(b, i * 2 + 1, 0, oby);
  }

  /* solve the system */
  cvSolve(a, b, x, CV_LU);

  calib->a1 = cvGetReal2D(x, 0, 0);
  calib->b1 = cvGetReal2D(x, 1, 0);
  calib->c1 = cvGetReal2D(x, 2, 0);
  calib->a2 = cvGetReal2D(x, 3, 0);
  calib->b2 = cvGetReal2D(x, 4, 0);
  calib->c2 = cvGetReal2D(x, 5, 0);
  calib->a3 = cvGetReal2D(x, 6, 0);
  calib->b3 = cvGetReal2D(x, 7, 0);

  cvReleaseMat(&a);
  cvReleaseMat(&x);
  cvReleaseMat(&b);

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

  /* calibration data */
  calib_data_t calib;

  /* wb pointer */
  int wb_x;
  int wb_y;

} ui_state_t;


static inline int inverse_y(ui_state_t* ui, int y)
{
  return cvGetSize(ui->image).height - y - 1;
  /* return y; */
}

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

    y = inverse_y(ui, y);

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

    y = inverse_y(ui, y);

    get_corrected_coords(&ui->calib, x, y, &ui->wb_x, &ui->wb_y);
    if (ui->wb_x < 0) ui->wb_x = 0;
    else if (ui->wb_x > ui->rsize.width) ui->wb_x = ui->rsize.width;
    if (ui->wb_y < 0) ui->wb_y = 0;
    else if (ui->wb_y > ui->rsize.height) ui->wb_y = ui->rsize.height;

    printf("%d, %d -> %d, %d\n", x, y, ui->wb_x, ui->wb_y);
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
      points[0] = cvPoint(x - 3, inverse_y(ui, y - 3));
      points[1] = cvPoint(x + 3, inverse_y(ui, y + 3));
      cvRectangle(ui->image, points[0], points[1], blue, CV_FILLED, 8, 0);
    }

    /* redraw whitebaord */
    points[0] = cvPoint(ui->lsize.width, inverse_y(ui, 0));
    points[1] = cvPoint
      (points[0].x + ui->rsize.width - 1, inverse_y(ui, ui->rsize.height) - 1);
    cvRectangle(ui->image, points[0], points[1], white, CV_FILLED, 8, 0);

    /* redraw the pointer */
    points[0] = cvPoint(ui->lsize.width + ui->wb_x - 2, inverse_y(ui, ui->wb_y - 2));
    points[1] = cvPoint(points[0].x + 2, inverse_y(ui, ui->wb_y + 2));
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
  calibrate(ob_points, im_points, npoints, im_width, im_height, &ui->calib);

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
