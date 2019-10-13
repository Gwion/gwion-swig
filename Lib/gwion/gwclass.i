%define %gwclassini(TYPE)
%nodefaultctor TYPE;
%feature(noclassend) TYPE;
%feature("gwmember", "TYPE");
%enddef

%define %gwclassctor(TYPE, CTOR)
%feature("gwctor", "CTOR") TYPE;
%feature("gwctor", "TYPE") CTOR;
%feature("gwmember", "") CTOR;
%enddef

%define %gwclassend
%insert("init") %{ CHECK_BB(gwi_class_end(gwi))%}
%feature("gwmember", "");
%enddef
