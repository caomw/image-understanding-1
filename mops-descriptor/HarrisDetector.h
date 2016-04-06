#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp> 
using namespace std;
using namespace cv;

void nonMaxSuppression(const Mat& srcRMatrix, Mat& dstRMatrix,  int bordSize=3);
map<int, map<int, float>> harrisDetector(string imagePath, int windowsSize,int sizeOfNMS, int thresholdOfR);
map<int, map<int, float>> harrisDetector(Mat srcGray, int windowsSize,int sizeOfNMS, int thresholdOfR);
void displayResult(Mat& srcRGB, map<int, map<int, float>>  harrisCorners, int windowSize);