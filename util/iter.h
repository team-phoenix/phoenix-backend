#pragma once

template< typename T >
class iter
{
public:
   class Iterator
   {
   public:
      Iterator( int value ) :
         mValue( value )
      {

      }

      T operator *( void ) const
      {
         return (T)mValue;
      }

      void operator ++( void )
      {
         ++mValue;
      }

      bool operator !=( Iterator rhs )
      {
         return mValue != rhs.mValue;
      }

   private:
      int mValue;
   };

};

template< typename T >
typename iter<T>::Iterator begin( iter<T> )
{
   return typename iter<T>::Iterator( static_cast<int>( T::First ) );
}

template< typename T >
typename iter<T>::Iterator end( iter<T> )
{
   return typename iter<T>::Iterator( ( static_cast<int>( T::Last ) ) + 1 );
}
