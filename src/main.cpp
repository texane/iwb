#include <iostream>
using namespace std;

// Blob manager lib
#include "cvblob.h"
using namespace cvb;

// OpenCV lib
#if (defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__) || (defined(__APPLE__) & defined(__MACH__)))
#include <opencv2\highgui\highgui_c.h>
#include <opencv2\imgproc\types_c.h>
#include <opencv2\imgproc\imgproc_c.h>
#else
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgproc/imgproc_c.h>
#endif

int main(int argc, char *argv[])
{
    IplImage *image, *frame = 0;
    CvTracks tracks;
    char key;

    // Open webcam flux
    CvCapture *capture;
    capture = cvCreateCameraCapture( CV_CAP_ANY );
    double test = cvGetCaptureProperty(capture, CV_CAP_PROP_FORMAT );

    if (!capture)
        printf("Ouverture du flux vidéo impossible !\n");

    // Create window for display
    cvNamedWindow("IRStylus Window", CV_WINDOW_AUTOSIZE);

    // Loop for webcam flux
    while(key != 'q' && key != 'Q')
    {
        // Get image from webcam flux
        image = cvQueryFrame(capture);

        // If not already done, create result image to filter and display
        if( !frame )
            frame = cvCreateImage(cvGetSize(image), image->depth, image->nChannels);
        cvResetImageROI(frame);
        cvConvertScale(image, frame); // Nota: this method can scale and shift values if neccessary

        // Get specific channels
        IplImage *chB=cvCreateImage(cvGetSize(frame),8,1);
        IplImage *chV=cvCreateImage(cvGetSize(frame),8,1);
        IplImage *chR=cvCreateImage(cvGetSize(frame),8,1);
        cvSplit(frame, chB, chV, chR, 0);
 
        IplImage *infraRed = cvCreateImage(cvGetSize(frame),8,1);
        cvAddWeighted(chB, 0.33f, chV, 0.33f, 0.0f, infraRed);
        cvAddWeighted(infraRed, 0.33f, chR, 0.33f, 0.0f, infraRed);

        // Detect blobs
        CvBlobs blobs;
        IplImage *labelImg = cvCreateImage(cvGetSize(frame), IPL_DEPTH_LABEL, 1);
        unsigned int result = cvLabel(infraRed, labelImg, blobs);

        // Filter blobs
        cvFilterByArea(blobs, 500, 2000);
        cvUpdateTracks(blobs, tracks, 5., 10);

        cvRenderBlobs(labelImg, blobs, frame, frame, CV_BLOB_RENDER_CENTROID|CV_BLOB_RENDER_BOUNDING_BOX);
        cvRenderTracks(tracks, frame, frame, CV_TRACK_RENDER_ID|CV_TRACK_RENDER_BOUNDING_BOX|CV_TRACK_RENDER_TO_LOG);
        
        // Display image
        cvShowImage("IRStylus Window", frame);

        // Release objects
        cvReleaseBlobs(blobs);
        cvReleaseImage(&chB);
        cvReleaseImage(&chV);
        cvReleaseImage(&chR);
        cvReleaseImage(&labelImg);
        cvReleaseImage(&infraRed);

        // Wait for 10ms
        key = cvWaitKey(10);

    }

    // Release capture and close window
    cvReleaseCapture(&capture);
	cvDestroyWindow("IRStylus Window");
}
