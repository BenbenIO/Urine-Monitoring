# Urine-Monitoring

<img src="/images/mini.JPG" width="50">

This project is part of my master thesis in KEIO university, concerning the development of an automatic urine monitoring device:
"Urine analysis is still a challenge to bring home even if urine is a great bio-marker, and home care system are rising1.
This paper demonstrates an automatic urine monitoring device using the capillarity effect inside a canal and cross-selectivity principle to create a modular analysis. 
We connected our device with an ESP32 module to bring the daily result to a healthcare cloud for further analysis. 
The developed device was tested on simulated urine and successfully determined its basic composition."
More information can be found [HERE](https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=8374315). This repository will briefly explain the device principle, and architecture.

# Principle
The idea of the device is to harvest the patient urine directly inside the toilet automatically. This is achieved by capillary effect which occurs spontaneously and doesn't need any energy.
Once the urine gets into the canal, I analyze it with conductivity and absorbance measurement. The extracted features are then send to a healthcare server.
The server create a medical database based on the urine analysis, patient information and other data from internet.

The first target of the device is to prevent heatwave related accident (hospitalization and death). 
To do so, I try to estimate the urine quality with the previous sensors, and heatwave indicator forecast ([HERE](https://github.com/BenbenIO/wbgt))

<img src="/images/device.JPG" width="250"> <img src="/images/capillarity.JPG" width="250">

<img src="/images/conduc.JPG" width="250"> <img src="/images/abs.JPG" width="250">

# Fabrication
The canal was fabricated with molded PDMS in which we introduce two electrodes. Concerning the absorbance measurement, the setup consisted of an LED on one side and a photoresistor on the other side.
All the support and case were 3D printed. For signal acquisition and communication with used the [ESP32 devkitC] (https://www.espressif.com/en/products/hardware/esp32-devkitc/overview).
Which is a low power 2.4 GHz Wi-Fi and Bluetooth board, equipped with 12 bit ADC and other interesting features. 
Finally, the HealthCare server was a simple TCP socket on a virtual box hosting DEBIAN distribution. The communication between the server and the ESP32(client) was done with Wi-Fi over a TCP socket.


<img src="/images/D1.PNG" width="250"> <img src="/images/D2.JPG" width="250">  <img src="/images/D3.JPG" width="250">
<img src="/images/D4.JPG" width="250"> <img src="/images/D5.JPG" width="250">

# Code
The ESP programming was done in C programming. The program check in there is a new urine sample in the toilet. This is achieved with a volume-flow sensor, it's resistivity change when the patient urinate in the toilet.
This sensor value is check with an interrupt, and a variation trigger the measurement process. Then the device does the absorbance measurement and the conductivity measurement inside the canal.
When the analysis is finished, the ESP32 create the message, connect to the server by opening a TCP socket and transmit the message. Once the server replies no error message, the device go to a standby mode to clean the canal.
<img src="/images/diagram.JPG" width="250">
Since the device doesn't have a screen I create a LED color code. 
On the server side the programming was done with python, and consist on several script. 
* to listen the device on a TCP socket, received an extract the message. 
* to collect the weather data from internet.
* to sum-up the different database and provide a graphical user interface for visualization (with Tkinter) 

<img src="/images/GUI1.PNG" width="250"> <img src="/images/GUI2.PNG" width="250">

Do not hesitate in you have any question or advice :)


