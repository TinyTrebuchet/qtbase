#include "qcpdbprintengine_p.h"

#include <qpa/qplatformprintplugin.h>
#include <qpa/qplatformprintersupport.h>

#include <qiodevice.h>
#include <qfile.h>
#include <qdebug.h>
#include <qbuffer.h>
#include <QtGui/qpagelayout.h>

#include "private/qcore_unix_p.h" // overrides QT_OPEN

#include <private/qcpdb_p.h>

QT_BEGIN_NAMESPACE

extern QMarginsF qt_convertMargins(const QMarginsF &margins, QPageLayout::Unit fromUnits, QPageLayout::Unit toUnits);

QCpdbPrintEngine::QCpdbPrintEngine(QPrinter::PrinterMode m, const QString &deviceId)
    : QPdfPrintEngine(*new QCpdbPrintEnginePrivate(m))
{
    Q_D(QCpdbPrintEngine);
    d->changePrinter(deviceId);
    state = QPrinter::Idle;
}

QCpdbPrintEngine::~QCpdbPrintEngine()
{
}

void QCpdbPrintEngine::setProperty(PrintEnginePropertyKey key, const QVariant &value)
{
    Q_D(QCpdbPrintEngine);

    switch (int(key)) {
    case PPK_Duplex: {
        QPrint::DuplexMode mode = QPrint::DuplexMode(value.toInt());
        if (d->m_printDevice.supportedDuplexModes().contains(mode)) {
            d->duplex = mode;
            d->duplexRequestedExplicitly = true;
        }
        break;
    }
    case PPK_PrinterName:
        d->changePrinter(value.toString());
        break;
    default:
        QPdfPrintEngine::setProperty(key, value);
        break;
    }
}

QVariant QCpdbPrintEngine::property(PrintEnginePropertyKey key) const
{
    Q_D(const QCpdbPrintEngine);

    QVariant ret;
    switch (int(key)) {
    case PPK_SupportsMultipleCopies:
        ret = true;
        break;
    case PPK_NumberOfCopies:
        ret = 1;
        break;
    case PPK_Duplex:
        ret = d->duplex;
        break;
    default:
        ret = QPdfPrintEngine::property(key);
        break;
    }

    return ret;
}

QCpdbPrintEnginePrivate::QCpdbPrintEnginePrivate(QPrinter::PrinterMode m)
    : QPdfPrintEnginePrivate(m)
    , duplex(QPrint::DuplexNone)
{
}

QCpdbPrintEnginePrivate::~QCpdbPrintEnginePrivate()
{
}

bool QCpdbPrintEnginePrivate::openPrintDevice()
{
    if (outDevice)
        return false;

    if (!outputFileName.isEmpty()) {
        QFile *file = new QFile(outputFileName);
        if (!file->open(QFile::WriteOnly|QFile::Truncate)) {
            delete file;
            return false;
        }
        outDevice = file;
    } else {
        QString tempFile = "/tmp/cpdbTempFile";
        cpdbTempFile = tempFile;
        outDevice = new QFile(cpdbTempFile);
        static_cast<QFile *>(outDevice)->open(QIODevice::WriteOnly);
    }

    return true;
}

