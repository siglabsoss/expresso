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
};

static const struct modversion_info ____versions[]
__attribute_used__
__attribute__((section("__versions"))) = {
	{ 0xf8e3dbd2, "struct_module" },
	{ 0xe695160a, "kobject_put" },
	{ 0x4f7f1c1a, "cdev_del" },
	{ 0x8b8fd95a, "pci_bus_read_config_byte" },
	{ 0x5a34a45c, "__kmalloc" },
	{ 0x894c0c4a, "cdev_init" },
	{ 0x42f8c19a, "up_read" },
	{ 0x89b301d4, "param_get_int" },
	{ 0xc8b57c27, "autoremove_wake_function" },
	{ 0xeedf048a, "class_device_destroy" },
	{ 0xee756c84, "class_device_create" },
	{ 0x89892632, "malloc_sizes" },
	{ 0x36249e28, "pci_disable_device" },
	{ 0x429328d9, "_spin_lock" },
	{ 0x39280472, "remove_proc_entry" },
	{ 0x338dcf4b, "kobject_set_name" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0xd20f583f, "pci_release_regions" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0x98bd6f46, "param_set_int" },
	{ 0xdd5a37a7, "_spin_lock_irqsave" },
	{ 0x1d26aa98, "sprintf" },
	{ 0x9a5fdbf3, "down_read" },
	{ 0xda4008e6, "cond_resched" },
	{ 0x490ef5ac, "pci_set_master" },
	{ 0xde0bdcff, "memset" },
	{ 0x52f4bcd2, "pci_set_dma_mask" },
	{ 0xdd132261, "printk" },
	{ 0xf397b9aa, "__tasklet_schedule" },
	{ 0x7ec9bfbc, "strncpy" },
	{ 0xbe499d81, "copy_to_user" },
	{ 0xd1b9b39e, "dma_free_coherent" },
	{ 0xccd52b12, "__down_failed_interruptible" },
	{ 0x9af21eb9, "class_create" },
	{ 0xdf15005, "_spin_unlock_irqrestore" },
	{ 0xa5808bbf, "tasklet_init" },
	{ 0x9eac042a, "__ioremap" },
	{ 0x26e9c85f, "pci_enable_msi" },
	{ 0xfb999f65, "dma_alloc_coherent" },
	{ 0x5777894f, "cdev_add" },
	{ 0x4c503ced, "kmem_cache_alloc" },
	{ 0xb11e2cf, "pci_bus_read_config_word" },
	{ 0x65b75503, "pci_bus_read_config_dword" },
	{ 0x8179a849, "get_user_pages" },
	{ 0x5271af5d, "request_irq" },
	{ 0x1000e51, "schedule" },
	{ 0xa1679700, "create_proc_entry" },
	{ 0x1a35c71c, "pci_unregister_driver" },
	{ 0x19cacd0, "init_waitqueue_head" },
	{ 0xdb09708f, "__wake_up" },
	{ 0x37a0cba, "kfree" },
	{ 0xec5f438f, "remap_pfn_range" },
	{ 0x4c5b4b9f, "pci_request_regions" },
	{ 0x65f4af5f, "prepare_to_wait" },
	{ 0xecbc6b8d, "pci_disable_msi" },
	{ 0xedc03953, "iounmap" },
	{ 0x769a5e96, "__pci_register_driver" },
	{ 0x4c6c6926, "put_page" },
	{ 0xd3e6b205, "class_destroy" },
	{ 0x271f4d2a, "finish_wait" },
	{ 0x1042cbb5, "__up_wakeup" },
	{ 0x399280fc, "pci_enable_device" },
	{ 0x945bc6a7, "copy_from_user" },
	{ 0xd64b746f, "dma_ops" },
	{ 0x29537c9e, "alloc_chrdev_region" },
	{ 0xf20dabd8, "free_irq" },
	{ 0xe914e41e, "strcpy" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("pci:v00001204d00005303sv00001204sd00003040bc*sc*i*");
MODULE_ALIAS("pci:v00001204d0000E250sv00001204sd00003040bc*sc*i*");
MODULE_ALIAS("pci:v00001204d0000EC30sv00001204sd00003040bc*sc*i*");

MODULE_INFO(srcversion, "88E6A95091E033E36D579DE");
