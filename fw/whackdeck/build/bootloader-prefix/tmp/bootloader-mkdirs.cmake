# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/larzuk/esp/esp-idf/components/bootloader/subproject"
  "/home/larzuk/GIT/WHackDeck/WHackDeck/fw/whackdeck/build/bootloader"
  "/home/larzuk/GIT/WHackDeck/WHackDeck/fw/whackdeck/build/bootloader-prefix"
  "/home/larzuk/GIT/WHackDeck/WHackDeck/fw/whackdeck/build/bootloader-prefix/tmp"
  "/home/larzuk/GIT/WHackDeck/WHackDeck/fw/whackdeck/build/bootloader-prefix/src/bootloader-stamp"
  "/home/larzuk/GIT/WHackDeck/WHackDeck/fw/whackdeck/build/bootloader-prefix/src"
  "/home/larzuk/GIT/WHackDeck/WHackDeck/fw/whackdeck/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/larzuk/GIT/WHackDeck/WHackDeck/fw/whackdeck/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/larzuk/GIT/WHackDeck/WHackDeck/fw/whackdeck/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
