#ifndef __common_nullablevalue_h__
#define __common_nullablevalue_h__

#include <string>

template <class T>
class NullableValue
{
    private:
        bool is_null;
        T value;

    public:
        NullableValue()
            :   is_null(true)
        {};

        NullableValue(T _value)
            :   is_null(false), value(_value)
        {};

        T & get()
        {
            return value;
        }

        void set(T _value)
        {
            value = _value;
        };

        void setIsNull(bool _is_null)
        {
            is_null = _is_null;
        };

        bool isNull()
        {
            return is_null;
        };
};


#endif // __common_nullablevalue_h__
