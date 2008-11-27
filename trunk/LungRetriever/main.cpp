#include <InsightToolkit/Common/itkExceptionObject.h>
#include <iostream>
#include <InsightToolkit/Common/itkImageRegionConstIterator.h>
#include <InsightToolkit/Common/itkImage.h>
#include <InsightToolkit/Common/itkMetaDataDictionary.h>
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include <InsightToolkit/BasicFilters/itkBinaryThresholdImageFilter.h>
#include <InsightToolkit/BasicFilters/itkRescaleIntensityImageFilter.h>
#include "itkNeighborhoodConnectedImageFilter.h"
#include "itkMaskNegatedImageFilter.h"
#include "itkMaskImageFilter.h"
#include "itkVotingBinaryIterativeHoleFillingImageFilter.h"
#include "itkBinaryErodeImageFilter.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkBinaryBallStructuringElement.h"
#include "itkRegionOfInterestImageFilter.h"
#include "Segmentation.hpp"

#include "itkScalarImageTextureCalculator.h"
#include "itkVectorContainer.h"
#include "itkGreyLevelCooccurrenceMatrixTextureCoefficientsCalculator.h"

#define IMAGE_DIMENSIONS 2
#define IMAGE_DATA_EXT short
#define IMAGE_DATA_INT unsigned char
#define INITIAL_THRESHOLD_VALUE 128

typedef itk::Image< IMAGE_DATA_EXT, IMAGE_DIMENSIONS > ImageTypeExt;
typedef itk::Image< IMAGE_DATA_INT, IMAGE_DIMENSIONS > ImageTypeInt;
typedef itk::ImageFileReader< ImageTypeExt > ReaderType;
typedef itk::ImageFileWriter< ImageTypeInt > WriterType;
typedef itk::RescaleIntensityImageFilter< ImageTypeExt, ImageTypeInt > RescaleType;
typedef itk::BinaryThresholdImageFilter < ImageTypeInt, ImageTypeInt >  ThresholdFilterType;
typedef itk::NeighborhoodConnectedImageFilter< ImageTypeInt, ImageTypeInt > ConnectedFilterType;
typedef itk::MaskNegatedImageFilter<ImageTypeInt, ImageTypeInt, ImageTypeInt> MaskNegatedFilterType;
typedef itk::MaskImageFilter<ImageTypeInt, ImageTypeInt> MaskFilterType;
typedef itk::VotingBinaryIterativeHoleFillingImageFilter< ImageTypeInt > HoleFillingType;
typedef itk::BinaryBallStructuringElement < IMAGE_DATA_INT, IMAGE_DIMENSIONS > StructuringElementType;
typedef itk::BinaryErodeImageFilter < ImageTypeInt, ImageTypeInt, StructuringElementType> ErodeFilterType;
typedef itk::BinaryDilateImageFilter < ImageTypeInt, ImageTypeInt, StructuringElementType> DilateFilterType;
typedef itk::RegionOfInterestImageFilter< ImageTypeInt, ImageTypeInt > ROIFilterType;
typedef itk::ImageRegionConstIterator< ImageTypeInt > ConstIteratorType;
typedef itk::ImageRegionIterator< ImageTypeInt > IteratorType;

typedef itk::Statistics::ScalarImageTextureCalculator<ImageTypeInt> TextureCalculator;

using namespace std;

