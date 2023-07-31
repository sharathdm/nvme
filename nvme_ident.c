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

//#define ALLOC_COHERENT

#define CAP 0x0
#define VS 0x8
#define CC 0x14
#define CSTS 0x1c
#define AQA 0x24
#define ASQ 0x28
#define ACQ 0x30
#define S_DB 0x1000
#define C_DB 0x1004
#define S_DB_Q1 0x1008
#define C_DB_Q1 0x100c


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
char *prp2_v;
dma_addr_t prp2_p;

u32 *sub_v_q1;
dma_addr_t sub_p_q1;
u32 *comp_v_q1;
dma_addr_t comp_p_q1;

static int nvme_driver_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	int bar, err;
	u16 vendor, device;
	unsigned long mmio_start,mmio_len;
	u32 temp;
	int i,j;
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

    printk("CAP %x %x\n",ioread32(hwmem+4),ioread32(hwmem));
    printk("VS %x \n",ioread32(hwmem+VS));
    /* admin submit & complete */
    sub_v  = dma_alloc_coherent(&pdev->dev, PAGE_SIZE, &sub_p, GFP_KERNEL);
    comp_v = dma_alloc_coherent(&pdev->dev, PAGE_SIZE, &comp_p, GFP_KERNEL);
    /* io submit & complete */
    sub_v_q1  = dma_alloc_coherent(&pdev->dev, PAGE_SIZE, &sub_p_q1, GFP_KERNEL);
    comp_v_q1 = dma_alloc_coherent(&pdev->dev, PAGE_SIZE, &comp_p_q1, GFP_KERNEL);
#ifdef ALLOC_COHERENT
    prp1_v = dma_alloc_coherent(&pdev->dev, PAGE_SIZE, &prp1_p, GFP_KERNEL);
    prp2_v = dma_alloc_coherent(&pdev->dev, PAGE_SIZE, &prp2_p, GFP_KERNEL);
#else
    prp1_v = kmalloc(PAGE_SIZE, GFP_KERNEL);
    prp1_p = dma_map_single(&pdev->dev, prp1_v, PAGE_SIZE, DMA_FROM_DEVICE);
    prp2_v = kmalloc(PAGE_SIZE, GFP_KERNEL);
    prp2_p = dma_map_single(&pdev->dev, prp2_v, PAGE_SIZE, DMA_FROM_DEVICE);
#endif

    iowrite32(0x001F001F, hwmem+AQA);
    iowrite32(sub_p, hwmem+ASQ);
    iowrite32((sub_p>>32), hwmem+ASQ+0x4);
    iowrite32(comp_p, hwmem+ACQ);
    iowrite32((comp_p>>32), hwmem+ACQ+0x4);

    temp = ioread32(hwmem+CC);
    temp = ((4<<20) | (6<<16));
    iowrite32(temp, hwmem+CC);
    printk("CC %x \n",ioread32(hwmem+CC));
    iowrite32(temp|0x1, hwmem+CC);

    mdelay(1000);
    printk("CSTS %x \n",ioread32(hwmem+CSTS));

    iowrite32(0x001F001F, hwmem+AQA);

    //printk("%px %px %pax %pax \n",sub_v, comp_v, sub_p, comp_p);
    //printk("%px %px  \n",sub_v, comp_v);
    printk("SUB_P %llx COMP_P %llx  \n",(u64)sub_p, (u64)comp_p);

/* new command */
    printk("\nidentify controller\n");
    *sub_v = 0x12340006; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;

    *sub_v = (u32)prp1_p; sub_v++;
    *sub_v = (u32)(prp1_p >> 32); sub_v++;

    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;

    *sub_v = 0x1; sub_v++; /* controller data structure */

    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;

    iowrite32(0x1, hwmem+S_DB);
    mdelay(1000);

#ifdef ALLOC_COHERENT
#else
    dma_unmap_single(&pdev->dev, prp1_p, PAGE_SIZE, DMA_FROM_DEVICE);
#endif

    printk("COMPLETION %x %x %x %x\n",*comp_v, *(comp_v+1), *(comp_v+2), *(comp_v+3));
    comp_v +=4;

    printk("VID %x %x\n", *(prp1_v+1), *(prp1_v+0));
    printk("SSVID %x %x\n", *(prp1_v+3), *(prp1_v+2));
    printk("IEE %x %x %x\n", *(prp1_v+75), *(prp1_v+74), *(prp1_v+73));
    printk("MTDS %x\n", *(prp1_v+77));
    printk("SQES %x\n", *(prp1_v+512));
    printk("CQES %x\n", *(prp1_v+513));
    printk("NN %x %x %x %x\n",*(prp1_v+519), *(prp1_v+518), *(prp1_v+517), *(prp1_v+516));

    iowrite32(0x1, hwmem+C_DB);
    printk("C_DB %x \n",ioread32(hwmem+C_DB));

