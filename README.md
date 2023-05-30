# nvme

Normal working

[  516.612895] Hello world

[  516.612963] Device vid: 0xC0A9 pid: 0x540A

[  516.613078] CAP ff01ffff 20

[  516.613086] SUB_P 1ad3c9000 COMP_P 1ad182000

[  517.608214] CSTS 9

[  518.603076] COMPLETION 0 0 1 11234

[  518.603077] identify ffffffa9 ffffffc0 ffffffa9 ffffffc0

[  518.603078] identify 32 33 30 39

[  518.603078] identify 45 36 42 35

[  518.603078] identify 43 42 41 46

 

 

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
