# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

add_library(widgets_customviewstyle_snippets OBJECT
    ../customstyle/customstyle.h
    ../customstyle/customstyle.cpp
    customviewstyle.cpp
)

target_link_libraries(widgets_customviewstyle_snippets PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
)