void QCpdbPrintEnginePrivate::closePrintDevice()
{
    QPdfPrintEnginePrivate::closePrintDevice();

    if (!cpdbTempFile.isEmpty()) {
        QString tempFile = cpdbTempFile;
        cpdbTempFile.clear();

        // Should never have got here without a printer, but check anyway
        if (printerName.isEmpty()) {
            qWarning("Could not determine printer to print to");
            QFile::remove(tempFile);
            return;
        }

        // Set up print options.
        QList<QPair<QByteArray, QByteArray> > options;

        options.append(QPair<QByteArray, QByteArray>("media", m_pageLayout.pageSize().key().toLocal8Bit()));

        if (copies > 1) {
            cpdbAddSettingToPrinter(m_printerObj, "copies", QString::number(copies).toLocal8Bit());
            if (collate)
                cpdbAddSettingToPrinter(m_printerObj, "multiple-document-handling", "separate-documents-collated-copies");
            else
                cpdbAddSettingToPrinter(m_printerObj, "multiple-document-handling", "separate-documents-uncollated-copies");
        }

        switch (pageOrder) {
        case QPrinter::FirstPageFirst:
            cpdbAddSettingToPrinter(m_printerObj, "page-delivery", "same-order");
            break;
        case QPrinter::LastPageFirst:
            cpdbAddSettingToPrinter(m_printerObj, "page-delivery", "reverse-order");
            break;
        }

        switch (duplex) {
        case QPrint::DuplexNone:
            cpdbAddSettingToPrinter(m_printerObj, "sides", "one-sided");
            break;
        case QPrint::DuplexAuto:
            if (m_pageLayout.orientation() == QPageLayout::Portrait)
                cpdbAddSettingToPrinter(m_printerObj, "sides", "two-sided-long-edge");
            else
                cpdbAddSettingToPrinter(m_printerObj, "sides", "two-sided-short-edge");
            break;
        case QPrint::DuplexLongSide:
            cpdbAddSettingToPrinter(m_printerObj, "sides", "two-sided-long-edge");
            break;
        case QPrint::DuplexShortSide:
            cpdbAddSettingToPrinter(m_printerObj, "sides", "two-sided-short-edge");
            break;
        }

        if (m_pageLayout.orientation() == QPageLayout::Landscape)
            cpdbAddSettingToPrinter(m_printerObj, "orientation-requested", "4");

        // Print the file
        cpdbPrintFile(m_printerObj, tempFile.toLocal8Bit().constData());

        QFile::remove(tempFile);
    }
}

void QCpdbPrintEnginePrivate::changePrinter(const QString &newPrinter)
{
    // Don't waste time if same printer name
    if (newPrinter == printerName)
        return;

    // Should never have reached here if no plugin available, but check just in case
    QPlatformPrinterSupport *ps = QPlatformPrinterSupportPlugin::get();
    if (!ps)
        return;

    // Try create the printer, only use it if it returns valid
    QPrintDevice printDevice = ps->createPrintDevice(newPrinter);
    if (!printDevice.isValid())
        return;

    m_printDevice.swap(printDevice);
    m_printerObj = qvariant_cast<cpdb_printer_obj_t *>(m_printDevice.property(PDPK_CpdbPrinterObj));
    printerName = m_printDevice.id();

    // in case a duplex value was explicitly set, check if new printer supports current value,
    // otherwise use device default
    if (!duplexRequestedExplicitly || !m_printDevice.supportedDuplexModes().contains(duplex)) {
        duplex = m_printDevice.defaultDuplexMode();
        duplexRequestedExplicitly = false;
    }

    QPrint::ColorMode colorMode = grayscale ? QPrint::GrayScale : QPrint::Color;
    if (!m_printDevice.supportedColorModes().contains(colorMode))
        grayscale = m_printDevice.defaultColorMode() == QPrint::GrayScale;

    // Get the equivalent page size for this printer as supported names may be different
    if (m_printDevice.supportedPageSize(m_pageLayout.pageSize()).isValid())
        setPageSize(m_pageLayout.pageSize());
    else
        setPageSize(QPageSize(m_pageLayout.pageSize().size(QPageSize::Point), QPageSize::Point));
}

void QCpdbPrintEnginePrivate::setPageSize(const QPageSize &pageSize)
{
    if (pageSize.isValid()) {
        // Find if the requested page size has a matching printer page size, if so use its defined name instead
        QPageSize printerPageSize = m_printDevice.supportedPageSize(pageSize);
        QPageSize usePageSize = printerPageSize.isValid() ? printerPageSize : pageSize;
        QMarginsF printable = m_printDevice.printableMargins(usePageSize, m_pageLayout.orientation(), resolution);
        m_pageLayout.setPageSize(usePageSize, qt_convertMargins(printable, QPageLayout::Point, m_pageLayout.units()));
    }
}

QT_END_NAMESPACE
