
pkg_check_modules(CPDB_PKG cpdb-frontend)

qt_internal_add_plugin(QCpdbPrinterSupportPlugin
    OUTPUT_NAME cpdbprintersupport
    PLUGIN_TYPE printsupport
    SOURCES
        main.cpp
        qcpdbprintengine.cpp qcpdbprintengine_p.h
        qcpdbprintersupport.cpp qcpdbprintersupport_p.h
        qcpdbprintdevice.cpp qcpdbprintdevice.h
    INCLUDE_DIRECTORIES
        ../../../printsupport/kernel
        ${CPDB_PKG_INCLUDE_DIRS}
    LIBRARIES
        ${CPDB_PKG_LIBRARIES}
        Qt::Core
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
        Qt::PrintSupport
        Qt::PrintSupportPrivate
)
