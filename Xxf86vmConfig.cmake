# - Find the Xxf86vm include file and library
#

SET(Xxf86vm_INC_SEARCH_PATH
    /usr/X11R6/include
    /usr/local/include
    /usr/include/X11
    /usr/openwin/include
    /usr/openwin/share/include
    /opt/graphics/OpenGL/include
    /usr/include)

SET(Xxf86v_LIB_SEARCH_PATH
    /usr/X11R6/lib
    /usr/local/lib
    /usr/openwin/lib
    /usr/lib)


FIND_PATH(Xxf86vm_INCLUDE_DIR X11/extensions/xf86vmode.h
          ${Xxf86vm_INC_SEARCH_PATH})

FIND_LIBRARY(Xxf86vm_LIBRARIES NAMES Xxf86vm PATH ${Xxf86vm_LIB_SEARCH_PATH})

IF (Xxf86vm_INCLUDE_DIR AND Xxf86vm_LIBRARIES)
    SET(Xxf86vm_FOUND TRUE)
ENDIF (Xxf86vm_INCLUDE_DIR AND Xxf86vm_LIBRARIES)

IF (Xxf86vm_FOUND)
    INCLUDE(CheckLibraryExists)

    CHECK_LIBRARY_EXISTS(${Xxf86vm_LIBRARIES}
                         "XF86VidModeAddModeLine"
                         ${Xxf86vm_LIBRARIES}
                         Xxf86vm_HAS_CONFIG)

    IF (NOT Xxf86vm_HAS_CONFIG AND Xxf86vm_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could NOT find Xxf86vm")
    ENDIF (NOT Xxf86vm_HAS_CONFIG AND Xxf86vm_FIND_REQUIRED)
ENDIF (Xxf86vm_FOUND)

MARK_AS_ADVANCED(
    Xxf86vm_INCLUDE_DIR
    Xxf86vm_LIBRARIES
    )
