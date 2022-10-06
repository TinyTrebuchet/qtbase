#include "qcpdbprintdevice.h"

#include <qpa/qplatformprintplugin.h>

#include "qcpdbprintersupport_p.h"
#include "private/qcpdb_p.h"

#if QT_CONFIG(mimetype)
#include <QtCore/QMimeDatabase>
#endif

QCpdbPrintDevice::QCpdbPrintDevice(cpdb_printer_obj_t * const printerObj)
    :m_printerObj (printerObj)
{
    if (printerObj) {
        m_name = printerObj->name;
        m_id = printerObj->id;
        m_location = printerObj->location;
        m_makeAndModel = printerObj->make_and_model;

        m_supportsMultipleCopies = true;
        m_supportsCollateCopies = true;

        m_isRemote = false;

        m_havePageSizes = false;
        m_supportsCustomPageSizes = false;
    }
}

QCpdbPrintDevice::~QCpdbPrintDevice()
{
}

QString QCpdbPrintDevice::id() const
{
    return QString(m_printerObj->id);
}

QString QCpdbPrintDevice::name() const
{
    return QString(m_printerObj->name);
}

QString QCpdbPrintDevice::makeAndModel() const
{
    return QString(m_printerObj->make_and_model);
}

QString QCpdbPrintDevice::location() const
{
    return QString(m_printerObj->location);
}

bool QCpdbPrintDevice::isValid() const
{
    return (m_printerObj != nullptr);
}

bool QCpdbPrintDevice::isDefault() const
{
    QPlatformPrinterSupport *ps = QPlatformPrinterSupportPlugin::get();
    if (!ps)
        return false;

    return (ps->defaultPrintDeviceId() == m_printerObj->name);
}

bool QCpdbPrintDevice::isRemote() const
{
    return false;
}

QPrint::DeviceState QCpdbPrintDevice::state() const
{
    QString state = cpdbGetState(m_printerObj);

    if (state == "idle")
        return QPrint::Idle;
    else if (state == "processing")
        return QPrint::Active;
    else
        return QPrint::Error;
}

void QCpdbPrintDevice::loadPageSizes() const
{
    m_havePageSizes = true;

    cpdb_option_t *opt = cpdbGetOption(m_printerObj, "media");
    if (!opt) return;

    for (int i = 0; i < opt->num_supported; i++) {
        int width = 0, length = 0;
        cpdbGetMediaSize(m_printerObj, opt->supported_values[i], &width, &length);

        if (!width || !length) continue;
        // width & length are currently reported in 0.01mm
        width = width / 100.0 * QCPDBSupport::pointsMultiplier;
        length = length / 100.0 * QCPDBSupport::pointsMultiplier;

        QString key = opt->supported_values[i];
        QSize size = QSize(width, length);
        QString name = cpdbGetHumanReadableChoiceName(m_printerObj, "media", opt->supported_values[i]);

        if (strncmp(opt->supported_values[i], "custom_min", 10) == 0) {
            m_supportsCustomPageSizes = true;
            m_minimumPhysicalPageSize = size;
        } else if (strncmp(opt->supported_values[i], "custom_max", 10) == 0) {
            m_supportsCustomPageSizes = true;
            m_maximumPhysicalPageSize = size;
        } else {
            m_pageSizes << createPageSize(key, size, name);
        }
    }
}

QPageSize QCpdbPrintDevice::defaultPageSize() const
{
    char *pageSize = cpdbGetDefault(m_printerObj, "media");

    int width = 0, length = 0;
    cpdbGetMediaSize(m_printerObj, pageSize, &width, &length);

    width = width / 100.0 * QCPDBSupport::pointsMultiplier;
    length = length / 100.0 * QCPDBSupport::pointsMultiplier;

    if (width && length) {
        QString key = pageSize;
        QSize size = QSize(width, length);
        QString name = cpdbGetHumanReadableChoiceName(m_printerObj, "media", pageSize);
        return createPageSize(key, size, name);
    }

    return QPageSize();
}

QMarginsF QCpdbPrintDevice::printableMargins(const QPageSize &pageSize,
                                             QPageLayout::Orientation orientation,
                                             int resolution) const
{
    Q_UNUSED(orientation);
    Q_UNUSED(resolution);

    cpdb_margin_t *margins;
    char *media = pageSize.key().toLocal8Bit().data();
    int num_margins = cpdbGetMediaMargins(m_printerObj, media, &margins);

    if (num_margins) {
        return QMarginsF(margins[0].left, margins[0].top, margins[0].right, margins[0].bottom);
    }

    return QMarginsF();
}

void QCpdbPrintDevice::loadResolutions() const
{
    m_haveResolutions = true;

    cpdb_option_t *opt = cpdbGetOption(m_printerObj, "printer-resolution");
    if (!opt) return;

    for (int i = 0; i < opt->num_supported; i++) {
        QString resolution = opt->supported_values[i];
        resolution.chop(3);
        m_resolutions << resolution.toInt();
    }
}

int QCpdbPrintDevice::defaultResolution() const
{
    char *val = cpdbGetDefault(m_printerObj, "printer-resolution");

    if (val) {
        QString resolution = val;
        resolution.chop(3);
        return resolution.toInt();
    }

    return QPlatformPrintDevice::defaultResolution();
}

void QCpdbPrintDevice::loadDuplexModes() const
{
    m_haveDuplexModes = true;

    cpdb_option_t *opt = cpdbGetOption(m_printerObj, "sides");
    if (!opt) return;

    for (int i = 0; i < opt->num_supported; i++) {
        QByteArray duplexMode = opt->supported_values[i];
        if (duplexMode == "one-sided")
            m_duplexModes << QPrint::DuplexNone;
        else if (duplexMode == "two-sided-short-edge")
            m_duplexModes << QPrint::DuplexShortSide;
        else if (duplexMode == "two-sided-long-edge")
            m_duplexModes << QPrint::DuplexLongSide;
    }
}

