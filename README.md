# Important Note

- IDE used : MPLAB X [download link](https://www.microchip.com/en-us/development-tools-tools-and-software/mplab-x-ide)
- Compiler used : XC16 v1.70 [download link](https://www.microchip.com/mplabxc16windows)
- Development board used : [dsPIC33CH512MP508](https://www.microchip.com/Developmenttools/ProductDetails/DM330028-2)

# How it started...

After watching/following this [YouTube tutorial](https://www.youtube.com/watch?v=sQLZ3tIcd44&ab_channel=MicrochipTechnology) on how to use MCC to generate the code needed to use UART, if you were to clean/build the entire project as is, you'll get the following errors

```
mcc_generated_files/examples/uart_example.c: In function 'UART_example_READ':
mcc_generated_files/examples/uart_example.c:116:5: error: 'for' loop initial declarations are only allowed in C99 mode
mcc_generated_files/examples/uart_example.c:116:5: note: use option -std=c99 or -std=gnu99 to compile your code
mcc_generated_files/examples/uart_example.c:126:14: error: redefinition of 'i'
mcc_generated_files/examples/uart_example.c:116:14: note: previous definition of 'i' was here
mcc_generated_files/examples/uart_example.c:126:5: error: 'for' loop initial declarations are only allowed in C99 mode
make[2]: *** [build/default/production/mcc_generated_files/examples/uart_example.o] Error 255
make[2]: *** Waiting for unfinished jobs....
nbproject/Makefile-default.mk:226: recipe for target 'build/default/production/mcc_generated_files/examples/uart_example.o' failed
make[1]: *** [.build-conf] Error 2
make: *** [.build-impl] Error 2
```
Since I usually program in C++, when it comes to using/implementing for loops, I use the following syntax and would assume everything was Gucci.

```cpp
for(int i = 0; i > 5; i++)
{
  /* Insert more pseudodocode here */
}
```

Well, I was wrong which resulted in the following errors to pop up

```
mcc_generated_files/examples/uart_example.c: In function 'UART_example_READ':
mcc_generated_files/examples/uart_example.c:116:5: error: 'for' loop initial declarations are only allowed in C99 mode
mcc_generated_files/examples/uart_example.c:116:5: note: use option -std=c99 or -std=gnu99 to compile your code
mcc_generated_files/examples/uart_example.c:126:14: error: redefinition of 'i'
mcc_generated_files/examples/uart_example.c:116:14: note: previous definition of 'i' was here
mcc_generated_files/examples/uart_example.c:126:5: error: 'for' loop initial declarations are only allowed in C99 mode
make[2]: *** [build/default/production/mcc_generated_files/examples/uart_example.o] Error 255
```
After spending hours googling the issue, the answer was rigth in front of my face.

I just had to use the following compiler flags :
- -std=c99
- -std=gnu99

Why? Well, this [Stackoverflow thread](https://stackoverflow.com/questions/29338206/error-for-loop-initial-declarations-are-only-allowed-in-c99-mode/29338269) can give a better explanation than I could.

But here's the TL:DR explanation
- Declaring variables inside a FOR loop wasn't a thing until C99 was released (Ironically in 1999)
- You can delcare the counter/variables outside the FOR loop
- Or you could use the -std=c99 flag to tell the compiler you're using the C99 standard and interpret that way

### How to add -std=c99 | -std==gnu99 flag(s)
- In MPLAB X, right click on the project you're working on, which in this case for me UART_Hello_World
![image](https://user-images.githubusercontent.com/39348633/125969890-ade2fc10-07d9-4123-aeb1-ec1c61383e44.png)

- Click on 'Properties'
- Under 'XC16 (Or whatever compiler you're using) click on 'xc16-gcc' 
- Click on 'Generated Command Line' and enter -std=c99 in 'Additional options :'
- Click apply 
![image](https://user-images.githubusercontent.com/39348633/125970543-f0114563-21c9-4126-8375-363aca76757f.png)

## Not out of the woods yet....
So when you clean/build the entire project again, you'll get the following errors

```
nbproject/Makefile-default.mk:196: recipe for target 'build/default/production/mcc_generated_files/traps.o' failed
mcc_generated_files/traps.c: In function 'use_failsafe_stack':
mcc_generated_files/traps.c:84:6: error: 'asm' undeclared (first use in this function)
mcc_generated_files/traps.c:84:6: note: each undeclared identifier is reported only once for each function it appears in
mcc_generated_files/traps.c:84:10: error: expected ';' before 'volatile'
make[2]: *** [build/default/production/mcc_generated_files/traps.o] Error 255
make[2]: *** Waiting for unfinished jobs....
make[2]: Leaving directory 'C:/Users/christophert/Desktop/Coding Projects/MPLAB X/UART_Hello_World'
nbproject/Makefile-default.mk:91: recipe for target '.build-conf' failed
make[1]: Leaving directory 'C:/Users/christophert/Desktop/Coding Projects/MPLAB X/UART_Hello_World'
nbproject/Makefile-impl.mk:39: recipe for target '.build-impl' failed
make[1]: *** [.build-conf] Error 2
make: *** [.build-impl] Error 2

BUILD FAILED (exit value 2, total time: 8s)
```

So if you go to your 'Source Files' and find 'traps.c' and scroll down to line ~84 or just clicking the following link generated in the MPLAB X output window will bring you to the source of the error

![image](https://user-images.githubusercontent.com/39348633/125971180-cfbf23ef-ca17-427a-96c5-f4358b716764.png)

![image](https://user-images.githubusercontent.com/39348633/125971465-41d02ac2-9ebf-408e-82f8-aa9096365232.png)


```
mcc_generated_files/traps.c:84:6: error: 'asm' undeclared (first use in this function)
```

### Why tho? 

Once again, this [Stackoverflow thread](![image](https://user-images.githubusercontent.com/39348633/125971465-41d02ac2-9ebf-408e-82f8-aa9096365232.png)) can explain it better than I can, but here's the TL:DR version if you're just wanting to get the program to run

- Using the -std=c99 compiler option disables some non-standard GCC extensions
  - Like the ```asm``` feature
- [Here's a link to some GCC docs if you're wanting to read them](https://gcc.gnu.org/onlinedocs/gcc/C-Dialect-Options.html#index-std-112)
- If you're using either -std=c99, or the -std=gnu99 command, use ```__asm__``` instead of ```asm```

![image](https://user-images.githubusercontent.com/39348633/126211948-48f8f1b2-d556-4951-916e-aaf8eaa2b5ba.png)

### Don't use 'UART1' when using MCC to generate UART code

Doing so will result with this

```
In file included from mcc_generated_files/examples/uart_example.h:33:0,
                 from main.c:49:
mcc_generated_files/examples/../drivers/uart.h:42:5: error: 'UART1' redeclared as different kind of symbol
C:/Program Files/Microchip/MPLABX/v5.50/packs/Microchip/dsPIC33CH-MP_DFP/1.7.194/xc16/bin/..\support\dsPIC33C\h/p33CH512MP508.h:2348:22: note: previous declaration of 'UART1' was here
nbproject/Makefile-default.mk:172: recipe for target 'build/default/production/main.o' failed
"C:\Program Files\Microchip\xc16\v1.70\bin\xc16-gcc.exe"   mcc_generated_files/traps.c  -o build/default/production/mcc_generated_files/traps.o  -c -mcpu=33CH512MP508  -MP -MMD -MF "build/default/production/mcc_generated_files/traps.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=default  -legacy-libc    -std=c99 -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=c99  -mdfp="C:/Program Files/Microchip/MPLABX/v5.50/packs/Microchip/dsPIC33CH-MP_DFP/1.7.194/xc16"
"C:\Program Files\Microchip\xc16\v1.70\bin\xc16-gcc.exe"   mcc_generated_files/mcc.c  -o build/default/production/mcc_generated_files/mcc.o  -c -mcpu=33CH512MP508  -MP -MMD -MF "build/default/production/mcc_generated_files/mcc.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=default  -legacy-libc    -std=c99 -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=c99  -mdfp="C:/Program Files/Microchip/MPLABX/v5.50/packs/Microchip/dsPIC33CH-MP_DFP/1.7.194/xc16"
make[2]: *** [build/default/production/main.o] Error 255
make[2]: *** Waiting for unfinished jobs....
make[2]: Leaving directory 'C:/Users/christophert/Desktop/Coding Projects/MPLAB X/UART_Hello_World'
nbproject/Makefile-default.mk:91: recipe for target '.build-conf' failed
make[1]: Leaving directory 'C:/Users/christophert/Desktop/Coding Projects/MPLAB X/UART_Hello_World'
nbproject/Makefile-impl.mk:39: recipe for target '.build-impl' failed
make[1]: *** [.build-conf] Error 2
make: *** [.build-impl] Error 2

BUILD FAILED (exit value 2, total time: 5s)
```

### Solution
- Open MCC back up
- Under 'Foundation Servies' click on 'UART' 
- Under the 'Easy Setup' make sure you're using a unique name for the UART stuff versus 'UART1' which is what I did originally....

![image](https://user-images.githubusercontent.com/39348633/126005870-a9f00c19-d1d8-4079-beed-3dcffcaae496.png)

After all that nonsense, you should be able to finally clean/build/compile your program.

![image](https://user-images.githubusercontent.com/39348633/125972447-600d9ef0-944b-4496-aab7-308162695fb2.png)

### Go back to the MPLAB Code Configuratior (MCC) 

![image](https://user-images.githubusercontent.com/39348633/126210598-a055f522-6d84-466e-9a34-2c35cdf9c4e5.png)

And find the 'Pin Manager : Package View'. Find pin(s) 66 and 67, right click on them individually and set 67 as the TX pin, and 66 as the RX pin.

![image](https://user-images.githubusercontent.com/39348633/126210853-cbbf6a3c-0b62-4e66-9d46-371ef7bb9bfa.png)

### Why can we only use RC10/RC11 for the UART communciation despite there being TWO UART ports?

In accordance to the [datasheet](http://ww1.microchip.com/downloads/en/DeviceDoc/DS50002801%20-%20dsPIC33CH%20Curiosity%20Development%20Board%20Users%20Guide.pdf), both pins RC10/RC11 are used for the external Isolated USB-ART port which is what's being used for the MPLAB's Data Visualizer while pin(s) RD0/RD1 AREN'T connected to the Isolated USB-UART port.

![image](https://user-images.githubusercontent.com/39348633/126211728-2c63f335-8a6d-436d-be50-5d6f689f9274.png)

### Which COM port to use?

To see what COM port the discovery board is using, search for your 'Device Manager' and unplug/re-plug the discovery board's Isolated USB_UART port to see if it's being detected or not.

In my case, COM9 is being used

After uploading the code to the board, open up the MPLAB Data Visualizer.

There's a drop down menu in the left most red circle that you'll need to enable both 'Plot raw' and 'Send to terminal'

![image](https://user-images.githubusercontent.com/39348633/126007172-22c98e38-9f9c-467d-90fe-998ba7cd9f99.png)

Once you've enabled both, you SHOULD see the COM port that's being read in the right most circle.

![image](https://user-images.githubusercontent.com/39348633/126007276-9018823c-a7bd-434d-bbf0-b9fdf5fb26b9.png)

If everything is good to go, whenever you type anything in the terminal window below, you should see the ASCII values being plotted in real time as shown below.

![image](https://user-images.githubusercontent.com/39348633/126006144-5c2c8785-4b10-4cf6-b9b1-324ce0fa810a.png)

