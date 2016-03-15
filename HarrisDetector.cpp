#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp> 
using namespace std;
using namespace cv;

/*
 My implentation of Non maximum suppression
 NOTE: bordSize must be odd number.
*/
void nonMaxSuppression(Mat& R, int bordSize=3)
{
	int size = int((bordSize-1) / 2);
	Mat region(bordSize, bordSize, CV_8UC1);					//searching region for nonmax supression
	Mat zeroPad = Mat::zeros(bordSize, bordSize, CV_8UC1);		//matrix used to fill the R matrix with zeros

	//parameters for minMaxLoc function
	double minVal;
	double maxVal;
	Point minLoc;
	Point maxLoc;

	for (int i = 0; i < R.cols; i++)
	{
		for (int j = 0; j < R.rows; j++)
		{
			if ( (i-size) >= 0 && (i+size) < R.cols && (j-size) >= 0 && (j+size)< R.rows)
			{
				//NOTE: in Rect function, the last two paramter are width and height of matrix.
				region = R(Rect(i-size, j-size, bordSize, bordSize));
				minMaxLoc(region, &minVal, &maxVal, &minLoc, &maxLoc);

				Mat tmp = R(Rect(i-size, j-size, bordSize, bordSize));
				zeroPad.copyTo(tmp);

				Point pos = Point(i+maxLoc.x-size, j+maxLoc.y-size);
				R.at<uchar>(pos) = (uchar)maxVal;
			}
		}
	}
}

void harrisDetector(string imagePath, int thresholdOfR)
{
	Mat srcGray = imread(imagePath, IMREAD_GRAYSCALE);
	Mat srcRGB = imread(imagePath, IMREAD_COLOR);

	Mat gradientOperator = (Mat_ <int>(3,1) << -1, 0, 1);
	Mat gradientOperatorTranspose;
	transpose(gradientOperator, gradientOperatorTranspose);

	Mat Ix;		//partial gradient of X direction
	Mat Iy;		//partial gradient of Y direction

	Mat IxIx;	//parameter to form Matrix M
	Mat IxIy;
	Mat IyIy;

	Mat A;		//parameter in Matrix M
	Mat B;		//
	Mat C;		

	filter2D(srcGray, Ix, -1, gradientOperator);
	filter2D(srcGray, Iy, -1, gradientOperatorTranspose);

	IxIx = Ix.mul(Ix);
	IxIy = Ix.mul(Iy);
	IyIy = Iy.mul(Iy);

	Size kSize = Size(3,3);
	//NOTE: in these function ,the paramter should be changed to find better detectors
	GaussianBlur(IxIx, A, kSize, 3);
	GaussianBlur(IxIy, B, kSize, 3);
	GaussianBlur(IyIy, C, kSize, 3);

	//Use formula to calculate response matrix.
	Mat trace = A + B;
	Mat det = A.mul(B) - C.mul(C);
	float k = 0.06;
	Mat ROrigin = det - k * trace;

	Mat RThreshold = ROrigin.clone();
	Mat RSuupress = ROrigin.clone();

	//Find points with large corner response
	for (int i = 0;i < RThreshold.rows; i++)
	{
		for (int j = 0; j < RThreshold.cols; j++)
		{
			 uchar c = RThreshold.at<uchar>(i, j);
			 if (c > 0 && c < thresholdOfR)
			 {

				RThreshold.at<uchar>(i, j) = '\0';
			 }
		}
	}

	//Do non maximum supression
	nonMaxSuppression(RSuupress, 17);
	//Add Harris detect result to RGB image
	Mat channels[3];
	split(srcRGB, channels);	//Split RGB image matrix to 3 channels, in the order of GBR.
	channels[2] += RSuupress;	//Only add red channel image to original image.
	merge(channels, 3, srcRGB);


	string strROrigin = " Original R Matrix";
	string strRThreshold = "R Matrix After threshold choose";
	string strRSupress = " R Matrix After non max Supression";
	string strResult= "Result of Harris Detector";

	namedWindow(strROrigin, CV_WINDOW_AUTOSIZE);
	namedWindow(strRThreshold, CV_WINDOW_AUTOSIZE);
	namedWindow(strRSupress, CV_WINDOW_AUTOSIZE);
	namedWindow(strResult, CV_WINDOW_AUTOSIZE);

	imshow(strROrigin, ROrigin);
	imshow(strRThreshold, RThreshold);
	imshow(strRSupress, RSuupress);
	imshow(strResult, srcRGB);

	waitKey(0);
}


int main()
{
	//string imagePath = "logo.jpg";
	string imagePath = "animal.jpg";
	harrisDetector(imagePath, 230);
}