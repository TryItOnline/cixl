n = 1000000

task1 = Fiber.new {n.times {Fiber.yield}}
task2 = Fiber.new {n.times {Fiber.yield}}

t1 = Time.now
n.times { task1.resume task2.resume }
t2 = Time.now
delta = (t2 - t1) * 1000
puts "#{delta.to_i}"