/* new command */
    printk("\nidentify namespace\n");
    *sub_v = 0x55aa0006; sub_v++;
    *sub_v = 0x1; sub_v++; /*namespace identifier*/
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;

    *sub_v = (u32)prp1_p; sub_v++;
    *sub_v = (u32)(prp1_p >> 32); sub_v++;

    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;

    *sub_v = 0x0; sub_v++; /*name space data structure*/

    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;

    iowrite32(0x2, hwmem+S_DB);
    mdelay(1000);

#ifdef ALLOC_COHERENT
#else
    dma_unmap_single(&pdev->dev, prp1_p, PAGE_SIZE, DMA_FROM_DEVICE);
#endif
    printk("COMPLETION %x %x %x %x\n",*comp_v, *(comp_v+1), *(comp_v+2), *(comp_v+3));
    comp_v +=4;

    printk("NSZE %x %x %x %x %x %x %x %x\n",*(prp1_v+7),*(prp1_v+6),*(prp1_v+5),*(prp1_v+4),*(prp1_v+3),*(prp1_v+2),*(prp1_v+1), *(prp1_v+0));
    printk("NCAP %x %x %x %x %x %x %x %x\n",*(prp1_v+15),*(prp1_v+14),*(prp1_v+13),*(prp1_v+12),*(prp1_v+11),*(prp1_v+10),*(prp1_v+9), *(prp1_v+8));
    printk("NUSE %x %x %x %x %x %x %x %x\n",*(prp1_v+23),*(prp1_v+22),*(prp1_v+21),*(prp1_v+20),*(prp1_v+19),*(prp1_v+18),*(prp1_v+17), *(prp1_v+16));
    printk("NLBAF %x\n", *(prp1_v+25));
    printk("FLBAS %x\n", *(prp1_v+26));
    printk("EUI %x %x %x %x %x %x %x %x\n",*(prp1_v+127),*(prp1_v+126),*(prp1_v+125),*(prp1_v+124),*(prp1_v+123),*(prp1_v+122),*(prp1_v+121), *(prp1_v+120));
    printk("LBA format-0 %x %x %x %x\n",*(prp1_v+131),*(prp1_v+130),*(prp1_v+129),*(prp1_v+128));
    printk("LBA format-1 %x %x %x %x\n",*(prp1_v+134),*(prp1_v+133),*(prp1_v+132),*(prp1_v+131));
    printk("LBA format-2 %x %x %x %x\n",*(prp1_v+138),*(prp1_v+137),*(prp1_v+136),*(prp1_v+135));

    iowrite32(0x2, hwmem+C_DB);
    printk("C_DB %x \n",ioread32(hwmem+C_DB));

/* new command */
    printk("\nset feature\n");
    *sub_v = 0xa5a50009; sub_v++; /* set feature */
    *sub_v = 0x0; sub_v++; /*namespace identifier*/
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;

    *sub_v = (u32)prp1_p; sub_v++;
    *sub_v = (u32)(prp1_p >> 32); sub_v++;

    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;

    *sub_v = 0x7; sub_v++; /* feature identifer = number of queues*/

    *sub_v = 0x30003; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;

    iowrite32(0x3, hwmem+S_DB);
    mdelay(1000);

#ifdef ALLOC_COHERENT
#else
    dma_unmap_single(&pdev->dev, prp1_p, PAGE_SIZE, DMA_FROM_DEVICE);
#endif
    printk("COMPLETION %x %x %x %x\n",*comp_v, *(comp_v+1), *(comp_v+2), *(comp_v+3));
    comp_v +=4;

    iowrite32(0x3, hwmem+C_DB);
    printk("C_DB %x \n",ioread32(hwmem+C_DB));

/* new command */
    printk("\nget feature\n");
    *sub_v = 0x5a5a000a; sub_v++; /* get feature */
    *sub_v = 0x0; sub_v++; /*namespace identifier*/
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;

    *sub_v = (u32)prp1_p; sub_v++;
    *sub_v = (u32)(prp1_p >> 32); sub_v++;

    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;

    *sub_v = 0x7; sub_v++; /* feature identifer = number of queues*/

    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;

    iowrite32(0x4, hwmem+S_DB);
    mdelay(1000);

#ifdef ALLOC_COHERENT
#else
    dma_unmap_single(&pdev->dev, prp1_p, PAGE_SIZE, DMA_FROM_DEVICE);
#endif
    printk("COMPLETION %x %x %x %x\n",*comp_v, *(comp_v+1), *(comp_v+2), *(comp_v+3));
    comp_v +=4;

    iowrite32(0x4, hwmem+C_DB);
    printk("C_DB %x \n",ioread32(hwmem+C_DB));

