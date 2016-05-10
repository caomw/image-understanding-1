
/////////////////////////////////////////////////////////////////////////////////////////////////
//	Project 4: Eigenfaces                                                                      //
//  CSE 455 Winter 2003                                                                        //
//	Copyright (c) 2003 University of Washington Department of Computer Science and Engineering //
//                                                                                             //
//  File: faces.cpp                                                                            //
//	Author: David Laurence Dewey                                                               //
//	Contact: ddewey@cs.washington.edu                                                          //
//           http://www.cs.washington.edu/homes/ddewey/                                        //
//                                                                                             //
/////////////////////////////////////////////////////////////////////////////////////////////////



#include "stdafx.h"

#include "jacob.h"

Faces::Faces()
:
Array<Face>(),
width(0),
height(0),
vector_size(0)
{
	//empty
}

Faces::Faces(int count, int width, int height)
:
Array<Face>(count),
width(width),
height(height),
vector_size(width*height)
{
	for (int i=0; i<getSize(); i++) {
		(*this)[i].resize(width, height, 1);
	}
}

void Faces::load(BinaryFileReader& file)
{
	resize(file.readInt());
	width=file.readInt();
	height=file.readInt();
	vector_size=width*height;
	for (int i=0; i<getSize(); i++) {
		(*this)[i].load(file);
	}
	average_face.load(file);
	std::cout << "Loaded faces from '" << file.getFilename() << "'" << std::endl;

}

void Faces::load(std::string filename)
{
	BinaryFileReader file(filename);
	load(file);
}

void Faces::save(std::string filename) const
{
	BinaryFileWriter file(filename);
	save(file);
}

void Faces::save(BinaryFileWriter& file) const
{
	file.write(getSize());
	file.write(width);
	file.write(height);
	for (int i=0; i<getSize(); i++) {
		(*this)[i].save(file);
	}
	average_face.save(file);
	std::cout << "Saved faces to '" << file.getFilename() << "'" << std::endl;
}

void Faces::output(std::string filepattern) const
{
	for (int i=0; i<getSize(); i++) {
		// normalize for output
		Image out_image;
		(*this)[i].normalize(0.0, 255.0, out_image);
		std::string filename=Functions::filenameNumber(filepattern, i, getSize()-1);
		out_image.saveTarga(filename);
	}
}

void addVecMultToA(Vector a, Vector b, double **A) {
	for(int i = 0; i < a.getSize(); i++) {
		for(int j = 0; j < a.getSize(); j++) { 
			A[i+1][j+1] += a[i] * b[j];
		}
	}
}

void Faces::eigenFaces(EigFaces& results, int n) const
{
	// size the results vector
	results.resize(n);
	results.setHeight(height);
	results.setWidth(width);
	// --------- TODO #1: fill in your code to compute the first n eigenvaces and store them in results
	// also be sure you store the average face in results.average_face (A "set" method is provided for this).
	// You can use the sortEigenvalues function at the bottom of this file to help you find the
	// ordering of eigenvectors.
	//Hint:jacobi, setAverage, (*this)[i] to get a face
	//思路：获取脸的图片，然后进行特征向量分解，然后对特征向量进行排序，选取排在前面的n个特征向量，然后输出

	//1. get average face
	Face average = Face(width, height);
	for (int i = 0; i < getSize(); ++i)
	{
		average += (*this)[i];
	}
	
	average /= getSize();
	results.setAverage(average);

	int size = (*this)[0].getSize();

	// set up covariance matrix
	double** A = new double*[size + 1];
	for(int i = 0; i < size + 1; i++) { // create size+1 arrays of size size+1
		A[i] = new double[size + 1];
		for(int j = 0; j < size + 1; j++) { // set each element of the new array to 0
			A[i][j] = 0;
		}
	}

	// add (a-avg) * (a-avg)^T to the covariance matrix for each face a 
	for (int i = 0; i < getSize(); i++) {
		Vector subbed;
		((Vector)(*this)[i]).sub(average, subbed);
		addVecMultToA(subbed, subbed, A);
	}

	// create eigenvector matrix
	double** v = new double*[size + 1];
	for(int i = 0; i < size + 1; i++) { // create size arrays of size size
		v[i] = new double[size + 1];
		for(int j = 0; j < size + 1; j++) { // set each element of the new array to 0
			v[i][j] = 0;
		}
	}

	//compute the eigenvectors of A
	int *nrot;
	double *d = new double[size];
	Jacobi::jacobi(A, size, d, v, nrot);

	//determine ordering
	Array<int> ordering = Array<int>();
	sortEigenvalues(d, ordering);

	// store the top n results
	std::cout << "result.size(): " << results.getSize() << "n:" << n << std::endl;
	for(int i = 0; i < n; i++) {
		Face f = Face(getWidth(), getHeight());
		for(int j = 0; j < size; j++) {
			f[j] = v[j+1][((ordering)[i])+1];
		}
		results[i] = f;
	}
	
	/*
	//2. construct residual of each face, treated as the a in jacobi function.
	double **resMatrix = new double*[vector_size];
	for (int i = 0; i < vector_size; ++i)
	{
		resMatrix[i] = new double[width*height];

		Face resFace = Face(width, height); 
		(*this)[i].sub(average, resFace);

		//TODO: convert resFace to double array.
		//resMatrix[i] = resFace;
	}

	//3. do PCA operation
	int nFeatSave = n; // the number of eigen vectors to save
	double *eigenValues = new double[nFeatSave];
	double **eigenVectors = new double*[nFeatSave];
	int *nRot = nullptr;

	Jacobi::jacobi(resMatrix, n, eigenValues, eigenVectors, nRot);

	//4. sort eigen vectors
	Array<int> ordering = Array<int>(nFeatSave);
	sortEigenvalues(eigenValues, ordering);

	//5.generate eigen face
	for (int i = 0; i < nFeatSave; ++i)
	{
	//TODO: how to convert double array to Face??
	//	results[i] = eigenVectors[ordering[i]];
		

	}
	*/



}



int Faces::getWidth() const
{
	return width;
}

int Faces::getHeight() const
{
	return height;
}

void Faces::setWidth(int width)
{
	width=width;
	vector_size=width*height;
}

void Faces::setHeight(int height)
{
	height=height;
	vector_size=width*height;
}

void Faces::sortEigenvalues(double *eigenvec, Array<int>& ordering) const
{
	// for now use simple bubble sort
	ordering.resize(vector_size);
	std::list<EigenVectorIndex> list;
	for (int i=0; i<vector_size; i++) {
		EigenVectorIndex e;
		e.eigenvalue=eigenvec[i+1];
		e.index=i;
		list.push_back(e);
	}
	bool change=true;
	list.sort();
	std::list<EigenVectorIndex>::iterator it=list.begin();
	int n=0;
	while (it!=list.end()) {
		ordering[n] = (*it).index;
		it++;
		n++;
	}
}

