
/////////////////////////////////////////////////////////////////////////////////////////////////
//	Project 4: Eigenfaces                                                                      //
//  CSE 455 Winter 2003                                                                        //
//	Copyright (c) 2003 University of Washington Department of Computer Science and Engineering //
//                                                                                             //
//  File: eigfaces.cpp                                                                         //
//	Author: David Laurence Dewey                                                               //
//	Contact: ddewey@cs.washington.edu                                                          //
//           http://www.cs.washington.edu/homes/ddewey/                                        //
//                                                                                             //
/////////////////////////////////////////////////////////////////////////////////////////////////



#include "stdafx.h"
#include <algorithm>
static bool overlaps(FacePosition, FacePosition, double, double, int, int);
static void drawBorders(Image &, int, int, int, int, double);

EigFaces::EigFaces()
:
Faces()
{
	//empty
}

EigFaces::EigFaces(int count, int width, int height)
:
Faces(count, width, height)
{
	//empty
}

void EigFaces::projectFace(const Face& face, Vector& coefficients) const
{
	if (face.getWidth()!=width || face.getHeight()!=height) {
		throw Error("Project: Face to project has different dimensions");
	}

	coefficients.resize(getSize());
	// ----------- TODO #2: compute the coefficients for the face and store in coefficients.


	Face diff  = Face(width, height);
	face.sub(getAverage(), diff);	// save residual of face and average face to diff
		
	for (int i = 0; i < getSize(); ++i)
	{
		coefficients[i] = diff.dot((*this)[i]);
	}
}

void EigFaces::constructFace(const Vector& coefficients, Face& result) const
{	
	// ----------- TODO #3: construct a face given the coefficients

	for (int i = 0; i < getSize(); ++i)
	{
		for (int j = 0; j < result.getSize(); ++j)
		{
			if (i == 0)
			{
				result[j] = average_face[j];
			}
			result[j] = (*this)[i][j] * coefficients[i];
		}
	}
}

bool EigFaces::isFace(const Face& face, double max_reconstructed_mse, double& mse) const
{
	// ----------- TODO #4: Determine if an image is a face and return true if it is. Return the actual
	// MSE you calculated for the determination in mse
	// Be sure to test this method out with some face images and some non face images
	// to verify it is working correctly.

	Vector coeff;
	Face constructedFace = Face(width, height);
	projectFace(face, coeff);
	constructFace(coeff, constructedFace);
	mse = face.mse(constructedFace);

	return mse <= max_reconstructed_mse;  
}

bool EigFaces::verifyFace(const Face& face, const Vector& user_coefficients, double max_coefficients_mse, double& mse) const
{
	// ----------- TODO #5 (Extra Credit): Determine if face is the same user give the user's coefficients.
	// return the MSE you calculated for the determination in mse.

	Face tmpFace = Face(width, height);
	constructFace(user_coefficients, tmpFace);
	mse = face.mse(tmpFace);
	
	return mse <= max_coefficients_mse;  
}

void EigFaces::recognizeFace(const Face& face, Users& users) const
{
	// ----------- TODO #6: Sort the users by closeness of match to the face

	for (int i = 0; i < users.getSize(); ++i)
	{
		double mse;
		verifyFace(face, users[i], 9999, mse);
		users[i].setMse(mse);
	}

	users.sort();
}
bool overlap(FacePosition a, FacePosition b) {
	int realXa = a.x / a.scale;
	int realYa = a.y / a.scale;
	int realSizea = 25 / a.scale;

	int realXb = b.x / b.scale;
	int realYb = b.y / b.scale;
	int realSizeb = 25 / b.scale;

	bool aboveORbelow = (realYa + realSizea < realYb) || (realYa > realYb + realSizeb);  // bottom of A above top of B || top of A below bottom of B
	bool leftORright = (realXa + realSizea < realXb) || (realXa > realXb + realSizeb); // right-side of A to the left of B || left-side of A to the right of B

	return !(aboveORbelow || leftORright);
}

