// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qcpdbjobwidget_p.h"

#include <QCheckBox>
#include <QDateTime>
#include <QFontDatabase>
#include <QLabel>
#include <QLayout>
#include <QTime>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QPrinter>
#include <QPrintEngine>

#include <kernel/qprintdevice_p.h>

QT_BEGIN_NAMESPACE

/*!
    \internal
    \class QCpdbJobWidget

    A widget to add to QPrintDialog to enable extra job options
    such as Job Scheduling, Job Priority or Job Billing
    \ingroup printing
    \inmodule QtPrintSupport
 */

QCpdbJobWidget::QCpdbJobWidget(QPrinter *printer, QPrintDevice *printDevice, QWidget *parent)
    : QWidget(parent),
      m_printer(printer)
{
    m_ui.setupUi(this);
    m_printerObj = qvariant_cast<cpdb_printer_obj_t *>(printDevice->property(PDPK_CpdbPrinterObj));

    // set all the default values
    initJobHold();
    initJobBilling();
    initJobPriority();
    initBannerPages();
}

QCpdbJobWidget::~QCpdbJobWidget()
{
}

void QCpdbJobWidget::setupPrinter()
{
    if (m_ui.startBannerPageCombo->isEnabled() && m_ui.endBannerPageCombo->isEnabled()) {
        QByteArray jobSheets = startBannerPage() + "," + endBannerPage();
        cpdbAddSettingToPrinter(m_printerObj, "job-sheets", jobSheets.constData());
    }

    if (m_ui.jobHoldComboBox->isEnabled()) {
        QByteArray jobHoldVal = jobHold();
        QByteArray jobHoldUntil = jobHoldVal;
        if (jobHoldVal == "specific") {
            QTime holdUntilTime = jobHoldTime();
            QDateTime localDateTime = QDateTime::currentDateTime();

            if (holdUntilTime < localDateTime.time()) 
                localDateTime = localDateTime.addDays(1);
            localDateTime.setTime(holdUntilTime);
            jobHoldUntil = localDateTime.toUTC().time().toString(u"HH:mm").toLocal8Bit();
        }
        cpdbAddSettingToPrinter(m_printerObj, "job-hold-until", jobHoldUntil.constData());
    }

    if (m_ui.jobPrioritySpinBox->isEnabled())
        cpdbAddSettingToPrinter(m_printerObj, "job-priority", QByteArray::number(jobPriority()).constData());
    if (m_ui.jobBillingLineEdit->isEnabled())
    cpdbAddSettingToPrinter(m_printerObj, "billing-info", jobBilling().toLatin1().constData());
}

void QCpdbJobWidget::initJobHold()
{
    cpdb_option_t *opt = cpdbGetOption(m_printerObj, "job-hold-until");

    if (opt && opt->num_supported > 0) {
        for (int i = 0; i < opt->num_supported; i++) {
            QByteArray val = opt->supported_values[i];
            QByteArray displayVal = cpdbGetHumanReadableChoiceName(m_printerObj, "job-hold-until", opt->supported_values[i]);
            m_ui.jobHoldComboBox->addItem(tr(displayVal), QVariant::fromValue(val));
        }

        QByteArray val = "specific";
        m_ui.jobHoldComboBox->addItem(tr("Specific Time"), QVariant::fromValue(val));

        QByteArray defaultVal = opt->default_value;
        int idx = m_ui.jobHoldComboBox->findData(QVariant::fromValue(defaultVal));
        if (idx >= 0) 
            m_ui.jobHoldComboBox->setCurrentIndex(idx);

    } else {
        m_ui.jobHoldComboBox->setEnabled(false);
    }

    toggleJobHoldTime();
    connect(m_ui.jobHoldComboBox, &QComboBox::currentIndexChanged, this, &QCpdbJobWidget::toggleJobHoldTime);
}

void QCpdbJobWidget::initJobBilling()
{
    setJobBilling();
}

void QCpdbJobWidget::initJobPriority()
{
    cpdb_option_t *opt = cpdbGetOption(m_printerObj, "job-priority");
    if (opt) {
        QByteArray defaultVal = opt->default_value;
        bool ok;
        int priority = defaultVal.toInt(&ok);
        if (ok && priority > 0)
            setJobPriority(priority);
        else
            setJobPriority(50);
    } else {
        m_ui.jobPrioritySpinBox->setEnabled(false);
    }
}

void QCpdbJobWidget::initBannerPages()
{
    cpdb_option_t *opt = cpdbGetOption(m_printerObj, "job-sheets");

    if (opt && opt->num_supported > 0) {
        for (int i = 0; i < opt->num_supported; i++) {
            QByteArray val = opt->supported_values[i];
            QByteArray displayVal = cpdbGetHumanReadableChoiceName(m_printerObj, "job-sheets", opt->supported_values[i]);
            m_ui.startBannerPageCombo->addItem(tr(displayVal), QVariant::fromValue(val));
            m_ui.endBannerPageCombo->addItem(tr(displayVal), QVariant::fromValue(val));
        }

        QByteArray defaultVal = opt->default_value;
        QList<QByteArray> defaultSheets = defaultVal.split(',');
        if (defaultSheets.size() >= 2) {
            QByteArray startSheet = defaultSheets[0];
            int startIndex = m_ui.startBannerPageCombo->findData(QVariant::fromValue(startSheet));
            m_ui.startBannerPageCombo->setCurrentIndex(startIndex);

            QByteArray endSheet = defaultSheets[1];
            int endIndex = m_ui.startBannerPageCombo->findData(QVariant::fromValue(endSheet));
            m_ui.endBannerPageCombo->setCurrentIndex(endIndex);
        }
    } else {
        m_ui.startBannerPageCombo->setEnabled(false);
        m_ui.endBannerPageCombo->setEnabled(false);
    }
}

QByteArray QCpdbJobWidget::jobHold() const
{
    return qvariant_cast<QByteArray>(m_ui.jobHoldComboBox->itemData(m_ui.jobHoldComboBox->currentIndex()));
}

QTime QCpdbJobWidget::jobHoldTime() const
{
    return m_ui.jobHoldTimeEdit->time();
}

QString QCpdbJobWidget::jobBilling() const
{
    return m_ui.jobBillingLineEdit->text();
}

int QCpdbJobWidget::jobPriority() const
{
    return m_ui.jobPrioritySpinBox->value();
}

QByteArray QCpdbJobWidget::startBannerPage() const
{
    return qvariant_cast<QByteArray>(m_ui.startBannerPageCombo->itemData(m_ui.startBannerPageCombo->currentIndex()));
}

QByteArray QCpdbJobWidget::endBannerPage() const
{
    return qvariant_cast<QByteArray>(m_ui.endBannerPageCombo->itemData(m_ui.endBannerPageCombo->currentIndex()));
}

void QCpdbJobWidget::setJobBilling(const QString &jobBilling)
{
    m_ui.jobBillingLineEdit->setText(jobBilling);
}

void QCpdbJobWidget::setJobPriority(int jobPriority)
{
    m_ui.jobPrioritySpinBox->setValue(jobPriority);
}

void QCpdbJobWidget::toggleJobHoldTime()
{
    if (jobHold() == "specific")
        m_ui.jobHoldTimeEdit->setEnabled(true);
    else
        m_ui.jobHoldTimeEdit->setEnabled(false);
}

QT_END_NAMESPACE

#include "moc_qcpdbjobwidget_p.cpp"
