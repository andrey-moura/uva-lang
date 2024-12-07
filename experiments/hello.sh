# Compare uva with python3 and ruby
# File to run: hello.py, hello.rb, hello.uva

# Number of times to run the program
N=50

# Run python3 and measure time
py_start=$(date +%s.%N)
for i in $(seq 1 $N); do python3 experiments/hello.py; done
py_end=$(date +%s.%N)
py_time=$(echo "$py_end - $py_start" | bc | sed 's/^\./0./')

# Run ruby and measure time
rb_start=$(date +%s.%N)
for i in $(seq 1 $N); do ruby experiments/hello.rb; done
rb_end=$(date +%s.%N)
rb_time=$(echo "$rb_end - $rb_start" | bc | sed 's/^\./0./')

# Run uva and measure time
uva_start=$(date +%s.%N)
for i in $(seq 1 $N); do uva experiments/hello.uva; done
uva_end=$(date +%s.%N)
uva_time=$(echo "$uva_end - $uva_start" | bc | sed 's/^\./0./')

printf "\n"

echo "Time to run $N times the hello program:"

printf "\n"

echo "Ruby: $rb_time"
echo "Python3: $py_time"
echo "Uva: $uva_time"