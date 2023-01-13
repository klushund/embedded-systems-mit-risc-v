switch_led app
==============

Beispiel des Buchs "Embedded Systems mit RISC-V", dpunkt.verlag

Die Arbeitsweise ist in Abschnitt 5.4 LED schalten beschrieben.
Im Beispiel werden zwei LEDs an GPIO4 und GPIO5 zyklisch an- und ausgeschaltet. 
Das Beispiel greift direkt auf die Peripherie zu. In der Konfiguration kann festgelegt 
werden, ob dies über Bitmaskierung oder ein Bitfield erfolgt.

Die LEDs werden in Beispiel leds_and_button über die Funktionen des ESP-IDF angesteuert. 
Zusätzlich wird dort auf einen Tastendruck reagiert.

Verwendet ein beliebiges Board mit ESP32-C3 Mikrocontroller.

Siehe auch die [Webseite zum Buch](https://ritschel.at/buch-embedded-systems-auf-den-punkt-gebracht/).

*The code of this project is in the Public Domain (or CC0 licensed, at your option).
Unless required by applicable law or agreed to in writing, this
software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.*
