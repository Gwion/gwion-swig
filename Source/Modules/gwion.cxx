#include "swigmod.h"

static const char *usage = "\
Gwion Options (available with -gwion)\n\
    [no additional options]\n\
\n";

class GWION : public Language {
  File *f_begin;
  File *f_runtime;
  File *f_header;
  File *f_types;
  File *f_wrappers;
  File *f_init;
  File *f_classInit;

  String *PrefixPlusUnderscore;
  String *gw_class_name;

  public:
  GWION() {
    f_begin = 0;
    f_runtime = 0;
    f_header = 0;
    f_types = 0;
    f_wrappers = 0;
    f_init = 0;
    f_classInit = 0;
    PrefixPlusUnderscore = 0;
    gw_class_name = NULL;
  }
  ~GWION() {
        Delete(gw_class_name);
        gw_class_name = NULL;
  }

  virtual void main(int argc, char *argv[]) {
    SWIG_library_directory("gwion");
    for (int i = 1; i < argc; i++) {
      if (argv[i]) {
        if (strcmp(argv[i], "-help") == 0)
          fputs(usage, stdout);
      }
    }
    Preprocessor_define("SWIGGWION 1", 0);
    SWIG_config_file("gwion.swg");
    SWIG_typemap_lang("gwion");
    allow_overloading();
  }

  virtual int top(Node *n) {
    String *outfile = Getattr(n, "outfile");
    String *swigmod = Getattr(n, "module");
    String *modname = Getattr(swigmod, "name");

    f_begin = NewFile(outfile, "w", SWIG_output_files());
    if (!f_begin) {
      FileErrorDisplay(outfile);
      SWIG_exit(EXIT_FAILURE);
    }
    f_runtime = NewString("");
    f_init = NewString("");
    f_classInit = NewString("");
    f_header = NewString("");
    f_types = NewString("");
    f_wrappers = NewString("");

    Swig_register_filebyname("header", f_header);
    Swig_register_filebyname("types", f_types);
    Swig_register_filebyname("wrapper", f_wrappers);
    Swig_register_filebyname("begin", f_begin);
    Swig_register_filebyname("runtime", f_runtime);
    Swig_register_filebyname("init", f_init);
    Swig_register_filebyname("classInit", f_classInit);

    Swig_name_register("wrapper",   "gw_%f");
    Swig_name_register("construct", "%n%c_ctor");
    Swig_name_register("destroy",   "%n%c_dtor");

    if(!CPlusPlus)
      Printf(f_init, "GWION_IMPORT(%s) {\n", modname);
    else
      Printf(f_init, "m_bool CPPIMPORT(Gwi gwi) {\n");
    Language::top(n);
    Printf(f_init, "return GW_OK;\n}\n");
    if(CPlusPlus)
      Printf(f_init, "extern \"C\" GWION_IMPORT(%s){ return CPPIMPORT(gwi);}\n", modname);

//    Dump(f_runtime, f_begin);
    Dump(f_header, f_begin);
    Dump(f_types, f_begin);
    Dump(f_wrappers, f_begin);
    Wrapper_pretty_print(f_init, f_begin);

    Delete(f_header);
    Delete(f_types);
    Delete(f_wrappers);
    Delete(f_init);
    Delete(f_classInit);
    Delete(f_runtime);
    Delete(f_begin);
    return SWIG_OK;
  }

  String *strip(const DOHconst_String_or_char_ptr name) {
    String *s = Copy(name);
    if(Strncmp(name, PrefixPlusUnderscore, Len(PrefixPlusUnderscore)) != 0)
      return s;
    Replaceall(s, PrefixPlusUnderscore, "");
    return s;
  }

  String *strip2(const DOHconst_String_or_char_ptr name) {
    String *s = Copy(name);
    if(!getCurrentClass())
      return s;
    String *prefix = NewStringf("%s::", Getattr(getCurrentClass(), "sym:name"));
    Replaceall(s, prefix, "");
    return s;
  }

  String* getWname(Node* n) {
    String *iname = Getattr(n, "sym:name");
    String *wname = Swig_name_wrapper(iname);

    if (Getattr(n, "sym:overloaded")) {
      String *overname = Getattr(n, "sym:overname");
      Replaceall(overname, "__SWIG_", "");
      Append(wname, overname);
      Delete(overname);
    } else if (!addSymbol(iname, n))
        return NULL;
    Setattr(n, "wrap:name", wname);
    return wname;
  }

