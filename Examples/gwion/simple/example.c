/* File : example.c */
/* A global variable */
double Foo = 3.0;

/* Compute the greatest common divisor of positive integers */
int gcd(int x, int y) {
  int g;
  g = y;
  while (x > 0) {
    g = x;
    x = y % x;
    y = g;
  }
  return g;
}


int binary_op(int a, int b, int (*op)(int,int)) {
printf("[%s] %p\n", __func__, op);
  return op(a, b);
}
int add(int a, int b) { return a + b; }
int sub(int a, int b) { return a - b; }
int mul(int a,  int b) { return a * b; }
