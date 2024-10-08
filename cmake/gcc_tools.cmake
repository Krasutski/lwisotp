GET_FILENAME_COMPONENT(STM32_CMAKE_DIR ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)
SET(CMAKE_MODULE_PATH ${STM32_CMAKE_DIR} ${CMAKE_MODULE_PATH})

IF(NOT TOOLCHAIN_PREFIX)
     SET(TOOLCHAIN_PREFIX "/usr")
     MESSAGE(STATUS "No TOOLCHAIN_PREFIX specified, using default: " ${TOOLCHAIN_PREFIX})
ELSE()
     FILE(TO_CMAKE_PATH "${TOOLCHAIN_PREFIX}" TOOLCHAIN_PREFIX)
ENDIF()

IF(NOT TARGET_TRIPLET)
    SET(TARGET_TRIPLET "arm-none-eabi")
    MESSAGE(STATUS "No TARGET_TRIPLET specified, using default: " ${TARGET_TRIPLET})
ENDIF()

SET(TOOLCHAIN_BIN_DIR "${TOOLCHAIN_PREFIX}/bin")
SET(TOOLCHAIN_INC_DIR "${TOOLCHAIN_PREFIX}/${TARGET_TRIPLET}/include")
SET(TOOLCHAIN_LIB_DIR "${TOOLCHAIN_PREFIX}/${TARGET_TRIPLET}/lib")

SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_PROCESSOR arm)

IF (WIN32)
    SET(TOOL_EXECUTABLE_SUFFIX ".exe")
ELSE()
    SET(TOOL_EXECUTABLE_SUFFIX "")
ENDIF()

IF(${CMAKE_VERSION} VERSION_LESS 3.6.0)
    INCLUDE(CMakeForceCompiler)
    CMAKE_FORCE_C_COMPILER("${TOOLCHAIN_BIN_DIR}/${TARGET_TRIPLET}-gcc${TOOL_EXECUTABLE_SUFFIX}" GNU)
    CMAKE_FORCE_CXX_COMPILER("${TOOLCHAIN_BIN_DIR}/${TARGET_TRIPLET}-g++${TOOL_EXECUTABLE_SUFFIX}" GNU)
ELSE()
    SET(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
    SET(CMAKE_C_COMPILER "${TOOLCHAIN_BIN_DIR}/${TARGET_TRIPLET}-gcc${TOOL_EXECUTABLE_SUFFIX}" CACHE PATH "" FORCE)
    SET(CMAKE_CXX_COMPILER "${TOOLCHAIN_BIN_DIR}/${TARGET_TRIPLET}-g++${TOOL_EXECUTABLE_SUFFIX}" CACHE PATH "" FORCE)
ENDIF()
SET(CMAKE_ASM_COMPILER "${TOOLCHAIN_BIN_DIR}/${TARGET_TRIPLET}-gcc${TOOL_EXECUTABLE_SUFFIX}")

SET(CMAKE_OBJCOPY "${TOOLCHAIN_BIN_DIR}/${TARGET_TRIPLET}-objcopy${TOOL_EXECUTABLE_SUFFIX}" CACHE INTERNAL "objcopy tool")
SET(CMAKE_OBJDUMP "${TOOLCHAIN_BIN_DIR}/${TARGET_TRIPLET}-objdump${TOOL_EXECUTABLE_SUFFIX}" CACHE INTERNAL "objdump tool")
SET(CMAKE_SIZE "${TOOLCHAIN_BIN_DIR}/${TARGET_TRIPLET}-size${TOOL_EXECUTABLE_SUFFIX}" CACHE INTERNAL "size tool")
SET(CMAKE_DEBUGER "${TOOLCHAIN_BIN_DIR}/${TARGET_TRIPLET}-gdb${TOOL_EXECUTABLE_SUFFIX}" CACHE INTERNAL "debuger")
SET(CMAKE_CPPFILT "${TOOLCHAIN_BIN_DIR}/${TARGET_TRIPLET}-c++filt${TOOL_EXECUTABLE_SUFFIX}" CACHE INTERNAL "C++filt")

SET(CMAKE_C_FLAGS_DEBUG "-Og -g" CACHE INTERNAL "c compiler flags debug")
SET(CMAKE_CXX_FLAGS_DEBUG "-Og -g" CACHE INTERNAL "cxx compiler flags debug")
SET(CMAKE_ASM_FLAGS_DEBUG "-g" CACHE INTERNAL "asm compiler flags debug")
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "" CACHE INTERNAL "linker flags debug")

SET(CMAKE_C_FLAGS_RELEASE "" CACHE INTERNAL "c compiler flags release")
SET(CMAKE_CXX_FLAGS_RELEASE "" CACHE INTERNAL "cxx compiler flags release")
SET(CMAKE_ASM_FLAGS_RELEASE "" CACHE INTERNAL "asm compiler flags release")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "" CACHE INTERNAL "linker flags release")

SET(CMAKE_FIND_ROOT_PATH "${TOOLCHAIN_PREFIX}/${TARGET_TRIPLET}" ${EXTRA_FIND_PATH})
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

FUNCTION(STM32_ADD_HEX_BIN_TARGETS TARGET)
    IF(EXECUTABLE_OUTPUT_PATH)
      SET(FILENAME "${EXECUTABLE_OUTPUT_PATH}/${TARGET}")
    ELSE()
      SET(FILENAME "${TARGET}")
    ENDIF()
    ADD_CUSTOM_TARGET(${TARGET}.hex DEPENDS ${TARGET} COMMAND ${CMAKE_OBJCOPY} -Oihex ${FILENAME} ${FILENAME}.hex)
    ADD_CUSTOM_TARGET(${TARGET}.bin DEPENDS ${TARGET} COMMAND ${CMAKE_OBJCOPY} -Obinary ${FILENAME} ${FILENAME}.bin)
ENDFUNCTION()

FUNCTION(STM32_ADD_DUMP_TARGET TARGET)
    IF(EXECUTABLE_OUTPUT_PATH)
      SET(FILENAME "${EXECUTABLE_OUTPUT_PATH}/${TARGET}")
    ELSE()
      SET(FILENAME "${TARGET}")
    ENDIF()
    ADD_CUSTOM_TARGET(${TARGET}.dump DEPENDS ${TARGET} COMMAND ${CMAKE_OBJDUMP} -x -D -S -s ${FILENAME} | ${CMAKE_CPPFILT} > ${FILENAME}.dump)
ENDFUNCTION()

FUNCTION(STM32_PRINT_SIZE_OF_TARGETS TARGET)
    IF(EXECUTABLE_OUTPUT_PATH)
      SET(FILENAME "${EXECUTABLE_OUTPUT_PATH}/${TARGET}")
    ELSE()
      SET(FILENAME "${TARGET}")
    ENDIF()
    add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_SIZE} ${FILENAME})
ENDFUNCTION()
