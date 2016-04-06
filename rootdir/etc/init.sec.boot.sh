#!/system/bin/sh

echo "init.sec.boot.sh: start" > /dev/kmsg

# An "initcall" is the initialization function for a module which is statically
# linked into the Linux kernel. Running of initcalls can consume a large amount
# of time during bootup. However, some modules do not need to be initialized so
# early, in order for the embedded device to become usable for its primary
# purpose. These initcalls can be run later, after first boot.
#
# This will trigger the initcall of the bcmdhd wifi module.
cat /proc/deferred_initcalls
