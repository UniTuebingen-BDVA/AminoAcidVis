cmake_minimum_required(VERSION 2.8.12)
set(UI_FILES aminoVisMainWindow.ui settingsWindow.ui debugDockWidget.ui plainTextBox.ui atomRenderWidget.ui frameControlWidget.ui filterHelpWindow.ui settingsHeatmapScreenshot.ui)
set(RESOURCE_FILES "${PROJECT_ICONS_PATH}/aminoVisIcon.res") #windres aminoVisIcon.rc -O coff -o aminoVisIcon.res

include(${CMAKE_MODULE_PATH}/QtExecutable.cmake)