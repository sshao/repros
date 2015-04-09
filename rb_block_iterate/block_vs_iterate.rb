require "lib/fun"

puts "when yielding:\n\tyield :CONST, :CONST2\nwith rb_funcall vs rb_block_call"
puts ""

Fun.iterate

puts ""

Fun.blockcall
