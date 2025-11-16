#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=binkyDebug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/azbduobt.o \
	${OBJECTDIR}/btraw.o \
	${OBJECTDIR}/main.o


# C Compiler Flags
CFLAGS=-m64 `pkg-config --cflags glib-2.0`  -ffunction-sections  -fdata-sections -pedantic -Wall -fsanitize=undefined -Wsuggest-override 

# CC Compiler Flags
CCFLAGS=-m64 `pkg-config --cflags glib-2.0` `pkg-config --cflags dbus-1` -W -Wshadow -Wall -Wextra -ffunction-sections -fdata-sections -fsanitize=undefined -Wsuggest-override
 
CXXFLAGS=-m64 `pkg-config --cflags glib-2.0` `pkg-config --cflags dbus-1` -W -Wshadow -Wall -Wextra -ffunction-sections -fdata-sections -fsanitize=undefined -Wsuggest-override
 

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=--64

# Link Libraries and Options
LDLIBSOPTIONS=`pkg-config --libs libusb-1.0` `pkg-config --libs glib-2.0` `pkg-config --libs gio-2.0` `pkg-config --libs dbus-1`  

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/azbduokbdbacklight

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/azbduokbdbacklight: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/azbduokbdbacklight ${OBJECTFILES} ${LDLIBSOPTIONS} -ffunction-sections -fdata-sections -fsanitize=undefined

${OBJECTDIR}/azbduobt.o: azbduobt.cc nbproject/Makefile-${CND_CONF}.mk
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall `pkg-config --cflags libusb-1.0` `pkg-config --cflags glib-2.0` `pkg-config --cflags gio-2.0` `pkg-config --cflags dbus-1` -std=c++14  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/azbduobt.o azbduobt.cc

${OBJECTDIR}/btraw.o: btraw.cc nbproject/Makefile-${CND_CONF}.mk
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall `pkg-config --cflags libusb-1.0` `pkg-config --cflags glib-2.0` `pkg-config --cflags gio-2.0` `pkg-config --cflags dbus-1`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/btraw.o btraw.cc

${OBJECTDIR}/main.o: main.cc nbproject/Makefile-${CND_CONF}.mk
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall `pkg-config --cflags libusb-1.0` `pkg-config --cflags glib-2.0` `pkg-config --cflags gio-2.0` `pkg-config --cflags dbus-1`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cc

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
