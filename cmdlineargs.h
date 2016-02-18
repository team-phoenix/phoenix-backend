#ifndef CMDLINEARGS_H
#define CMDLINEARGS_H

#include <QObject>

class CmdLineArgs : public QObject
{
    Q_OBJECT
    Q_PROPERTY( QString coreName READ coreName )
    Q_PROPERTY( QString gameName READ gameName )

public:
    explicit CmdLineArgs(QObject *parent = 0);

    static void checkArgs( int argc, char *argv[] );

    QString coreName() const
    {
        return qmlCore;
    }

    QString gameName() const
    {
        return qmlGame;
    }

    static char **argv;
    static int argc;

    static bool mCoreFound;
    static bool mGameFound;

    static QString qmlCore;
    static QString qmlGame;


public slots:
    bool coreFound() const {
        return CmdLineArgs::mCoreFound;
    }

    bool gameFound() const {
        return CmdLineArgs::mGameFound;
    }

private:

};

#endif // CMDLINEARGS_H
