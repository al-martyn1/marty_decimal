#pragma once

#define MARTY_IMPLEMENT_RELATIONAL_OPERATORS_BY_COMPARE( typeT2 )\
                                                              \
    bool operator<( const typeT2 v2 ) const                   \
    {                                                         \
        return compare(v2)<0;                                 \
    }                                                         \
                                                              \
    bool operator<=( const typeT2 v2 ) const                  \
    {                                                         \
        return compare(v2)<=0;                                \
    }                                                         \
                                                              \
    bool operator>( const typeT2 v2 ) const                   \
    {                                                         \
        return compare(v2)>0;                                 \
    }                                                         \
                                                              \
    bool operator>=( const typeT2 v2 ) const                  \
    {                                                         \
        return compare(v2)>=0;                                \
    }                                                         \
                                                              \
    bool operator==( const typeT2 v2 ) const                  \
    {                                                         \
        return compare(v2)==0;                                \
    }                                                         \
                                                              \
    bool operator!=( const typeT2 v2 ) const                  \
    {                                                         \
        return compare(v2)!=0;                                \
    }
