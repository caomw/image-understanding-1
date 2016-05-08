///////////////////////////////////////////////////////////////////////////
//
// NAME
//  FeatureAlign.h -- image registration using feature matching
//
// SEE ALSO
//  FeatureAlign.h      longer description
//
// Copyright ?Richard Szeliski, 2001.  See Copyright.h for more details
// (modified for CSE576 Spring 2005)
//
///////////////////////////////////////////////////////////////////////////

#include <iostream>
#include "ImageLib/ImageLib.h"
#include "FeatureAlign.h"
#include <math.h>

using namespace std;
/******************* TO DO *********************
* alignPair:
*	INPUT:
*		f1, f2: source feature sets
*		matches: correspondences between f1 and f2
*               *NOTE* Each match in 'matches' contains two feature ids of matching features, id1 (in f1) and id2 (in f2).
*               These ids are 1-based indices into the feature arrays,
*               so you access the appropriate features as f1[id1-1] and f2[id2-1].
*		m: motion model
*		f: focal length
*		nRANSAC: number of RANSAC iterations
*		RANSACthresh: RANSAC distance threshold
*		M: transformation matrix (output)
*	OUTPUT:
*		repeat for nRANSAC iterations:
*			choose a minimal set of feature matches
*			estimate the transformation implied by these matches
*			count the number of inliers
*		for the transformation with the maximum number of inliers,
*		compute the least squares motion estimate using the inliers,
*		and store it in M
*/
int alignPair(const FeatureSet &f1, const FeatureSet &f2,
              const vector<FeatureMatch> &matches, MotionModel m, float f,
              int nRANSAC, double RANSACthresh, CTransform3x3& M)
{
    // BEGIN TODO
    // write this entire method

/*	if (m == 0)  // only contains translation
	{
		int NumOfIter = 0;
		int maxNumOfInliers = 0;
		vector<int> bestInliers;
		CTransform3x3 bestM;

		while (NumOfIter < nRANSAC)
		{
			++NumOfIter;
			if (NumOfIter == 158)
			{
				int a = 0;
				int b = a+3;
			}
			int size = matches.size();
			int randNum = 0 + (rand()  % (int)(size - 0)); // pick up a number in range randomly
			int id1 = matches[randNum].id1;
			int id2 = matches[randNum].id2;
			//cout << "algin- iter:" << NumOfIter <<", id1:" << id1 << ", id2:" << id2 <<endl;

			if(id1< 1 || id2 < 1) continue;
			Feature randF1 = f1[id1 - 1];
			Feature randF2 = f2[id2 - 1];
		
			//calculate translate parameters
			double u = randF2.x - randF1.x;
			double v = randF2.y - randF1.y;

			//construct M matrix
			M[0][0] = 1;M[0][1] = 0;M[0][2] = u;
			M[1][0] = 0;M[1][1] = 1;M[1][2] = v;
			M[2][0] = 0;M[2][1] = 0;M[2][2] = 1;

			//Computer the number of inliners
			vector<int> inliers;
			countInliers(f1, f2, matches, m, f, M, RANSACthresh, inliers);
			leastSquaresFit(f1, f2, matches, m, f, inliers, M);
			if (maxNumOfInliers < inliers.size())
			{
				maxNumOfInliers = inliers.size();
				bestInliers = inliers;
//				bestM = M;
			}

		}
//		M = bestM;
		leastSquaresFit(f1, f2, matches, m ,f, bestInliers, M);
		
	}
	else // contains rotation. Not finish yet.
	{
	}
	*/
	int d=1, bestCount = -1;
    vector<int> tryMatches, inliers, bestInliers;
    for (int i=0;i<nRANSAC;i++){
      tryMatches.clear();

     
      while (tryMatches.size() < d){
        int s = rand() % matches.size();
        // deal with dupes
        bool dupe = false;
        for (int j=0;j<tryMatches.size();j++){
            if (s == tryMatches[j]){ dupe = true; }
        }
        if ( !dupe ) { tryMatches.push_back(s); }
      }
      CTransform3x3 trans;
      leastSquaresFit(f1, f2, matches, m, f, tryMatches, trans);

      countInliers(f1, f2, matches, m, f, trans, RANSACthresh, inliers);

      if (((int)inliers.size()) > bestCount){
        bestCount = ((int)inliers.size());
        bestInliers = inliers;
      }
    }
    fprintf(stderr, "num_inliers: %d / %d\n", bestCount, (int)matches.size());
    leastSquaresFit(f1, f2, matches, m, f, bestInliers, M);
    // END TODO

    return 0;
}

