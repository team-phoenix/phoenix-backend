#pragma once

// Automatically constructs a basic getter and setter for a class.
//
// "_class" ==> the name of the class you are using this in.
// "_type" ==> the specific class type you are trying to construct.
// "_member_var" ==> the name of the getter and setter.
//
// Member variables will have the "m_" prefix added to the "_member_var"'s name.
//
#define GET_SET( _class, _type, _member_var )   \
    private:                                    \
        _type m_##_member_var;                  \
                                                \
    public:                                     \
    const _type &_member_var() const {          \
        return m_##_member_var;                 \
    }                                           \
                                                \
    _class& _member_var( const _type &t_val ) { \
        m_##_member_var = t_val;                \
        return *this;                           \
    }                                           \
                                                \
    _class& _member_var( _type&& t_val ) {      \
        return _member_var( t_val );            \
    }


