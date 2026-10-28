# The V821 BSP Kconfig uses BSP_TOP to locate the platform tree.  Its legacy
# NAND module makefile also expects KERNEL_SRC_DIR, which the original vendor
# build environment supplied implicitly. Buildroot builds in-tree, so the
# unpacked kernel directory is the correct replacement.
LINUX_MAKE_FLAGS += BSP_TOP=bsp/
LINUX_MAKE_FLAGS += KERNEL_SRC_DIR=$(LINUX_DIR)
