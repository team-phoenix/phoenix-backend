#include "remappermodel.h"
#include "remapper.h"


RemapperModel::RemapperModel( QAbstractListModel *parent ) : QAbstractListModel( parent ) {

}

void RemapperModel::controllerAdded(QString GUID) {
    qDebug() << "REMAPPER MODEL " << GUID;
}

Remapper *RemapperModel::getRemapper() {
    return nullptr;
}

void RemapperModel::setRemapper(Remapper *t_remapper) {
    connect( t_remapper, &Remapper::controllerAdded, this, &RemapperModel::controllerAdded );
    connect( t_remapper, &Remapper::controllerRemoved, this, &RemapperModel::controllerRemoved );
    connect( t_remapper, &Remapper::buttonUpdate, this, &RemapperModel::buttonUpdate );
    connect( t_remapper, &Remapper::remapModeEnd, this, &RemapperModel::remapModeEnd );
}
