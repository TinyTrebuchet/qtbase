#ifndef QCPDBSUPPORT_H
#define QCPDBSUPPORT_H

#include <QtPrintSupport/private/qtprintsupportglobal_p.h>
#include <QtPrintSupport/private/qprint_p.h>
#include "QtCore/qstring.h"
#include "QtCore/qstringlist.h"
#include "QtPrintSupport/qprinter.h"
#include "QtCore/qdatetime.h"

QT_REQUIRE_CONFIG(cpdb);

QT_BEGIN_NAMESPACE

class QPrintDevice;

// HACK! Define these here temporarily so they can be used in the dialogs
// without a circular reference to QCupsPrintEngine in the plugin.
// Move back to qcupsprintengine_p.h in the plugin once all usage
// removed from the dialogs.
#define PPK_CpdbOptions   QPrintEngine::PrintEnginePropertyKey(QPrintEngine::PPK_CustomBase + 0x10)

#define     PDPK_CpdbPrinterObj     QPrintDevice::PrintDevicePropertyKey(QPrintDevice::PDPK_CustomBase + 0x10)
#define     PDPK_CpdbOptions        QPrintDevice::PrintDevicePropertyKey(QPrintDevice::PDPK_CustomBase + 0x11)

class QCPDBSupport
{
public:
    // Enum for valid page set
    enum PageSet {
        AllPages = 0,
        OddPages,
        EvenPages,
        Invalid
    };

    static QCPDBSupport::PageSet getPageSet(QByteArray val);

    constexpr static qreal pointsMultiplier = 2.83464566929; // 1mm to points
};

Q_DECLARE_TYPEINFO(QCPDBSupport::PageSet, Q_PRIMITIVE_TYPE);

QT_END_NAMESPACE

QT_DECL_METATYPE_EXTERN_TAGGED(QCPDBSupport::PageSet, QCPDBSupport__PageSet, Q_PRINTSUPPORT_EXPORT)

#endif // QCPDBSUPPORT_H
