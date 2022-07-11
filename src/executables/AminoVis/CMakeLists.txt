cmake_minimum_required(VERSION 3.0)
set(CMAKE_CONFIGURATION_TYPES Debug;Release)
set(UI_FILES aminoVisMainWindow.ui settingsWindow.ui debugDockWidget.ui plainTextBox.ui atomRenderWidget.ui frameControlWidget.ui filterHelpWindow.ui settingsHeatmapScreenshot.ui)

execute_process(COMMAND windres aminoVisIcon.rc -O coff -o aminoVisIcon.res
        WORKING_DIRECTORY ${PROJECT_ICONS_PATH})
set(RESOURCE_FILES "${PROJECT_ICONS_PATH}/aminoVisIcon.res") #windres aminoVisIcon.rc -O coff -o aminoVisIcon.res
include(${CMAKE_MODULE_PATH}/QtExecutable.cmake)

#Copy required DLLs.
file( COPY  "${CMAKE_CXX_COMPILER}/../libwinpthread-1.dll" DESTINATION "${EXECUTABLE_OUTPUT_PATH}" )
file( COPY  "${CMAKE_CXX_COMPILER}/../libstdc++-6.dll" DESTINATION "${EXECUTABLE_OUTPUT_PATH}/" )
file( COPY  "${CMAKE_CXX_COMPILER}/../libgcc_s_seh-1.dll" DESTINATION "${EXECUTABLE_OUTPUT_PATH}" )