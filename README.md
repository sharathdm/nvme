# nvme

Normal working

root@root:~/sk# rmmod nvme

root@root:~/sk# rmmod nvme_fabrics

root@root:~/sk# rmmod nvme_core

root@root:~/sk# insmod nvme_ident.ko

root@root:~/sk# rmmod nvme_ident.ko

root@root:~/sk# insmod nvme_ident.ko

[  183.570442] Hello world

[  183.587152] Device vid: 0x144D pid: 0xA808

[  183.587338]  irq vectors = 2

[  183.587481] CAP 30 3c033fff

[  183.587485] VS 10300

[  183.587498] CC 460000

[  184.580791] CSTS 1

[  184.580792] SUB_P 14ab3f000 COMP_P 14ab3e000

[  184.580793]

               identify controller

[  185.574303] COMPLETION 0 0 1 11234

[  185.574304] VID 14 4d

[  185.574305] SSVID 14 4d

[  185.574306] IEE 0 25 38

[  185.574306] MTDS 9

[  185.574306] SQES 66

[  185.574307] CQES 44

[  185.574307] NN 0 0 0 1

[  185.574307] C_DB 1

[  185.574323]

               identify namespace

[  186.568181] COMPLETION 0 0 2 155aa

[  186.568182] NSZE 0 0 0 0 1d 1c 59 70

[  186.568183] NCAP 0 0 0 0 1d 1c 59 70

[  186.568183] NUSE 0 0 0 0 6 fc 4 38

[  186.568184] NLBAF 0

[  186.568184] FLBAS 0

[  186.568184] EUI 10 49 50 91 56 38 25 0

[  186.568185] LBA format-0 0 9 0 0

[  186.568186] LBA format-1 0 0 0 0

[  186.568186] LBA format-2 0 0 0 0

[  186.568186] C_DB 2

[  186.568202]

               set feature

[  187.562232] COMPLETION 1f001f 0 3 1a5a5

[  187.562233] C_DB 3

[  187.562248]

               get feature

[  188.556569] COMPLETION 1f001f 0 4 15a5a

[  188.556570] C_DB 4

[  188.556585]

               create io completion queue

[  189.551141] COMPLETION 0 0 5 11234

[  189.551142] C_DB 5

[  189.551157]

               create io submission queue

[  190.545254] COMPLETION 0 0 6 14321

[  190.545270] C_DB 6

[  190.545270]

               read block

[  190.545397] Handle IRQ #128

[  191.539387] COMPLETION 0 0 10001 1dada

[  191.539388] C_DB_Q1 1

[  191.539403] 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

[  191.539404] 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

[  191.539405] 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

[  191.539406] 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

[  191.539407] 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

[  191.539408] 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

[  191.539409] 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

[  191.539410] 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

[  191.539411] 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

[  191.539412] 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

[  191.539413] 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

[  191.539413] 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

[  191.539414] 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

[  191.539415] 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

[  191.539416] 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

[  191.539417] 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

[  191.539418] 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

[  191.539419] 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

[  191.539420] 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

[  191.539421] 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

[  191.539421] 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

[  191.539422] 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

[  191.539423] 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

[  191.539424] 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

[  191.539425] 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

[  191.539426] 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

[  191.539427] 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

[  191.539428] 00 00 00 00 00 00 00 00 bb e2 ff 14 00 00 00 00

[  191.539429] 00 00 ee 00 00 00 01 00 00 00 ff 07 00 00 00 00

[  191.539430] 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

[  191.539430] 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

[  191.539431] 00 00 00 00 00 00 00 00 00 00 00 00 00 00 55 aa

 

[  198.627299] Bye world

root@root:~/sk#
 

AER
 

[  626.242395] Hello world

[  626.242484] Device vid: 0xC0A9 pid: 0x540A

[  626.242546] CAP ffffffff ffffffff

[  626.242553] SUB_P 1b860d000 COMP_P 157264000

[  626.242640] pcieport 0000:00:1b.4: AER: Multiple Uncorrected (Non-Fatal) error received: 0000:03:00.0

[  626.242677] nvme_pci_driver 0000:03:00.0: PCIe Bus Error: severity=Uncorrected (Non-Fatal), type=Transaction Layer, (Requester ID)

[  626.242682] nvme_pci_driver 0000:03:00.0:   device [c0a9:540a] error status/mask=00100000/00400000

[  626.242686] nvme_pci_driver 0000:03:00.0:    [20] UnsupReq               (First)

[  626.242690] nvme_pci_driver 0000:03:00.0: AER:   TLP Header: 40000001 0000000f 4b200024 1f001f00

[  627.235734] CSTS ffffffff

[  628.229012] COMPLETION 0 0 0 0

[  628.229014] identify 0 0 0 0

[  628.229014] identify 0 0 0 0

[  628.229014] identify 0 0 0 0

[  628.229015] identify 0 0 0 0

[  628.229166] nvme_error_detected

[  628.229167] nvme_slot_reset

[  628.229168] nvme_error_resume

[  628.229196] pcieport 0000:00:1b.4: AER: device recovery successful

[  628.229199] pcieport 0000:00:1b.4: AER: Uncorrected (Non-Fatal) error received: 0000:03:00.0
