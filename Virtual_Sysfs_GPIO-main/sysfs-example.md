
# Linux Sysfs Driver 

# 1. Introduction

Sysfs is a virtual filesystem in Linux that exposes:

- Kernel objects
- Devices
- Drivers
- Bus information
- Runtime attributes

to user space through files and directories.

Sysfs is mounted at:

```bash id="q2m7vx"
/sys
````

It is one of the most important interfaces used in Linux device driver development.

Unlike procfs, sysfs is:

* Structured
* Device-oriented
* Stable
* Recommended for modern Linux drivers

---

# 2. What is a Sysfs Driver?

A Sysfs Driver is a Linux kernel module that creates entries inside:

```bash id="n6r8pk"
/sys
```

These entries allow:

* Reading kernel/device data
* Writing configuration values
* Runtime control of drivers

User space interacts using:

```bash id="b3v5zw"
cat
echo
```

commands.

---

# 3. Why Do We Use Sysfs?

Sysfs provides a clean interface between:

```text id="g4w7yp"
Kernel Space ↔ User Space
```

It is mainly used for:

| Usage                | Purpose                  |
| -------------------- | ------------------------ |
| Device configuration | Runtime settings         |
| Driver control       | Enable/disable features  |
| Monitoring           | View device status       |
| Hardware abstraction | Standard Linux interface |
| Debugging            | Device diagnostics       |

---

# 4. Real-Time Examples

| Sysfs Path      | Purpose         |
| --------------- | --------------- |
| /sys/class/gpio | GPIO control    |
| /sys/class/leds | LED control     |
| /sys/class/pwm  | PWM management  |
| /sys/block      | Block devices   |
| /sys/bus        | Bus information |

Driver examples:

* Sensor value reading
* LED brightness control
* GPIO direction setup
* Device enable/disable
* Runtime tuning

---

# 5. Sysfs Architecture

```text id="m7v2xq"
+-------------------------------+
| User Space Application        |
|-------------------------------|
| cat                           |
| echo                          |
+---------------+---------------+
                |
                v
+-------------------------------+
| Sysfs Virtual Filesystem      |
+---------------+---------------+
                |
                v
+-------------------------------+
| Kernel Object (kobject)       |
+---------------+---------------+
                |
                v
+-------------------------------+
| Sysfs Driver                  |
|-------------------------------|
| show()                        |
| store()                       |
+---------------+---------------+
                |
                v
+-------------------------------+
| Device / Kernel Data          |
+-------------------------------+
```

---

# 6. Sysfs vs Procfs

| Feature            | Sysfs        | Procfs              |
| ------------------ | ------------ | ------------------- |
| Purpose            | Device model | Process/kernel info |
| Structure          | Structured   | Free-form           |
| Recommended        | Yes          | Limited             |
| Device Integration | Excellent    | Weak                |

---

# 7. Important Sysfs Concepts

## kobject

Represents kernel object.

Sysfs is built around:

```c id="c4n7mt"
struct kobject
```

---

## Attribute

Represents sysfs file.

Each attribute has:

* show() → Read
* store() → Write

---

## kset

Collection of kobjects.

---

# 8. Important Header Files

```c id="v9m3qx"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/string.h>
```

---

# 9. Sysfs Driver Flow

## Step 1 – Module Loaded

Driver creates kobject.

---

## Step 2 – Sysfs File Created

Attribute file added.

---

## Step 3 – User Reads File

Kernel calls:

```c id="p8w1rv"
show()
```

---

## Step 4 – User Writes File

Kernel calls:

```c id="j6m4yx"
store()
```

---

## Step 5 – Module Removed

Sysfs files removed.

---

# 10. Full Sysfs Driver Example

## sysfs_driver.c

```c id="u5n8vp"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

static struct kobject *my_kobject;
static int value = 0;

static ssize_t value_show(struct kobject *kobj,
                          struct kobj_attribute *attr,
                          char *buf)
{
    return sprintf(buf, "%d\n", value);
}

static ssize_t value_store(struct kobject *kobj,
                           struct kobj_attribute *attr,
                           const char *buf,
                           size_t count)
{
    sscanf(buf, "%d", &value);

    printk(KERN_INFO "Sysfs Value = %d\n", value);

    return count;
}

static struct kobj_attribute value_attribute =
    __ATTR(value, 0664, value_show, value_store);

static int __init sysfs_driver_init(void)
{
    int ret;

    my_kobject = kobject_create_and_add("my_sysfs", kernel_kobj);

    if (!my_kobject)
        return -ENOMEM;

    ret = sysfs_create_file(my_kobject,
                            &value_attribute.attr);

    if (ret) {
        printk(KERN_ERR "Failed to create sysfs file\n");
        kobject_put(my_kobject);
        return ret;
    }

    printk(KERN_INFO "Sysfs Driver Loaded\n");

    return 0;
}

static void __exit sysfs_driver_exit(void)
{
    kobject_put(my_kobject);

    printk(KERN_INFO "Sysfs Driver Removed\n");
}

