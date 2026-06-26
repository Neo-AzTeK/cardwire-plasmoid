add_compile_definitions(TRANSLATION_DOMAIN="plasma_applet_dev.opengamingcollective.cardwire")

add_library(dev.opengamingcollective.cardwire MODULE
    CardwireApplet.cpp
    CardwireApplet.h
    DaemonController.cpp
    DaemonController.h
    CardwireGpu.cpp
    CardwireGpu.h
)

target_link_libraries(dev.opengamingcollective.cardwire
    Qt6::Gui
    Qt6::Qml
    Qt6::DBus
    Plasma::Plasma
    KF6::I18n
    KF6::Solid
)

install(TARGETS dev.opengamingcollective.cardwire DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/plasma/applets)

plasma_install_package(package dev.opengamingcollective.cardwire)
