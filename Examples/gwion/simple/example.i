/* File : example.i */
%module example

%insert("header") %{
static OP_CHECK(opck_i2i) {
  Exp_Binary *bin = (Exp_Binary*)data;
  CHECK_OO(opck_const_rhs(env, bin, mut))
  bin->rhs->emit_var = 1;
  return bin->rhs->info->type;
}
%}

%insert(init) %{
  const Type t_i2i = gwi_mk_type(gwi, "i2i", SZ_INT, NULL);
  GWI_BB(gwi_add_type(gwi, t_i2i))
  GWI_BB(gwi_oper_ini(gwi, "i2i", "i2i", "i2i"))
  GWI_BB(gwi_oper_add(gwi, opck_i2i))
  GWI_BB(gwi_oper_end(gwi, "@=>", int_r_assign))
%}

%insert(init) %{
  ALLOC_PTR(gwi->gwion->mp, add_raw, m_uint, add);
  GWI_BB(gwi_item_ini(gwi, "i2i", "add_raw"))
  GWI_BB(gwi_item_end(gwi, ae_flag_none, add_raw))
  SET_FLAG(t_i2i, abstract);
%}

%nspc_ini(example)
%typemap(ffi) int (*)(int,int) "i2i";
%typemap(in) int (*)(int,int)  %{
  $1 = *(m_uint*)MEM($offset); // typemap(in)
%}

%inline %{
extern int    gcd(int x, int y);
extern double Foo;

int binary_op(int a, int b, int (*op)(int,int));
int add(int a, int b);
int sub(int a, int b);
int mul(int a,  int b);
%}

%nspc_end()
