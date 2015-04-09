require "action_mailer"

h = TMail::HeaderField.new('Sender', 'aamine@loveruby.net')
puts h.addr