/******************* TO DO *********************
* countInliers:
*	INPUT:
*		f1, f2: source feature sets
*		matches: correspondences between f1 and f2
*               *NOTE* Each match contains two feature ids of matching features, id1 (in f1) and id2 (in f2).
*               These ids are 1-based indices into the feature arrays,
*               so you access the appropriate features as f1[id1-1] and f2[id2-1].
*		m: motion model
*		f: focal length
*		M: transformation matrix
*		RANSACthresh: RANSAC distance threshold
*		inliers: inlier feature IDs
*	OUTPUT:
*		transform the matched features in f1 by M
*
*		count the number of matching features for which the transformed
*		feature f1[id1-1] is within SSD distance RANSACthresh of its match
*		f2[id2-1]
*
*		store the indices of these matches in inliers
*
*		
*/
int countInliers(const FeatureSet &f1, const FeatureSet &f2,
                 const vector<FeatureMatch> &matches, MotionModel m, float f,
                 CTransform3x3 M, double RANSACthresh, vector<int> &inliers)
{
    inliers.clear();
    int count = 0;

    for (unsigned int i=0; i<(int) matches.size(); i++) {
        // BEGIN TODO
        // determine if the ith matched feature f1[id1-1], when transformed by M,
        // is within RANSACthresh of its match in f2
        //
        // if so, increment count and append i to inliers
        //
        // *NOTE* Each match contains two feature ids of matching features, id1 and id2.
        //        These ids are 1-based indices into the feature arrays,
        //        so you access the appropriate features as f1[id1-1] and f2[id2-1].
		int id1 = matches[i].id1;
		int id2 = matches[i].id2;
		
	//	cout << "count-id1:" << id1 << ", id2:" << id2 << endl;
		
		
		/*
		Feature a = f1[matches[i].id1 - 1];
        Feature b = f2[matches[i].id2 - 1];
		 CVector3 p1(a.x, a.y, 1);
        p1 = M*p1; // perform the transform
        // scale into 2D
        p1[0] /= p1[2];
        p1[1] /= p1[2];
        // diffs
        double d1 = p1[0] - b.x;
        double d2 = p1[1] - b.y;

        if (d1*d1 + d2*d2 < RANSACthresh * RANSACthresh){
            inliers.push_back(i);
			++count;
        }
		*/
		
		Feature feature1 = f1[id1 - 1];
		Feature feature2 = f2[id2 - 1];

		double translatedX = 0;
		double translatedY = 0;
		
		bool findTarget = false;
		if (m == 0) // Only translate, how To translate feature 
		{
			translatedX = feature1.x + M[0][2];
			translatedY = feature1.y + M[1][2];
		}
		else //Rotation, how to rotate feature?
		{

		}
		
		//compare the distance of moved f1 and f2
		//TODO: how to calculate the distance? -> SSD 

//		double matchedDist = pow(feature1.x - feature2.x, 2) + pow(feature1.y - feature2.y, 2);
		double d1 = feature2.x - translatedX;
		double d2 = feature2.y - translatedY;

	
		if (d1 * d1 + d2 * d2 < RANSACthresh * RANSACthresh)
		{
			inliers.push_back(i); //NOTE: add index of FeatureMatch in inlier.
			++count;
		}


        // END TODO
    }

    return inliers.size();
}

/******************* TO DO *********************
* leastSquaresFit:
*	INPUT:
*		f1, f2: source feature sets
*		matches: correspondences between f1 and f2
*		m: motion model
*		f: focal length
*		inliers: inlier match indices (indexes into 'matches' array)
*		M: transformation matrix (output)
*	OUTPUT:
*		compute the transformation from f1 to f2 using only the inliers
*		and return it in M
*/
int leastSquaresFit(const FeatureSet &f1, const FeatureSet &f2,
                    const vector<FeatureMatch> &matches, MotionModel m, float f,
                    const vector<int> &inliers, CTransform3x3& M)
{
    // for project 2, the transformation is a translation and
    // only has two degrees of freedom
    //
    // therefore, we simply compute the average translation vector
    // between the feature in f1 and its match in f2 for all inliers
    double u = 0;
    double v = 0;

    for (int i=0; i<inliers.size(); i++) {
        double xTrans, yTrans;

        // BEGIN TODO
        // compute the translation implied by the ith inlier match
        // and store it in (xTrans,yTrans)
		
		int id1 = matches[inliers[i]].id1;
		int id2 = matches[inliers[i]].id2;
	//	cout << "least-id1: " << id1 << ", id2: " << id2 << endl;
		
		Feature Feature1 = f1[id1 - 1];
		Feature Feature2 = f2[id2 - 1];

		xTrans = Feature2.x - Feature1.x;
		yTrans = Feature2.y - Feature1.y;


        // END TODO

        u += xTrans;
        v += yTrans;
    }

    u /= inliers.size();
    v /= inliers.size();

    M[0][0] = 1;
    M[0][1] = 0;
    M[0][2] = u;
    M[1][0] = 0;
    M[1][1] = 1;
    M[1][2] = v;
    M[2][0] = 0;
    M[2][1] = 0;
    M[2][2] = 1;

    return 0;
}
