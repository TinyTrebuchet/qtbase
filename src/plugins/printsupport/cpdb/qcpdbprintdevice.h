#ifndef QCPDBPRINTDEVICE_H
#define QCPDBPRINTDEVICE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <cpdb/cpdb-frontend.h>

#include <qpa/qplatformprintdevice.h>

#include <QtCore/qmargins.h>

QT_BEGIN_NAMESPACE

class QCpdbPrintDevice : public QPlatformPrintDevice
{
public:
    explicit QCpdbPrintDevice(cpdb_printer_obj_t * const pObj);
    virtual ~QCpdbPrintDevice();

    QString id() const override;
    QString name() const override;
    QString location() const override;
    QString makeAndModel() const override;

    bool isValid() const override;
    bool isDefault() const override;
    bool isRemote() const override;

    QPrint::DeviceState state() const override;

    QPageSize defaultPageSize() const override;

    QMarginsF printableMargins(const QPageSize &pageSize, QPageLayout::Orientation orientation,
                               int resolution) const override;

    int defaultResolution() const override;

    QPrint::InputSlot defaultInputSlot() const override;

    QPrint::OutputBin defaultOutputBin() const override;

    QPrint::DuplexMode defaultDuplexMode() const override;

    QPrint::ColorMode defaultColorMode() const override;

    QVariant property(QPrintDevice::PrintDevicePropertyKey key) const override;
    bool setProperty(QPrintDevice::PrintDevicePropertyKey key, const QVariant &value) override;
    bool isFeatureAvailable(QPrintDevice::PrintDevicePropertyKey key, const QVariant &params) const override;

protected:
    void loadPageSizes() const override;
    void loadResolutions() const override;
    void loadInputSlots() const override;
    void loadOutputBins() const override;
    void loadDuplexModes() const override;
    void loadColorModes() const override;
#if QT_CONFIG(mimetype)
    void loadMimeTypes() const override;
#endif

private:
    cpdb_printer_obj_t *m_printerObj;
};

QT_END_NAMESPACE

#endif // QCPDBPRINTDEVICE_H
