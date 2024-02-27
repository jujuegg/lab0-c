# Demonstration of queue testing framework
# Use help command to see list of commands and options
# Initial queue is NULL.
show
# Create empty queue
new

# Fill it with some values.
it 0
it 1 2
it 3 3
it 2

# Another queue
new

it 4
it 6 2
it 5

# Merge
merge

# Delete queue.
free
# Exit program
quit
