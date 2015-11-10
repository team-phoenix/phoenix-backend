#ifndef LIBRETROVARIABLE_H
#define LIBRETROVARIABLE_H

#include "backendcommon.h"

#include "libretro.h"

// Container class for a libretro core variable
class LibretroVariable {
    public:
        LibretroVariable(); // default constructor

        LibretroVariable( const retro_variable *var );

        LibretroVariable( const std::string key );

        virtual ~LibretroVariable();

        const std::string &key() const;

        const std::string &value( const std::string &default_ ) const;

        const std::string &value() const;

        const std::string &description() const;

        const QVector<std::string> &choices() const;

        bool setValue( std::string value );

        bool isValid() const;

    private:
        // use std::strings instead of QStrings, since the later store data as 16bit chars
        // while cores probably use ASCII/utf-8 internally..
        std::string m_key;
        std::string m_value; // XXX: value should not be modified from the UI while a retro_run() call happens
        std::string m_description;
        QVector<std::string> m_choices;

};

QDebug operator<<( QDebug debug, const LibretroVariable &var );

#endif // LIBRETROVARIABLE_H
