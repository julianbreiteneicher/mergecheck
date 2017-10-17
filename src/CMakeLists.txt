add_executable(mergecheck
  mergecheck.cpp
  conflict.cpp
  merge.cpp
  rebase.cpp
  remote.cpp
  string_utils.cpp
  utils.cpp
  )

target_link_libraries(mergecheck ${COMMON_LIBS})
