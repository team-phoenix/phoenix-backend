#include "libretrovariable.h"

QDebug operator<<( QDebug debug, const LibretroVariable &var ) {

    // join a QVector of std::strings. (Really, C++ ?)
    auto &choices = var.choices();
    std::string joinedchoices;

    foreach( auto &choice, choices ) {
        joinedchoices.append( choice );

        if( &choice != &choices.last() ) {
            joinedchoices.append( ", " );
        }
    }

    auto QStr = QString::fromStdString; // shorter alias

    debug << qPrintable( QString( "Core::Variable(%1=%2, description=\"%3\", choices=[%4])" ).
                         arg( QStr( var.key() ) ).arg( QStr( var.value( "<not set>" ) ) ).
                         arg( QStr( var.description() ) ).arg( QStr( joinedchoices ) ) );
    return debug;

}


LibretroVariable::LibretroVariable() {}

LibretroVariable::LibretroVariable( const retro_variable *var ) {
    m_key = var->key;

    // "Text before first ';' is description. This ';' must be followed by a space,
    // and followed by a list of possible values split up with '|'."
    QString valdesc( var->value );
    int splitidx = valdesc.indexOf( "; " );

    if( splitidx != -1 ) {
        m_description = valdesc.mid( 0, splitidx ).toStdString();
        auto _choices = valdesc.mid( splitidx + 2 ).split( '|' );

        foreach( auto &choice, _choices ) {
            m_choices.append( choice.toStdString() );
        }
    } else {
        // unknown value
    }
}

LibretroVariable::~LibretroVariable() {}

const std::string &LibretroVariable::key() const {
    return m_key;
}

const std::string &LibretroVariable::value( const std::string &default_ ) const {
    if( m_value.empty() ) {
        return default_;
    }

    return m_value;
}

const std::string &LibretroVariable::value() const {
    static std::string default_( "" );
    return value( default_ );
}

const std::string &LibretroVariable::description() const {
    return m_description;
}

const QVector<std::string> &LibretroVariable::choices() const {
    return m_choices;
}

bool LibretroVariable::isValid() const {
    return !m_key.empty();
}
