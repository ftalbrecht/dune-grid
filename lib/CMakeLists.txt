
set(ALULIBS _DUNE_TARGET_OBJECTS:alu2d_ _DUNE_TARGET_OBJECTS:alu3d_)

if(UG_FOUND)
  set(UGLIB _DUNE_TARGET_OBJECTS:uggrid_)
endif(UG_FOUND)

# OBJECT is needed since CMake 3.0.0
# but it conflicts with shared libs
set(_OBJECT_FLAG "OBJECT")
if(BUILD_SHARED_LIBS)
  set(_OBJECT_FLAG "")
endif()

dune_add_library(dunegrid
  _DUNE_TARGET_OBJECTS:onedgrid_
  _DUNE_TARGET_OBJECTS:dgfparser_
  _DUNE_TARGET_OBJECTS:dgfparserblocks_
  ${ALULIBS} ${UGLIB}
  ADD_LIBS ${DUNE_LIBS})
add_dune_ug_flags(dunegrid ${_OBJECT_FLAG})
add_dune_alugrid_flags(dunegrid ${_OBJECT_FLAG})

foreach(_dim ${ALBERTA_WORLD_DIMS})
  dune_add_library(dunealbertagrid_${_dim}d
    _DUNE_TARGET_OBJECTS:albertagrid_${_dim}d_
    _DUNE_TARGET_OBJECTS:dgfparser_
    _DUNE_TARGET_OBJECTS:dgfparserblocks_
    ADD_LIBS ${DUNE_LIBS})
  add_dune_alberta_flags(dunealbertagrid_${_dim}d ${_OBJECT_FLAG} GRIDDIM ${_dim})
  list(APPEND DUNE_ALBERTA_LIBS dunealbertagrid_${_dim}d)
endforeach(_dim "${ALBERTA_DIMS}")

install(FILES dunegridam2cmake.lib
  DESTINATION ${CMAKE_INSTALL_BINDIR}/../lib)
