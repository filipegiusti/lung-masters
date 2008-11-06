#include <iostream>
#include <InsightToolkit/Common/itkImage.h>
#include <InsightToolkit/Common/itkMetaDataDictionary.h>
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkNeighborhoodConnectedImageFilter.h"
#include "itkMaskNegatedImageFilter.h"
#include "itkMaskImageFilter.h"
#include "itkVotingBinaryIterativeHoleFillingImageFilter.h"
#include "itkBinaryErodeImageFilter.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkBinaryBallStructuringElement.h"
#include "itkRegionOfInterestImageFilter.h"
#include "itkGDCMImageIO.h"
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
typedef itk::MaskImageFilter<ImageType, ImageType> MaskFilterType;
typedef itk::VotingBinaryIterativeHoleFillingImageFilter< ImageType > HoleFillingType;
typedef itk::BinaryBallStructuringElement < IMAGE_DATA, IMAGE_DIMENSIONS > StructuringElementType;
typedef itk::BinaryErodeImageFilter < ImageType, ImageType, StructuringElementType> ErodeFilterType;
typedef itk::BinaryDilateImageFilter < ImageType, ImageType, StructuringElementType> DilateFilterType;
typedef itk::RegionOfInterestImageFilter< ImageType, ImageType > ROIFilterType;

typedef itk::GDCMImageIO ImageIOType;
using namespace std;

int isThereMoreThan2Islands(ImageType *image) {
	unsigned int x = image->GetLargestPossibleRegion().GetSize()[0];
	unsigned int y = image->GetLargestPossibleRegion().GetSize()[1];
	int hasWhite = 0;
	
	ConnectedFilterType::Pointer neighborhoodConnected = ConnectedFilterType::New();
	MaskNegatedFilterType::Pointer maskNegatedFilter = MaskNegatedFilterType::New();
	
	ImageType::IndexType index;
	for (int i = 0; i < x; i++) {
		for (int j = 0; j < y; j++) {
			index[0] = i;
			index[1] = j;
			if(image->GetPixel(index)) {
				if(hasWhite)
					return 1;
				hasWhite = 1;
				
				neighborhoodConnected->SetInput( image );
				neighborhoodConnected->SetLower(1);
				neighborhoodConnected->SetUpper(1);
				neighborhoodConnected->SetReplaceValue(1);//TODO: magic number
				//Seeds
				index[0] = i;
				index[1] = j;
				neighborhoodConnected->SetSeed(index);
				ImageType::SizeType radius;
				radius[0] = 0;
				radius[1] = 0;
				neighborhoodConnected->SetRadius(radius);

				maskNegatedFilter->SetInput1(image);
				maskNegatedFilter->SetInput2(neighborhoodConnected->GetOutput());
				maskNegatedFilter->Update();

				image = maskNegatedFilter->GetOutput();
			}
		}
	}
	return 0;
}

int smallerHeightPosition(ImageType *image) {
	unsigned int x = image->GetLargestPossibleRegion().GetSize()[0];
	unsigned int y = image->GetLargestPossibleRegion().GetSize()[1];
	int minHeight, minHeightPosition, tmp;
	int halfWeight = x/2;
	int foundFirst = 0;
	
	minHeight = 0;
	minHeightPosition = halfWeight;
	ImageType::IndexType index;
	index[0] = halfWeight;
	for (int j = 0; j < y; j++) {
		index[1] = j;
		if(image->GetPixel(index)) {
			foundFirst = 1;
			minHeight++;
		} else if (foundFirst) {
			break;
		}
	}

	int stopLeft = 0, stopRight = 0;
	for (int i = 1; i < halfWeight; i++) {
		if (!stopRight) {
			tmp = 0;
			foundFirst = 0;
			index[0] = halfWeight + i;
			for (int j = 0; j < y; j++) {
				index[1] = j;
				if(image->GetPixel(index)) {
					foundFirst = 1;
					tmp++;
				} else if (foundFirst) {
					break;
				}
			}
			if(!tmp) {
				stopRight = 1;
			}
			else if(tmp <= minHeight) {
				minHeight = tmp;
				minHeightPosition = halfWeight + i;
			} else {
				stopRight = 1;
			}
		}
		if(!stopLeft) {
			tmp = 0;
			foundFirst = 0;
			index[0] = halfWeight - i;
			for (int j = 0; j < y; j++) {
				index[1] = j;
				if(image->GetPixel(index)) {
					foundFirst = 1;
					tmp++;
				} else if (foundFirst) {
					break;
				}
			}
			if(!tmp) {
				stopLeft = 1;
			}
			else if(tmp <= minHeight) {
				minHeight = tmp;
				minHeightPosition = halfWeight - i;
			}
			else {
				stopLeft = 1;
			}
		}
	}
	
	return minHeightPosition;
}