void EigFaces::findFace(const Image& img, double min_scale, double max_scale, double step, int n, bool crop, Image& result) const
{
	// ----------- TODO #7: Find the faces in Image. Search image scales from min_scale to max_scale inclusive,
	// stepping by step in between. Find the best n faces that do not overlap each other. If crop is true,
	// n is one and you should return the cropped original img in result. The result must be identical
	// to the original besides being cropped. It cannot be scaled and it must be full color. If crop is
	// false, draw green boxes (use r=100, g=255, b=100) around the n faces found. The result must be
	// identical to the original image except for the addition of the boxes.

	std::list<FacePosition> top;

	// Loop over scales
	for (double scale = min_scale; scale <= max_scale; scale += step)
	{
		Image scaledImg(scale * img.getWidth(), scale * img.getHeight(), img.getColors()); // Create a new image scaled appropriately
		img.resample(scaledImg); // Resample to it

		// Loop over positions in the image
		for (int y = 0; y + this->height < scaledImg.getHeight(); y++)
		{
			for (int x = 0; x + this->width < scaledImg.getWidth(); x++)
			{
				Face temp(this->width, this->height);	// face corresponding to current position
				FacePosition current;					// FacePosition struct
				double mse;

				temp.subimage(x, x + this->width - 1, y, y + this->height - 1, scaledImg, false); // get face
				isFace(temp, 9999, mse);											// get mse

				// Construct current FacePosition
				current.x = x;
				current.y = y;
				current.scale = scale;
				current.error = (mse * temp.mse(this->average_face)) / temp.var();

				bool skip = false; // set to true if current overlaps a better face. Will cause current to be ignored.
				std::list<FacePosition> removeIfSafe(0);
				FacePosition worst;
				worst.error = -1; // Sentinel value indicating worst is not set

				std::list<FacePosition>::iterator it = top.begin();

				while (it != top.end())
				{
					FacePosition pos = *it;

					if (pos.error > current.error)
					{
						// The new face is a better match than the current face being examined
						if (overlaps(current, pos, current.scale, pos.scale, this->width, this->height))
						{
							// If it's an overlap, mark for removal
							removeIfSafe.push_back(pos);
						}
						else
						{
							// If not an overlap, mark as the worst face in the list
							if (worst.error < pos.error)
								worst = pos;
						}
					}
					else
					{
						// The new face is a worse match than the current face being examined
						if (overlaps(current, pos, current.scale, pos.scale, this->width, this->height))
						{
							// If it's an overlap, continue with another new face as the new face is worse. Set
							// skip to indicate this face should not be inserted no matter what
							skip = true;
							break;
						}
						// Else we do nothing because the new face is better but there may be other worse faces
					}
					it++;
				}

				if (skip) continue;

				// If the new face doesn't overlap anything better, remove the faces that overlap it.
				for (std::list<FacePosition>::iterator it = removeIfSafe.begin(); it != removeIfSafe.end(); it++)
				{
					FacePosition pos = (*it);
					top.remove(pos);
				}

				// If there is a worse face and the list is full, remove it
				if (top.size() == n && worst.error != -1)
				{
					top.remove(worst);
				}

				// If the list is not full (either because we removed a face or because we just don't have enough) add the new face
				if (top.size() < n)
				{
					top.push_back(current);
				}
			} // x
		} // y
	} // scales

	if (crop)
	{
		FacePosition target = *(top.begin());
		result.resize((this->width / target.scale), (this->height / target.scale), img.getColors());
		for (int j = 0; j < (this->height / target.scale); j++)
		{
			for (int i = 0; i < (this->width / target.scale); i++)
			{
				for (int c = 0; c < img.getColors(); c++)
				{
					result.pixel(i, j, c) = img.pixel((target.x / target.scale) + i, (target.y / target.scale) + j, c);
				}
			}
		}
	}
	else
	{
		// Copy img into result
		result.resize(img.getWidth(), img.getHeight(), img.getColors());
		for (int i = 0; i < img.getSize(); i++)
		{
			result[i] = img[i];
		}

		// Draw boxes
		for (std::list<FacePosition>::iterator it = top.begin(); it != top.end(); it++)
		{
			// Get the face position
			FacePosition pos = *it;
			drawBorders(result, pos.x, pos.y, this->width, this->height, pos.scale);
		}
	}
	
		/*
	std::list<FacePosition> *bestFaces = new std::list<FacePosition>();

	for (double d = min_scale; d <= max_scale; d += step) {
		Image *scaled = new Image(img.getWidth() * d, img.getHeight() * d, img.getColors());
		img.resample(*scaled);

		Image *debug = new Image(img.getWidth() * d, img.getHeight() * d, 1);

		//std::cout << "before loop" <<std::endl;
		for(int x = 0; x < (img.getWidth() * d) - 25; x++) {
			for(int y = 0; y < (img.getHeight() * d) - 25; y++) {
				//for each pixel that can be the upper-left of a 25x25 pixel FacePosition
				
				Face cur = Face(25, 25);
				scaled->crop(x, y, x+24, y+24, cur);

				FacePosition *fp = new FacePosition();
				fp->x = x;
				fp->y = y;
				fp->scale = d;
				isFace(cur, -175.226, fp->error);

				Face subbed = cur;
				Face neg = this->average_face;
				neg *= -1;
				subbed += neg;

				fp->error = fp->error * subbed.mag() / cur.var();

				debug->pixel(x, y, 0) = fp->error;

				bestFaces->push_back(*fp);
			}
		}

		debug->normalize(0, 255, *debug);
		std::string filename = "debug.tga";
		debug->saveTarga(filename);
	}
	bestFaces->sort();
	std::list<FacePosition>::iterator it=bestFaces->begin();
	std::list<FacePosition> *nBest = new std::list<FacePosition>();
	while(nBest->size() < n && it != bestFaces->end()) {
		std::list<FacePosition>::iterator check=nBest->begin();
		bool overlapping = false;
		while(check != nBest->end()) {
			if(overlap(*it, *check)) {
				overlapping = true;
				break;
			}
			check++;
		}
		if(!overlapping) {
			nBest->push_back(*it);
		}
		it++;
	}

	if(crop) {
		result.resize(img.getWidth(), img.getHeight(), img.getColors());
		img.resample(result);
		FacePosition best = bestFaces->front();
		int xmin = best.x / best.scale;
		int ymin = best.y / best.scale;
		int sizz = 25 / best.scale;
		img.crop(xmin, ymin, xmin + sizz, ymin + sizz, result);
	} else {
		result.resize(img.getWidth(), img.getHeight(), img.getColors());
		img.resample(result);
		for(std::list<FacePosition>::iterator it=nBest->begin(); it!=nBest->end(); it++) {
			//draw 4 lines
			int xmin = (*it).x / (*it).scale;
			int ymin = (*it).y / (*it).scale;
			int sizee = 25 / (*it).scale;
			result.line(xmin, ymin, xmin, ymin + sizee, 100, 255, 100); // left
			result.line(xmin, ymin, xmin + sizee, ymin, 100, 255, 100); // top
			result.line(xmin + sizee, ymin, xmin + sizee, ymin + sizee, 100, 255, 100); // right 
			result.line(xmin, ymin + sizee, xmin + sizee, ymin + sizee, 100, 255, 100); // bottom
		}
	}
	*/
}

