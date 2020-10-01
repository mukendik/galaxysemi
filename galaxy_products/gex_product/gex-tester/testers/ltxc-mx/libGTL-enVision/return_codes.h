#ifndef RETURN_CODES_h
#define RETURN_CODES_h


#define IS_FLOAT_ARRAY        (LTX_IS_ARRAY+LTX_IS_FLOAT)
#define IS_DOUBLE_ARRAY       (LTX_IS_ARRAY+LTX_IS_DOUBLE)
#define IS_INTEGER_ARRAY      (LTX_IS_ARRAY+LTX_IS_INTEGER)
#define IS_SHORT_ARRAY        (LTX_IS_ARRAY+LTX_IS_SHORT)
#define IS_WORD_ARRAY         (LTX_IS_ARRAY+LTX_IS_WORD)
#define IS_UNSIGNED_ARRAY     (LTX_IS_ARRAY+LTX_IS_UNSIGNED)

#define IS_FLOAT_POINTER      (LTX_IS_POINTER+LTX_IS_FLOAT)
#define IS_DOUBLE_POINTER     (LTX_IS_POINTER+LTX_IS_DOUBLE)
#define IS_INTEGER_POINTER    (LTX_IS_POINTER+LTX_IS_INTEGER)
#define IS_SHORT_POINTER      (LTX_IS_POINTER+LTX_IS_SHORT)
#define IS_WORD_POINTER       (LTX_IS_POINTER+LTX_IS_WORD)
#define IS_UNSIGNED_POINTER   (LTX_IS_POINTER+LTX_IS_UNSIGNED)

#define SCALAR_MASK           (LTX_TYPE_MASK+LTX_PTR_MASK)
#define TYPE_BITS             (LTX_IS_ARRAY+LTX_IS_POINTER)
#define FLOAT_SCALAR_BITS     (TYPE_BITS + LTX_IS_FLOAT)
#define INTEGER_SCALAR_BITS   (TYPE_BITS + LTX_IS_INTEGER)
#define IS_FLOAT_SCALAR       (LTX_IS_FLOAT)
#define IS_DOUBLE_SCALAR      (LTX_IS_DOUBLE)
#define IS_INTEGER_SCALAR     (LTX_IS_INTEGER)
#define IS_SHORT_SCALAR       (LTX_IS_SHORT)
#define IS_WORD_SCALAR        (LTX_IS_WORD)
#define IS_UNSIGNED_SCALAR    (LTX_IS_UNSIGNED)

#define IS_FLOAT_SCALAR       (LTX_IS_FLOAT)
#define IS_DOUBLE_SCALAR      (LTX_IS_DOUBLE)
#define IS_INTEGER_SCALAR     (LTX_IS_INTEGER)
#define IS_SHORT_SCALAR       (LTX_IS_SHORT)
#define IS_WORD_SCALAR        (LTX_IS_WORD)
#define IS_UNSIGNED_SCALAR    (LTX_IS_UNSIGNED)


#define NOTfloat_array     00
#define NOTfloat_scalar    01
#define NOTfloat_ptr       02

#define NOTinteger_array   03
#define NOTinteger_scalar  04
#define NOTinteger_ptr     05

#define NOTword_array      06
#define NOTword_scalar     07
#define NOTword_ptr        08

#define NOTshort_array     09
#define NOTshort_scalar    10
#define NOTshort_ptr       11

#define NOTdouble_array    12
#define NOTdouble_scalar   13
#define NOTdouble_ptr      14

#define NOTunsigned_array  15
#define NOTunsigned_scalar 16
#define NOTunsigned_ptr    17

#define SMALLarray_size    18
#define INVarray_size      19

#define NOTstring          20

#define PAR1         0
#define PAR2         1
#define PAR3         2
#define PAR4         3
#define PAR5         4
#define PAR6         5
#define PAR7         6
#define PAR8         7
#define PAR9         8
#define PAR10        9
#define PAR11       10
#define PAR12       11
#define PAR13       12
#define PAR14       13
#define PAR15       14
#define PAR16       15
#define PAR17       16
#define PAR18       17
#define PAR19       18
#define PAR20       19

#define PAR1E      100
#define PAR2E      200
#define PAR3E      300
#define PAR4E      400
#define PAR5E      500
#define PAR6E      600
#define PAR7E      700
#define PAR8E      800
#define PAR9E      900
#define PAR10E    1000
#define PAR11E    1100
#define PAR12E    1200
#define PAR13E    1300
#define PAR14E    1400
#define PAR15E    1500
#define PAR16E    1600
#define PAR17E    1700
#define PAR18E    1800
#define PAR19E    1900
#define PAR20E    2000



#endif
