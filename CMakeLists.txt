PROJECT(thermocam)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)
set(CMAKE_BUILD_TYPE Debug)


## Check which architecture we have
if(“${CMAKE_SYSTEM_PROCESSOR}” STREQUAL “x86_64”)
  set(MYARCH "x86_64")
else(“${CMAKE_SYSTEM_PROCESSOR}” STREQUAL “x86_64”)
  set(MYARCH "i386")
endif(“${CMAKE_SYSTEM_PROCESSOR}” STREQUAL “x86_64”)


find_package(OpenCV REQUIRED)

###########
## Build ##
###########

add_definitions(-D_UNIX_)

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(
  include
  /opt/pleora/ebus_sdk/RHEL-6-i686/include
  ${pleora_INCLUDE_DIRS}
)

# find Pleora Libraries
LINK_DIRECTORIES(/opt/pleora/ebus_sdk/RHEL-6-i686/lib)
if(“${MYARCH}” STREQUAL “x86_64”)
  LINK_DIRECTORIES(/opt/pleora/ebus_sdk/RHEL-6-i686/lib/genicam/bin/Linux64_x64)
else(“${MYARCH}” STREQUAL “x86_64”)
  LINK_DIRECTORIES(/opt/pleora/ebus_sdk/RHEL-6-i686/lib/genicam/bin/Linux32_i86)
endif(“${MYARCH}” STREQUAL “x86_64”)

# set library collections for easier linking
set(EBUS_LIB
        PvBase
        PvDevice
        PvBuffer
        PvGUIUtils
        PvGUI
        PvPersistence
        PvGenICam
        PvStreamRaw
        PvStream
        PvTransmitterRaw
        PvVirtualDevice
        EbNetworkLib
        PtUtilsLib
        PtConvertersLib
        PvSerial
        EbTransportLayerLib
)

set(GENICAM_LIB
        GCBase_gcc40_v2_3
        GenApi_gcc40_v2_3
        log4cpp_gcc40_v2_3
        Log_gcc40_v2_3
        MathParser_gcc40_v2_3
)



## Add src and target
ADD_EXECUTABLE(Main thermocam.cpp utils.cpp main.cpp)

## Specify libraries to link a library or executable target against
target_link_libraries(Main 
  ${EBUS_LIB}
  ${GENICAM_LIB}
  ${OpenCV_LIBS}
)


INCLUDE_DIRECTORIES(/opt/pleora/ebus_sdk/RHEL-6-i686/include)
LINK_DIRECTORIES(/opt/pleora/ebus_sdk/RHEL-6-i686/lib)
LINK_DIRECTORIES(/opt/pleora/ebus_sdk/RHEL-6-i686/lib/genicam/bin/Linux32_i86)
