// These macros serve to make more sophisticated enums
// that support returning their names as strings and
// support a description string.  Note that 'assign'
// may be left empty to use the default value that C
// would give the enum.

// This is based on code by 'Suma' available here:
// https://stackoverflow.com/questions/147267/easy-way-to-use-variables-of-enum-types-as-string-in-c#202511

// expansion macro for enum value definition
#define ENUM_VALUE(name,assign,descrip) name assign,

// expansion macro for enum to string conversion
#define ENUM_CASE(name,assign,descrip) case name: return #name;

// expansion macro for enum to description conversion
#define ENUM_CASE_DESC(name,assign,descrip) case name: return descrip;

// expansion macro for string to enum conversion
#define ENUM_STRCMP(name,assign,descrip) if (!strcmp(str,#name)) return name;

/// declare the access function and define enum values
#define DECLARE_ENUM(EnumType,ENUM_DEF) \
  enum EnumType { \
    ENUM_DEF(ENUM_VALUE) \
  }; \
  const char* GetString(EnumType dummy); \
  const char* GetDescription(EnumType dummy); \
  EnumType Get##EnumType##Value(const char *string); \

/// define the access function names
#define DEFINE_ENUM(EnumType,ENUM_DEF) \
  const char* GetString(EnumType value) \
  { \
    switch(value) \
    { \
      ENUM_DEF(ENUM_CASE) \
      default: return ""; /* handle input error */ \
    } \
  } \
  const char* GetDescription(EnumType value) \
  { \
    switch(value) \
    { \
      ENUM_DEF(ENUM_CASE_DESC) \
      default: return ""; /* handle input error */ \
    } \
  } \
  EnumType Get##EnumType##Value(const char *str) \
  { \
    ENUM_DEF(ENUM_STRCMP) \
    return (EnumType)0; /* handle input error */ \
  } \
