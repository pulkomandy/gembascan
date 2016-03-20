#******************************************************************************
#
# Makefile - Rules for building GBS8200 controller.
#
#******************************************************************************

#
# Defines the part type that this project uses.
#
PART=LM4F120H5QR

#
# Set the processor variant.
#
VARIANT=cm4f

#
# The base directory for StellarisWare.
#
ROOT=stellarisware

#
# Include the common make definitions.
#
include ${ROOT}/makedefs

#
# Where to find source files that do not live in this directory.
#
VPATH=${ROOT}/drivers
VPATH+=${ROOT}/utils

#
# Where to find header files that do not live in the source directory.
#
IPATH=..
IPATH+=${ROOT}

#
# The default rule, which causes the Quickstart RGB to be built.
#
all: ${COMPILER}
all: ${COMPILER}/qs-rgb.axf

#
# The rule to clean out all the build products.
#
clean:
	@rm -rf ${COMPILER} ${wildcard *~}

#
# The rule to create the target directory.
#
${COMPILER}:
	@mkdir -p ${COMPILER}

#
# Rules for building the Quickstart RGB.
#
#${COMPILER}/qs-rgb.axf: ${COMPILER}/buttons.o
${COMPILER}/qs-rgb.axf: ${COMPILER}/cmdline.o
${COMPILER}/qs-rgb.axf: ${COMPILER}/qs-rgb.o
${COMPILER}/qs-rgb.axf: ${COMPILER}/rgb.o
${COMPILER}/qs-rgb.axf: ${COMPILER}/rgb_commands.o
${COMPILER}/qs-rgb.axf: ${COMPILER}/startup_${COMPILER}.o
${COMPILER}/qs-rgb.axf: ${COMPILER}/uartstdio.o
${COMPILER}/qs-rgb.axf: ${COMPILER}/ustdlib.o
${COMPILER}/qs-rgb.axf: ${ROOT}/usblib/${COMPILER}-cm4f/libusb-cm4f.a
${COMPILER}/qs-rgb.axf: ${ROOT}/driverlib/${COMPILER}-cm4f/libdriver-cm4f.a
${COMPILER}/qs-rgb.axf: qs-rgb.ld
SCATTERgcc_qs-rgb=qs-rgb.ld
ENTRY_qs-rgb=ResetISR
CFLAGSgcc=-DTARGET_IS_BLIZZARD_RA2 -DUART_BUFFERED

#
# Include the automatically generated dependency files.
#
ifneq (${MAKECMDGOALS},clean)
-include ${wildcard ${COMPILER}/*.d} __dummy__
endif
