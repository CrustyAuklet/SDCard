cmake_minimum_required(VERSION 3.13)
project(SDCard)

set(CMAKE_CXX_STANDARD 17)

add_library(SDCard INTERFACE)

target_sources(SDCard
        INTERFACE
            sdCard/SDCard.hpp
            sdCard/SDCard_info.h
)

target_include_directories(SDCard INTERFACE sdCard/)

if(UNIX OR WIN32 OR CYGWIN OR MSYS OR MINGW)
    add_executable(SDCardTest)

    target_sources(SDCardTest
            PRIVATE
                main.cpp
                external/spiDriver/spidriver.h
                external/spiDriver/spidriver.c

                external/FatLib/source/ff.c
                external/FatLib/source/ffsystem.c
                external/FatLib/source/ffunicode.c
                external/FatLib/source/ffconf.h
                external/FatLib/source/ff.h
                external/FatLib/source/diskio.h
    )

    target_include_directories(SDCardTest
            PRIVATE
                external/

    )

    target_link_libraries(SDCardTest SDCard)

endif()