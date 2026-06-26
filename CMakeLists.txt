cmake_minimum_required(VERSION 3.16)

project(cardwire-plasmoid)

set(QT_MIN_VERSION "6.4.0")
set(KF6_MIN_VERSION "6.0.0")

find_package(ECM 1.4.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS Config I18n Solid)
find_package(Plasma ${KF6_MIN_VERSION} REQUIRED COMPONENTS)
find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Qml Gui Core DBus)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)
include(FindGettext)

add_subdirectory(src)

install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/icons
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable
        FILES_MATCHING PATTERN "*.svg"
        )

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
