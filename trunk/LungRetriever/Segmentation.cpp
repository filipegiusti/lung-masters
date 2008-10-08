#include "Segmentation.hpp"

template <class T, unsigned int I>
void Segmentation< T,I >::doIt() {
	
		
	return;
}

template <class T, unsigned int I>
itk::Image< T,I >* Segmentation< T,I >::GetOutput() {
	return input;
}