tdbp
====

Befehl
----

tdbp &ndash; parse einen Satz anhand einer separierten Grammatik und gib alle möglichen Baume aus

Übersicht
----

tdbp&ensp;GRAMMATIK-DATEI&ensp;LEXIKON-DATEI&ensp;SATZ&ensp;BAUM-SPEICHERZIEL

Beschreibung
----
Der Befehl tdbp ruft einen Top-Down-Backtracking-Parser auf, um einem Satz
seine möglichen Strukturen zuzuweisen. Grundlage ist eine separierte, kontextfreie
Grammatik im Prolog-Format. Der Parser sucht mittels Backtracking alle möglichen Parsebäume und speichert sie in einer Datei.

Linksrekursive Regeln kann der Parser nicht verarbeiten, weshalb er sie ignoriert.
Parsebäume auf Grundlage dieser Regeln werden nicht gefunden. Beim
Fund einer linksrekursiven Regel wird eine Warnung ausgegeben.

Dateien und Parameter
----

- GRAMMATIK-DATEI
  - ist eine Textdatei mit einer Definite Clause Grammar in der logischen Sprache Prolog. Sie enthält die Nichtterminal-Regeln einer separierten, kontextfreien Grammatik. Das Startsymbol *s* wird darin, gegebenfalls über Zwischenschritte mit weiteren Nichtterminalen, komplett in einen Satz von Präterminalen übersetzt, für die es keine Produktionsregeln, sondern nur Lexikoneinträge gibt. Auf der rechten Seite einer Produktionsregel können beliebig viele Nichtterminale stehen. Prolog-Kommentare (mit *%* gekennzeichnet) sind erlaubt.
- LEXIKON-DATEI
  - ist eine Textdatei mit einer Definite Clause Grammar in der logischen Sprache Prolog. Sie enthält die Lexikonregeln der separierten Grammatik. Auf der rechten Seite einer Regel steht genau ein Terminal als String. Leere Strings sind nicht als Terminal erlaubt. Prolog-Kommentare (mit *%* gekennzeichnet) sind möglich.
- SATZ
  - ist ein String mit Wörtern, dessen mögliche Strukturen gefunden werden sollen. Der Parser tokenisiert den Satz anhand von Leerzeichen und Zeichensetzung. Groß- und Kleinschreibung müssen den Einträgen im Lexikon entsprechen. Sollte ein Token nicht im Lexikon verzeichnet sein, wird eine Fehlermeldung ausgegeben. Um einen String mit Leerzeichen als Argument in der Kommandozeile zu übergeben, kann er in Anführungszeichen eingeschlossen werden.
- BAUM-SPEICHERZIEL
  - ist eine Textdatei, in die die gefundenen Bäume gespeichert werden. Die Bäume werden als Strings im Wishtree/Showtree-Format ausgegeben, getrennt mit Zeilenumbruch.

Beispiel
----

Datei grammatik.pl:

```
s --> np, vp. % Kommentar

np --> pron.
np -->
    det, n.
vp --> v ,
       np .
```

Datei lexikon.pl:

```
pron --> ich. det --> ein.
n --> 'Beispielsatz'.

% Kommentar
v
 --> bin.
```

Befehl:

`tdbp lexikon.pl grammatik.pl "ich bin ein Beispielsatz" baeume.txt`

Ausgabe in Datei baeume.txt:

```
s/[np/[pron/[-ich]],vp/[v/[-bin],np/[det/[-ein],n/[-Beispielsatz]]]]
```

Autor
----

Matthias Wegel, Universität Potsdam, Oktober 2013
