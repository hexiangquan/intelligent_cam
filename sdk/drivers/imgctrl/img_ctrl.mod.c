#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x8b28cb9c, "module_layout" },
	{ 0x875c6704, "misc_register" },
	{ 0x67c2fa54, "__copy_to_user" },
	{ 0x6f00b2de, "mutex_unlock" },
	{ 0x69ba05c8, "mutex_lock_interruptible" },
	{ 0xfa2a45e, "__memzero" },
	{ 0xfbc74f64, "__copy_from_user" },
	{ 0xe64e7b94, "fpga_read" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0xc22826d3, "fpga_write" },
	{ 0xa85c710, "kmalloc_caches" },
	{ 0xf70017a4, "fpga_get_base" },
	{ 0xbe5f0fe1, "kmem_cache_alloc" },
	{ 0xea147363, "printk" },
	{ 0x37a0cba, "kfree" },
	{ 0x107cb1fa, "misc_deregister" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

