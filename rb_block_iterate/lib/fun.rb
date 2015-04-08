require File.expand_path("../fun/fun", __FILE__)

class Fun
  def self.yield_this
    yield :CONST, :CONST2
  end
end
