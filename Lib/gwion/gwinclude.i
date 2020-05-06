%define %gwinclude(HEADER)
%insert("header") %{#include "HEADER"%}
%include HEADER
%enddef

#ifndef __cplusplus
%insert("header") %{#include "gwion_util.h"%}
%insert("header") %{#include "gwion_ast.h"%}
%insert("header") %{#include "gwion_env.h"%}
%insert("header") %{#include "vm.h"%}
%insert("header") %{#include "instr.h"%}
%insert("header") %{#include "object.h"%}
%insert("header") %{#include "gwion.h"%}
%insert("header") %{#include "operator.h"%}
%insert("header") %{#include "import.h"%}
%insert("header") %{#include "gwi.h"%}
#else
%insert("header") %{#include <stdlib.h>%}
%insert("header") %{#include "Gwion.hpp"%}
%insert("header") %{#include "gwi.h"%}
#endif
