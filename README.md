# Description

FpgaCha is an IP-core for acceleration of [ChaCha20](https://tools.ietf.org/html/rfc8439) encryption algorithm in FPGA. Multiple such cores can be easily connected to an ARM processor and accessed from Linux via the [UIO](https://www.kernel.org/doc/html/v4.14/driver-api/uio-howto.html) device driver. My performance evaluation shows that two FpgaCha cores working at 50 MHz are 5 times faster than a software ChaCha20 implementation working on two ARM Cortex-A9 cores clocked at 800 MHz.

## Project structure

The project consists of the following directories:

`linux-system` —  Platform Designer project that contains the HPS connected with four FpgaCha modules

`quartus-project` — Quartus project that is used to synthesize the entire system

`ip-cores` — Custom IP cores used in this project

`ip-cores\ChaCha20` — SystemVerilog ChaCha20 implementation

`ip-cores\S2MAdapter` — DMA for transferring data from 512-bit Avalon-ST interface to DRAM

`device-tree`  — device tree overlay for configuring FPGA and initializing device drivers

## Performance

When two FpgaCha cores are used for real file encryption in the Linux environment they demonstrate the following performance compared to a two-threaded software solution:

| Configuration                     | Performance |
| --------------------------------- | ----------- |
| Two ARM Cortex-A9 cores @ 800 MHz | 23.8 MiB/s  |
| Two FphaCha IP-cores @ 50 MHz     | 120.5 MiB/s |

Using more than two FpgaCha cores turned out to be inefficient as the cores utilize the entire DRAM bandwidth.

## System view

The diagram of the hardware system containing multiple FphaCha cores is shown below:

![](https://user-images.githubusercontent.com/4264235/88441542-042de200-cdd7-11ea-97bf-4851e0cef21a.png)

The HPS (the system containing ARM cores) configures ChaCha20 accelerator and S2M adapter via the lightweight HPS-to-DRAM bridge. Once configured ChaCha20 accelerator starts producing blocks of ChaCha20 one-time pad and placing them in the FIFO. S2M adapter reads these blocks and transfers them in DRAM through the FPGA-to-SDRAM bridge. Once all requested blocks are transferred, S2M adapter sends an interrupt request to the HPS, signalizing about finishing the job. 

## Register map

### ChaCha20 accelerator

| Name              | Byte offset   | Description                                                  |
| ----------------- | ------------- | ------------------------------------------------------------ |
| INIT\_STATE[0:15] | 0x0000–0x001F | A 16-word (32 bits per word) initial state for ChaCha20 algorithm. This register consists of multiple fields such as key, nonce, and block count. |
| PAD\_COUNTER      | 0x0020        | The number of one-time pad blocks (without the summation stage) to generate. The module start computation upon modification of this register. |

### S2M adapter

| Name    | Byte offset | Description                                                  |
| ------- | ----------- | ------------------------------------------------------------ |
| LENGTH  | 0x0000      | The number of 256-bit chunks of data to be transferred. Modification of this register initiates a data transfer. |
| ADDRESS | 0x0004      | Starting address in the DRAM for transferring data at.       |
| IRQ     | 0x0008      | Modification of this register clears a pending interrupt request |

## `chacha20` utility

The `chacha20` utility is a program that uses FpgaCha cores for file encryption and decryption. The program can utilize any number of such cores (only 4 available in the default FPGA firmware). The program is also capable of utilizing CPU cores for ChaCha20 file encryption. This is helpful for comparing performance of hardware and software solutions. The diagram below demonstrates the architecture of the utility:

![](https://user-images.githubusercontent.com/4264235/88470369-f9e81280-cec0-11ea-8303-b778ba9a7995.png)

Yellow rectangles depict fixed capacity blocking queues. The number inside shows the capacity of a queue. 

Workers are ChaCha20 [one-time pad](https://en.wikipedia.org/wiki/One-time_pad) producers. Based on the utility's configuration there may be a different number of workers of different types: some workers use one FpgaCha core to generate one-time pad, some use one CPU core, and the other produce fake one-time pads, but do it very fast (useful for performance measurements).

Cryptor is a one-time pad consumer. Based on its type, it can either use the one-time pad to encrypt a file, or can just discard it (useful for performance measurements).

Workers and the Cryptor communicate via tasks: Cryptor decides which part of one-time pad it needs and asks for this part by placing a certain task in the queue of scheduled tasks. Workers extract the tasks from the queue, generate the necessary blocks of one-time pad accordingly, and place the result in the queue of finished tasks, which puts the results in order (workers may process tasks at different rates, so the results may be un-ordered). Cryptor consumes the results from that queue and performs file encryption.  

# Running the project

## Booting Linux

DE10-Nano board is capable of running Linux on its ARM processor, and FpgaCha can be used in this Linux environment. I used [this](https://github.com/ikwzm/FPGA-SoC-Linux) Debian distribution as the basis for my experiments with FpgaCha. However, I modified it slightly to automatically enable the FPGA-to-SDRAM bridge in U-Boot (see [this gist](https://gist.github.com/Goshik92/68af8ee64a3fa469d81017385a814cad) for more detail).

Download [this](
https://drive.google.com/file/d/1k0qxvy9RfGG8k0pqKzSqvhWvKfCM8nFA) archive, unpack it, and write it to a 16 GB SD card (you can use [Win32 Disk Imager](https://sourceforge.net/projects/win32diskimager/) in Windows). Insert the card in your DE10-Nano board and turn on the power. Connect the board to your local network using an Ethernet-cable. The network in this Linux image is configured to work with DHCP. Use your router's admin panel to find out the boards IP address (look for `debian-fpga` hostname). Use the following credentials to connect to the board via SSH:

* login: `root`
* password: `admin`

The image contains a local copy of this repository. To update it to the latest version use the following commands:

```
cd ~/FpgaCha
git pull origin master
```

If you pull a newer version, you may need to re-synthesize the firmware for FPGA to make everything work (see the next section).   

## Synthesis

If you do not want to make any changes to the hardware design of FpgaCha, just ignore this section. The Linux image from the previous section already contains a working firmware for FPGA.

If you want to change something in the hardware design, you will need to synthesize the HDL code and produce the `.rbf` file that can be loaded into FPGA from the Linux running on the board. First, open `quartus-project/DE10_NANO_SoC_GHRD.qpf` in Intel Quartus and choose Processing -> Start Compilation. When the process finishes, go to the `quartus-project` folder and run the `sof_to_rbf.bat` script. After the script finishes, you should be able to find the `fpgacha.rbf` file in the same folder.

## Configuring FPGA

If you synthesized your own `fpgacha.rbf` copy it to `/lib/firmware` on DE10-Nano (you can use [WinSCP](https://winscp.net) for this purpose). Alternatively, you can keep the old version of the file.

Configure FPGA and load all necessary device drivers by running the following command:

`dtbocfg.rb --install fpgacha --dts ~/FpgaCha/device-tree/fpgacha.dts`

## Compiling `chacha20` utility

Build the utility using the following command:

```
cd ~/FpgaCha/software
make
```

The default configuration of the utility uses two FpgaCha cores for encryption/decryption.

## Re-configuring `chacha20` utility

If you want to you use a different number of FpgaCha cores (up to 4 is supported by default), you need to re-configure the `chacha20` utility. Right it can only be done by modifying `main.cpp` and recompiling the code. Open the file and choose the Cryptor by uncommenting one of the following lines:

* `FileCryptor<N> fc(m, inFile, outFile)` — to encrypt or decrypt real files
* `FakeCryptor<N> fc(m, 1024*1024*256)` — to load workers for seeing their maximum throughput; no real file will be encrypted; the second parameter specifies how many bytes of one-time pad is consumed from workers before terminating.

Chose the workers by uncommenting some of the following lines:
* `FakeWorker<N> fw(m)` — for loading `FileCryptor` to test file access speed; fake one-time pad will be produced; it does not make sense to use this worker together with any other one
* `ChaCha20Worker<N> ccw0(m, state)` — for using a software ChaCha20 implementation; it is possible to uncomment multiple such lines to allow multi-threading
* `FpgaChaWorker<N, 1> fcw0(m, state, "uio0", uDmaBuf)` — for using a hardware ChaCha20 implementation; it is possible to uncomment multiple such lines to employ multiple FpgaCha cores (max 4 with the current hardware configuration)

Re-compile the utility as described in the previous section.

## Ramdisk creation

Creating a ramdisk (a file system in RAM) can eliminate the overhead introduced by the SD card when encrypting or decrypting files. Run the following command to create a 800 MiB ramdisk:

```
cd ~/FpgaCha/software
make mktmpfs size=800M
```

This will create `~/FpgaCha/software/ramdisk` directory with a mounted file system stored in RAM. You can copy the files you want to encrypt in this directory. Alternatively, you can use the following command to create a text file with pseudorandom content (a 256 MiB file will be created):

```
make mkfile size=256M
```

The name of the created file will be `~/FpgaCha/software/ramdisk/in`.

To remove the ramdisk and free up memory, use the following command:

```
make rmtmpfs
```

## Encryption and decryption with `chacha20` utility

In order to encrypt a file use the following command:

```
cd ~/FpgaCha/software
time ./chacha20 ./ramdisk/in ./ramdisk/out
```

The `time` utility is used to report the execution time. The two parameters of the `chacha20` utility are the encrypted file (`./ramdisk/in`) and the output file (`./ramdisk/out`).

In stream cyphers encryption and decryption are basically the same operation. So, if you need to decrypt `/ramdisk/out` execute the following command:

```
time ./chacha20 ./ramdisk/out ./ramdisk/decrypted
```