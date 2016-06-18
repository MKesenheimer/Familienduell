# Familienduell
Einfache Umsetzung der bekannten Fernsehshow "Familienduell" in C++

# Vorraussetzungen:
- C++ Compiler
- die Grafikausgabe basiert auf SDL, für alle Betriebssysteme erhältlich unter https://www.libsdl.org/download-2.0.php.
- eine zusätzliche Bibliothek SDL2_gfx/SDL2_gfxPrimitives muss installiert werden, zu finden unter https://sourceforge.net/projects/sdl2gfx/ oder im Ordner ./Frameworks (für Mac OSX).

# Steuerung:
Das Spiel wird durch ein Terminal im Hintergrund (nicht sichtbar für das Publikum) gesteuert. Dafür stehen folgende Befehle zur Verfügung:
- start: startet das Spiel
- restart: beginnt das Spiel von neuem
- end: beendet das Spiel
- fullscreen: schält den Fullscreen modus an oder aus
- next: nächste Frage bzw. nächste Folie
- previous: vorige Frage
- set <Group>: setzt die Gruppe, die gerade spielt (A, B)
- reveal <Number>: deckt die Frage mit der Nummer <Number> auf
- wrong <Group>: die Gruppe hat eine falsche Antwort gegeben
- hide <Number>: verdeckt die Frage
- add <Group> <Number>: addiert zu den Punkten der Gruppe eine bestimmte Zahl
- sub <Group> <Number>: subtrahiert -"-
- display <Text>: gibt einen Text auf dem Bildschirm aus
- pigs: zeigt die Schweinchen an (noch nicht vollständig implementiert)
- back: beendet die Textausgabe auf dem Bildschirm oder das Anzeigen der Schweinchen