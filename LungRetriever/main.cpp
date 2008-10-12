#include <iostream>
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkNeighborhoodConnectedImageFilter.h"
#include "itkMaskNegatedImageFilter.h"
#include "Segmentation.hpp"

#define IMAGE_DIMENSIONS 2
#define IMAGE_DATA unsigned short
#define INITIAL_THRESHOLD_VALUE 128

typedef itk::Image< IMAGE_DATA, IMAGE_DIMENSIONS > ImageType;
typedef itk::ImageFileReader< ImageType > ReaderType;
typedef itk::ImageFileWriter< ImageType > WriterType;
typedef itk::BinaryThresholdImageFilter < ImageType, ImageType >  ThresholdFilterType;
typedef itk::NeighborhoodConnectedImageFilter< ImageType, ImageType > ConnectedFilterType;
typedef itk::MaskNegatedImageFilter<ImageType, ImageType, ImageType> MaskNegatedFilterType;

using namespace std;

int main () {
	ReaderType::Pointer reader = ReaderType::New();
	reader->SetFileName("IM000011");
	
	IMAGE_DATA thresholdValue = INITIAL_THRESHOLD_VALUE;
	IMAGE_DATA thresholdValueOLD = INITIAL_THRESHOLD_VALUE + 1;
	ImageType::IndexType index;
	ThresholdFilterType::Pointer thresholdFilter = ThresholdFilterType::New();
	thresholdFilter->SetOutsideValue(1); //TODO: magic number
	thresholdFilter->SetInsideValue(0); 
	thresholdFilter->SetLowerThreshold(0);
	thresholdFilter->SetInput(reader->GetOutput());
	
	while(thresholdValue != thresholdValueOLD) {
		unsigned long long averageAbove, averageBelow;
		unsigned int quantityAbove, quantityBelow;
		
		averageAbove = averageBelow = quantityAbove = quantityBelow = 0;
		
		thresholdFilter->SetUpperThreshold(thresholdValue);
		thresholdFilter->Update();

		unsigned int x = thresholdFilter->GetOutput()->GetLargestPossibleRegion().GetSize()[0];
		unsigned int y = thresholdFilter->GetOutput()->GetLargestPossibleRegion().GetSize()[1];
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
	}
	
	ConnectedFilterType::Pointer neighborhoodConnected = ConnectedFilterType::New();
 	neighborhoodConnected->SetInput( thresholdFilter->GetOutput() );
	neighborhoodConnected->SetLower(1);
	neighborhoodConnected->SetUpper(1);
	neighborhoodConnected->SetReplaceValue(1);//TODO: magic number
	//Seeds
	index[0] = 0;
	index[1] = 0;
	neighborhoodConnected->SetSeed(index);
	{
		unsigned int x = thresholdFilter->GetOutput()->GetLargestPossibleRegion().GetSize()[0];
		unsigned int y = thresholdFilter->GetOutput()->GetLargestPossibleRegion().GetSize()[1];
		int i;
		for (i = 1; i < x; i++) {
			index[0] = i;
			index[1] = 0;
			neighborhoodConnected->AddSeed(index);
		}
		for (i = 0; i < x; i++) {
			index[0] = i;
			index[1] = y-1;
			neighborhoodConnected->AddSeed(index);
		}
		for (i = 1; i < y-1; i++) {
			index[0] = 0;
			index[1] = i;
			neighborhoodConnected->AddSeed(index);
		}
		for (i = 1; i < y-1; i++) {
			index[0] = x-1;
			index[1] = i;
			neighborhoodConnected->AddSeed(index);
		}
	}
	ImageType::SizeType radius;
 	radius[0] = 0;
 	radius[1] = 0;
 	neighborhoodConnected->SetRadius(radius);
	
	MaskNegatedFilterType::Pointer maskNegatedFilter = MaskNegatedFilterType::New();
	maskNegatedFilter->SetInput1(thresholdFilter->GetOutput());
	maskNegatedFilter->SetInput2(neighborhoodConnected->GetOutput());
	
	WriterType::Pointer writer = WriterType::New();
	writer->SetFileName( "IM000011.dcm" );
	writer->SetInput(maskNegatedFilter->GetOutput());
	
	writer->Update();
	
	return 0;
}

