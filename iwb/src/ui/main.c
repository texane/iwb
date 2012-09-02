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

  double a1;
  double b1;
  double c1;
  double a2;
  double b2;
  double c2;
  double a3;
  double b3;

} calib_data_t;


typedef struct point2d
{
  double x;
  double y;
} point2d_t;


static void get_coord_jochenz
(
 calib_data_t* calib,
 int im_x, int im_y,
 int* corr_x, int* corr_y
)
{
  /* from equation at http://www.zaunert.de/jochenz/wii:
     corrected_x = (a1 * x + b1 * y + c1) / (a3 * x + b3 * y + 1);
     corrected_y = (a2 * x + b2 * y + c2) / (a3 * x + b3 * y + 1);
  */

  const double X = (double)im_x;
  const double Y = (double)im_y;

  const double a1 = calib->a1;
  const double a2 = calib->a2;
  const double a3 = calib->a3;
  const double b1 = calib->b1;
  const double b2 = calib->b2;
  const double b3 = calib->b3;
  const double c1 = calib->c1;
  const double c2 = calib->c2;

  const double corrX = (a1 * X + b1 * Y + c1 ) / (a3 * X + b3 * Y + 1);
  const double corrY = (a2 * X + b2 * Y + c2 ) / (a3 * X + b3 * Y + 1);

  *corr_x = (int)corrX;
  *corr_y = (int)corrY;
}

