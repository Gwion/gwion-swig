%define %nspc_ini(NSPC)
%feature("nspc", "NSPC");
%insert("init") %{
const Type t_NSPC = gwi_mk_type(gwi, (m_str)"NSPC", 0, NULL);
CHECK_BB(gwi_class_ini(gwi, t_NSPC, NULL, NULL))
%}
%enddef

%define %nspc_end()
%feature("nspc", "");
%insert("init") %{
CHECK_BB(gwi_class_end(gwi))
%}
%enddef
