# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation

option(BUILD_DOC "Build documentation" OFF)

if(BUILD_DOC)
  find_package(Doxygen QUIET)
  if (DOXYGEN_FOUND)
      # set input and output files
      set(DOXYGEN_IN docs/Doxyfile.in)
      set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

      # request to configure the file
      configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
      message("Doxygen build started")

      # note the option ALL which allows to build the docs together with the application
      add_custom_target(doc_doxygen ALL
          COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
          COMMENT "Generating API documentation with Doxygen"
          VERBATIM )
  else (DOXYGEN_FOUND)
    message("Doxygen need to be installed to generate the doxygen documentation ")
  endif (DOXYGEN_FOUND)
endif(BUILD_DOC)
