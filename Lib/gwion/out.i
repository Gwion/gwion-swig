%typemap(out) void %{/* this a a void function */ %}
%typemap(out) void* %{ *(m_int*)RETURN = (m_int)$1; %}

%typemap(out) int %{*(m_int*)RETURN = (m_int)$1;%}
%typemap(out) const int, short int, const short int, long int, const long int,
  unsigned int, unsigned short int, unsigned long int,
  char, signed char, unsigned char, size_t, bool,
  const unsigned int, const unsigned short int, const unsigned long int,
  uint16_t, uint32_t, intptr_t, uintptr_t, int16_t, uint16_t = int;

%typemap(out) float
%{*(m_float*)RETURN = (m_float)(float)$1;%}

%typemap(out) double
%{*(m_float*)RETURN = (m_float)(double)$1;%}

%typemap(out) int*, int[]
%{
const Type t_int1 = array_type(shred->info->vm->gwion->env, shred->info->vm->gwion->type[et_int], 1);
const M_Object ret_obj = new_array(shred->info->mp, t_int1, 0);
// copy data here
*(M_Object*)RETURN = ret_obj;
%}

%typemap(out) double*, double[]
%{
const Type t_float1 = array_type(shred->info->vm->gwion->env, shred->info->vm->gwion->type[et_float], 1);
const M_Object ret_obj = new_array(shred->info->mp, t_float1, 0);
// copy data here
*(M_Object*)RETURN = ret_obj;
%}

%typemap(out) double**, double[][]
%{
const Type t_float1 = array_type(shred->info->vm->gwion->env, shred->info->vm->gwion->type[et_float], 2);
const M_Object ret_obj = new_array(shred->info->mp, t_float1, 0);
// copy data here
*(M_Object*)RETURN = ret_obj;
%}

%typemap(out) char*, const char*, char[], const char[],
  char[ANY], const char[ANY]
%{*(M_Object*)RETURN = $1 ? new_string(shred->info->mp, shred, (m_str)$1) : NULL;%}

%typemap(out) std::string
%{*(M_Object*)RETURN = $1.data() ? new_string(shred->info->mp, shred, (m_str)$1.data()) : NULL;%}


%typemap(out) struct SWIGTYPE* %{
  *(m_int*)RETURN = (m_int)$1_obj;
   // test
%}

%typemap(out) SWIGTYPE* %{
  M_Object ret_obj = new_object(shred->info->mp, shred, t_$*ltype);
// change this to get correct offset
  *($ltype*)ret_obj->data = result;
  *(M_Object*)RETURN = ret_obj;
   // test!
%}

%typemap(out) enum SWIGTYPE %{
  *(m_int*)RETURN = (m_int)$1;
   // test
%}
