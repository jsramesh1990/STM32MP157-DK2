#!/bin/bash
set -e
SIMROOT=${VIRT_GPIO_ROOT:-/tmp/virt_gpio}
./simulator.py --dir "$SIMROOT" &
SIM_PID=$!
sleep 0.2

# use the sysfs program but point it to simulator path by using a wrapper
export SYSFS_ROOT="$SIMROOT/class/gpio"

# small wrapper function to write into simulator
sys_write() {
  # usage: sys_write export 17
  cmd=$1; pin=$2; val=$3
  if [ "$cmd" = "export" ]; then
    echo "$pin" > "$SYSFS_ROOT/export"
  elif [ "$cmd" = "unexport" ]; then
    echo "$pin" > "$SYSFS_ROOT/unexport"
  elif [ "$cmd" = "write" ]; then
    dir="$SYSFS_ROOT/gpio$pin"
    echo "out" > "$dir/direction"
    echo "$val" > "$dir/value"
  elif [ "$cmd" = "read" ]; then
    cat "$SYSFS_ROOT/gpio$pin/value"
  fi
}

echo "Export pin 17 via wrapper"
sys_write export 17
sleep 0.1

echo "Write 1 to pin 17"
sys_write write 17 1
echo "Value now: $(sys_write read 17)"

echo "Cleaning up"
sys_write unexport 17

kill $SIM_PID

