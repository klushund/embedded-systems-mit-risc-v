sdcard_demo app
===============

Beispiel des Buchs "Embedded Systems mit RISC-V", dpunkt.verlag

Es zeigt die Ansteuerung einer SD-Karte über SPI. Ein entsprechender SD-Kartenadapter muss dafür 
an die Pins 5 (MOSI), 6 (MISO), 4 (SCK) und 7 (CS) angeschlossen werden. Micro-SD-Kartenadapter
sind auch geeignet. Ein beliebiges Board mit ESP32-C3 Mikrocontroller ist geeignet.

Beim ausgewählten SD-Kartenadapter muss auf die Spannung geachtet werden. Der Adapter von
AZ-Delivery (https://www.amazon.de/gp/product/B077MCQS9P) arbeitet mit einer Versorgungsspannung von
5V. Hier wird wie folgt verbunden (die Pins sind auf der Platine beschriftet):
ESP32-C3 <->  SDCard-Adapter
* GPIO7  <->  CS 
* GPIO5  <->  MOSI
* GPIO6  <->  MISO
* GPIO4  <->  SCK
* 5V     <->  VCC
* GND    <->  GND

Als Basis diente das ESP-IDF Beispiel sdspi, das auf die Basisfunktionalität reduziert wurde.

Siehe auch die [Webseite zum Buch](https://ritschel.at/buch-embedded-systems-auf-den-punkt-gebracht/).

*The code of this project is in the Public Domain (or CC0 licensed, at your option).
Unless required by applicable law or agreed to in writing, this
software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.*
