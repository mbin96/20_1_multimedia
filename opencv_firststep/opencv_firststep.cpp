﻿#include "opencv2/opencv.hpp"
using namespace cv;
int main(int, char**) {
    VideoCapture cap(0); // open the default camera if (!cap.isOpened())  // check if we succeeded return -1;
    Mat edges, noB; 
    namedWindow("edges", 1); 
    for (;;) {
        Mat frame; 
        cap >> frame; // get a new frame from camera 
        cvtColor(frame, edges, COLOR_BGR2GRAY);   //<- OpenCV4.3에서는 “CV_BGR2GRAY”를 “COLOR_BGR2GRAY”로 
        noB = edges.clone();
        GaussianBlur(edges, edges, Size(7, 7), 1.5, 1.5); 
        Canny(edges, edges, 0, 30, 3);
        Canny(noB, noB, 0, 30, 3);
        imshow("no_gaussianBlur", noB);
        imshow("edges", edges); 
        imshow("orig", frame);
        if (waitKey(30) == 27) break; 
    } return 0;
}