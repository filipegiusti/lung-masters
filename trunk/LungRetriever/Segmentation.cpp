#include "Segmentation.hpp"

void Segmentation::doIt() {
	ReaderType::Pointer reader = ReaderType::New();
	
	//reader->SetFileName( file );
	//typedef itk::GDCMImageIO           ImageIOType;
	//ImageIOType::Pointer gdcmImageIO = ImageIOType::New();
	//reader->SetImageIO( gdcmImageIO );

	cout << file->data() << " de tamanho " << file->length() << "\n";
	return;
}