/* new command */
    printk("\ncreate io completion queue\n");
    *sub_v = 0x12340005; sub_v++; /* create io completion queue */
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;

    *sub_v = (u32)comp_p_q1; sub_v++;
    *sub_v = (u32)(comp_p_q1 >> 32); sub_v++;

    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;

    *sub_v = 0xf0001; sub_v++; /* queuesize|queueident*/
    *sub_v = 0x1; sub_v++; /* contigeous pages */
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;

    iowrite32(0x5, hwmem+S_DB);
    mdelay(1000);

#ifdef ALLOC_COHERENT
#else
    dma_unmap_single(&pdev->dev, prp1_p, PAGE_SIZE, DMA_FROM_DEVICE);
#endif
    printk("COMPLETION %x %x %x %x\n",*comp_v, *(comp_v+1), *(comp_v+2), *(comp_v+3));
    comp_v +=4;

    iowrite32(0x5, hwmem+C_DB);
    printk("C_DB %x \n",ioread32(hwmem+C_DB));

/* new command */
    printk("\ncreate io submission queue\n");
    *sub_v = 0x43210001; sub_v++; /* create io submission queue */
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;

    *sub_v = (u32)sub_p_q1; sub_v++;
    *sub_v = (u32)(sub_p_q1 >> 32); sub_v++;

    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;

    *sub_v = 0xf0001; sub_v++;  /* queuesize|queueident*/
    *sub_v = 0x10001; sub_v++; /* conpletion queue |contigeous pages */
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;
    *sub_v = 0x0; sub_v++;

    iowrite32(0x6, hwmem+S_DB);
    mdelay(1000);

#ifdef ALLOC_COHERENT
#else
    dma_unmap_single(&pdev->dev, prp1_p, PAGE_SIZE, DMA_FROM_DEVICE);
#endif
    printk("COMPLETION %x %x %x %x\n",*comp_v, *(comp_v+1), *(comp_v+2), *(comp_v+3));
    comp_v +=4;

    iowrite32(0x6, hwmem+C_DB);
    printk("C_DB %x \n",ioread32(hwmem+C_DB));

/* new command */
    printk("\nread block\n");
    *sub_v_q1 = 0xdada0002; sub_v_q1++; /* read */
    *sub_v_q1 = 0x1; sub_v_q1++;
    *sub_v_q1 = 0x0; sub_v_q1++;
    *sub_v_q1 = 0x0; sub_v_q1++;
    *sub_v_q1 = 0x0; sub_v_q1++;
    *sub_v_q1 = 0x0; sub_v_q1++;

    *sub_v_q1 = (u32)prp1_p; sub_v_q1++;
    *sub_v_q1 = (u32)(prp1_p >> 32); sub_v_q1++;

    *sub_v_q1 = 0x0; sub_v_q1++;
    *sub_v_q1 = 0x0; sub_v_q1++;

    *sub_v_q1 = 0x0; sub_v_q1++;  /* starting LBA low */
    *sub_v_q1 = 0x0; sub_v_q1++; /* starting LBA high */
    *sub_v_q1 = (0x0|(1<<30)); sub_v_q1++; /* number of logical blocks */
    *sub_v_q1 = 0x0; sub_v_q1++;
    *sub_v_q1 = 0x0; sub_v_q1++;
    *sub_v_q1 = 0x0; sub_v_q1++;

    iowrite32(0x1, hwmem+S_DB_Q1);
    mdelay(1000);

#ifdef ALLOC_COHERENT
#else
    dma_unmap_single(&pdev->dev, prp1_p, PAGE_SIZE, DMA_FROM_DEVICE);
#endif
    printk("COMPLETION %x %x %x %x\n",*comp_v_q1, *(comp_v_q1+1), *(comp_v_q1+2), *(comp_v_q1+3));
    comp_v_q1 +=4;

    iowrite32(0x1, hwmem+C_DB_Q1);
    printk("C_DB_Q1 %x \n",ioread32(hwmem+C_DB_Q1));

    for (i=0; i<32; i++) {
            printk("%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n",*(prp1_v+(16*i)+0),*(prp1_v+(16*i)+1),*(prp1_v+(16*i)+2),*(prp1_v+(16*i)+3),*(prp1_v+(16*i)+4),*(prp1_v+(16*i)+5),*(prp1_v+(16*i)+6),*(prp1_v+(16*i)+7),*(prp1_v+(16*i)+8),*(prp1_v+(16*i)+9),*(prp1_v+(16*i)+10),*(prp1_v+(16*i)+11),*(prp1_v+(16*i)+12),*(prp1_v+(16*i)+13),*(prp1_v+(16*i)+14),*(prp1_v+(16*i)+15));
    }

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
	dma_free_coherent(&pdev->dev, PAGE_SIZE, prp2_v, prp2_p);
#else
    kfree(sub_v);
    kfree(comp_v);
    kfree(prp1_v);
	kfree(prp2_v);
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
