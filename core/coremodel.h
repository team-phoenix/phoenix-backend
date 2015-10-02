#ifndef COREMODEL_H
#define COREMODEL_H

#include <QtCore>

#include "libretro.h"
#include "libretrocore.h"

/*
 * CoreModel encapsulates a game session. Its states are defined in the class that instantiates it, CoreController.
 * It lives in the same thread as the cores, unlike CoreController.
 * By default, it will output audio to the system's default audio device and video to the given VideoOutput instance
 */

class CoreModel : public QObject {
        Q_OBJECT

    public:
        explicit CoreModel( QObject *parent = 0 );

    signals:

    public slots:
        void loadLibretro( QString core, QString game );

    private:
        LibretroCore *libretroCore;


};

#endif // COREMODEL_H
