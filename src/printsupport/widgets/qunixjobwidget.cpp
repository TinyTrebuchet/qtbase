// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qunixjobwidget_p.h"

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
    \class QUnixJobWidget

    A widget to add to QPrintDialog to enable extra job options
    such as Job Scheduling, Job Priority or Job Billing
    \ingroup printing
    \inmodule QtPrintSupport
 */

QUnixJobWidget::QUnixJobWidget(QPrinter *printer, QPrintDevice *printDevice, QWidget *parent)
    : QWidget(parent),
      m_printer(printer),
      m_printDevice(printDevice)
{
    m_ui.setupUi(this);

#if QT_CONFIG(cpdb)
    m_printerObj = qvariant_cast<cpdb_printer_obj_t *>(printDevice->property(PDPK_CpdbPrinterObj));
#endif

    // set all the default values
    initJobHold();
    initJobBilling();
    initJobPriority();
    initBannerPages();

#if QT_CONFIG(cups)
    updateSavedValues();
#endif
}

QUnixJobWidget::~QUnixJobWidget()
{
}

void QUnixJobWidget::updateSavedValues()
{
#if QT_CONFIG(cups)
    m_savedJobHoldWithTime = {jobHold(), jobHoldTime()};
    m_savedJobBilling = jobBilling();
    m_savedPriority = jobPriority();
    m_savedJobSheets = {startBannerPage(), endBannerPage()};
#endif
}

void QUnixJobWidget::revertToSavedValues()
{
#if QT_CONFIG(cups)
    setJobHold(m_savedJobHoldWithTime.jobHold, m_savedJobHoldWithTime.time);
    toggleJobHoldTime();

    setJobBilling(m_savedJobBilling);

    setJobPriority(m_savedPriority);

    setStartBannerPage(m_savedJobSheets.startBannerPage);
    setEndBannerPage(m_savedJobSheets.endBannerPage);
#endif
}

void QUnixJobWidget::setupPrinter()
{
#if QT_CONFIG(cpdb)
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
#elif QT_CONFIG(cups)
    QCUPSSupport::setJobHold(m_printer, jobHold(), jobHoldTime());
    QCUPSSupport::setJobBilling(m_printer, jobBilling());
    QCUPSSupport::setJobPriority(m_printer, jobPriority());
    QCUPSSupport::setBannerPages(m_printer, startBannerPage(), endBannerPage());
#endif
}

