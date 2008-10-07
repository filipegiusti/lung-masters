PROJECT(teste)

FIND_PACKAGE(ITK)

cmake_minimum_required(VERSION 2.6)

IF(ITK_FOUND)
	INCLUDE(${ITK_USE_FILE})
ELSE(ITK_FOUND)
	MESSAGE(FATAL_ERROR
		"ITK not found. Please set ITK_DIR.")
ENDIF(ITK_FOUND)

ADD_EXECUTABLE(Segmentation Segmentation.cpp main.cpp)

TARGET_LINK_LIBRARIES(Segmentation ITKCommon ITKIO)
