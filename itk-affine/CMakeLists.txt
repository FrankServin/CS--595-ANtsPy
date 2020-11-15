cmake_minimum_required(VERSION 2.8)

project (ITKaffine)

find_package (ITK REQUIRED)
include (${ITK_USE_FILE})

add_executable (ITKaffine itkaffinereg.cxx)

target_link_libraries (ITKaffine ${ITK_LIBRARIES})
