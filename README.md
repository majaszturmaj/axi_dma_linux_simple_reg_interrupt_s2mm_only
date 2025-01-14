*****!!! READ CAUTION NOTE !!!*****

# S2MM DMA channel with Interrupts receiving on Linux with userspace application
An linux C application that utilizes S2MM DMA in Direct Register Mode with Interrupts to constantly receive packets of data (8 x 32 each) (you can edit it to read only specified number).

**This setup was orginally based on the Hackster article ["AXI DMA interrupt with UIO driver in Embedded Linux"](https://www.hackster.io/sasha-falkovich/axi-dma-interrupt-with-uio-driver-in-embedded-linux-6dc155) by Alex Falkovich (shared under the GPL3+ license),
but I've removed interrupt-handling thread.**
|
|
 \>  Instead, my code utilizes`dma_s2mm_sync()` function that reads status register until **IOC_IRQ_FLAG** or **IDLE_FLAG** is set.
 Then it must reset this flag. But it is not enough. The DMA must be completly reset again (somehow two times), and then feed with dst address register and buffer length register to start new operation. I've tried multiple combinations but the one you can see in the code works the best for me. 

But still it is having a serious problem described below.

> [!CAUTION]
> The data read from destination registers is sometimes incomplete - it is truncated from lower registers as can be seen on the commandline screenshot:
> ![console_output](https://github.com/user-attachments/assets/c0da17ce-7945-45e0-bcb7-111c031f1d67)
> Running some time and counting by script the received packets from each channel, it shows:

| Channel number | Packets received | % of total |
|-------|------------------|------------------|
| 1     | 567              | 7.54%           |
| 2     | 670              | 8.91%           |
| 3     | 799              | 10.63%          |
| 4     | 916              | 12.18%          |
| 5     | 1022             | 13.59%          |
| 6     | 1112             | 14.79%          |
| 7     | 1192             | 15.85%          |
| 8     | 1241             | 16.50%          |


I don't have a solution for this problem yet.






[!NOTE]  
> The dst address space is bigger than actual bytes to be received because when it was equal the data truncation was happening more often.




## When to use
Use this setup when you need to transfer data from a custom AXI master data source (e.g., a counter or other streaming device) to memory via an AXI DMA, and you want to trigger data handling in your userspace Linux application using interrupts. This is particularly suitable for applications requiring deterministic handling of packetized data with end-of-frame signaling.

# Hardware setup
How to connect Processing System, AXI DMA and your data source (let's say it is a **counter with master axi interface output**) in Vivado.

## Warnings:
* If you use different clock frequency for *your counter* as for the DMA, then you must add Axi4-Stream Data FIFO block between your counter and S2MM DMA channel. It should be configured with proper depth and it must have Independend Clocks setting set to Yes, and Enable TLAST (with Interrupt mode DMA will send Interrupt when tvalid with tlast is received, indicating the end of frame).
* Your data source to match this example should send 8 32-bit data via AXI master interface, with the tlast signal accompanying the tvalid signal during the eighth sending. And then, in the same manner, you continue transmitting in a loop.
  
  ![image](https://github.com/user-attachments/assets/4ab2d01c-5e61-420d-9dde-e8a1d79c3b54)
  *AXI Stream that recieves the data from the counter connected to S2MM DMA channel*

  ![image](https://github.com/user-attachments/assets/df085c19-4167-47b0-9a8a-32905321ad14)
  *Simulation shows the data flow between the Counter (27 MHz) and AXI FIFO (100 MHz) modules in my setup*

  ## 1. Processing System setup

After applaying board preset you must:
1. Enable *S AXI HP0 Interface* for communicating with the DMA. In *PS-PL Configuration* -> *HP Slave AXI Interface*.
2. Enable *IRQ_F2P* interrupts: *Interrupts* -> *Fabric interrupts* ✔️ -> *PL-PS Interrup Ports*.
3. Connect FCLK_CLK0 output to M_AXI_GP0_ACLK and S_AXI_HP0_ACLK inputs.

  ## 2. DMA setup
  ![image](https://github.com/user-attachments/assets/e3f0c500-fb72-4892-9121-7ad8f86036b4)
  *The DMA setup is shown in the screenshot*

  ## 3. AXI Stream FIFO setup
  ![image](https://github.com/user-attachments/assets/66ae50af-1eca-4808-abe2-55e1e161341f)
  *The AXI Stream FIFO setup is shown in the screenshot above. You should use proper FIFO Depth for your own design*

  ## Overall setup
Clicking Run Block Automation after proper DMA and PS setup will make connections between DMA and Processing system.
You must now connect the interrupt from S2MM to IRQ_F2P via concat IP core of width 1 or try to connect it directly.

  ![image](https://github.com/user-attachments/assets/fc7a29e2-425c-4941-9391-f9784085f07c)