void QUnixJobWidget::initJobHold()
{
#if QT_CONFIG(cpdb)
    cpdb_option_t *opt = cpdbGetOption(m_printerObj, "job-hold-until");

    if (opt && opt->num_supported > 0) {
        for (int i = 0; i < opt->num_supported; i++) {
            QByteArray val = opt->supported_values[i];
            QByteArray displayName = cpdbGetHumanReadableChoiceName(m_printerObj, "job-hold-until", opt->supported_values[i]);
            m_ui.jobHoldComboBox->addItem(tr(displayName), QVariant::fromValue(val));
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
#elif QT_CONFIG(cups)
    m_ui.jobHoldComboBox->addItem(tr("Print Immediately"), QVariant::fromValue(QCUPSSupport::NoHold));
    m_ui.jobHoldComboBox->addItem(tr("Hold Indefinitely"), QVariant::fromValue(QCUPSSupport::Indefinite));
    m_ui.jobHoldComboBox->addItem(tr("Day (06:00 to 17:59)"), QVariant::fromValue(QCUPSSupport::DayTime));
    m_ui.jobHoldComboBox->addItem(tr("Night (18:00 to 05:59)"), QVariant::fromValue(QCUPSSupport::Night));
    m_ui.jobHoldComboBox->addItem(tr("Second Shift (16:00 to 23:59)"), QVariant::fromValue(QCUPSSupport::SecondShift));
    m_ui.jobHoldComboBox->addItem(tr("Third Shift (00:00 to 07:59)"), QVariant::fromValue(QCUPSSupport::ThirdShift));
    m_ui.jobHoldComboBox->addItem(tr("Weekend (Saturday to Sunday)"), QVariant::fromValue(QCUPSSupport::Weekend));
    m_ui.jobHoldComboBox->addItem(tr("Specific Time"), QVariant::fromValue(QCUPSSupport::SpecificTime));

    QCUPSSupport::JobHoldUntilWithTime jobHoldWithTime;

    if (m_printDevice)
    {
        const QString jobHoldUntilString = m_printDevice->property(PDPK_CupsJobHoldUntil).toString();
        jobHoldWithTime = QCUPSSupport::parseJobHoldUntil(jobHoldUntilString);
    }

    setJobHold(jobHoldWithTime.jobHold, jobHoldWithTime.time);
#endif

    toggleJobHoldTime();
    connect(m_ui.jobHoldComboBox, &QComboBox::currentIndexChanged, this, &QUnixJobWidget::toggleJobHoldTime);
}

void QUnixJobWidget::initJobBilling()
{
#if QT_CONFIG(cpdb)
    setJobBilling();
#elif QT_CONFIG(cups)
    QString jobBilling;
    if (m_printDevice)
        jobBilling = m_printDevice->property(PDPK_CupsJobBilling).toString();

    setJobBilling(jobBilling);
#endif
}

void QUnixJobWidget::initJobPriority()
{
#if QT_CONFIG(cpdb)
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
#elif QT_CONFIG(cups)
    int priority = -1;
    if (m_printDevice)
    {
        bool ok;
        priority = m_printDevice->property(PDPK_CupsJobPriority).toInt(&ok);
        if (!ok)
            priority = -1;
    }

    if (priority < 0 || priority > 100)
        priority = 50;

    setJobPriority(priority);
#endif
}

void QUnixJobWidget::initBannerPages()
{
#if QT_CONFIG(cpdb)
    cpdb_option_t *opt = cpdbGetOption(m_printerObj, "job-sheets");

    if (opt && opt->num_supported > 0) {
        for (int i = 0; i < opt->num_supported; i++) {
            QByteArray val = opt->supported_values[i];
            QByteArray displayName = cpdbGetHumanReadableChoiceName(m_printerObj, "job-sheets", opt->supported_values[i]);
            m_ui.startBannerPageCombo->addItem(tr(displayName), QVariant::fromValue(val));
            m_ui.endBannerPageCombo->addItem(tr(displayName), QVariant::fromValue(val));
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
#elif QT_CONFIG(cups)
    m_ui.startBannerPageCombo->addItem(tr("None", "CUPS Banner page"), QVariant::fromValue(QCUPSSupport::NoBanner));
    m_ui.startBannerPageCombo->addItem(tr("Standard", "CUPS Banner page"), QVariant::fromValue(QCUPSSupport::Standard));
    m_ui.startBannerPageCombo->addItem(tr("Unclassified", "CUPS Banner page"), QVariant::fromValue(QCUPSSupport::Unclassified));
    m_ui.startBannerPageCombo->addItem(tr("Confidential", "CUPS Banner page"), QVariant::fromValue(QCUPSSupport::Confidential));
    m_ui.startBannerPageCombo->addItem(tr("Classified", "CUPS Banner page"), QVariant::fromValue(QCUPSSupport::Classified));
    m_ui.startBannerPageCombo->addItem(tr("Secret", "CUPS Banner page"), QVariant::fromValue(QCUPSSupport::Secret));
    m_ui.startBannerPageCombo->addItem(tr("Top Secret", "CUPS Banner page"), QVariant::fromValue(QCUPSSupport::TopSecret));

    m_ui.endBannerPageCombo->addItem(tr("None", "CUPS Banner page"), QVariant::fromValue(QCUPSSupport::NoBanner));
    m_ui.endBannerPageCombo->addItem(tr("Standard", "CUPS Banner page"), QVariant::fromValue(QCUPSSupport::Standard));
    m_ui.endBannerPageCombo->addItem(tr("Unclassified", "CUPS Banner page"), QVariant::fromValue(QCUPSSupport::Unclassified));
    m_ui.endBannerPageCombo->addItem(tr("Confidential", "CUPS Banner page"), QVariant::fromValue(QCUPSSupport::Confidential));
    m_ui.endBannerPageCombo->addItem(tr("Classified", "CUPS Banner page"), QVariant::fromValue(QCUPSSupport::Classified));
    m_ui.endBannerPageCombo->addItem(tr("Secret", "CUPS Banner page"), QVariant::fromValue(QCUPSSupport::Secret));
    m_ui.endBannerPageCombo->addItem(tr("Top Secret", "CUPS Banner page"), QVariant::fromValue(QCUPSSupport::TopSecret));

    QCUPSSupport::JobSheets jobSheets;

    if (m_printDevice)
    {
        const QString jobSheetsString = m_printDevice->property(PDPK_CupsJobSheets).toString();
        jobSheets = QCUPSSupport::parseJobSheets(jobSheetsString);
    }

    setStartBannerPage(jobSheets.startBannerPage);
    setEndBannerPage(jobSheets.endBannerPage);
#endif
}

#if QT_CONFIG(cpdb)
QByteArray QUnixJobWidget::jobHold() const
{
    return qvariant_cast<QByteArray>(m_ui.jobHoldComboBox->itemData(m_ui.jobHoldComboBox->currentIndex()));
}
#elif QT_CONFIG(cups)
QCUPSSupport::JobHoldUntil QUnixJobWidget::jobHold() const
{
    return qvariant_cast<QCUPSSupport::JobHoldUntil>(m_ui.jobHoldComboBox->itemData(m_ui.jobHoldComboBox->currentIndex()));
}
#endif

QTime QUnixJobWidget::jobHoldTime() const
{
    return m_ui.jobHoldTimeEdit->time();
}

QString QUnixJobWidget::jobBilling() const
{
    return m_ui.jobBillingLineEdit->text();
}

int QUnixJobWidget::jobPriority() const
{
    return m_ui.jobPrioritySpinBox->value();
}

#if QT_CONFIG(cpdb)
QByteArray QUnixJobWidget::startBannerPage() const
{
    return qvariant_cast<QByteArray>(m_ui.startBannerPageCombo->itemData(m_ui.startBannerPageCombo->currentIndex()));
}

QByteArray QUnixJobWidget::endBannerPage() const
{
    return qvariant_cast<QByteArray>(m_ui.endBannerPageCombo->itemData(m_ui.endBannerPageCombo->currentIndex()));
}
#elif QT_CONFIG(cups)
QCUPSSupport::BannerPage QUnixJobWidget::startBannerPage() const
{
    return qvariant_cast<QCUPSSupport::BannerPage>(m_ui.startBannerPageCombo->itemData(m_ui.startBannerPageCombo->currentIndex()));
}

QCUPSSupport::BannerPage QUnixJobWidget::endBannerPage() const
{
    return qvariant_cast<QCUPSSupport::BannerPage>(m_ui.endBannerPageCombo->itemData(m_ui.endBannerPageCombo->currentIndex()));
}
#endif

void QUnixJobWidget::setJobBilling(const QString &jobBilling)
{
    m_ui.jobBillingLineEdit->setText(jobBilling);
}

void QUnixJobWidget::setJobPriority(int jobPriority)
{
    m_ui.jobPrioritySpinBox->setValue(jobPriority);
}

void QUnixJobWidget::toggleJobHoldTime()
{
#if QT_CONFIG(cpdb)
    if (jobHold() == "specific")
#elif QT_CONFIG(cups)
    if (jobHold() == QCUPSSupport::SpecificTime)
#endif
        m_ui.jobHoldTimeEdit->setEnabled(true);
    else
        m_ui.jobHoldTimeEdit->setEnabled(false);
}

#if QT_CONFIG(cups)
void QUnixJobWidget::setJobHold(QCUPSSupport::JobHoldUntil jobHold, QTime holdUntilTime)
{
    if (jobHold == QCUPSSupport::SpecificTime && holdUntilTime.isNull())
    {
        jobHold = QCUPSSupport::NoHold;
        toggleJobHoldTime();
    }
    m_ui.jobHoldComboBox->setCurrentIndex(m_ui.jobHoldComboBox->findData(QVariant::fromValue(jobHold)));
    m_ui.jobHoldTimeEdit->setTime(holdUntilTime);
}

void QUnixJobWidget::setStartBannerPage(const QCUPSSupport::BannerPage bannerPage)
{
    m_ui.startBannerPageCombo->setCurrentIndex(m_ui.startBannerPageCombo->findData(QVariant::fromValue(bannerPage)));
}

void QUnixJobWidget::setEndBannerPage(const QCUPSSupport::BannerPage bannerPage)
{
    m_ui.endBannerPageCombo->setCurrentIndex(m_ui.endBannerPageCombo->findData(QVariant::fromValue(bannerPage)));
}
#endif

QT_END_NAMESPACE

#include "moc_qunixjobwidget_p.cpp"
