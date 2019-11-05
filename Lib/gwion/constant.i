%typemap(constant) int
%{ALLOC_PTR(gwi->gwion->mp, $symname_value, m_int, $1);%}

%typemap(constant) SWIGTYPE
%{ALLOC_PTR(gwi->gwion->mp, $symname_value, m_int, $1);%}

%typemap(constant) char
%{ALLOC_PTR(gwi->gwion->mp, $symname_value, m_int, '$1');%}

%typemap(constant) unsigned int, unsigned long int,
  const unsigned int, const unsigned long int
%{ALLOC_PTR(gwi->gwion->mp, $symname_value, m_uint, $1);%}

%typemap(constant) float, double, const float, const double
%{ALLOC_PTR(gwi->gwion->mp, $symname_value, m_float, $1);%}

%typemap(constant) char*, const char*
%{const M_Object $symname_value = new_object(gwi->gwion->mp, NULL, gwi->gwion->type[et_string]);
  STRING($symname_value) = s_name(insert_symbol(gwi->gwion->st, "$1"));%}

