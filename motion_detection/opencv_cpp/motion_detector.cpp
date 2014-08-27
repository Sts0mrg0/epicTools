
#include <stdio.h>
#include <cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include <string>
#include <iostream>
#include <fstream>


const int _TRESHOLD = 35;

std::string frameToTime(const int &frameNum, const int &fps){
    char digits[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
    std::string out;
    out = "00:00:00.00";
    int frm = frameNum % fps;
    int min = (frameNum / fps) / 60;
    int sec = (frameNum / fps) % 60;
    out[3] = digits[min/10];
    out[4] = digits[min%10];
    out[6] = digits[sec/10];
    out[7] = digits[sec%10];
    out[9] = digits[frm/10];
    out[10] = digits[frm%10];
    return out;
}

int main()
{
    std::vector<std::string> cuts;
    
    //file part
    std::ofstream cut_log;
    cut_log.open ("cut_log.txt");
    
    if 	(cut_log.is_open()){
        int key = 0;
    
        CvCapture * capture = cvCaptureFromFile( "output_1.mp4" );
        cv::VideoCapture cap("output_1.mp4");
        cv::Mat image_mat;
        cap.read(image_mat);
        CvMat oldMat = image_mat;
        CvScalar scal = cvGet2D( &oldMat,0,0);
        IplImage* frame = cvQueryFrame( capture );
        IplImage* currframe = cvCreateImage(cvGetSize(frame),IPL_DEPTH_8U,3);
        IplImage* destframe = cvCreateImage(cvGetSize(frame),IPL_DEPTH_8U,3);
    
        if ( !capture )
        
        {
            fprintf( stderr, "Cannot open AVI!\n" );
            return 1;
        }
        if ( !cap.isOpened() )  // if not success, exit program
        {
            std::cout << "Cannot open the video file" << std::endl;
            return -1;
        }
    
    
        int fps = ( int )cvGetCaptureProperty( capture, CV_CAP_PROP_FPS );
        std::cout << fps << " <<FPS\n";
        CvMat *mat = cvCreateMat(destframe->height,destframe->width,CV_32FC3 );
    
        int frameNum = 0;
        int prev_cutFrameNum = 0;
        long int summ = 0;
        std::string frameTime;
        bool cut_Flag = true;
        system("ls -lR > foo.txt");
        while( key != 'x' )
        {
            frame = cvQueryFrame( capture );
            if (!frame) break;
            currframe = cvCloneImage( frame );
            frame = cvQueryFrame( capture );
            if (!frame) break;

            cvSub(frame,currframe,destframe);
            cvConvert( destframe, mat);
            summ = 0;
            for(int i=0;i<1920;i++)
                    {
                        for(int j=0;j<1080;j++)
                        {
                            scal = cvGet2D( mat,j,i);
                            if (scal.val[0] > _TRESHOLD) {
                                summ += scal.val[0];
                                //std::cout<<i<<" "<<j<<" "<<scal.val[0]<<"\n";
                            }
                        }
                    }
        
            //If motion detected, set marker to stop
            if (summ != 0) {
                if (!cut_Flag){
                    if (!(frameNum - prev_cutFrameNum < fps)){
                        cut_Flag = true;
                        frameTime = frameToTime(frameNum, fps);
                        std::cout<<"x00 "<<frameTime<<" \t#Include from this frame: MOTION DETECTED \n";
                        cut_log<<"x00 "<<frameTime<<" \t#Include from this frame: MOTION DETECTED \n";
                        cuts.push_back(frameTime);
                        prev_cutFrameNum = frameNum;
                    }
                }
            }
        
            //If there is no motion in frame
            if (summ == 0 && cut_Flag){
                if (!(frameNum - prev_cutFrameNum < fps) || frameNum == 0){
                    cut_Flag = false;
                    frameTime = frameToTime(frameNum, fps);
                    std::cout<<"x01 "<<frameTime<<" \t#Cut here: NO MOTION FROM THIS FRAME \n";
                    cut_log<<"x01 "<<frameTime<<" \t#Cut here: NO MOTION FROM THIS FRAME \n";
                    cuts.push_back(frameTime);
                    prev_cutFrameNum = frameNum;
                }
            }
        
            if(key==27 )break;
            cvShowImage( "dest",destframe);
            key = cvWaitKey( 1000 / fps );
            frameNum += 2;
            //cvReleaseImage(&frame);
            cvReleaseImage(&currframe);
        }
        cvDestroyWindow( "dest" );
        cvReleaseCapture( &capture );
        cut_log.close();
        std::cout<<"VECTOR CONTAINS "<<cuts.size();
        return 0;
    } else {
        std::cout << "Unable to open file";
        return -2;
    }
}