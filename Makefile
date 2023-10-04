.PHONY: clean all release test help 
CC=clang++.exe
objects := klim.o helperFunctions.o
BUILD_DIR := .\build
SRC_DIR := .\src
RELEASE_DIR := .\release
INC_DIRS := .\phnt,.\WinDivert,${SRC_DIR}
INC_FLAGS := -I,$(INC_DIRS)

all: ${BUILD_DIR}\krekens_overlay.dll ${BUILD_DIR}\klim.exe 

help:
	@echo available targets:
	@echo    all 				 - compiles all targets
	@echo    release             - makes a release dir with a ready to use klim (hopefully).
	@echo    clean               - deletes ${BUILD_DIR}.
	@echo    test                - runs ${RELEASE_DIR}\klim.exe --debug.
	@echo    help                - prints this message.
	@echo    ..there are other targets but they should not be used.

${BUILD_DIR}\klim.o: ${SRC_DIR}\main.cpp
	-mkdir ${BUILD_DIR} 
	${CC} -o ${BUILD_DIR}\main.o -D "_UNICODE" -D "UNICODE" -Wall -c ${INC_FLAGS} ${SRC_DIR}\main.cpp 
	
${BUILD_DIR}\helperFunctions.o: ${SRC_DIR}\helperFunctions.cpp
	-mkdir ${BUILD_DIR} 
	${CC} -o ${BUILD_DIR}\helperFunctions.o -D "_UNICODE" -D "UNICODE" -Wall -c ${INC_FLAGS} ${SRC_DIR}\helperFunctions.cpp 

${BUILD_DIR}\windivertFunctions.o: ${SRC_DIR}\windivertFunctions.cpp 
	-mkdir ${BUILD_DIR} 
	${CC} -o ${BUILD_DIR}\windivertFunctions.o -D "_UNICODE" -D "UNICODE" -Wall -c ${INC_FLAGS} ${SRC_DIR}\windivertFunctions.cpp 

${BUILD_DIR}\klim.exe: ${BUILD_DIR}\klim.o ${BUILD_DIR}\helperFunctions.o ${BUILD_DIR}\windivertFunctions.o
	${CC} -o ${BUILD_DIR}\klim.exe -l ".\Windivert\Windivert.lib" -l "${BUILD_DIR}\krekens_overlay.lib" ${BUILD_DIR}\main.o ${BUILD_DIR}\helperFunctions.o ${BUILD_DIR}\windivertFunctions.o

${BUILD_DIR}\krekens_overlay.dll:  ${SRC_DIR}\krekens_overlay.cpp
	-mkdir ${BUILD_DIR} 
	${CC} -o ${BUILD_DIR}\krekens_overlay.dll -D "_UNICODE" -D "UNICODE" -shared -Wall ${SRC_DIR}\krekens_overlay.cpp 

clean: 
	-del /F /Q ${BUILD_DIR}\*
	-rmdir ${BUILD_DIR}
	
release: ${BUILD_DIR}\krekens_overlay.dll ${BUILD_DIR}\klim.exe
	-mkdir release
	-copy ${BUILD_DIR}\krekens_overlay.dll ${RELEASE_DIR}\ 
	-copy ${BUILD_DIR}\klim.exe ${RELEASE_DIR}\ 
	-copy .\WinDivert\WinDivert.dll ${RELEASE_DIR}\ 
	-copy .\WinDivert\WinDivert64.sys ${RELEASE_DIR}\ 

test:
	${RELEASE_DIR}\klim.exe --debug

