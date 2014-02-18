// Adapted from http://blog.inventic.eu/2012/08/qt-and-google-breakpad/
#ifndef CRASHHANDLER_H
#define CRASHHANDLER_H
#include <QtCore/QString>

class CrashHandlerPrivate;
class CrashHandler
{
public:
    static CrashHandler* instance();
void Init(const QString&  reportPath);
 
    void setReportCrashesToSystem(bool report);
    bool writeMinidump();
 
private:
    CrashHandler();
    ~CrashHandler();
    Q_DISABLE_COPY(CrashHandler)
    CrashHandlerPrivate* d;
};

#endif