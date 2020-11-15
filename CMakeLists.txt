cmake_minimum_required(VERSION 3.7)

project (ITKdeform)

find_package (ITK REQUIRED)
include (${ITK_USE_FILE})

add_executable (ITKdeform itkdeformreg.cxx)

target_link_libraries (ITKdeform ${ITK_LIBRARIES})
