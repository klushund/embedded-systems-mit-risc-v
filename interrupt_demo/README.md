interrupt_demo app
==================

Beispiel des Buchs "Embedded Systems mit RISC-V", dpunkt.verlag

Die Arbeitsweise ist in Kapitel 6 Interrupts und Exceptions beschrieben.

Die Systick-Funktionalität wurde mit einem General Purpose Timer 
(siehe https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/api-reference/peripherals/gptimer.html)
implementiert.

Eine LED an GPIO4 wird bei Betätigung des Tasters an GPIO6 mit Interruptauslösung geschaltet.
Zusätzlich wird über den General Purpose Timer ermittelte Zeit in der Konsole ausgegeben.
 
Über die Konfiguration kann eingestellt werden, ob das Beispiel den Tastendruck über eine globale Variable
(CALLBACK_NONE), einen statischen Callback (CALLBACK_STATIC) oder einen dynamischen Callback (CALLBACK_DYNAMIC)
verarbeiten soll. 

Im Gegensatz zur Beschreibung im Buch wurde das Beispiel folgende Komponenten aufgeteilt:
* systick: für die Systick-Funktionalität mit General Purpose Timer 
* led: zur Ansteuerung der LED
* button: zur Ansteuerung des Tasters und Aufruf des Callbacks 
* main: mit der Hauptapplikation, die auf den Tastendruck reagiert

Das Beispiel verwendet die Funktionen des ESP-IDF für den Zugriff auf die Peripherie.

Siehe auch die Beispiele switch_led und leds_and_buttons für den Zugriff auf die Peripherie.

Verwendet ein beliebiges Board mit ESP32-C3 Mikrocontroller.

Siehe auch die [Webseite zum Buch](https://ritschel.at/buch-embedded-systems-auf-den-punkt-gebracht/).

*The code of this project is in the Public Domain (or CC0 licensed, at your option).
Unless required by applicable law or agreed to in writing, this
software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.*
