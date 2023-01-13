ESP-IDF template app
====================

Beispiel des Buchs "Embedded Systems mit RISC-V", dpunkt.verlag

Die Arbeitsweise ist in Abschnitt 4.2.4 Cache beschrieben.
Die im Abschnitt Linker beschriebenen Segmente .noinit und .rtc-noinit werden mit globalen Variablen belegt. Nach Ausgabe und Inkrement dieser Variablen wird ein Reset durchgeführt. So wird gezeigt, dass der Variablenwert über den Reset hinweg erhalten bleibt.

Die erwähnten Linker Scripts memory.ld und sections.ld befinden sich nach dem Build im Unterverzeichnis build/esp-idf/esp_system/ld/ des Projekts.

Der Zugriff auf die Linker-Adressen _rtc_noinit_start und _rtc_noinit_end wird im Beispiel ebenso gezeigt.

Verwendet ein beliebiges Board mit ESP32-C3 Mikrocontroller.

Siehe auch die [Webseite zum Buch](https://ritschel.at/buch-embedded-systems-auf-den-punkt-gebracht/).

*The code of this project is in the Public Domain (or CC0 licensed, at your option).
Unless required by applicable law or agreed to in writing, this
software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.*