QPrint::DuplexMode QCpdbPrintDevice::defaultDuplexMode() const
{
    char *val = cpdbGetDefault(m_printerObj, "sides");

    if (val) {
        QString duplexMode = cpdbGetDefault(m_printerObj, "sides");
        if (duplexMode == "two-sided-short-edge")
            return QPrint::DuplexShortSide;
        else if (duplexMode == "two-sided-long-edge")
            return QPrint::DuplexLongSide;
        else
            return QPrint::DuplexNone;
    }

    return QPlatformPrintDevice::defaultDuplexMode();
}

void QCpdbPrintDevice::loadColorModes() const
{
    m_haveColorModes = true;

    cpdb_option_t *opt = cpdbGetOption(m_printerObj, "print-color-mode");
    if (!opt) return;

    for (int i = 0; i < opt->num_supported; i++) {
        QString colorMode = opt->supported_values[i];
        if (colorMode == "monochrome")
            m_colorModes << QPrint::GrayScale;
        else if (colorMode == "color")
            m_colorModes << QPrint::Color;
    }
}

QPrint::ColorMode QCpdbPrintDevice::defaultColorMode() const
{
    char *val = cpdbGetDefault(m_printerObj, "print-color-mode");

    if (val) {
        QString colorMode = val;
        if (colorMode == "color")
            return QPrint::Color;
        else
            return QPrint::GrayScale;
    }

    return QPlatformPrintDevice::defaultColorMode();
}

void QCpdbPrintDevice::loadInputSlots() const
{
    m_haveInputSlots = true;

    cpdb_option_t *opt = cpdbGetOption(m_printerObj, "media-source");
    if (!opt) return;

    for (int i = 0; i < opt->num_supported; i++) {
        QPrint::InputSlot inputSlot;
        inputSlot.key = opt->supported_values[i];
        inputSlot.name = cpdbGetHumanReadableChoiceName(m_printerObj, "media-source", opt->supported_values[i]);
        inputSlot.id = QPrint::CustomInputSlot;
        inputSlot.windowsId = DMBIN_USER;
        m_inputSlots << inputSlot;
    }
}

QPrint::InputSlot QCpdbPrintDevice::defaultInputSlot() const
{
    char *val = cpdbGetDefault(m_printerObj, "media-source");

    if (val) {
        QPrint::InputSlot inputSlot;
        inputSlot.key = val;
        inputSlot.name = cpdbGetHumanReadableChoiceName(m_printerObj, "media-source", val);
        inputSlot.id = QPrint::CustomInputSlot;
        inputSlot.windowsId = DMBIN_USER;
        return inputSlot;
    }

    return QPlatformPrintDevice::defaultInputSlot();
}

void QCpdbPrintDevice::loadOutputBins() const
{
    m_haveOutputBins = true;

    cpdb_option_t *opt = cpdbGetOption(m_printerObj, "output-bin");
    if (!opt) return;

    for (int i = 0; i < opt->num_supported; i++) {
        QPrint::OutputBin outputBin;
        outputBin.key = opt->supported_values[i];
        outputBin.name = cpdbGetHumanReadableChoiceName(m_printerObj, "output-bin", opt->supported_values[i]);
        outputBin.id = QPrint::CustomOutputBin;
        m_outputBins << outputBin;
    }
}

QPrint::OutputBin QCpdbPrintDevice::defaultOutputBin() const
{
    char *val = cpdbGetDefault(m_printerObj, "output-bin");

    if (val) {
        QPrint::OutputBin outputBin;
        outputBin.key = val;
        outputBin.name = cpdbGetHumanReadableChoiceName(m_printerObj, "output-bin", val);
        outputBin.id = QPrint::CustomOutputBin;
        return outputBin;
    }

    return QPlatformPrintDevice::defaultOutputBin();
}

void QCpdbPrintDevice::loadMimeTypes() const
{
    QMimeDatabase db;
    m_mimeTypes.append(db.mimeTypeForName(QStringLiteral("application/pdf")));
    m_mimeTypes.append(db.mimeTypeForName(QStringLiteral("application/postscript")));
    m_mimeTypes.append(db.mimeTypeForName(QStringLiteral("image/gif")));
    m_mimeTypes.append(db.mimeTypeForName(QStringLiteral("image/png")));
    m_mimeTypes.append(db.mimeTypeForName(QStringLiteral("image/jpeg")));
    m_mimeTypes.append(db.mimeTypeForName(QStringLiteral("image/tiff")));
    m_mimeTypes.append(db.mimeTypeForName(QStringLiteral("text/html")));
    m_mimeTypes.append(db.mimeTypeForName(QStringLiteral("text/plain")));
    m_haveMimeTypes = true;
}

bool QCpdbPrintDevice::isFeatureAvailable(QPrintDevice::PrintDevicePropertyKey key, const QVariant &params) const
{
    Q_UNUSED(key);
    Q_UNUSED(params);

    return true;
}

QVariant QCpdbPrintDevice::property(QPrintDevice::PrintDevicePropertyKey key) const
{
    if (key == PDPK_CpdbPrinterObj) {
        return QVariant::fromValue<cpdb_printer_obj_t *>(m_printerObj);
    }
    return QPlatformPrintDevice::property(key);
}

bool QCpdbPrintDevice::setProperty(QPrintDevice::PrintDevicePropertyKey key, const QVariant &value)
{
    return QPlatformPrintDevice::setProperty(key, value);
}