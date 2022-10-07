#include "qcpdbprintersupport_p.h"

#include "qcpdbprintengine_p.h"
#include "qcpdbprintdevice.h"

#include <QTime>
#include <QCoreApplication>

#include <QAbstractEventDispatcher>

QT_BEGIN_NAMESPACE

extern "C" {

static int addPrinterCallback(cpdb_printer_obj_t *printerObj)
{
    qDebug() << "Found printer: " << printerObj->name;
    return 0;
}

static int removePrinterCallback(cpdb_printer_obj_t *printerObj)
{
    qDebug() << "Lost printer: " << printerObj->name;
    return 0;
}

} // extern "C"

QCpdbPrinterSupport::QCpdbPrinterSupport()
    : QPlatformPrinterSupport()
{
    cpdb_event_callback add_cb = static_cast<cpdb_event_callback>(addPrinterCallback);
    cpdb_event_callback rem_cb = static_cast<cpdb_event_callback>(removePrinterCallback);

    char instanceName[] = "Qt";
    f = cpdbGetNewFrontendObj(instanceName, add_cb, rem_cb);
    cpdbConnectToDBus(f);
    cpdbIgnoreLastSavedSettings(f);

    // wait for backends to connect
    QTime dieTime = QTime::currentTime().addMSecs(1000);
    while (QTime::currentTime() < dieTime) 
        QCoreApplication::processEvents(QEventLoop::AllEvents | QEventLoop::WaitForMoreEvents, 200);

}

QCpdbPrinterSupport::~QCpdbPrinterSupport()
{
    cpdbDisconnectFromDBus(f);
}

QPrintEngine *QCpdbPrinterSupport::createNativePrintEngine(QPrinter::PrinterMode printerMode, const QString &deviceId)
{
    return new QCpdbPrintEngine(printerMode, (deviceId.isEmpty() ? defaultPrintDeviceId() : deviceId));
}

QPaintEngine *QCpdbPrinterSupport::createPaintEngine(QPrintEngine *engine, QPrinter::PrinterMode printerMode)
{
    Q_UNUSED(printerMode);
    return static_cast<QCpdbPrintEngine *>(engine);
}

QPrintDevice QCpdbPrinterSupport::createPrintDevice(const QString &id)
{
    GHashTableIter iter;
    gpointer key, value;

    cpdb_printer_obj_t *printerObj = nullptr;
    g_hash_table_iter_init(&iter, f->printer);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        cpdb_printer_obj_t *printerObjIter = static_cast<cpdb_printer_obj_t *>(value);
        if (id == printerObjIter->name) {
            printerObj = printerObjIter;
            break;
        }
    }

    return QPlatformPrinterSupport::createPrintDevice(new QCpdbPrintDevice(printerObj));
}

QStringList QCpdbPrinterSupport::availablePrintDeviceIds() const
{
    QStringList list;

    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init (&iter, f->printer);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        list << (static_cast<cpdb_printer_obj_t *>(value))->name;
    }

    return list;
}

QString QCpdbPrinterSupport::defaultPrintDeviceId() const
{
    cpdb_printer_obj_t *pObj = cpdbGetDefaultPrinter(f);
    if (pObj)
        return QString(pObj->name);
    return QString();
}

QT_END_NAMESPACE
