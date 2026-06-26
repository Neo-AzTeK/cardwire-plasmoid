add_compile_definitions(TRANSLATION_DOMAIN="plasma_applet_dev.neo-aztek.cardwire")

add_library(dev.neo-aztek.cardwire MODULE
    CardwireApplet.cpp
    CardwireApplet.h
    DaemonController.cpp
    DaemonController.h
    CardwireGpu.cpp
    CardwireGpu.h
)

target_link_libraries(dev.neo-aztek.cardwire
    Qt6::Gui
    Qt6::Qml
    Qt6::DBus
    Plasma::Plasma
    KF6::I18n
    KF6::Solid
)

install(TARGETS dev.neo-aztek.cardwire DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/plasma/applets)

plasma_install_package(package dev.neo-aztek.cardwire)
