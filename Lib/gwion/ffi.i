%typemap(ffi) void "void";
%typemap(ffi) char* ,const char*, char[ANY], const char[ANY], std::string, std::string const & "string";

%typemap(ffi) bool, unsigned int, const unsigned int    "int";
%typemap(ffi) char, signed char, unsigned char "int";
%typemap(ffi) short, unsigned short, const short, const unsigned short "int";
%typemap(ffi) short int, unsigned short int, const short int, const unsigned short int "int";
%typemap(ffi) long int, const long int    "int";
%typemap(ffi) long unsigned int, const long unsigned int    "int";
%typemap(ffi) intptr_t, uintptr_t, size_t, ssize_t, int16_t, uint16_t "int";
%typemap(ffi) float, const float "float";
%typemap(ffi) float*, const float* "float[]";
%typemap(ffi) double *, const double* "float[]";
%typemap(ffi) double **, const double** "float[][]";

%typemap(ffi) int* "int[]";

%typemap(ffi) int "int";
%typemap(ffi) double "float";

%typemap(ffi) SWIGTYPE * %{$*1_ltype%};


//%typemap(ffi) SWIGTYPE[] %{$*1_ltype[]%};
//%rename t_OdsPolarBin t_polarbin;
//%rename OdsPolarBin polar;
//%typemap(ffi) enum SWIGTYPE %{$*1_ltype%};
//%typemap(ffi) SWIGTYPE %{$1%};
