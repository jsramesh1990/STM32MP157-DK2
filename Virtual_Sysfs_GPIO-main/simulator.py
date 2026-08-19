#!/usr/bin/env python3
# simulator.py
# Simple simulator for /sys/class/gpio in a directory.
# Run: ./simulator.py --dir /tmp/virt_gpio
# It will create export/unexport and gpioN/value/direction files.
# Use other programs by setting ENV: VIRT_GPIO_ROOT=/tmp/virt_gpio
# The sysfs C program in this package will check the path and use it if present.

import os, argparse, time, threading

parser = argparse.ArgumentParser()
parser.add_argument('--dir', default='/tmp/virt_gpio', help='root dir for simulated /sys/class/gpio')
args = parser.parse_args()

root = args.dir
cls = os.path.join(root, 'class', 'gpio')
os.makedirs(cls, exist_ok=True)

export_path = os.path.join(cls, 'export')
unexport_path = os.path.join(cls, 'unexport')

# helpers
def write_export(val):
    try:
        n = int(val.strip())
    except:
        return
    gpio_dir = os.path.join(cls, f'gpio{n}')
    os.makedirs(gpio_dir, exist_ok=True)
    # create direction and value
    open(os.path.join(gpio_dir, 'direction'), 'w').write('in')
    open(os.path.join(gpio_dir, 'value'), 'w').write('0')

def write_unexport(val):
    try:
        n = int(val.strip())
    except:
        return
    gpio_dir = os.path.join(cls, f'gpio{n}')
    if os.path.exists(gpio_dir):
        for f in os.listdir(gpio_dir):
            os.remove(os.path.join(gpio_dir, f))
        os.rmdir(gpio_dir)

# create handlers by watching files
open(export_path, 'w').close()
open(unexport_path, 'w').close()

print("Simulator root:", root)
print("Write a pin number into export/unexport to create/remove a pin.")
print("You can also echo into gpioN/value and/or gpioN/direction.")

def loop():
    last_export = ''
    last_unexport = ''
    while True:
        try:
            with open(export_path, 'r') as f:
                data = f.read().strip()
            if data and data != last_export:
                write_export(data)
                last_export = data
            with open(unexport_path, 'r') as f:
                data2 = f.read().strip()
            if data2 and data2 != last_unexport:
                write_unexport(data2)
                last_unexport = data2
        except Exception as e:
            pass
        time.sleep(0.2)

# start a small REPL to interact
t = threading.Thread(target=loop, daemon=True)
t.start()

try:
    while True:
        cmd = input("sim> ")
        if cmd.strip() == '':
            continue
        if cmd.startswith('cat '):
            p = cmd.split(' ',1)[1]
            ppath = os.path.join(cls, p)
            if os.path.exists(ppath):
                print(open(ppath).read())
            else:
                print("no such path", ppath)
        elif cmd.startswith('echo '):
            # echo <val> > path
            parts = cmd.split('>')
            if len(parts) == 2:
                left = parts[0].strip()  # echo 12
                right = parts[1].strip() # gpioN/value
                val = left.split(' ',1)[1].strip()
                ppath = os.path.join(cls, right)
                # create parent if needed
                d = os.path.dirname(ppath)
                os.makedirs(d, exist_ok=True)
                open(ppath, 'w').write(val)
        elif cmd == 'ls':
            for rootp, dirs, files in os.walk(cls):
                for d in dirs:
                    print(os.path.join(rootp, d))
                for f in files:
                    print(os.path.join(rootp, f))
        elif cmd in ('quit','exit'):
            break
        else:
            print("commands: cat <path>, echo <val> > <relpath>, ls, quit")
except KeyboardInterrupt:
    pass