int isThereMoreThan2Islands(ImageTypeInt *image) {
	unsigned int x = image->GetLargestPossibleRegion().GetSize()[0];
	unsigned int y = image->GetLargestPossibleRegion().GetSize()[1];
	int hasWhite = 0;
	
	ConnectedFilterType::Pointer neighborhoodConnected = ConnectedFilterType::New();
	MaskNegatedFilterType::Pointer maskNegatedFilter = MaskNegatedFilterType::New();
	
	ImageTypeInt::IndexType index;
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
			ImageTypeInt::SizeType radius;
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

int smallerHeightPosition(ImageTypeInt *image) {
	unsigned int x = image->GetLargestPossibleRegion().GetSize()[0];
	unsigned int y = image->GetLargestPossibleRegion().GetSize()[1];
	int minHeight, minHeightPosition, tmp;
	
	if(x == 0 || y == 0) {
		cout << "Imagem passada pra smallerHeightPosition() é inválida.";
		exit(1);
	}
	
	int halfWeight = x/2;
	int foundFirst = 0;
	
	minHeight = 0;
	minHeightPosition = halfWeight;
	ImageTypeInt::IndexType index;
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

int limit(int direction, ImageTypeInt *image){
	ImageTypeInt::IndexType index;
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

void invertBinaryValues(ImageTypeInt *image) {
	IteratorType iterator(image, image->GetLargestPossibleRegion());
	for(iterator.GoToBegin(); !iterator.IsAtEnd(); ++iterator) {
		iterator.Set(1 - iterator.Value());
	}
}

int main (int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage: Segment filename");
		return 1;
	}
	
	if(strlen(argv[1]) > 200 ) {
		printf("Nome do arquivo muito grande.");
		return 1;
	}
	
	ReaderType::Pointer reader = ReaderType::New();
	reader->SetFileName(argv[1]);
	
	RescaleType::Pointer rescalerFilter = RescaleType::New();
	rescalerFilter->SetInput(reader->GetOutput());
	rescalerFilter->SetOutputMinimum(0);
	rescalerFilter->SetOutputMaximum(255);
	
	IMAGE_DATA_INT thresholdValue = INITIAL_THRESHOLD_VALUE;
	IMAGE_DATA_INT thresholdValueOLD = INITIAL_THRESHOLD_VALUE + 1;
	ImageTypeInt::IndexType index;
	ThresholdFilterType::Pointer thresholdFilter = ThresholdFilterType::New();
	thresholdFilter->SetOutsideValue(1); //TODO: magic number
	thresholdFilter->SetInsideValue(0); 
	thresholdFilter->SetLowerThreshold(0);
	thresholdFilter->SetInput(rescalerFilter->GetOutput());
	
	while(thresholdValue != thresholdValueOLD) {
		unsigned long long averageAbove, averageBelow;
		unsigned int quantityAbove, quantityBelow;
		
		averageAbove = averageBelow = quantityAbove = quantityBelow = 0;
		
		thresholdFilter->SetUpperThreshold(thresholdValue);
		thresholdFilter->Update();

		ConstIteratorType threshold_it(thresholdFilter->GetOutput(), thresholdFilter->GetOutput()->GetLargestPossibleRegion());
		ConstIteratorType reader_it(rescalerFilter->GetOutput(), rescalerFilter->GetOutput()->GetLargestPossibleRegion());
		for(threshold_it.GoToBegin(), reader_it.GoToBegin(); !threshold_it.IsAtEnd(); ++threshold_it, ++reader_it) {
			if(threshold_it.Get()) {
				averageAbove += reader_it.Get();
				quantityAbove++;
			}
			else {
				averageBelow += reader_it.Get();
				quantityBelow++;
			}
		}
		averageAbove /= quantityAbove;
		averageBelow /= quantityBelow;
		thresholdValueOLD = thresholdValue;
		thresholdValue = (averageAbove + averageBelow)/2;
	}
	invertBinaryValues(thresholdFilter->GetOutput());
	
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
	ImageTypeInt::SizeType radius;
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
	
	DilateFilterType::Pointer binaryDilateFilter = DilateFilterType::New();   
 	ErodeFilterType::Pointer binaryErodeFilter = ErodeFilterType::New();
 	
 	StructuringElementType structuringElement;
 	structuringElement.SetRadius( 6 ); 
 	structuringElement.CreateStructuringElement();
 	binaryDilateFilter->SetKernel( structuringElement );
 	binaryErodeFilter->SetKernel( structuringElement );
 	
 	binaryDilateFilter->SetInput( holeFillingFilter->GetOutput() );
 	binaryErodeFilter->SetInput( binaryDilateFilter->GetOutput() );
 	binaryDilateFilter->SetDilateValue( 1 );
 	binaryErodeFilter->SetErodeValue( 1 );
	
	ImageTypeInt *image;
	binaryErodeFilter->Update();
	image = binaryErodeFilter->GetOutput();
	if (!isThereMoreThan2Islands(image)) {
		unsigned int y = image->GetLargestPossibleRegion().GetSize()[1];
		index[0] = smallerHeightPosition(image);
		for (index[1] = 0; index[1] < y; index[1]++) {
			if(image->GetPixel(index)) {
				break;
			} 
		}
		if (index[1] == y) {
			cout << "Algoritmo de busca do ponto de separação falhou.\nABORTANDO...\n";
			return 1;
		}
		while(image->GetPixel(index)) {
			image->SetPixel(index, 0);
			index[1]++;
		}
	}
	
	//Split lungs by centre of mass!
	unsigned int x = image->GetLargestPossibleRegion().GetSize()[0];
	unsigned int y = image->GetLargestPossibleRegion().GetSize()[1];
	
	ImageTypeInt::Pointer leftLung = ImageTypeInt::New();
	ImageTypeInt::Pointer rightLung = ImageTypeInt::New();
	
	ImageTypeInt::IndexType start;
	start[0] =   0;
	start[1] =   0;
	
	ImageTypeInt::SizeType  size;
	size[0]  = x;
	size[1]  = y;
	
	ImageTypeInt::RegionType region;
	region.SetSize( size );
	region.SetIndex( start );

	leftLung->SetRegions( region );
	leftLung->Allocate();
	rightLung->SetRegions( region );
	rightLung->Allocate();
	
	{
		IteratorType left_it(leftLung, leftLung->GetLargestPossibleRegion());
		for(left_it.GoToBegin(); !left_it.IsAtEnd(); ++left_it) {
			left_it.Set(0);
		}
		IteratorType right_it(rightLung, rightLung->GetLargestPossibleRegion());
		for(right_it.GoToBegin(); !right_it.IsAtEnd(); ++right_it) {
			right_it.Set(0);
		}
	}
	
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
				ImageTypeInt::SizeType radius;
				radius[0] = 0;
				radius[1] = 0;
				neighborhoodConnectedSpliter->SetRadius(radius);
				neighborhoodConnectedSpliter->Update();

				int leftLimit, rightLimit;
				leftLimit = limit( 1 ,neighborhoodConnectedSpliter->GetOutput());
				rightLimit = limit( 3 ,neighborhoodConnectedSpliter->GetOutput());
				
				if(leftLimit + rightLimit > x) {
					// Right side
					ConstIteratorType neightbor_it(neighborhoodConnectedSpliter->GetOutput(), neighborhoodConnectedSpliter->GetOutput()->GetLargestPossibleRegion());
					IteratorType right_it(rightLung, rightLung->GetLargestPossibleRegion());
					for(right_it.GoToBegin(), neightbor_it.GoToBegin(); !right_it.IsAtEnd(); ++right_it, ++neightbor_it) {
						if(neightbor_it.Value()) {
							right_it.Set(1);
						}
					}
				} else {
					// Left side
					ConstIteratorType neightbor_it(neighborhoodConnectedSpliter->GetOutput(), neighborhoodConnectedSpliter->GetOutput()->GetLargestPossibleRegion());
					IteratorType left_it(leftLung, leftLung->GetLargestPossibleRegion());
					for(left_it.GoToBegin(), neightbor_it.GoToBegin(); !left_it.IsAtEnd(); ++left_it, ++neightbor_it) {
						if(neightbor_it.Value()) {
							left_it.Set(1);
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
	maskFilterLeft->SetInput1(rescalerFilter->GetOutput());
	maskFilterLeft->SetInput2(leftLung);
	maskFilterLeft->Update();
	
	MaskFilterType::Pointer maskFilterRight = MaskFilterType::New();
	maskFilterRight->SetInput1(rescalerFilter->GetOutput());
	maskFilterRight->SetInput2(rightLung);
	maskFilterRight->Update();
	
	ROIFilterType::Pointer roiFilterLeft = ROIFilterType::New();
	ROIFilterType::Pointer roiFilterRight = ROIFilterType::New();
	ImageTypeInt::RegionType roi;
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

	char filename[230];
	
	WriterType::Pointer writer = WriterType::New();
	strcpy(filename, argv[1]);
	strcat(filename, "_left.dcm");
	writer->SetFileName( filename );
	writer->SetInput(roiFilterLeft->GetOutput());
	writer->Update();
	strcpy(filename, argv[1]);
	strcat(filename, "_right.dcm");
	writer->SetFileName( filename );
	writer->SetInput(roiFilterRight->GetOutput());
	writer->Update();
	
	{
		TextureCalculator::Pointer calc  = TextureCalculator::New();
		TextureCalculator::FeatureNameVector::Pointer featureVector = TextureCalculator::FeatureNameVector::New();
		featureVector->InsertElement(0, itk::Statistics::Energy);
		featureVector->InsertElement(1, itk::Statistics::Entropy);
		featureVector->InsertElement(2, itk::Statistics::InverseDifferenceMoment);
		featureVector->InsertElement(3, itk::Statistics::Inertia);
		featureVector->InsertElement(4, itk::Statistics::HaralickCorrelation);
		calc->SetRequestedFeatures(featureVector);
		
		calc->SetInput(roiFilterLeft->GetOutput());
		calc->Compute();

		strcpy(filename, argv[1]);
		strcat(filename, "_left.txt");
		FILE *fp = fopen(filename, "w");
		if (!fp) {
			printf("Falha ao gravar caracteristicas.");
			return 1;
		}
		
		unsigned long size = calc->GetFeatureMeans()->Size();
		for(x = 0; x < size; x++) {
			fprintf(fp, "%d: %f\n",calc->GetRequestedFeatures()->GetElement(x), calc->GetFeatureMeans()->GetElement(x));
		}
		fclose(fp);
		
		calc->SetInput(roiFilterRight->GetOutput());
		calc->Compute();

		strcpy(filename, argv[1]);
		strcat(filename, "_right.txt");
		fp = fopen(filename, "w");
		if (!fp) {
			printf("Falha ao gravar caracteristicas.");
			return 1;
		}
		
		size = calc->GetFeatureMeans()->Size();
		for(x = 0; x < size; x++) {
			fprintf(fp, "%d: %f\n",calc->GetRequestedFeatures()->GetElement(x), calc->GetFeatureMeans()->GetElement(x));
		}
	}
	return 0;
}

