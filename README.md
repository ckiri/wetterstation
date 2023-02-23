# wetterstation

Ein Microcontrollerproject im Zuge des Kurses **Signalverarbeitung 2**.

Die Wetterstation nimmt Daten über Sensoren auf und veröffentlicht diese
auf einer publisher-subscriber Middleware, dem MQTT-Broker. Hierüber Können
andere Systeme, beliebige Clients das Topic abonnieren und Nachrichten der
Wetterstation empfangen. Es wird ein Digitaler Sensor eingelesen welcher
Temperatur und Luftfeuctigkeit misst, sowie ein Analoger Sensor welcher
für die Detektierung von Rauch zuständig ist.

Für genauere Informationen lohnt sich ein Blick in die [Dokumentation](https://github.com/ckiri/wetterstation-dokumentation)
oder in den [Sourcecode](https://github.com/ckiri/wetterstation/tree/main/src).