module_init(sysfs_driver_init);
module_exit(sysfs_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Simple Sysfs Driver");
```

---

# 11. Makefile

```Makefile id="w0m7xy"
obj-m += sysfs_driver.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean
```

---

# 12. Compile Driver

```bash id="h4x2qv"
make
```

Output:

```bash id="m6r9np"
sysfs_driver.ko
```

---

# 13. Insert Driver

```bash id="p5n8ry"
sudo insmod sysfs_driver.ko
```

---

# 14. Verify Sysfs Entry

```bash id="x1v7kw"
ls /sys/kernel/my_sysfs/
```

Expected:

```text id="q7m3wy"
value
```

---

# 15. Read Sysfs File

```bash id="n9v0px"
cat /sys/kernel/my_sysfs/value
```

Expected Output:

```text id="g8w6vr"
0
```

---

# 16. Write Sysfs File

```bash id="d2m5zx"
echo 100 > /sys/kernel/my_sysfs/value
```

---

# 17. Check Kernel Logs

```bash id="t5v9qy"
dmesg | tail
```

Expected:

```text id="y6r1np"
Sysfs Value = 100
```

---

# 18. Important Sysfs APIs

## kobject_create_and_add()

Creates sysfs directory.

Example:

```c id="q0m3vk"
kobject_create_and_add(name,
                       parent);
```

---

## sysfs_create_file()

Creates sysfs file.

Example:

```c id="n8v2qy"
sysfs_create_file(kobj,
                  &attr);
```

---

## kobject_put()

Releases kobject.

Example:

```c id="r7w5xp"
kobject_put(kobj);
```

---

# 19. __ATTR Macro

Creates sysfs attribute.

Syntax:

```c id="f3v9mz"
__ATTR(name,
       permission,
       show,
       store)
```

---

# 20. Sysfs Permissions

Example:

```text id="j0x8rw"
0664
```

Meaning:

| Value | Permission   |
| ----- | ------------ |
| 6     | Read + Write |
| 4     | Read Only    |

---

# 21. show() Function

Used when user reads sysfs file.

Example:

```c id="u2m7pn"
cat /sys/kernel/my_sysfs/value
```

calls:

```c id="x9r4wy"
value_show()
```

---

# 22. store() Function

Used when user writes sysfs file.

Example:

```bash id="m5v8qp"
echo 10 > value
```

calls:

```c id="n1w7rx"
value_store()
```

---

# 23. Advantages of Sysfs

| Advantage             | Description                  |
| --------------------- | ---------------------------- |
| Structured Interface  | Clean device representation  |
| Device-Oriented       | Integrated with driver model |
| Standard Linux Method | Widely used                  |
| Easy User Access      | cat/echo support             |
| Runtime Configuration | Dynamic control              |

---

# 24. Disadvantages of Sysfs

| Disadvantage          | Description                |
| --------------------- | -------------------------- |
| Text Only             | No binary support          |
| Limited Complexity    | Simple interfaces only     |
| Permission Management | Security considerations    |
| Not for Large Data    | Small attributes preferred |

---

# 25. Sysfs vs Procfs vs Debugfs

| Feature                 | Sysfs        | Procfs              | Debugfs           |
| ----------------------- | ------------ | ------------------- | ----------------- |
| Purpose                 | Device model | Process/kernel info | Debugging         |
| Stable ABI              | Yes          | Partial             | No                |
| Recommended for Drivers | Yes          | Limited             | Debug only        |
| Location                | /sys         | /proc               | /sys/kernel/debug |

---

# 26. Common Interview Questions

## Q1. What is Sysfs?

A virtual filesystem exposing kernel objects and device attributes to user space.

---

## Q2. Why Use Sysfs?

For:

* Runtime configuration
* Device management
* Monitoring

---

## Q3. Difference Between show() and store()?

| Function | Purpose         |
| -------- | --------------- |
| show()   | Read operation  |
| store()  | Write operation |

---

## Q4. What is kobject?

Core Linux kernel object used in sysfs hierarchy.

---

## Q5. Why is Sysfs Preferred Over Procfs?

Sysfs provides structured device representation.

---

# 27. Common Errors

## Error: Sysfs File Not Created

Cause:

* sysfs_create_file() failure

Fix:

* Verify return values

---

## Error: Permission Denied

Cause:

* Wrong attribute permissions

Fix:

* Verify mode bits

---

## Error: Kernel Crash

Cause:

* Invalid buffer handling

Fix:

* Validate inputs

---

# 28. Sysfs Debugging Techniques

## Check Sysfs Entries

```bash id="b6x2nr"
ls /sys/kernel/
```

---

## Read Sysfs File

```bash id="g5v9pw"
cat /sys/kernel/my_sysfs/value
```

---

## Kernel Logs

```bash id="m8r1qy"
dmesg | tail
```

---

# 29. Advanced Sysfs Topics

After learning basic sysfs drivers, move to:

* Device attributes
* Attribute groups
* Binary sysfs attributes
* Driver model integration
* Bus subsystem
* Power management
* Runtime PM

---

# 30. Sysfs Attribute Groups

Used to create multiple attributes together.

Example:

```c id="r9m5vx"
sysfs_create_group()
```

---

# 31. Best Practices

## Keep Sysfs Files Simple

One value per file.

---

## Validate User Input

Always validate:

```c id="t4w7my"
store()
```

input.

---

## Use Sysfs for Configuration

Preferred over procfs for modern drivers.

---

## Avoid Large Data

Sysfs is designed for small text attributes.

---

# 32. Real Hardware Platforms

Sysfs is heavily used on:

* Raspberry Pi 5
* BeagleBone Black
* NVIDIA Jetson Nano
* STM32MP157

---

# 33. Real Linux Sysfs Examples

| Path            | Purpose          |
| --------------- | ---------------- |
| /sys/class/gpio | GPIO control     |
| /sys/class/leds | LED management   |
| /sys/block      | Storage devices  |
| /sys/bus        | Bus subsystem    |
| /sys/devices    | Device hierarchy |

---

