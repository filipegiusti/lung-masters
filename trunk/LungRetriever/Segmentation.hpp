#include "itkImage.h"
#include <iostream>

using namespace std;

template <class T, unsigned int I>
class Segmentation {
private:
	itk::Image<T,I> *input;
public:
	Segmentation(itk::Image<T,I> *input) {
		this->input = input;
	}
	itk::Image<T,I>* GetOutput();
	void doIt();
};
