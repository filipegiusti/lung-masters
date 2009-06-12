IMAGE_DATA_INT maxValue(ImageTypeInt *image){
	ImageTypeInt::IndexType index;
	IMAGE_DATA_INT max = 0;
	int x = image->GetLargestPossibleRegion().GetSize()[0];
	int y = image->GetLargestPossibleRegion().GetSize()[1];
	for (int i = 0; i < x; i++) {
		for (int j = 0; j < y; j++) {
			index[0] = i;
			index[1] = j;
			if(image->GetPixel(index) > max) {
				max = image->GetPixel(index);
			}
		}
	}
	return max;
}

IMAGE_DATA_INT minValue(ImageTypeInt *image){
	ImageTypeInt::IndexType index;
	IMAGE_DATA_INT max = 255;
	int x = image->GetLargestPossibleRegion().GetSize()[0];
	int y = image->GetLargestPossibleRegion().GetSize()[1];
	for (int i = 0; i < x; i++) {
		for (int j = 0; j < y; j++) {
			index[0] = i;
			index[1] = j;
			if(image->GetPixel(index) < max) {
				max = image->GetPixel(index);
			}
		}
	}
	return max;
}
