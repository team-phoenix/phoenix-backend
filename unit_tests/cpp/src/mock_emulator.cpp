
#include "emulator.h"

#include <QDebug>

Emulator::Emulator( QObject *parent ) : QObject( parent )
{
    qDebug() << "I am a mock emulator!";
}

Emulator::~Emulator() {

}

void Emulator::runEmu() {

}

void Emulator::initEmu( const QString &t_corePath, const QString &t_gamePath, const QString &hwType ) {

}

void Emulator::shutdownEmu() {

}
void Emulator::restartEmu() {

}
void Emulator::killEmu() {

}
void Emulator::handleVariableUpdate( const QByteArray &t_key, const QByteArray &t_value ) {

}