static void drawBorders(Image &img, int x, int y, int width, int height, double scale)
{
	int left = (double) x / scale;
	int right = (double) (x + width - 1) / scale;
	int top = (double) y / scale;
	int bottom = (double) (y + height - 1) / scale;

	// bottom and top
	for (int i = left; i <= right; i++)
	{
		// top
		img.pixel(i, top, 0) = 100;
		img.pixel(i, top, 1) = 255;
		img.pixel(i, top, 2) = 100;

		// bottom
		img.pixel(i, bottom, 0) = 100;
		img.pixel(i, bottom, 1) = 255;
		img.pixel(i, bottom, 2) = 100;
	}

	// left and right
	for (int j = top; j <= bottom; j++)
	{
		// left
		img.pixel(left, j, 0) = 100;
		img.pixel(left, j, 1) = 255;
		img.pixel(left, j, 2) = 100;

		// right
		img.pixel(right, j, 0) = 100;
		img.pixel(right, j, 1) = 255;
		img.pixel(right, j, 2) = 100;
	}
}

static bool overlaps(FacePosition a, FacePosition b, double scale_a, double scale_b, int width, int height)
{
	// Position A
	int al = (double) a.x / scale_a;					// Left
	int ar = (double) (a.x + width - 1) / scale_a;		// Right
	int at = (double) a.y / scale_a;					// Top
	int ab = (double) (a.y + height - 1) / scale_a;		// Bottom

	// Position B
	int bl = (double) b.x / scale_b;
	int br = (double) (b.x + width - 1) / scale_b;
	int bt = (double) b.y / scale_b;
	int bb = (double) (b.y + height - 1) / scale_b;

	return (al < br && ar > bl && ab > bt && at < bb);
}
void EigFaces::morphFaces(const Face& face1, const Face& face2, double distance, Face& result) const
{
	// TODO (extra credit): MORPH along *distance* fraction of the vector from face1 to face2 by
	// interpolating between the coefficients for the two faces and reconstructing the result.
	// For example, distance 0.0 will approximate the first, while distance 1.0 will approximate the second.
	// Negative distances are ok two.


}

const Face& EigFaces::getAverage() const
{
	return average_face;
}

void EigFaces::setAverage(const Face& average)
{
	average_face=average;
}