  int import_args(String* target, Parm * p) {
    while(p) {
      const String*  name = Getattr(p, "sym:name");
      const String* lname = Getattr(p, "lname");
      const String*   arg = name ? name : lname;
      const String* tm = Swig_typemap_lookup("ffi", p, Swig_cresult_name(), 0);
      if(!tm) {
        const String* type = Getattr(p, "type");
        Swig_warning(WARN_NONE, input_file, line_number,
            "'ffi' undefined for argument %s %s\n",
            SwigType_str(type, 0), lname);
        return SWIG_ERROR;
      }
      Printf(target, "CHECK_BB(gwi_func_arg(gwi, \"%s\", \"%s\"))\n", tm, arg);
      p = nextSibling(p);
    }
    return SWIG_OK;
  }

  virtual int functionWrapper(Node *n) {
//return SWIG_OK;
    bool is_member = GetFlag(n, "ismember") != 0;
    bool is_setter = GetFlag(n, "memberset") != 0 || GetFlag(n, "varset") != 0;
    bool is_getter = GetFlag(n, "memberget") != 0 || GetFlag(n, "varget") != 0;
    String* member_attr = Getattr(n, "feature:gwmember");
    String *nodetype = Getattr(n, "nodeType");
    bool dtor_attr = !Cmp(nodetype, "destructor")  ||
      Getattr(n, "feature:gwdtor");
    bool ctor_attr = !Cmp(nodetype, "constructor") ||
      Getattr(n, "feature:gwctor");
    bool static_attr = Equal(Getattr(n, "storage"), "static");
    bool svar  = static_attr && (is_setter || is_getter);
    bool is_func = !(is_getter || is_getter) && !(ctor_attr || dtor_attr);
    String *name = Copy(Getattr(n, "name"));
    String *iname = Getattr(n, "sym:name");
    ParmList *l = Getattr(n, "parms");
    SwigType *type = Getattr(n, "type");
    String *wname = getWname(n);
    bool mvar = GetFlag(n, "memberset") || GetFlag(n, "memberget");
    bool mfunc = is_func && is_member && !static_attr;
    Node* interrest = is_setter ?
      mvar ? nextSibling(l) : l : n;

      String* ffi = Swig_typemap_lookup("ffi", n, Swig_cresult_name(), 0);
if(is_setter)
  ffi = Swig_typemap_lookup("ffi", interrest, "", 0);
    Parm *p;
    String *tm;
    Wrapper *f = NewWrapper();
//    if(!dtor_attr)
{
      emit_parameter_variables(l, f);
    emit_attach_parmmaps(l, f);
} //else puts("got dtor");
    Setattr(n, "wrap:parms", l);

    int num_arguments = emit_num_arguments(l);
    int varargs = emit_isvarargs(l);
    bool default_ctor = 0;

    if(ctor_attr) {
      if(!num_arguments) {
        if(Getattr(getCurrentClass(), "abstracts"))
          return SWIG_OK;
        Setattr(getCurrentClass(), "gwion:default_ctor", wname);
        default_ctor = 1;
      } else
        Setattr(getCurrentClass(), "gwion:has_ctor", wname);
    }

    if(dtor_attr){
      Printf(f->def, "static DTOR(%s) {\n", wname);
}    else if(default_ctor)
      Printf(f->def, "static CTOR(%s) {\n", wname);
    else if(mfunc || (member_attr != 0))
      Printf(f->def, "static MFUN(%s) {\n", wname);
    else
      Printf(f->def, "static SFUN(%s) {\n", wname);
/*
    if (dtor_attr) {
      Printf(f->code, "%s* arg1;\n", member_attr ? member_attr :
        Getattr(getCurrentClass(), "name"));
      l = nextSibling(l);
    }
*/
    String* offset = NewString("0");
if (!dtor_attr) {
Printf(f->code, "// emit arg in\n");
    for(p = member_attr ? nextSibling(l) : l; p;) {
//    for(p = l; p;) {
      while (checkAttribute(p, "tmap:in:numinputs", "0"))
        p = Getattr(p, "tmap:in:next");
      SwigType *arg_type = Getattr(p, "type");
      String   *arg_name = Getattr(p, "lname");
      /* Look for an input typemap */
      if ((tm = Getattr(p, "tmap:in"))) {
        Replaceall(tm, "$1", arg_name);
        Replaceall(tm, "$offset", offset);
        Printv(f->code, tm, "\n", NIL);
      } else {
        Swig_warning(WARN_TYPEMAP_IN_UNDEF, input_file, line_number,
          "in function '%s'\n"
          "\tUnable to use type %s as a function argument.\n",
          name, SwigType_str(arg_type, 0));
        return SWIG_ERROR;
      }
      Parm* next = nextSibling(p);
      if ((tm = Swig_typemap_lookup("offset", p, Swig_cresult_name(), 0)))
        Append(offset, tm);
      p = next;
    }
}
    Delete(offset);
    /* Check for trailing varargs */
    if (varargs) {
      if (p && (tm = Getattr(p, "tmap:in"))) {
        Replaceall(tm, "$input", "varargs");
        Printv(f->code, tm, "\n", NIL);
      }
    }
    /* Insert constraint checking code */
    for (p = l; p;) {
      if ((tm = Getattr(p, "tmap:check"))) {
        Replaceall(tm, "$target", Getattr(p, "name"));
        Printv(f->code, tm, "\n", NIL);
        p = Getattr(p, "tmap:check:next");
      } else {
        p = nextSibling(p);
      }
    }

Printf(f->code, "// emit free arg\n");
    /* Insert cleanup code */
    String *cleanup = NewString("");
    for (p = l; p;) {
      if ((tm = Getattr(p, "tmap:freearg"))) {
        Printv(cleanup, tm, "\n", NIL);
        p = Getattr(p, "tmap:freearg:next");
      } else {
        p = nextSibling(p);
      }
    }

Printf(f->code, "// emit arg out\n");
    /* Insert argument output code */
    String *outarg = NewString("");

    for (p = l; p;) {
      if ((tm = Getattr(p, "tmap:argout"))) {
        Replaceall(tm, "$target", "resultobj");
        Replaceall(tm, "$arg", Getattr(p, "emit:input"));
        Replaceall(tm, "$input", Getattr(p, "emit:input"));
        Printv(outarg, tm, "\n", NIL);
        p = Getattr(p, "tmap:argout:next");
      } else {
        p = nextSibling(p);
      }
    }

    String *actioncode = emit_action(n);

    if(is_getter && svar) {
          String* class_name = NewString(") ");
          if(getCurrentClass()) {
            Append(class_name, Getattr(getCurrentClass(), "name"));
            Append(class_name, "::");
          }
          Replaceall(actioncode, ")", class_name);
          Delete(class_name);
    }
/*
    if(is_setter && Cmp(Getattr(n, "storage"), "static")) {
      Printf(f->code, "result = arg%i;\n", is_member ? 2 : 1);
      //Replaceall(f->code, "RETURN = ;\n");
    }
*/
    if (default_ctor) {
      Clear(actioncode);
      if(!Getattr(getCurrentClass(), "abstracts")) {
        Printf(actioncode, "if(o->type_ref->xid == t_%s->xid)\n %s(o) = ",
        Getattr(getCurrentClass(), "sym:name"), Getattr(getCurrentClass(),
          "gwion:swig_getter"));
        if(CPlusPlus)
          Printf(actioncode, "new %s();\n",
            Getattr(getCurrentClass(), "name"));
        else
          Printf(actioncode, "(%s*)xcalloc(1, sizeof(%s));\n",
            Getattr(getCurrentClass(), "name"),
            Getattr(getCurrentClass(), "name"));
        }
    } else if (dtor_attr)
      Setattr(getCurrentClass(), "gwion:dtor", wname);
    else {

//if ((tm = Swig_typemap_lookup_out("out", is_setter ? interrest : n, Swig_cresult_name(), f, actioncode))) {
if ((tm = Swig_typemap_lookup_out("out", n, Swig_cresult_name(), f, actioncode))) {
        actioncode = 0;
        Replaceall(tm, "$result", "resultobj");
        if(getCurrentClass()) {
          String *class_sym = Getattr(getCurrentClass(), "sym:name");
          Replaceall(tm, "$ltype", class_sym);
        }
//        if (GetFlag(n, "feature:new"))
        Printf(f->code, "%s\n", tm);
      } else /* if(!is_setter) */ {
  Swig_warning(WARN_TYPEMAP_OUT_UNDEF, input_file, line_number,
              "Unable to use return type %s in function %s.\n",
              SwigType_str(type, 0), name);
          return SWIG_ERROR;
      }
    }
/*
    if (actioncode) {
      Append(f->code, actioncode);
      Delete(actioncode);
    }
*/
Printf(f->code, "// emit return variable\n");
//    emit_return_variable(n, is_setter ? Getattr(interrest, "type") : type, f);
    emit_return_variable(n, type, f);

    if(dtor_attr) {
      String *swig_getter = Getattr(getCurrentClass(), "gwion:swig_getter");
      Printf(f->code, "if(%s(o)) {\n", swig_getter);
      if(CPlusPlus)
        Printf(f->code, "delete"" (%s*)%s(o);\n",
            Getattr(getCurrentClass(), "name"), swig_getter);
      else
        Printf(f->code,"free(%s(o));\n", swig_getter);
      Printf(f->code, " %s(o) = NULL;\n}\n", swig_getter);
    }
    /* Output argument output code */
    Printv(f->code, outarg, NIL);

    /* Output cleanup code */
    Printv(f->code, cleanup, NIL);

    /* Look to see if there is any newfree cleanup code */
    if (GetFlag(n, "feature:new")) {
      if ((tm = Swig_typemap_lookup("newfree", n, Swig_cresult_name(), 0))) {
        Printf(f->code, "%s\n", tm);
      }
    }

    /* See if there is any return cleanup code */
//    if ((tm = Swig_typemap_lookup("ret", n, Swig_cresult_name(), 0))) {
//      Printf(f->code, "%s\n", tm);
//    }
    Printf(f->code, "}\n");
    Replaceall(f->code, "$cleanup", cleanup);
    Replaceall(f->code, "$symname", iname);
    Replaceall(f->code, "$result", "resultobj");

    /* Dump the function out */
    Wrapper_print(f, f_wrappers);

    /* Now register the function with the interpreter. */
    String *symname = strip2(Getattr(n, "name"));
    if(!dtor_attr && !default_ctor) {
      String *f_target = !getCurrentClass() ? f_init : f_classInit;
      Printf(f_target, "\nCHECK_BB(gwi_func_ini(gwi, \"%s\", \"%s\", %s))\n",
        ffi, symname, wname);
    Delete(symname);
    if(import_args(f_target, ((mvar || mfunc))? nextSibling(l) : l) == SWIG_ERROR)
      return SWIG_ERROR;
    const char* flag = ((is_member && !static_attr) || mfunc) &&
          !ctor_attr?
        "ae_flag_member" : (getCurrentClass() || Getattr(n, "feature:nspc"))?
        "ae_flag_static" : "(ae_flag)ae_flag_global";
      Printf(f_target, "CHECK_BB(gwi_func_end(gwi, %s))\n\n", flag);
    }
    Delete(cleanup);
    Delete(outarg);
    Delete(wname);
    DelWrapper(f);
    return SWIG_OK;
  }

