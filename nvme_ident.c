#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <asm/hw_irq.h>
#include <linux/irq.h>
#include <linux/pci.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/aer.h>

#define NVME_DRIVER "nvme_pci_driver"

#define ALLOC_COHERENT

#define CAP 0x0
#define VS 0x8
#define CC 0x14
#define CSTS 0x1c
#define AQA 0x24
#define ASQ 0x28
#define ACQ 0x30
#define S_DB 0x1000

#define COMMAND_IDENT   0x1234
#define IDENTIFY_OPCODE 0x06


u8 __iomem *hwmem;

static struct pci_device_id nvme_driver_id_table[] = {
    { PCI_DEVICE(0xc0a9, 0x540a) },
    { PCI_DEVICE(0x144d, 0xa808) },
    {0,}
};

MODULE_DEVICE_TABLE(pci, nvme_driver_id_table);

static int nvme_driver_probe(struct pci_dev *pdev, const struct pci_device_id *ent);
static void nvme_driver_remove(struct pci_dev *pdev);

static pci_ers_result_t nvme_error_detected(struct pci_dev *pdev,
                                                pci_channel_state_t state)
{
	printk("nvme_error_detected\n");
	return PCI_ERS_RESULT_NEED_RESET;
}
static pci_ers_result_t nvme_slot_reset(struct pci_dev *pdev)
{
	printk("nvme_slot_reset\n");
	return PCI_ERS_RESULT_RECOVERED;
}

static void nvme_error_resume(struct pci_dev *pdev)
{
	printk("nvme_error_resume\n");
}

static void nvme_reset_prepare(struct pci_dev *pdev)
{
	printk("nvme_reset_prepare\n");
}

static void nvme_reset_done(struct pci_dev *pdev)
{
	printk("nvme_reset_done\n");
}


static const struct pci_error_handlers nvme_driver_err_handler = {
        .error_detected = nvme_error_detected,
        .slot_reset     = nvme_slot_reset,
        .resume         = nvme_error_resume,
        .reset_prepare  = nvme_reset_prepare,
        .reset_done     = nvme_reset_done,
};

/* Driver registration structure */
static struct pci_driver nvme_driver = {
    .name = NVME_DRIVER,
    .id_table = nvme_driver_id_table,
    .probe = nvme_driver_probe,
    .remove = nvme_driver_remove,
    .err_handler    = &nvme_driver_err_handler,
};

u32 *sub_v;
dma_addr_t sub_p;
u32 *comp_v;
dma_addr_t comp_p;
char *prp1_v;
dma_addr_t prp1_p;

static int nvme_driver_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	int bar, err;
	u16 vendor, device;
	unsigned long mmio_start,mmio_len;
	u32 temp;
	/* Let's read data from the PCI device configuration registers */
    pci_read_config_word(pdev, PCI_VENDOR_ID, &vendor);
    pci_read_config_word(pdev, PCI_DEVICE_ID, &device);

    printk(KERN_INFO "Device vid: 0x%X pid: 0x%X\n", vendor, device);

    pci_enable_device(pdev);
    pci_set_master(pdev);
     /* Request IO BAR */
    bar = pci_select_bars(pdev, IORESOURCE_MEM);

    /* Enable device memory */
    err = pci_enable_device_mem(pdev);
    if (err) {
        return err;
    }
	
	/* uncomment for testing AER */
    //pci_write_config_word(pdev, PCI_COMMAND, 0x0);

    /* Request memory region for the BAR */
    err = pci_request_region(pdev, bar, NVME_DRIVER);
    if (err) {
        pci_disable_device(pdev);
        return err;
    }

    /* Get start and stop memory offsets */
    mmio_start = pci_resource_start(pdev, 0);
    mmio_len = pci_resource_len(pdev, 0);

    /* Remap BAR to the local pointer */
    hwmem = ioremap(mmio_start, mmio_len);

    pci_enable_pcie_error_reporting(pdev);
	
    printk("CAP %x %x\n",ioread32(hwmem),ioread32(hwmem+4));
    printk("VS %x \n",ioread32(hwmem+VS));
#ifdef ALLOC_COHERENT
    sub_v  = dma_alloc_coherent(&pdev->dev, PAGE_SIZE, &sub_p, GFP_KERNEL);
    comp_v = dma_alloc_coherent(&pdev->dev, PAGE_SIZE, &comp_p, GFP_KERNEL);
    prp1_v = dma_alloc_coherent(&pdev->dev, PAGE_SIZE, &prp1_p, GFP_KERNEL);
