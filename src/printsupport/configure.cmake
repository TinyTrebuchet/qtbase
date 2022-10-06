# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0



#### Inputs



#### Libraries

qt_find_package(Cups PROVIDED_TARGETS Cups::Cups MODULE_NAME printsupport QMAKE_LIB cups)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CPDB_PKG cpdb-frontend)



#### Tests



#### Features

qt_feature("cpdb" PUBLIC PRIVATE
    SECTION "PAINTING"
    LABEL "CPDB"
    PURPOSE "Provides Common Print Dialog Backend support"
    CONDITION CPDB_PKG_FOUND AND QT_FEATURE_printer AND QT_FEATURE_datestring
)
qt_feature_definition("cpdb" "QT_NO_CPDB" NEGATE VALUE "1")
qt_feature("cups" PUBLIC PRIVATE
    SECTION "Painting"
    LABEL "CUPS"
    PURPOSE "Provides support for the Common Unix Printing System."
    CONDITION Cups_FOUND AND QT_FEATURE_printer AND QT_FEATURE_datestring AND NOT CPDB_PKG_FOUND
)
qt_feature_definition("cups" "QT_NO_CUPS" NEGATE VALUE "1")
qt_feature("unixjobwidget" PUBLIC PRIVATE
    SECTION "Widgets"
    LABEL "UNIX job control widget"
    CONDITION ( QT_FEATURE_buttongroup ) AND ( QT_FEATURE_calendarwidget ) AND ( QT_FEATURE_checkbox ) AND ( QT_FEATURE_combobox ) AND ( QT_FEATURE_cups OR QT_FEATURE_cpdb ) AND ( QT_FEATURE_datetimeedit ) AND ( QT_FEATURE_groupbox ) AND ( QT_FEATURE_tablewidget )
)
qt_feature_definition("unixjobwidget" "QT_NO_UNIXJOBWIDGET" NEGATE VALUE "1")
qt_feature("printer" PUBLIC
    SECTION "Painting"
    LABEL "QPrinter"
    PURPOSE "Provides a printer backend of QPainter."
    CONDITION NOT UIKIT AND QT_FEATURE_picture AND QT_FEATURE_temporaryfile AND QT_FEATURE_pdf
)
qt_feature_definition("printer" "QT_NO_PRINTER" NEGATE VALUE "1")
qt_feature("printpreviewwidget" PUBLIC
    SECTION "Widgets"
    LABEL "QPrintPreviewWidget"
    PURPOSE "Provides a widget for previewing page layouts for printer output."
    CONDITION QT_FEATURE_graphicsview AND QT_FEATURE_printer AND QT_FEATURE_mainwindow
)
qt_feature_definition("printpreviewwidget" "QT_NO_PRINTPREVIEWWIDGET" NEGATE VALUE "1")
qt_feature("printdialog" PUBLIC
    SECTION "Dialogs"
    LABEL "QPrintDialog"
    PURPOSE "Provides a dialog widget for specifying printer configuration."
    CONDITION ( QT_FEATURE_buttongroup ) AND ( QT_FEATURE_checkbox ) AND ( QT_FEATURE_combobox ) AND ( QT_FEATURE_dialog ) AND ( QT_FEATURE_datetimeedit ) AND ( QT_FEATURE_dialogbuttonbox ) AND ( QT_FEATURE_printer ) AND ( QT_FEATURE_radiobutton ) AND ( QT_FEATURE_spinbox ) AND ( QT_FEATURE_tabwidget ) AND ( QT_FEATURE_treeview )
)
qt_feature_definition("printdialog" "QT_NO_PRINTDIALOG" NEGATE VALUE "1")
qt_feature("printpreviewdialog" PUBLIC
    SECTION "Dialogs"
    LABEL "QPrintPreviewDialog"
    PURPOSE "Provides a dialog for previewing and configuring page layouts for printer output."
    CONDITION QT_FEATURE_printpreviewwidget AND QT_FEATURE_printdialog AND QT_FEATURE_toolbar AND QT_FEATURE_formlayout
)
qt_feature_definition("printpreviewdialog" "QT_NO_PRINTPREVIEWDIALOG" NEGATE VALUE "1")
qt_configure_add_summary_section(NAME "Qt PrintSupport")
qt_configure_add_summary_entry(ARGS "cpdb")
qt_configure_add_summary_entry(ARGS "cups")
qt_configure_end_summary_section() # end of "Qt PrintSupport" section
