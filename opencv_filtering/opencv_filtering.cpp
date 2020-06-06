#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "opencv2/opencv.hpp"

using namespace cv;

void ex1(){
    Mat image(300, 300, CV_8UC3);
    // char fname[200];
    // printf("\nEnter image : ");
    // scanf("%s", fname);
    char fname[] = "photo.jpg";
    Mat sub = imread(fname);
    float x, y;
    // Project image onto newwith 45-deg rotation

    // for (int i = 0; i < sub.rows; i++)
    //     for (int j = 0; j < sub.cols; j++)
    //     {
    //         x = j * cos(0.85398) - i * sin(0.85398);
    //         y = j * sin(0.85398) + i * cos(0.85398);
    //         if (x + 90 >= 0 && y + 30 >= 0 && x + 90 < image.cols && y + 30 < image.rows)
    //             image.at<Vec3b>(y + 30, x + 90) = sub.at<Vec3b>(i, j);
        // }
    for (int i = 0; i < image.rows; i++)
        for (int j = 0; j < image.cols; j++)
        {
            // x = j * cos(0.85398) - i * sin(0.85398);
            // y = j * sin(0.85398) + i * cos(0.85398);
            // if (x + 90 >= 0 && y + 30 >= 0 && x + 90 < image.cols && y + 30 < image.rows)
            //     image.at<Vec3b>(y + 30, x + 90) = sub.at<Vec3b>(i, j);
            int jj = j-90 ,ii = i-30;
            x = (jj * cos(0.85398) + ii * sin(0.85398));
            y = -1 * jj * sin(0.85398) + ii * cos(0.85398);
            if (x >= 0 && y >= 0 && x < sub.cols && y  < sub.rows)
                image.at<Vec3b>(i,j) = sub.at<Vec3b>(y, x );
        }

    // Draw an ellipse
    RotatedRect rotrect(Point(100, 20), Size(90, 170), 101);
    ellipse(image, rotrect, Scalar(0, 0, 255), 3);
    // Draw a circle
    circle(image, Point(240, 200), 25, Scalar(255, 0, 0, 0), -1);
    // Draw a box
    rectangle(image, Point(30, 190), Point(150, 270), Scalar(0, 255, 0), 1);
    // Place Text

    putText(image, "Hello World!", Point(50, 200), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255));
    putText(image, "popipapapipopa!", Point(10, 150), 
            FONT_HERSHEY_SCRIPT_SIMPLEX,1,Scalar(0,0,255));
    do {
        imshow("sub image", image);
        if (waitKey(30) == 27)
            break; 
    }while (1);
}

struct OPTIONS
    {
        OPTIONS() : X(-1), Y(-1), drawing_dot(false) {}
        int X;
        int Y;
        bool drawing_dot;
    };
    OPTIONS options;
void my_mouse_callback(int event, int x, int y, int flags, void *param)
{
    //IplImage 삭제됨. mat 사용
    Mat *image = (Mat *)param;
    switch (event)
    {
        //CV_ 접두사 없애야 동작
    case EVENT_LBUTTONDOWN:
        options.X = x;
        options.Y = y;
        options.drawing_dot = true;
        break;
    }
}
void ex2()
{
    
    //IplImage 삭제됨. mat 사용
    Mat frame = imread("photo.jpg");
    namedWindow("Window1");
    setMouseCallback("Window1", my_mouse_callback, (void *)&frame);
    // Take new points from user
    while (waitKey(15) != 27)
    {
        if (options.drawing_dot)
        {
            rectangle(frame, Point(options.X-4, options.Y-4), Point(options.X+4, options.Y+4), Scalar(0, 0, 255), 2);
            options.drawing_dot = false;
        }
        imshow("Window1", frame);
        waitKey(10);
    }
    // releaseImage(&image);
}

int main()
{
    //ex1();
    ex2();
}