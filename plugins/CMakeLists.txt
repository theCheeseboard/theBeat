add_subdirectory(InternetRadioPlugin)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

ENDIF()

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_subdirectory(CdrdaoPlugin)
    add_subdirectory(LinuxIntegration)
ENDIF()

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

ENDIF()