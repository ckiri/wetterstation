# wetterstation

Ein Microcontrollerproject im Zuge des Kurses **Signalverarbeitung 2**.

Die Wetterstation nimmt Daten über Sensoren auf und veröffentlicht diese
auf einer publisher-subscriber Middleware, dem MQTT-Broker. Hierüber Können
andere Systeme, beliebige Clients das Topic abonnieren und Nachrichten der
Wetterstation empfangen. Es wird ein Digitaler Sensor eingelesen welcher
Temperatur und Luftfeuctigkeit misst, sowie ein Analoger Sensor welcher
für die Detektierung von Rauch zuständig ist.

Für genauere Informationen lohnt sich ein Blick in die [Dokumentation](https://github.com/ckiri/wetterstation/tree/main/docu)
oder in den [Sourcecode](https://github.com/ckiri/wetterstation/tree/main/src).
***
Um die Dokumentation compilen zu können wird eine LaTeX Distribution
benötigt. Zu empfehlen ist, sich das Meta Package `texlive-most` zu installieren.

Unter Arch-Linux:
```sh
sudo pacman -S texlive-most
```
Im Ordner dann das Dokument `wetterstation.tex` compilen:
```sh
pdflatex wetterstation.tex
```
**Wichtig:** Das compilen muss 2x durchgeführt werden da für
das Inhaltsverzeichis *kreuzreferenziert* werden muss.
