#include "qcpdb_p.h"

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

QT_IMPL_METATYPE_EXTERN_TAGGED(QCPDBSupport::PageSet, QCPDBSupport__PageSet)

QCPDBSupport::PageSet QCPDBSupport::getPageSet(QByteArray val)
{
    //if (strcmp(val, "all") == 0)
        //return QCPDBSupport::AllPages;
    //else if (strcmp(val, "even") == 0)
        //return QCPDBSupport::EvenPages;
    //else if (strcmp(val, "odd") == 0)
        //return QCPDBSupport::OddPages;
    //return QCPDBSupport::Invalid;
    
    if (val == "all") 
        return QCPDBSupport::AllPages;
    else if (val == "even") 
        return QCPDBSupport::EvenPages;
    else if (val == "odd")
        return QCPDBSupport::OddPages;
    else
        return QCPDBSupport::Invalid;
}

QT_END_NAMESPACE