static void calibrate_jochenz
(
 calib_data_t* calib,
 const point2d_t* dots,
 const point2d_t* cal
)
{
  double matrix[8][8] =
  {
    { -1, -1, -1, -1, 0, 0, 0, 0 },
    { -cal[0].x, -cal[1].x, -cal[2].x, -cal[3].x, 0, 0, 0, 0 },
    { -cal[0].y, -cal[1].y, -cal[2].y, -cal[3].y, 0,0,0,0 },
    { 0,0,0,0,-1,-1,-1,-1 },
    { 0,0,0,0, -cal[0].x, -cal[1].x, -cal[2].x, -cal[3].x },
    { 0,0,0,0, -cal[0].y, -cal[1].y, -cal[2].y, -cal[3].y },
    { cal[0].x * dots[0].x, cal[1].x * dots[1].x, cal[2].x * dots[2].x, cal[3].x * dots[3].x, cal[0].x * dots[0].y, cal[1].x * dots[1].y, cal[2].x * dots[2].y, cal[3].x * dots[3].y },
    { cal[0].y * dots[0].x, cal[1].y * dots[1].x, cal[2].y * dots[2].x, cal[3].y * dots[3].x, cal[0].y * dots[0].y, cal[1].y * dots[1].y, cal[2].y * dots[2].y, cal[3].y * dots[3].y },
  };

  double bb[8] =
  {
    -dots[0].x, -dots[1].x, -dots[2].x, -dots[3].x, -dots[0].y, -dots[1].y, -dots[2].y, -dots[3].y
  };

  unsigned int i;
  unsigned int j;

  /* gauss elimination */
  for( j = 1; j < 4; j ++ )
{

   for( i = 1; i < 8; i ++ )
   {
      matrix[i][j] = - matrix[i][j] + matrix[i][0];
   }
   bb[j] = -bb[j] + bb[0];
   matrix[0][j] = 0;

}


  for( i = 2; i < 8; i ++ )
  {
    matrix[i][2] = -matrix[i][2] / matrix[1][2] * matrix[1][1] + matrix[i][1];
  }
bb[2] = - bb[2] / matrix[1][2] * matrix[1][1] + bb[1];
matrix[1][2] = 0;


  for( i = 2; i < 8; i ++ )
  {
    matrix[i][3] = -matrix[i][3] / matrix[1][3] * matrix[1][1] + matrix[i][1];
  }
bb[3] = - bb[3] / matrix[1][3] * matrix[1][1] + bb[1];
matrix[1][3] = 0;



  for( i = 3; i < 8; i ++ )
  {
    matrix[i][3] = -matrix[i][3] / matrix[2][3] * matrix[2][2] + matrix[i][2];
  }
bb[3] = - bb[3] / matrix[2][3] * matrix[2][2] + bb[2];
matrix[2][3] = 0;

for( j = 5; j < 8; j ++ )
{
  for( i = 4; i < 8; i ++ )
  {
     matrix[i][j] = -matrix[i][j] + matrix[i][4];
  }
  bb[j] = -bb[j] + bb[4];
  matrix[3][j] = 0;
}


for( i = 5; i < 8; i ++ )
  {
    matrix[i][6] = -matrix[i][6] / matrix[4][6] * matrix[4][5] + matrix[i][5];
  }

bb[6] = - bb[6] / matrix[4][6] * matrix[4][5] + bb[5];
matrix[4][6] = 0;


for( i = 5; i < 8; i ++ )
  {
    matrix[i][7] = -matrix[i][7] / matrix[4][7] * matrix[4][5] + matrix[i][5];
  }
bb[7] = - bb[7] / matrix[4][7] * matrix[4][5] + bb[5];
matrix[4][7] = 0;


for( i = 6; i < 8; i ++ )
  {
    matrix[i][7] = -matrix[i][7] / matrix[5][7] * matrix[5][6] + matrix[i][6];
  }
bb[7] = - bb[7] / matrix[5][7] * matrix[5][6] + bb[6];
matrix[5][7] = 0;



matrix[7][7] = - matrix[7][7]/matrix[6][7]*matrix[6][3] + matrix[7][3];
bb[7] = -bb[7]/matrix[6][7]*matrix[6][3] + bb[3];
matrix[6][7] = 0;

const double b3 =  bb[7] /matrix[7][7];
const double a3 = (bb[3]-(matrix[7][3]*b3))/matrix[6][3];
const double b2 = (bb[6]-(matrix[7][6]*b3+matrix[6][6]*a3))/matrix[5][6];
const double a2 = (bb[5]-(matrix[7][5]*b3+matrix[6][5]*a3+matrix[5][5]*b2))/matrix[4][5];
const double c2 = (bb[4]-(matrix[7][4]*b3+matrix[6][4]*a3+matrix[5][4]*b2+matrix[4][4]*a2))/matrix[3][4];
const double b1 = (bb[2]-(matrix[7][2]*b3+matrix[6][2]*a3+matrix[5][2]*b2+matrix[4][2]*a2+matrix[3][2]*c2))/matrix[2][2];
const double a1 = (bb[1]-(matrix[7][1]*b3+matrix[6][1]*a3+matrix[5][1]*b2+matrix[4][1]*a2+matrix[3][1]*c2+matrix[2][1]*b1))/matrix[1][1];
const double c1 = (bb[0]-(matrix[7][0]*b3+matrix[6][0]*a3+matrix[5][0]*b2+matrix[4][0]*a2+matrix[3][0]*c2+matrix[2][0]*b1+matrix[1][0]*a1))/matrix[0][0];

calib->a1 = a1;
calib->a2 = a2;
calib->a3 = a3;
calib->b1 = b1;
calib->b2 = b2;
calib->b3 = b3;
calib->c1 = c1;
calib->c2 = c2;
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
#if 0

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

#else

#if 0

  CvMat* const a = cvCreateMat(8, 8, CV_32FC1);
  CvMat* const x = cvCreateMat(8, 1, CV_32FC1);
  CvMat* const b = cvCreateMat(8, 1, CV_32FC1);

  unsigned int i;

  cvSetReal2D(a, 0, 0, );

  for (i = 0; i < 8; ++i) cvSetReal2D(x, i, 0, 0);

  cvSolve(&a, &b, &x);

  calib->a1 = cvGetReal2D(x, 0, 0);
  calib->b1 = cvGetReal2D(x, 1, 0);
  calib->c1 = cvGetReal2D(x, 1, 0);
  calib->a2 = cvGetReal2D(x, 1, 0);
  calib->b2 = cvGetReal2D(x, 1, 0);
  calib->c2 = cvGetReal2D(x, 1, 0);
  calib->a3 = cvGetReal2D(x, 1, 0);
  calib->a4 = cvGetReal2D(x, 1, 0);

  cvReleaseMat(a);
  cvReleaseMat(x);
  cvReleaseMat(b);

#else

  /* assume npoints == 4 */

  point2d_t dots[4];
  point2d_t cal[4];
  unsigned int i;

  for (i = 0; i < 4; ++i)
  {
    dots[i].x = ob_points_arr[i * 3 + 0];
    dots[i].y = ob_points_arr[i * 3 + 1];

    cal[i].x = im_points_arr[i * 2 + 0];
    cal[i].y = im_points_arr[i * 2 + 1];
  }

  calibrate_jochenz(calib, dots, cal);

#endif

#endif

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

    get_coord_jochenz(&ui->calib, x, y, &ui->wb_x, &ui->wb_y);
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
