# source

originally found using actionmailer v1.3.6 on rbx 1.4.4:

```
irb(main):001:0> require "action_mailer"
irb(main):002:0> h = TMail::HeaderField.new('Sender', 'aamine@loveruby.net')
irb(main):003:0> h.addr
TypeError: parse_in() yielded Symbol (must be Array[2])
   from TMail::Scanner_R#scan_main at /export/web/cnu_gems/gems-2.3.0...
   <stacktrace>
```

to reproduce, run: `bundle exec ruby action_mailer_repro.rb`

# problem

TMail uses `racc`, and hits `yyparse` as an entry point for the parser. `yyparse` [expects the receiver to yield a token in the format of `[TOKEN-SYMBOL, VALUE]`](https://github.com/tenderlove/racc/blob/0409b1df1dadc50e35e73c34214891e628f51707/lib/racc/parser.rb#L323-L327)

the action\_mailer repro fails because it is only getting `TOKEN-SYMBOL` (of class Symbol) instead of the `[TOKEN-SYMBOL, VALUE]` array it expects.

this is because in rbx-1.4.4, `rb_block_call` is being called instead of `rb_iterate` (more on that below).

although `rb_block_call` "replaced" `rb_iterate` when MRI went from 1.8.7 &rarr; 1.9, `rb_block_call` does not function in exactly the same way that `rb_iterate` does.

# `rb_iterate` vs `rb_block_call`

this repro demonstrates the difference, and how it breaks compatibility.

```ruby
chruby rbx-1.4.4
bundle
rake compile
ruby block_vs_iterate.rb
```

result:

```
when yielding:
  yield :CONST, :CONST2
with rb_funcall vs rb_block_call

rb_iterate yields an array:
[:CONST, :CONST2]

rb_block_call yields the first argument:
:CONST
```

# explanation

* rbx-1.4.4 implements both `rb_iterate` and `rb_funcall`.
* rbx-1.4.4 includes racc with `Racc::Parser::Racc_Runtime_Core_Version_C == "1.4.12"`
* if `HAVE_RB_BLOCK_CALL` is defined, racc will [use `rb_block_call` instead of `rb_iterate`](https://github.com/tenderlove/racc/blob/0409b1df1dadc50e35e73c34214891e628f51707/ext/racc/cparse.c#L267-L273)
  * `rb_block_call` became the replacement for `rb_iterate` going from MRI 1.8 to MRI 1.9
* TMail's Scanner implementation as included in actionmailer 1.3.6 [yields multiple values](https://github.com/rails/rails/blob/v1.2.6/actionmailer/lib/action_mailer/vendor/tmail/scanner_r.rb#L150-L158) instead of yielding an *array* of multiple values

this isn't an issue on MRI 1.8.7/REE because:
* MRI 1.8.7/REE come with racc version 1.4.5, which doesn't have the `HAVE_RB_BLOCK_CALL` check
* so `rb_iterate` is used, which returns the 2-valued array as expected

on rbx-1.4.4:
* rbx-1.4.4 comes with racc version 1.4.12, which does have the `HAVE_RB_BLOCK_CALL` check
* rbx-1.4.4 does have `HAVE_RB_BLOCK_CALL` defined.
* so `rb_block_call` is used, which returns just the first argument, and causes an exception to be thrown.

side note: `rb_block_call` *does* return all other arguments as well, but they are contained in a different argument than what racc checks. see `ext/fun.c` for more details
