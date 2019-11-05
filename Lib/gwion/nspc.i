%define %nspc_ini(NSPC)
%feature("nspc", "NSPC");
%insert("init")
%{CHECK_BB(gwi_class_spe(gwi, "NSPC", 0))%}
%enddef

%define %nspc_end()
%feature("nspc", "");
%insert("init")
%{CHECK_BB(gwi_class_end(gwi))%}
%enddef
