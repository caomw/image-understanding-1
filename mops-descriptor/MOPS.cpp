#include <iostream>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "HarrisDetector.h"
using namespace cv;
using namespace std;

//My implementation of MOPs descriptor
//NOTE: numOfScale doesn't include the extra layers used for completing gaps
vector<vector<vector<vector<float>>>> MOPsDescrpitor(std::string imagePath, int numOfOctave, int numOfScale, double sigma0)
{
	Mat src = imread(imagePath);

	// the data structure storing data in all octaves and layers
	vector<vector<Mat>> pyramids(numOfOctave, vector<Mat>(numOfScale, Mat()));

	//save the harris corner data for all scales.NOTE: numOfScale +3 is to include extra 3 layers
	vector<vector<map<int, map<int, float>>>> harrisCorners(numOfOctave, vector<map<int, map<int, float>>>(numOfScale+3, map<int, map<int,float>>())); 

	//save all MOPs features.NOTE: numOfScale +3 is to include extra 3 layers
	vector<vector<vector<vector<float>>>> mopsFeatures(numOfOctave, vector<vector<vector<float>>>(numOfScale+3, vector<vector<float>>(100, vector<float>(64,0.0f))));

	//0. Preprocess
	GaussianBlur(src, src, Size(3,3), 1.0);
	Mat srcGray;
	cvtColor(src, srcGray, CV_RGB2GRAY);

	namedWindow("image", CV_WINDOW_AUTOSIZE);
	imshow("image", src);
	
	//1. Build Laplacian pyramid 
	Mat tmp;
	double k = pow(2, double(1/numOfOctave));
	for (int i = 0; i < numOfOctave; ++i)
	{
		for (int j = 0; j < numOfScale + 3; ++j)
		{
			//initialize vectors

			//the fourth parameter is scale. let the size of first octave be original size *2.
			Laplacian(srcGray, tmp, -1, 3, (i+1) * 2 * sigma0 * std::pow(k, j)); 

			pyramids[i].push_back(tmp);

			//2. Find Harris corners in each scale
			map<int, map<int, float>> harrisTmp =harrisDetector(tmp, 15, 17, 240); 
			harrisCorners[i].push_back(harrisTmp);

			//displayResult(src, harrisTemp, 3);

			//3. extract feature on 8x8 patch 
			for (map<int, map<int,float>>::iterator k = harrisTmp.begin(); k != harrisTmp.end(); ++k)
			{
				int x =k->first;
				map<int, float> tmpMap = k->second;
				if (tmpMap.empty())
				{
					continue;
				}
				for (map<int, float>::iterator m = tmpMap.begin(); m != tmpMap.end(); ++m)
				{
					int y = m->first;
					float response = m->second;

					int bordSize = 4/(i+1); // half of patch size, considering the scale change.
					if (bordSize < 0)
					{
						break;
					}
					if (x-bordSize > 0 && x + bordSize< src.rows && y-bordSize > 0 && y+bordSize < src.cols)
					{
						Mat mopTmp;
						//cout << "bordSize:" << bordSize << "x+bordSize:" << x+bordSize << "y + bordSize" << y+bordSize<< endl;
						src(Rect(y-bordSize, x-bordSize, bordSize*2, bordSize*2)).copyTo(mopTmp);
						normalize(mopTmp, mopTmp);

						//convert Mat to vector
						vector<float> mop;
						if (mopTmp.isContinuous())
						{
							mop.assign(mopTmp.datastart, mopTmp.dataend);
						}
						else
						{
							for (int n = 0; n < mopTmp.rows; ++n)
							{
								mop.insert(mop.end(), mopTmp.ptr<float>(n), mopTmp.ptr<float>(n)+mopTmp.cols);
							}
						}

						//push mop feature
						mopsFeatures[i][j].push_back(mop);
					}
				}
			}
		}
	}

	return mopsFeatures;
}

int main()
{
	std::string imagePath = "animal.jpg";	
//	std::string imagePath = "logo.jpg";	

	//get mopFeatures of each image
	vector<vector<vector<vector<float>>>> mopFeatures =MOPsDescrpitor(imagePath, 2, 3, 1);

	return 0;
}