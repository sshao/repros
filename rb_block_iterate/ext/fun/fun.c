#include <ruby.h>

static VALUE yielder(VALUE data) {
  return rb_funcall(rb_const_get(rb_cObject, rb_intern("Fun")), rb_intern("yield_this"), 0);
}

static VALUE lexer_i(VALUE block_args, VALUE data) {
  rb_funcall(rb_mKernel, rb_intern("puts"), 1, rb_str_new2("rb_iterate yields an array:"));
  rb_funcall(rb_mKernel, rb_intern("p"), 1, block_args);
  return Qnil;
}

static VALUE my_iterate(void) {
  rb_iterate(yielder, Qnil, lexer_i, Qnil);
  return Qnil;
}

static VALUE lexer_2(VALUE block_args, VALUE data, int argc, VALUE argv[]) {
  rb_funcall(rb_mKernel, rb_intern("puts"), 1, rb_str_new2("rb_block_call yields the first argument:"));

  // if the method yielding yields more than one argument, "block_args"
  // only gets the first -- hence racc saying "got Symbol, need Array[2]"
  //
  // argc is the number of total arguments passed to the block
  // argv[] contains all the arguments passed to the block
  rb_funcall(rb_mKernel, rb_intern("p"), 1, block_args);
  return Qnil;
}

static VALUE my_blockcall(void) {
  rb_block_call(rb_const_get(rb_cObject, rb_intern("Fun")), rb_intern("yield_this"), 0, NULL, RUBY_METHOD_FUNC(lexer_2), Qnil);
  return Qnil;
}

void Init_fun(void) {
  VALUE cFun = rb_define_class("Fun", rb_cObject);

  rb_define_singleton_method(cFun, "iterate", my_iterate, 0);
  rb_define_singleton_method(cFun, "blockcall", my_blockcall, 0);
}