int limit(int direction, ImageType *image){
	ImageType::IndexType index;
	int limit = -1;
	int backwards, max1, max2, inc;
	char zero, um;
	int x,y;
	x = image->GetLargestPossibleRegion().GetSize()[0];
	y = image->GetLargestPossibleRegion().GetSize()[1];
	if (direction == 1) {
		// Left-Right
		zero = 0;
		um = 1;
		backwards = 0;
		max1 = x;
		max2 = y;
	} else if (direction == 3) {
		// Right-Left
		zero = 0;
		um = 1;
		backwards = 1;
		max1 = x;
		max2 = y;
		
	} else if (direction == 2) {
		// Top-Down
		zero = 1;
		um = 0;
		backwards = 0;
		max1 = y;
		max2 = x;
	} else {
		// Bottom-Up
		zero = 1;
		um = 0;
		backwards = 1;
		max1 = x;
		max2 = y;
	}
	
	if(!backwards) {
		for (int k = 0; k < max1; k++) {
			index[zero] = k;
			for (int l = 0; l < max2; l++) {
				index[um] = l;
				if(image->GetPixel(index)) {
					limit = k;
					break;
				}
			}
			if(limit != -1)
				break;
		}
	} else {
		for (int k = max1-1; k >= 0; k--) {
			index[zero] = k;
			for (int l = 0; l < max2; l++) {
				index[um] = l;
				if(image->GetPixel(index)) {
					limit = k;
					break;
				}
			}
			if(limit != -1)
				break;
		}
	}
}

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
	
 	HoleFillingType::Pointer holeFillingFilter = HoleFillingType::New();
	radius[0] = 2;
 	radius[1] = 2;
 	holeFillingFilter->SetRadius(radius);
	holeFillingFilter->SetInput(maskNegatedFilter->GetOutput());
	holeFillingFilter->SetBackgroundValue( 1 );
	holeFillingFilter->SetForegroundValue( 0 );
	holeFillingFilter->SetMajorityThreshold( 2 );
	holeFillingFilter->SetMaximumNumberOfIterations( 40 );
	
 	ErodeFilterType::Pointer binaryErodeFilter = ErodeFilterType::New();
 	DilateFilterType::Pointer binaryDilateFilter = DilateFilterType::New();   
 	
 	StructuringElementType structuringElement;
 	structuringElement.SetRadius( 2 ); 
 	structuringElement.CreateStructuringElement();
 	binaryDilateFilter->SetKernel( structuringElement );
 	binaryErodeFilter->SetKernel( structuringElement );
 	
 	binaryDilateFilter->SetInput( holeFillingFilter->GetOutput() );
 	binaryErodeFilter->SetInput( binaryDilateFilter->GetOutput() );
 	binaryDilateFilter->SetDilateValue( 1 );
 	binaryErodeFilter->SetErodeValue( 1 );
	
	HoleFillingType::Pointer inverseHoleFillingFilter = HoleFillingType::New();
	radius[0] = 2;
 	radius[1] = 2;
 	inverseHoleFillingFilter->SetRadius(radius);
	inverseHoleFillingFilter->SetInput(binaryErodeFilter->GetOutput());
	inverseHoleFillingFilter->SetBackgroundValue( 0 );
	inverseHoleFillingFilter->SetForegroundValue( 1 );
	inverseHoleFillingFilter->SetMajorityThreshold( 2 );
	inverseHoleFillingFilter->SetMaximumNumberOfIterations( 10 );
	inverseHoleFillingFilter->Update();
	
	ImageType *image;
	if (!isThereMoreThan2Islands(inverseHoleFillingFilter->GetOutput())) {
		
		image = inverseHoleFillingFilter->GetOutput();
		unsigned int y = image->GetLargestPossibleRegion().GetSize()[1];
		index[0] = smallerHeightPosition(image);
		for (index[1] = 0; index[1] < y; index[1]++) {
			if(image->GetPixel(index)) {
				break;
			} 
		}
		while(image->GetPixel(index)) {
			image->SetPixel(index, 0);
			index[1]++;
		}
	}
	
	//Split lungs by centre of mass!
	unsigned int x = image->GetLargestPossibleRegion().GetSize()[0];
	unsigned int y = image->GetLargestPossibleRegion().GetSize()[1];
	
	ImageType::Pointer leftLung = ImageType::New();
	ImageType::Pointer rightLung = ImageType::New();
	
	ImageType::IndexType start;
	start[0] =   0;
	start[1] =   0;
	
	ImageType::SizeType  size;
	size[0]  = x;
	size[1]  = y;

	ImageType::RegionType region;
	region.SetSize( size );
	region.SetIndex( start );

	leftLung->SetRegions( region );
	leftLung->Allocate();
	rightLung->SetRegions( region );
	rightLung->Allocate();
	
	ConnectedFilterType::Pointer neighborhoodConnectedSpliter = ConnectedFilterType::New();
	MaskNegatedFilterType::Pointer maskNegatedFilterSpliter = MaskNegatedFilterType::New();
	
	for (int i = 0; i < x; i++) {
		for (int j = 0; j < y; j++) {
			index[0] = i;
			index[1] = j;
			if(image->GetPixel(index)) {
				neighborhoodConnectedSpliter->SetInput( image );
				neighborhoodConnectedSpliter->SetLower(1);
				neighborhoodConnectedSpliter->SetUpper(1);
				neighborhoodConnectedSpliter->SetReplaceValue(1);//TODO: magic number
				//Seeds
				index[0] = i;
				index[1] = j;
				neighborhoodConnectedSpliter->SetSeed(index);
				ImageType::SizeType radius;
				radius[0] = 0;
				radius[1] = 0;
				neighborhoodConnectedSpliter->SetRadius(radius);
				neighborhoodConnectedSpliter->Update();

				int leftLimit, rightLimit;
				leftLimit = limit( 1 ,neighborhoodConnectedSpliter->GetOutput());
				rightLimit = limit( 3 ,neighborhoodConnectedSpliter->GetOutput());
				
				if(leftLimit + rightLimit > x) {
					// Right side
					ImageType::IndexType auxIndex;
					int xAxis = neighborhoodConnectedSpliter->GetOutput()->GetLargestPossibleRegion().GetSize()[0];
					int yAxis = neighborhoodConnectedSpliter->GetOutput()->GetLargestPossibleRegion().GetSize()[1];
					int lalala = 0;
					for(int l = 0; l < xAxis; l++) {
						auxIndex[0] = l;
						for(int c = 0; c < yAxis; c++) {
							auxIndex[1] = c;
							if(neighborhoodConnectedSpliter->GetOutput()->GetPixel(auxIndex)) {
								rightLung->SetPixel(auxIndex, 1); // TODO: magic number
							}
						}
					}
				} else {
					// Left side
					ImageType::IndexType auxIndex;
					int xAxis = neighborhoodConnectedSpliter->GetOutput()->GetLargestPossibleRegion().GetSize()[0];
					int yAxis = neighborhoodConnectedSpliter->GetOutput()->GetLargestPossibleRegion().GetSize()[1];
					
					for(int l = 0; l < xAxis; l++) {
						auxIndex[0] = l;
						for(int c = 0; c < yAxis; c++) {
							auxIndex[1] = c;
							if(neighborhoodConnectedSpliter->GetOutput()->GetPixel(auxIndex)) {
								leftLung->SetPixel(auxIndex, 1); // TODO: magic number
							}
						}
					}
				}
				
				maskNegatedFilterSpliter->SetInput1(image);
				maskNegatedFilterSpliter->SetInput2(neighborhoodConnectedSpliter->GetOutput());
				maskNegatedFilterSpliter->Update();

				image = maskNegatedFilterSpliter->GetOutput();
			}
		}
	}
	
	MaskFilterType::Pointer maskFilterLeft = MaskFilterType::New();
	maskFilterLeft->SetInput1(reader->GetOutput());
	maskFilterLeft->SetInput2(leftLung);
	maskFilterLeft->Update();
	
	MaskFilterType::Pointer maskFilterRight = MaskFilterType::New();
	maskFilterRight->SetInput1(reader->GetOutput());
	maskFilterRight->SetInput2(rightLung);
	maskFilterRight->Update();
	
	ROIFilterType::Pointer roiFilterLeft = ROIFilterType::New();
	ROIFilterType::Pointer roiFilterRight = ROIFilterType::New();
	ImageType::RegionType roi;
	int left,top;
	
	left = limit(1, leftLung);
	top = limit(2, leftLung);
	
	index[0] = left;
	index[1] = top;
	size[0] = limit(3, leftLung) - left;
	size[1] = limit(4, leftLung) - top;
	
	roi.SetSize( size );
	roi.SetIndex( index );
	roiFilterLeft->SetRegionOfInterest( roi );
	roiFilterLeft->SetInput(maskFilterLeft->GetOutput());
	roiFilterLeft->Update();
	
	left = limit(1, rightLung);
	top = limit(2, rightLung);
	
	index[0] = left;
	index[1] = top;
	size[0] = limit(3, rightLung) - left;
	size[1] = limit(4, rightLung) - top;
	
	roi.SetSize( size );
	roi.SetIndex( index );
	roiFilterRight->SetRegionOfInterest( roi );
	roiFilterRight->SetInput(maskFilterRight->GetOutput());
	roiFilterRight->Update();

	ImageType::Pointer leftLungCutted = ImageType::New();
	ImageType::Pointer rightLungCutted = ImageType::New();
	
	ImageType::RegionType regionLeft;
	regionLeft.SetSize( roiFilterLeft->GetOutput()->GetLargestPossibleRegion().GetSize() );
	regionLeft.SetIndex( start );
	
	ImageType::RegionType regionRight;
	regionRight.SetSize( roiFilterRight->GetOutput()->GetLargestPossibleRegion().GetSize() );
	regionRight.SetIndex( start );

	leftLungCutted->SetRegions( regionLeft );
	leftLungCutted->SetMetaDataDictionary(reader->GetOutput()->GetMetaDataDictionary());
	leftLungCutted->Allocate();
	rightLungCutted->SetRegions( regionRight );
	rightLungCutted->SetMetaDataDictionary(reader->GetOutput()->GetMetaDataDictionary());
	rightLungCutted->Allocate();
	
	ImageType::IndexType auxIndex;
	int xAxis = leftLungCutted->GetLargestPossibleRegion().GetSize()[0];
	int yAxis = leftLungCutted->GetLargestPossibleRegion().GetSize()[1];
	for(int l = 0; l < xAxis; l++) {
		auxIndex[0] = l;
		for(int c = 0; c < yAxis; c++) {
			auxIndex[1] = c;
			leftLungCutted->SetPixel(auxIndex, roiFilterLeft->GetOutput()->GetPixel(auxIndex));
		}
	}
	
	xAxis = rightLungCutted->GetLargestPossibleRegion().GetSize()[0];
	yAxis = rightLungCutted->GetLargestPossibleRegion().GetSize()[1];
	for(int l = 0; l < xAxis; l++) {
		auxIndex[0] = l;
		for(int c = 0; c < yAxis; c++) {
			auxIndex[1] = c;
			rightLungCutted->SetPixel(auxIndex, roiFilterRight->GetOutput()->GetPixel(auxIndex));
		}
	}
	
	WriterType::Pointer writer = WriterType::New();
	writer->SetFileName( "leftLung.dcm" );
	writer->SetInput(leftLungCutted);
	writer->Update();
	writer->SetFileName( "rightLung.dcm" );
	writer->SetInput(rightLungCutted);
	writer->Update();
	
	return 0;
}