  void enumitem(Node *n) {
    Swig_require("enumvalueDeclaration", n, "name", "value", NIL);
    String   *name  = Getattr(n, "name");
    SwigType *value  = Getattr(n, "value");
    Printf(f_init, "CHECK_BB(gwi_enum_add(gwi, (m_str)\"%s\", (m_uint)%s))\n", name, value);
  }

  virtual int enumDeclaration(Node *n) {
//    Swig_require("enumDeclaration", n, "type", NIL);
    String   *name  = Getattr(n, "name");
    SwigType *type  = Getattr(n, "type");
    Printf(f_init, "CHECK_BB(gwi_enum_ini(gwi, (m_str)\"%s\"))\n", name, type);
    Node *c;
    for (c = firstChild(n); c; c = nextSibling(c))
      enumitem(c);
    Printf(f_init, "CHECK_BB(gwi_enum_end(gwi))\n");
    return SWIG_OK;
  }

  virtual int constantWrapper(Node *n) {
//    Swig_save("constantWrapper", n);
    Swig_require("constantWrapper", n, "*sym:name", "type", "value", NIL);
    String *symname = Getattr(n, "sym:name");
    SwigType *type = Getattr(n, "type");
    String *value = Getattr(n, "value");
    String *tm = Swig_typemap_lookup("constant", n, value, 0);
    String *ffi  = Swig_typemap_lookup("ffi", n, value, 0);
    if(!ffi) {
      Swig_warning(WARN_TYPEMAP_CONST_UNDEF, input_file, line_number,
          "'ffi' undefined for constant value %s = %s\n",
          SwigType_str(type, 0), value);
      return SWIG_ERROR;
    }
    if(tm) {
      Replaceall(tm, "$symname", symname);
      Replaceall(tm, "$value", value);
      Printf(f_init, "%s\n", tm);
      String *sym = strip(symname);
      Printf(f_init, "CHECK_BB(gwi_item_ini(gwi, \"%s\", \"%s\"))\n", ffi, sym);
      Delete(sym);
      String *flag = (gw_class_name  || Getattr(n, "feature:nspc"))?
        NewString("ae_flag_const | ae_flag_static") :
        NewString("ae_flag_const");
      if(Getattr(n, "feature:callback"))
        Append(flag, " | ae_flag_func");
      Printf(f_init, "CHECK_BB(gwi_item_end(gwi, %s | ae_flag_builtin, "
          " (const m_uint*)%s_value))\n\n", flag, symname);
      Delete(flag);
    } else {
      Swig_warning(WARN_TYPEMAP_CONST_UNDEF, input_file, line_number,
          "Unsupported constant value %s = %s\n", SwigType_str(type, 0), value);
      return SWIG_ERROR;
    }
//    Swig_restore(n);
    return SWIG_OK;
  }

