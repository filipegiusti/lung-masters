#
# Gererated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=

# Include project Makefile
include pcc-Makefile.mk

# Object Directory
OBJECTDIR=build/Default/GNU-Linux-x86

# Object Files
OBJECTFILES=

# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS} 
	cd . && cd LungRetriever && cmake . && make -f Makefile && cd ..

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf:
	cd . && cd LungRetriever/ && make -f Makefile clean && cd ..

# Subprojects
.clean-subprojects:
