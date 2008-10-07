#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageIORegion.h"
#include <iostream>

using namespace std;

typedef itk::Image< int, 2 > InputImageType;
typedef itk::ImageFileReader< InputImageType > ReaderType;

class Segmentation {
private:
	string *file;
public:
	Segmentation(string file) {
		this->file = new string(file);
	}
	void doIt();
};
