#ifndef QCPDBPRINTERSUPPORT_H
#define QCPDBPRINTERSUPPORT_H

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

#include <qpa/qplatformprintersupport.h>

QT_BEGIN_NAMESPACE

class QCpdbPrinterSupport : public QPlatformPrinterSupport
{
public:
    QCpdbPrinterSupport();
    ~QCpdbPrinterSupport();

    QPrintEngine *createNativePrintEngine(QPrinter::PrinterMode p, const QString &deviceId = QString()) override;
    QPaintEngine *createPaintEngine(QPrintEngine *printEngine, QPrinter::PrinterMode) override;

    QPrintDevice createPrintDevice(const QString &id) override;
    QStringList availablePrintDeviceIds() const override;
    QString defaultPrintDeviceId() const override;

    cpdb_frontend_obj_t *f;
    cpdb_printer_obj_t *m_defaultPrinter;

private:

};

QT_END_NAMESPACE

#endif // QCPDBPRINTERSUPPORT_H
