#ifndef __common_nullablevalue_h__
#define __common_nullablevalue_h__

#include <string>

/**
 * A value which may contain an actual value or may be null
 *
 * This is more or less an implmentation of a Option/Maybe type.
 */
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

        T get() const
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

        bool isNull() const
        {
            return is_null;
        };
};


#endif // __common_nullablevalue_h__