  virtual int classHandler(Node *n) {
    String* basename = NULL;
    String *symname = Getattr(n, "sym:name");
    if (!addSymbol(symname, n))
      return SWIG_ERROR;

    PrefixPlusUnderscore = NewStringf("%s_", getClassPrefix());
    Node * base_class = NULL;
    /* Handle inheritance */
    List *baselist = Getattr(n, "bases");
    if (baselist && Len(baselist) > 0) {
      Iterator base = First(baselist);
      while (base.item) {
        basename = Getattr(base.item, "name");
      Printf(f_types, "static Type t_%s;\n", symname);
      Printf(f_init,
          "t_%s = gwi_mk_type(gwi, (m_str)\"%s\", SZ_INT, \"%s\");\n",
          symname, symname, basename);
        base_class = base.item;
        base = Next(base);
      }
      Setattr(n, "base_class", base_class);
    } else {
      Printf(f_types, "static Type t_%s;\n", symname);
      Printf(f_init,
          "t_%s = gwi_mk_type(gwi, (m_str)\"%s\", SZ_INT, \"%s\");\n",
          symname, symname, Getattr(n, "feature:inherit") ?: "Object"); // TODO test mem
    }
    if(!baselist) {
      String *swig_getter = NewStringf("GW_%s", symname);
      Printf(f_wrappers, "static m_int o_%s_swig;\n", symname);
      Printf(f_wrappers, "#define GW_%s(a) *(void**)(a->data + o_%s_swig)\n",
      symname, symname);
      Setattr(n, "gwion:swig_getter", swig_getter);
    } else
      Setattr(n, "gwion:swig_getter", Getattr(base_class, "gwion:swig_getter"));

    Language::classHandler(n);

    String* default_ctor = Getattr(n, "gwion:default_ctor");
    bool is_abstract = (!default_ctor && Getattr(n, "gwion:has_ctor")) || GetFlag(n, "abstracts");
    String *dtor = is_abstract ? Getattr(n, "gwion:dtor") : NULL;
    Printf(f_init,
        "CHECK_BB(gwi_class_ini(gwi, t_%s, %s, %s));\n",
        symname, default_ctor ? default_ctor : "NULL", dtor ? dtor : "NULL");
    if(is_abstract || !default_ctor) {
      Printf(f_init, "SET_FLAG(t_%s, abstract);\n", symname);
//      Printf(f_init, "int %s_flag = t_%s->flag;", symname, symname);
//      Printf(f_init, "%s_flag |= ae_flag_abstract;\n", symname, symname);
//      Printf(f_init, "t_%s->flag = (ae_flag)%s_flag;\n", symname, symname);
    }

    if(!baselist)
      Printf(f_init,
"CHECK_BB(gwi_item_ini(gwi, \"int\", \"@Swig_%s_Object\"))\n"
          "CHECK_BB((o_%s_swig = gwi_item_end(gwi, ae_flag_member, NULL)))\n",
          symname, symname, symname, symname);

    /* Done, close the class and dump its definition to the init function */
    Dump(f_classInit, f_init);
    Clear(f_classInit);
    if(!GetFlag(n, "feature:noclassend")) {
      if(gw_class_name)
        Delete(gw_class_name);
      Printf(f_init, "CHECK_BB(gwi_class_end(gwi)); // %s\n", symname);
      gw_class_name = NewString(symname);
    }

    Delete(PrefixPlusUnderscore);
    PrefixPlusUnderscore = 0;
    return SWIG_OK;
  }

  };

  /* -----------------------------------------------------------------------------
   * swig_gwion()    - Instantiate module
   * ----------------------------------------------------------------------------- */

  static Language *new_swig_gwion() {
    return new GWION();
  }
  extern "C" Language *swig_gwion(void) {
    return new_swig_gwion();
  }
