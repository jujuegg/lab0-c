# Demonstration of queue testing framework
# Use help command to see list of commands and options
# Initial queue is NULL.
show
# Create empty queue
new
# See how long it is
size
# Fill it with some values.  First at the head
ih dolphin
ih bear
ih gerbil
# Now at the tail
it meerkat
it bear
# Reverse it
reverse
# See how long it is
size
# Delete queue.  Goes back to a NULL queue.
free

# Remove Head
rh
# Remove Tail
rt
# Create empty queue
new
# Remove Head
rh
# Remove Tail
rt
# Fill it with some values.
it 0
it 1
it 2
it 3
it 4

# Exit program
quit
