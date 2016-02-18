#include "cmdlineargs.h"
#include <QDebug>
#include <QFile>

int CmdLineArgs::argc = 0;
char **CmdLineArgs::argv = nullptr;

bool CmdLineArgs::mCoreFound = false;
bool CmdLineArgs::mGameFound = false;

QString CmdLineArgs::qmlCore = QStringLiteral( "" );
QString CmdLineArgs::qmlGame = QStringLiteral( "" );

CmdLineArgs::CmdLineArgs(QObject *parent)
    : QObject(parent)
{

}

void CmdLineArgs::checkArgs( int argc, char *argv[] ) {

    if ( argc > 1 ) {
        for ( int i=0; i < argc; ++i ) {
            QString arg = QString(argv[i]);
            if ( arg == QStringLiteral( "-c" ) && argc > 2 ) {
                CmdLineArgs::qmlCore = QString(argv[++i]);

                if ( ++i < argc ) {
                    CmdLineArgs::qmlGame = QString(argv[i]);
                }
            }
        }
    }


    if ( !qmlCore.isEmpty() ) {
        CmdLineArgs::mCoreFound = QFile::exists( CmdLineArgs::qmlCore );
    }

    if ( !qmlGame.isEmpty() ) {
        CmdLineArgs::mGameFound = QFile::exists( CmdLineArgs::qmlGame );
    }

}

