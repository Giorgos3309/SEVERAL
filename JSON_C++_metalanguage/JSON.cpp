#include "JSONlang.h"

PROGRAMM_BEGIN
JSON(NUM) = NUMBER(9)
JSON(STR) = STRING("JS1")
JSON(N1) = NULL
JSON(B1) = FALSE||TRUE
JSON(ar) = ARRAY[ NUMBER(3), STRING("S1"),STRING("S3") ]
JSON(obj) = OBJECT{
	KEY(key1) : STRING("heraklion") ,
	KEY(key5) : OBJECT{ KEY(key1.1) : STRING("athens") }, 
	KEY(key2) : ARRAY[STRING("hello")+STRING(" world")] + ARRAY[NUMBER(100)],
	KEY(key4) : NULL,
}

//JSON(obj2) = OBJECT{ KEY(key1.1) : STRING("athens") }
//JSON(B2) = obj["key5"]==obj2

SET NUM ASSIGN NUMBER(10)
SET ar[2]["KEY1"] ASSIGN STRING("NEW_STR")
//SET obj["key2"] APPEND STRING("APPEND"), OBJECT{KEY(KK):STRING("APPEND")}
//ERASE obj["key4"]

PRINT NUM
PRINT STR
PRINT N1
PRINT B1
//PRINT B2
PRINT ar
//PRINT obj
//PRINT SIZE_OF(obj)
//PRINT HAS_KEY(obj , key5)
//PRINT HAS_KEY(obj, key6)
PRINT TYPE_OF(ar)
; keep_window_open();
PROGRAMM_END
