# S2MM DMA channel with Interrupts receiving on Linux with userspace application
An linux C application that utilizes S2MM DMA in Direct Register Mode with Interrupts to receive 10 packets of data (8 x 32 each).

**This setup is based on the Hackster article ["AXI DMA interrupt with UIO driver in Embedded Linux"](https://www.hackster.io/sasha-falkovich/axi-dma-interrupt-with-uio-driver-in-embedded-linux-6dc155) by Alex Falkovich, shared under the GPL3+ license.**

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
  ![image](https://github.com/user-attachments/assets/d81d4a4b-b258-4de2-bb02-600ef22b8f61)
  *The AXI Stream FIFO setup is shown in the screenshot above. You should use proper FIFO Depth for your own design*

  ## Overall setup
Clicking Run Block Automation after proper DMA and PS setup will make connections between DMA and Processing system.
You must now connect the interrupt from S2MM to IRQ_F2P via concat IP core of width 1 or try to connect it directly.

  ![image](https://github.com/user-attachments/assets/fc7a29e2-425c-4941-9391-f9784085f07c)
