# source

originally found using [actionmailer v1.2.6](https://github.com/rails/rails/tree/v1.2.6/actionmailer) on rbx 1.4.4:

```
irb(main):001:0> require "action_mailer"
irb(main):002:0> h = TMail::HeaderField.new('Sender', 'aamine@loveruby.net')
irb(main):003:0> h.addr
TypeError: parse_in() yielded Symbol (must be Array[2])
   from TMail::Scanner_R#scan_main at /export/web/cnu_gems/gems-2.3.0...
   <stacktrace>
```

# to reproduce using this repro

```ruby
chruby rbx-1.4.4
gem install "rake-compiler"
rake compile
ruby test.rb
```
# reason

* rbx-1.4.4 implements both `rb_iterate` and `rb_funcall`.
* rbx-1.4.4 includes racc with `Racc::Parser::Racc_Runtime_Core_Version_C = "1.4.12"`
* if `HAVE_RB_BLOCK_CALL` is defined, racc will [use `rb_block_call` instead of `rb_iterate`](https://github.com/tenderlove/racc/blob/0409b1df1dadc50e35e73c34214891e628f51707/ext/racc/cparse.c#L267-L273)
* (`rb_block_call` became the replacement for `rb_iterate` going from MRI 1.8 to MRI 1.9)
* tmail's scanner implementation as included in actionmailer 1.2.6 [yields multiple values](https://github.com/rails/rails/blob/v1.2.6/actionmailer/lib/action_mailer/vendor/tmail/scanner_r.rb#L150-L158) instead of yielding an array of multiple values
  * this works for racc on MRI 1.8.7/REE, which use `rb_iterate`, and gets the 2-valued array as returned by `rb_iterate`
  * this breaks racc on rbx-1.4.4, which uses `rb_block_call`, and gets just the first value as returned by `rb_block_call`
    * `rb_block_call` does return all other arguments as well, but they are contained in a different argument than what racc checks. see `ext/fun.c` for more details
