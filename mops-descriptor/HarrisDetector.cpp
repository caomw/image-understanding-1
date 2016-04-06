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
void nonMaxSuppression(const Mat& srcRMatrix, Mat& dstRMatrix,  int bordSize=3)
{
	int size = int((bordSize-1) / 2);
	Mat region(bordSize, bordSize, CV_8UC1);					//searching region for nonmax supression
	Mat zeroPad = Mat::zeros(bordSize, bordSize, CV_8UC1);		//matrix used to fill the R matrix with zeros

	//parameters for minMaxLoc function
	double minVal;
	double maxVal;
	Point minLoc;
	Point maxLoc;

	for (int i = 0; i < srcRMatrix.cols; i++)
	{
		for (int j = 0; j < srcRMatrix.rows; j++)
		{
			if ( (i-size) >= 0 && (i+size) < srcRMatrix.cols && (j-size) >= 0 && (j+size)< srcRMatrix.rows)
			{
				//NOTE: in Rect function, the last two paramter are width and height of matrix.
				region = srcRMatrix(Rect(i-size, j-size, bordSize, bordSize));
				minMaxLoc(region, &minVal, &maxVal, &minLoc, &maxLoc);

				Mat tmp = dstRMatrix(Rect(i-size, j-size, bordSize, bordSize));
				zeroPad.copyTo(tmp);

				Point pos = Point(i+maxLoc.x-size, j+maxLoc.y-size);
				dstRMatrix.at<uchar>(pos) = (uchar)maxVal;
			}
		}
	}
}

//My implementation of Harris Detection:http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.434.4816&rep=rep1&type=pdf
map<int, map<int, float>> harrisDetector(Mat srcGray, int windowsSize,int sizeOfNMS, int thresholdOfR)
{
	Mat gradientOperator = (Mat_ <int>(3,1) << -1, 0, 1);
	//Mat gradientOperator = (Mat_ <int>(5,1) << -2, -1, 0, 1, 2); //this is what parameter that said in slides, but the result is terrible
	Mat gradientOperatorTranspose;
	transpose(gradientOperator, gradientOperatorTranspose);

	Mat Ix;		//partial gradient of X direction
	Mat Iy;		//partial gradient of Y direction

	Mat IxIx;	//parameter to form Matrix M
	Mat IxIy;
	Mat IyIy;


	Mat A;		//    (A C)
	Mat B;		//M=  (C B)	
	Mat C;		//

	//Blur first to remove the impression of noise
	GaussianBlur(srcGray, srcGray, Size(3,3), 0);

//	Sobel(srcGray, Ix, -1,  1, 0, 3); //don't know why sobel opertors don't work
//	Sobel(srcGray, Iy, -1,  0, 1, 3);

	filter2D(srcGray, Ix, -1, gradientOperator);
	filter2D(srcGray, Iy, -1, gradientOperatorTranspose);

	IxIx = Ix.mul(Ix);
	IxIy = Ix.mul(Iy);
	IyIy = Iy.mul(Iy);

	Size kSize = Size(3,3);
	//NOTE: in these function ,the paramter should be changed to find better detectors
	GaussianBlur(IxIx, A, kSize, 0);
	GaussianBlur(IxIy, B, kSize, 0);
	GaussianBlur(IyIy, C, kSize, 0);

	//Use formula to calculate response matrix.
	Mat trace = A + B;
	Mat det = A.mul(B) - C.mul(C);
	float k = 0.06;
	Mat ROrigin = det - k * trace;

	Mat RThreshold = ROrigin.clone();
	Mat RSupress = ROrigin.clone();

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
	nonMaxSuppression(RThreshold, RSupress, sizeOfNMS);

	//Write out result. Use a 3 dimensional map to store the position and response value.
	map<int, map<int, float> > HarrisCorners;
	for (int i = 0; i < RSupress.rows; ++i)
	{
		for (int j = 0; j < RSupress.cols; ++j)
		{
			if (RSupress.at<uchar>(i, j) != '\0')
			{
				HarrisCorners[i][j] = RSupress.at<uchar>(i, j);
			}
		}
	}

	return HarrisCorners;
}

map<int, map<int, float>> harrisDetector(string imagePath, int windowsSize,int sizeOfNMS, int thresholdOfR)
{
	Mat srcGray = imread(imagePath, IMREAD_GRAYSCALE);
	return harrisDetector(srcGray, windowsSize, sizeOfNMS, thresholdOfR);
}


void displayResult(Mat& srcRGB, map<int, map<int, float>>  harrisCorners, int windowSize)
{
	//Add Harris detect result to RGB image
	Mat channels[3];
	split(srcRGB, channels);	//Split RGB image matrix to 3 channels, in the order of GBR. then change the red channel to label harris corner

	//It's too ugly and runs slowly here! 
	for (int i = 0; i < srcRGB.rows; ++i)
	{
		for (int j = 0; j < srcRGB.cols; ++j)
		{
			if (harrisCorners[i][j] != 0.0f && i-1 > 0 && i+1 < srcRGB.rows && j-1> 0 && j+1 < srcRGB.cols)
			{
				for (int m = -1; m < 2; ++m)
				{
					for (int n = -1; n < 2; ++n)
					{
						//NOTE: here we must use type larger than uchar, otherwise the result will mod 256 automatically,
						//since the largest number of uchar is 255
						int adaptedValue = channels[0].at<uchar>(i+m, j+n) + harrisCorners[i][j];
						if (adaptedValue > 255)	// if larger than 255, we must let it be 255, otherwise the pixel will change to blue
						{
							channels[2].at<uchar>(i+m, j+n) = 255;
						}
						else
						{
							channels[2].at<uchar>(i+m, j+n) = adaptedValue;
						}
					}
				}
			}
		}
	}

	merge(channels, 3, srcRGB);

	string strResult= "Result of Harris Detector";
	namedWindow(strResult, CV_WINDOW_AUTOSIZE);
	imshow(strResult, srcRGB);

	waitKey(0);
}
