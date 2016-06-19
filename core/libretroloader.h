#pragma once

#include <QObject>

#include "pipeline.h"

#include "libretrocore.h"

class LibretroLoader : public Node {
        Q_OBJECT

    public:
        LibretroLoader();

    public slots:
        void commandIn( Command command, QVariant data, qint64 timeStamp ) override;

    private:
        bool connectedToCore { false };
};
