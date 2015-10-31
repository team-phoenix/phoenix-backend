#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include "backendcommon.h"

#include "core.h"
#include "logging.h"

/*
 * CoreControl is a QML type that manages the execution of an emulation session via an instance of Core.
 *
 * Internally, CoreControl is a QML proxy for the Core. It manages Core's lifecycle and connects it to consumers,
 * keeping them in a separate thread separate from the UI (except VideoOutput).
 *
 * From the perspective of QML, the interface exposed is similar to that of a game console. See Core for more details.
 */

class CoreControl : public QObject {
        Q_OBJECT

        // Properties

    public:
        explicit CoreControl( QObject *parent = 0 );

    signals:
        void load();
        void play();
        void pause();
        void stop();
        void reset();
    public slots:
    private:
        Core *core;
        QThread *coreThread;
};

#endif // GAMEMANAGER_H
