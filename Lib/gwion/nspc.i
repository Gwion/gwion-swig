%define %nspc_ini(NSPC)
%feature("nspc", "NSPC");
%insert("init")
%{CHECK_BB(gwi_struct_ini(gwi, (m_str)"NSPC"))%}
%enddef

%define %nspc_end()
%feature("nspc", "");
%insert("init")
%{CHECK_BB(gwi_class_end(gwi))%}
%enddef
