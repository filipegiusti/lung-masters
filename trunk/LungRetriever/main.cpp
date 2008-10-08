#include <iostream>
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "Segmentation.hpp"

#define IMAGE_DIMENSIONS 2
#define IMAGE_DATA unsigned short
#define INITIAL_THRESHOLD_VALUE 128

typedef itk::Image< IMAGE_DATA, IMAGE_DIMENSIONS > ImageType;
typedef itk::ImageFileReader< ImageType > ReaderType;
typedef itk::ImageFileWriter< ImageType > WriterType;
typedef itk::BinaryThresholdImageFilter < ImageType, ImageType >  ThresholdFilterType;

using namespace std;

int main () {
	ReaderType::Pointer reader = ReaderType::New();
	reader->SetFileName("IM000011");
	
	int thresholdValue = INITIAL_THRESHOLD_VALUE;
	int thresholdValueOLD = INITIAL_THRESHOLD_VALUE + 1;
	ThresholdFilterType::Pointer thresholdFilter = ThresholdFilterType::New();
	thresholdFilter->SetOutsideValue(0);
	thresholdFilter->SetInsideValue(512); //TODO: magic number
	thresholdFilter->SetLowerThreshold(0);
	thresholdFilter->SetInput(reader->GetOutput());
	
	while(thresholdValue != thresholdValueOLD) {
		int averageAbove, averageBelow, quantityAbove, quantityBelow;
		ImageType::IndexType index;
		
		quantityAbove = quantityBelow = 0;
		
		thresholdFilter->SetUpperThreshold(thresholdValue);
		thresholdFilter->Update();

		int x = thresholdFilter->GetOutput()->GetLargestPossibleRegion().GetSize()[0];
		int y = thresholdFilter->GetOutput()->GetLargestPossibleRegion().GetSize()[1];
		for (int i = 0; i < x; i++) {
			for (int j = 0; j < y; j++) {
				index[0] = i;
				index[1] = j;
				if(thresholdFilter->GetOutput()->GetPixel(index)) {
					averageAbove += reader->GetOutput()->GetPixel(index);
					quantityAbove++;
				}
				else {
					averageBelow += reader->GetOutput()->GetPixel(index);
					quantityBelow++;
				}
			}
		}
		averageAbove /= quantityAbove;
		averageBelow /= quantityBelow;
		thresholdValueOLD = thresholdValue;
		thresholdValue = (averageAbove + averageBelow)/2;
		cout << "thresholdValue: " << thresholdValue << "\n";
		cout << "thresholdValueOLD: " << thresholdValueOLD << "\n\n";
	}
	
	WriterType::Pointer writer = WriterType::New();
	writer->SetFileName( "IM000011.dcm" );
	writer->SetInput(thresholdFilter->GetOutput());
	
	writer->Update();
	
	return 0;
}

