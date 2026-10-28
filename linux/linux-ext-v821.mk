# The V821 BSP Kconfig uses BSP_TOP to locate the platform tree.  Its legacy
# GPU/NAND module makefiles also expect KERNEL_SRC_DIR, which Tina normally
# injects from its private build environment.  Buildroot builds in-tree, so
# the unpacked kernel directory is the correct replacement.
LINUX_MAKE_FLAGS += BSP_TOP=bsp/
LINUX_MAKE_FLAGS += KERNEL_SRC_DIR=$(LINUX_DIR)
