// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only


#ifndef QUNIXJOBWIDGET_P_H
#define QUNIXJOBWIDGET_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// to version without notice, or even be removed.
//
// We mean it.
//
//

#include <QtPrintSupport/private/qtprintsupportglobal_p.h>

#if QT_CONFIG(cpdb)
#include <cpdb/cpdb-frontend.h>
#include <private/qcpdb_p.h>
#endif

#if QT_CONFIG(cups)
#include <private/qcups_p.h>
#endif

QT_REQUIRE_CONFIG(unixjobwidget);

#include <ui_qunixjobwidget.h>

QT_BEGIN_NAMESPACE

class QString;
class QTime;
class QPrinter;
class QPrintDevice;

class QUnixJobWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QUnixJobWidget(QPrinter *printer, QPrintDevice *printDevice, QWidget *parent = nullptr);
    ~QUnixJobWidget();
    void setupPrinter();

    void updateSavedValues();
    void revertToSavedValues();

private Q_SLOTS:
    void toggleJobHoldTime();

private:

    QTime jobHoldTime() const;
    QString jobBilling() const;
    int jobPriority() const;

    void setJobBilling(const QString &jobBilling = QString());
    void setJobPriority(int priority = 50);

#if QT_CONFIG(cpdb)
    QByteArray startBannerPage() const;
    QByteArray endBannerPage() const;
    QByteArray jobHold() const;

#elif QT_CONFIG(cups)
    void setJobHold(QCUPSSupport::JobHoldUntil jobHold = QCUPSSupport::NoHold, QTime holdUntilTime = QTime());
    QCUPSSupport::JobHoldUntil jobHold() const;
    
    void setStartBannerPage(const QCUPSSupport::BannerPage bannerPage = QCUPSSupport::NoBanner);
    QCUPSSupport::BannerPage startBannerPage() const;

    void setEndBannerPage(const QCUPSSupport::BannerPage bannerPage = QCUPSSupport::NoBanner);
    QCUPSSupport::BannerPage endBannerPage() const;
#endif

    void initJobHold();
    void initJobBilling();
    void initJobPriority();
    void initBannerPages();

    QPrinter *m_printer;
    QPrintDevice *m_printDevice;
    Ui::QUnixJobWidget m_ui;

    QString m_savedJobBilling;
    int m_savedPriority;
#if QT_CONFIG(cpdb)
    cpdb_printer_obj_t *m_printerObj;
#endif
#if QT_CONFIG(cups)
    QCUPSSupport::JobHoldUntilWithTime m_savedJobHoldWithTime;
    QCUPSSupport::JobSheets m_savedJobSheets;
#endif

    Q_DISABLE_COPY_MOVE(QUnixJobWidget)
};

QT_END_NAMESPACE

#endif  // QUNIXJOBWIDGET_P_H
