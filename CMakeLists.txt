cmake_minimum_required(VERSION 2.8)

project(qtpanel)
set (CMAKE_INSTALL_PREFIX "/usr")

# The version number
file (READ "${CMAKE_CURRENT_SOURCE_DIR}/date.txt" qtpanel_VERSION)
set (qtpanel_TRANSLATIONS_TARGET ${CMAKE_INSTALL_PREFIX}/share/qtpanel/translations)
set (qtpanel_IMAGES_TARGET ${CMAKE_INSTALL_PREFIX}/share/qtpanel/images)
set (qtpanel_SOUNDS_TARGET ${CMAKE_INSTALL_PREFIX}/share/qtpanel)
set (qtpanel_ORGANIZATION madfish)
set (qtpanel_APPLICATION qtpanel)
set (qtpanel_APPLETS_FILE_CONFIG applets.xml)

# uninstall target
configure_file (
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	IMMEDIATE @ONLY)
add_custom_target (uninstall
	COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

# configure a header file to pass of the CMake settings
configure_file (
	"${PROJECT_SOURCE_DIR}/config.h.in"
	"${PROJECT_BINARY_DIR}/config.h"
)

# add the binary tree to the search path for include files
include_directories ("${PROJECT_BINARY_DIR}")

find_package(X11 REQUIRED)
find_package(Qt4 REQUIRED)
set(QT_USE_PHONON TRUE)
#set(QT_USE_QTNETWORK TRUE)
set(QT_USE_QTSQL TRUE)
find_package(ALSA REQUIRED)

# includes Xxf86vm library
set(Xxf86vm_DIR ${CMAKE_SOURCE_DIR})
find_package(Xxf86vm REQUIRED)
include_directories(${Xxf86vm_INCLUDE_DIRS})
##

file (GLOB HEADER_FILES *.h *.hpp)
file (GLOB CPP_FILES *.cpp)
file (GLOB UI_FILES *.ui)
file (GLOB IMAGE_FILES images/*.png images/*.svg)
file (GLOB SOUND_FILES images/*.wav)

set ( FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${CPP_FILES} ${UI_FILES} ${HEADER_FILES} )

file (GLOB TRANSLATIONS_FILES translations/*.ts)

option (UPDATE_TRANSLATIONS "Update source translation translations/*.ts" ON)
if (UPDATE_TRANSLATIONS)
	qt4_create_translation(QM_FILES ${FILES_TO_TRANSLATE}
		${TRANSLATIONS_FILES}
	)
else (UPDATE_TRANSLATIONS)
	qt4_add_translation(QM_FILES ${TRANSLATIONS_FILES})
endif (UPDATE_TRANSLATIONS)

add_custom_target (translations_target DEPENDS ${QM_FILES})

set_directory_properties (PROPERTIES CLEAN_NO_CUSTOM true)

set(qtpanel_UIS
	panelapplicationsettings.ui
	appletlaunchersettings.ui
	appletbatterysettings.ui
	appletapplicationsmenusettings.ui
	appletclocksettings.ui
	appletdemosettings.ui
	appletvolumesettings.ui
	appletpagersettings.ui
	appletmemorysettings.ui
	appletbacklightsettings.ui
	appletsessionsettings.ui
	#appletmediasettings.ui
	dialogappletoptions.ui
	launchersettings.ui
	#mediaplayersettings.ui
	workinghours.ui
)
set(qtpanel_HEADERS
	dpisupport.h
	iconloader.h
	mydebug.h
	textgraphicsitem.h
	animationutils.h
	battery.h
	xmlconfigreader.h
	xmlconfigwriter.h
	xfitman.h
	atoms.h
	qxclient.h
	debug.h
	extensions.h
	utils.h
	workspace.h
	applets.h
	gtkrc.h
)
set(qtpanel_MOC_HEADERS
	x11support.h
	desktopapplications.h
	panelapplication.h
	panelwindow.h
	applet.h
	demoapplet.h
	launcherapplet.h
	spacerapplet.h
	clockapplet.h
	applicationsmenuapplet.h
	dockapplet.h
	trayapplet.h
	docklauncherapplet.h
	volumeapplet.h
	batteryapplet.h
	pagerapplet.h
	memoryapplet.h
	mediaapplet.h
	mediaplayer.h
	backlightapplet.h
	sessionapplet.h
	compmgr.h
	dialogappletoptions.h
	organizer.h
	calendarworkinghours.h
)
set(qtpanel_SOURCES
	dpisupport.cpp
	iconloader.cpp
	x11support.cpp
	desktopapplications.cpp
	main.cpp
	panelapplication.cpp
	panelwindow.cpp
	textgraphicsitem.cpp
	applet.cpp
	demoapplet.cpp
	launcherapplet.cpp
	spacerapplet.cpp
	clockapplet.cpp
	applicationsmenuapplet.cpp
	dockapplet.cpp
	docklauncherapplet.cpp
	trayapplet.cpp
	volumeapplet.cpp
	battery.cpp
	batteryapplet.cpp
	pagerapplet.cpp
	mediaapplet.cpp
	mediaplayer.cpp
	memoryapplet.cpp
	backlightapplet.cpp
	sessionapplet.cpp
	xmlconfigreader.cpp
	xmlconfigwriter.cpp
	xfitman.cpp
	atoms.cpp
	qxclient.cpp
	debug.cpp
	extensions.cpp
	workspace.cpp
	compmgr.cpp
	dialogappletoptions.cpp
	organizer.cpp
	calendarworkinghours.cpp
	gtkrc.cpp
)
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})
qt4_wrap_ui(qtpanel_UI_HEADERS ${qtpanel_UIS})
qt4_wrap_cpp(qtpanel_MOC_OUTPUT ${qtpanel_MOC_HEADERS})
include_directories(${CMAKE_BINARY_DIR})
add_executable(qtpanel ${qtpanel_SOURCES} ${qtpanel_MOC_OUTPUT} ${QM_FILES} ${qtpanel_HEADERS} ${qtpanel_UI_HEADERS})
target_link_libraries(qtpanel ${QT_LIBRARIES} ${PHONON} ${X11_LIBRARIES} ${X11_Xcomposite_LIB} ${X11_Xrender_LIB} ${X11_Xext_LIB} ${X11_Xfixes_LIB} ${X11_Xdamage_LIB} ${ALSA_LIBRARIES} ${Xxf86vm_LIBRARIES})

# add the install targets
install (PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/qtpanel DESTINATION bin)
install (FILES ${QM_FILES} DESTINATION ${qtpanel_TRANSLATIONS_TARGET})
install (FILES ${IMAGE_FILES} DESTINATION ${qtpanel_IMAGES_TARGET})
install (FILES ${SOUND_FILES} DESTINATION ${qtpanel_SOUNDS_TARGET})

# CPACK
set (CPACK_PACKAGE_CONTACT "Marcello Mortaro <mortaromarcello@gmail.com>")
set (CPACK_PACKAGE_DESCRIPTION "QtPanel")
set (CPACK_PACKAGE_NAME "qtpanel")
set (CPACK_PACKAGE_VERSION "${qtpanel_VERSION}")
file (READ "${CMAKE_CURRENT_SOURCE_DIR}/architecture.txt" ARCH)
set (CPACK_PACKAGE_FILE_NAME "qtpanel_${qtpanel_VERSION}-1_${ARCH}")
set (CPACK_GENERATOR "DEB")
set (CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${ARCH})
set (CPACK_DEBIAN_PACKAGE_DEPENDS "libqtgui4, phonon, libasound2")
set (CPACK_DEBIAN_PACKAGE_SECTION "x11")
set (CPACK_DEBIAN_PACKAGE_RECOMMENDS "openbox")
include (CPack)
#
