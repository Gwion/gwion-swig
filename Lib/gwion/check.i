%typemap(check) SWIG_TYPE*
%{exit(2);%};
