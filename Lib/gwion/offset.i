//%typemap(offset) float, double %{ offset += SZ_FLOAT; %}
//%typemap(offset) SWIGTYPE %{ offset += SZ_INT; %}
%typemap(offset) float, double %{+SZ_FLOAT%}
%typemap(offset) SWIGTYPE %{+SZ_INT%}
