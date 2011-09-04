
# small trick to get the real source directory at this stage
STRING(REPLACE "/PreLoad.cmake" "" ODYSSEY_HOME_DIR ${CMAKE_CURRENT_LIST_FILE})

#message("/PreLoad.cmake ... ${ODYSSEY_HOME_DIR}")

SET(CMAKE_MODULE_PATH "${ODYSSEY_HOME_DIR}/cmake" CACHE INTERNAL "")

#message("CMAKE_MODULE_PATH = ${CMAKE_MODULE_PATH}")