#else
    sub_v  = kmalloc(PAGE_SIZE, GFP_KERNEL);
    comp_v = kmalloc(PAGE_SIZE, GFP_KERNEL);
    prp1_v = kmalloc(PAGE_SIZE, GFP_KERNEL);
    comp_p = dma_map_single(&pdev->dev, comp_v, PAGE_SIZE, DMA_FROM_DEVICE);
    prp1_p = dma_map_single(&pdev->dev, prp1_v, PAGE_SIZE, DMA_FROM_DEVICE);
#endif

    //printk("%px %px %pax %pax \n",sub_v, comp_v, sub_p, comp_p);
    //printk("%px %px  \n",sub_v, comp_v);
    printk("SUB_P %llx COMP_P %llx  \n",(u64)sub_p, (u64)comp_p);
    *sub_v = (COMMAND_IDENT << 16)|IDENTIFY_OPCODE; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;

    *sub_v = (u32)prp1_p; sub_v++; /* PRP entry 1 */
    *sub_v = (u32)(prp1_p >> 32); sub_v++;

    *sub_v = 0x0; sub_v++; /* PRP entry 2 */
    *sub_v = 0x0; sub_v++;

    *sub_v = 0x1; sub_v++; /* controller data structure*/

    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;

#ifdef ALLOC_COHERENT
#else
    sub_p = dma_map_single(&pdev->dev, sub_v, PAGE_SIZE, DMA_TO_DEVICE);
#endif

    iowrite32(0x001F001F , hwmem+AQA);
    iowrite32(sub_p , hwmem+ASQ);
    iowrite32((sub_p>>32) , hwmem+0x2c+0x4);
    iowrite32(comp_p , hwmem+ACQ);
    iowrite32((comp_p>>32) , hwmem+0x34+0x4);

    temp = ioread32(hwmem+CC);
    iowrite32(temp|0x1, hwmem+CC);

    mdelay(1000);
    printk("CSTS %x \n",ioread32(hwmem+CSTS));

    iowrite32(0x1, hwmem+S_DB);
    mdelay(1000);

#ifdef ALLOC_COHERENT
#else
    dma_unmap_single(&pdev->dev, comp_p, PAGE_SIZE, DMA_FROM_DEVICE);
#endif

    printk("if success COMPLETION sould have %x\n,COMMAND_IDENT); 
    printk("COMPLETION %x %x %x %x\n",*comp_v, *(comp_v+1), *(comp_v+2), *(comp_v+3));

    printk("identify %x %x %x %x\n",*prp1_v, *(prp1_v+1), *(prp1_v+2), *(prp1_v+3));
    printk("identify %x %x %x %x\n",*(prp1_v+4), *(prp1_v+5), *(prp1_v+6), *(prp1_v+7));
    printk("identify %x %x %x %x\n",*(prp1_v+8), *(prp1_v+9), *(prp1_v+10), *(prp1_v+11));
    printk("identify %x %x %x %x\n",*(prp1_v+12), *(prp1_v+13), *(prp1_v+14), *(prp1_v+15));

    return 0;
}

/* Clean up */
static void nvme_driver_remove(struct pci_dev *pdev)
{
	pci_disable_pcie_error_reporting(pdev);
	
    /* reset device */
    u32 temp = ioread32(hwmem+CC);
    iowrite32(temp&0xfffffffe, hwmem+CC);

#ifdef ALLOC_COHERENT
    dma_free_coherent(&pdev->dev, PAGE_SIZE, sub_v, sub_p);
    dma_free_coherent(&pdev->dev, PAGE_SIZE, comp_v, comp_p);
    dma_free_coherent(&pdev->dev, PAGE_SIZE, prp1_v, prp1_p);
#else
    kfree(sub_v);
    kfree(comp_v);
    kfree(prp1_v);
#endif

    /* Free memory region */
    pci_release_region(pdev, pci_select_bars(pdev, IORESOURCE_MEM));
    /* And disable device */
    pci_disable_device(pdev);
}


MODULE_LICENSE("GPL");
MODULE_AUTHOR("SHARATHDM");
MODULE_DESCRIPTION("nvme sample driver");
MODULE_VERSION("0.01");

static int __init hello_mod_init(void)
{
        printk(KERN_ALERT "Hello world\n");
	return pci_register_driver(&nvme_driver);
        return 0;
}

static void __exit hello_mod_exit(void)
{
	pci_unregister_driver(&nvme_driver);
        printk(KERN_ALERT "Bye world \n");
}

module_init(hello_mod_init);
module_exit(hello_mod_exit);
