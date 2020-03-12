# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Build libs from source
set( HsmComm_src_DIR ../../libHsmComm/Android )
set( HsmFirmwareReplace_src_DIR ../../libHsmFirmwareReplace/Android )

set( HsmComm_build_DIR build_HsmComm )
set( HsmFirmwareReplace_build_DIR build_HsmFirmwareReplace )

add_subdirectory( # Specifies the directory of the CMakeLists.txt file.
        ${HsmComm_src_DIR}

        # Specifies the directory for the build outputs.
        ${HsmComm_build_DIR} )

add_subdirectory( # Specifies the directory of the CMakeLists.txt file.
        ${HsmFirmwareReplace_src_DIR}

        # Specifies the directory for the build outputs.
        ${HsmFirmwareReplace_build_DIR} )

# import created libs
add_library( lHsmComm STATIC IMPORTED )
set_target_properties( lHsmComm PROPERTIES IMPORTED_LOCATION
        ${HsmComm_build_DIR}/${ANDROID_ABI}/libHsmComm.a )

add_library( lHsmFirmwareReplace STATIC IMPORTED )
set_target_properties( lHsmFirmwareReplace PROPERTIES IMPORTED_LOCATION
        ${HsmFirmwareReplace_build_DIR}/${ANDROID_ABI}/libHsmFirmwareReplace.a )

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
    hedc

    # Sets the library as a shared library.
    SHARED

    src/main/cpp/JniInterface.cpp
)

include_directories(
    src/main/cpp
    ../../../Include
    ../../../Include/Android
)


# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       hedc

                       HsmComm
                       HsmFirmwareReplace
                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )
