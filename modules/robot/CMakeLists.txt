# Add optional 3rd parties
set(opt_incs "")
set(opt_libs "")

if(USE_AFMA4 OR USE_AFMA6 OR USE_VIPER650 OR USE_VIPER850)
  list(APPEND opt_incs ${RAW1394_INCLUDE_DIRS})
  list(APPEND opt_incs ${CALINUX_INCLUDE_DIRS})
  list(APPEND opt_incs ${IRISA_INCLUDE_DIRS})
  list(APPEND opt_libs ${IRISA_LIBRARIES})
  list(APPEND opt_libs ${CALINUX_LIBRARIES})
  list(APPEND opt_libs ${RAW1394_LIBRARIES})
  list(APPEND opt_libs ${RT_LIBRARIES})
endif()

if(USE_BICLOPS)
  list(APPEND opt_incs ${BICLOPS_INCLUDE_DIRS})
  list(APPEND opt_libs ${BICLOPS_LIBRARIES})
endif()

if(USE_PTU46)
  list(APPEND opt_incs ${PTU46_INCLUDE_DIRS})
  list(APPEND opt_libs ${PTU46_LIBRARIES})
endif()

if(USE_ARIA AND UNIX)
  # Under Unix we need Aria, pthread, dl and rt libraries
  if(USE_PTHREAD AND RT_FOUND AND DL_FOUND)
    list(APPEND opt_incs ${ARIA_INCLUDE_DIRS})
    list(APPEND opt_libs ${ARIA_LIBRARIES})
    list(APPEND opt_incs ${PTHREAD_INCLUDE_DIRS})
    list(APPEND opt_libs ${PTHREAD_LIBRARIES})
    list(APPEND opt_libs ${RT_LIBRARIES})
    list(APPEND opt_libs ${DL_LIBRARIES})
  endif()
elseif(USE_ARIA AND NOT UNIX)
  list(APPEND opt_incs ${ARIA_INCLUDE_DIRS})
  list(APPEND opt_libs ${ARIA_LIBRARIES})
endif()

# for the wireframe simulator we can use optional opengl and coin
if(USE_COIN3D)
  if(WIN32)
    add_definitions("-DCOIN_DLL")
  endif()
  list(APPEND opt_incs ${COIN3D_INCLUDE_DIRS})
  # On OSX cmake 2.8 found OpenGL but OPENGL_INCLUDE_DIR was set to NOT_FOUND
  # We add a test to be sure that the OPENGL vars exist.
  if(OPENGL_INCLUDE_DIR)
    list(APPEND opt_incs ${OPENGL_INCLUDE_DIR})
  endif()
  if(OPENGL_LIBRARIES)
    list(APPEND opt_libs ${OPENGL_LIBRARIES})
  endif()
  list(APPEND opt_libs ${COIN3D_LIBRARIES})
endif()

vp_add_module(robot visp_core)
vp_glob_module_sources()

# copy robot and wireframe simulator data
vp_glob_module_copy_data("src/simulator-robot/arms/*.bnd"      data/robot-simulator)
vp_glob_module_copy_data("src/wireframe-simulator/scene/*.bnd" data/wireframe-simulator)
vp_glob_module_copy_data("src/wireframe-simulator/scene/*.sld" data/wireframe-simulator)

vp_module_include_directories(${opt_incs})
vp_create_module(${opt_libs})
vp_add_tests(DEPENDS_ON visp_sensor visp_vision)