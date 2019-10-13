%typemap(in) int
%{$1 = ($1_ltype)*(m_int*)MEM($offset); // in.i%}

%typemap(in) float, double
%{$1 = ($1_ltype)*(m_float*)MEM($offset);%}

%typemap(in) unsigned int, uint32_t, uint16_t size_t, long, bool, uint16_t, size_t = int;

%typemap(in) char* (M_Object temp) %{
  temp = *(M_Object*)MEM($offset);
  $1 = (char*)STRING(temp);
%}

%typemap(in) SWIGTYPE (M_Object temp) %{
  if(!((temp = *(M_Object*)MEM($offset)) &&
         ($1 = *($ltype*)(temp->data))))
    Except(shred, "NullPtrException");
%}
