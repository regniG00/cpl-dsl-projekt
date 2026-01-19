cmake_minimum_required(VERSION 3.15)
project(AufgabenDsl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
  add_compile_options(/utf-8)
endif()

# vcpkg-Pfade anpassen, falls bei dir anders
set(VCPKG_ROOT "C:/Users/Malte/tools/vcpkg")
set(VCPKG_TRIPLET "x64-windows")

set(ANTLR4_INCLUDE_DIR "${VCPKG_ROOT}/installed/${VCPKG_TRIPLET}/include/antlr4-runtime")
set(ANTLR4_LIB_DIR     "${VCPKG_ROOT}/installed/${VCPKG_TRIPLET}/lib")

add_executable(aufgaben_dsl
    src/main.cpp

    src/ir/IRBuilder.cpp

    src/domain/DomainConvert.cpp
    src/domain/DomainJson.cpp

    grammar/AufgabenerstellungsgrammatikLexer.cpp
    grammar/AufgabenerstellungsgrammatikParser.cpp
    grammar/AufgabenerstellungsgrammatikBaseVisitor.cpp
    grammar/AufgabenerstellungsgrammatikVisitor.cpp
)

target_include_directories(aufgaben_dsl PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/grammar
    ${ANTLR4_INCLUDE_DIR}
)

# --- Runtime DLLs (Windows) ---
if (WIN32)
  add_custom_command(TARGET aufgaben_dsl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "C:/Users/Malte/tools/vcpkg/installed/x64-windows/bin/antlr4-runtime.dll"
      $<TARGET_FILE_DIR:aufgaben_dsl>
  )
endif()

target_link_directories(aufgaben_dsl PRIVATE
    ${ANTLR4_LIB_DIR}
)

target_link_libraries(aufgaben_dsl PRIVATE
    antlr4-runtime
)
