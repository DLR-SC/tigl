
$Id: README.txt 2120 2009-05-13 13:32:28Z litz_ma $

Anmerkungen und Informationen zur TIGL-Bibliothek
=================================================


Ben�tigte Softwarekomponenten und Einstellungen zur Verwendung von TIGL unter MS Windows XP
-------------------------------------------------------------------------------------------

- OpenCasacade: 
  
  Download unter http://www.opencascade.org oder �ber die CPACSIntegration-Teamsite. Installation
  �ber das Setup-Programm von OpenCascade. Wichtig ist, dass man bei der w�hrend der
  Installation gestellten Frage, ob die Benutzerumgebung angepasst werden soll, mit "Ja"
  antwortet. Hierdurch werden wichtige Umgebungsvariablen f�r die Verwendung von OpenCasacade
  gesetzt und die OpenCascade-dll's werden mit in PATH aufgenommen. Hierdurch wird auch
  die Umgebungsvariable CASROOT gesetzt, die auf das OpenCascade-Installationsverzeichnis
  zeigt und beim Build-Prozess f�r die Referenzierung von includes und libs verwendet werden
  kann.

- TIXI:

  Die TIXI-Bibliothek muss zusammen mit den zugeh�rigen libxml2 Bibliotheken und Include-Dateien
  installiert sein. 

Evtl. ist es sinnvoll ein Verzeichnis C:\TIVA\local mit den Unterverzeichnissen lib und include
anzulegen, in die alle ben�tigten includes und libs (ausser OpenCascade) hineinkopiert werden:

C:\TIVA\local
C:\TIVA\local\include
C:\TIVA\local\lib

Weiterhin hilfreich ist es dann, das Verzeichnis C:\TIVA\local\lib mit in die Umgebungsvariable PATH
aufzunehmen. Hier liegen dann n�mlich neben statischen Bibliotheken auch notwendige dll's, die
bei der Ausf�hrung von selbst erstellten Programmen ben�tigt werden. Umgebungsvariable k�nnen in
Windows XP unter "Start->Systemsteuerung->System->Erweitert->Umgebungsvariable" bearbeitet werden.

- Wichtige Einstellungen f�r das Arbeiten mit Microsoft Visual C++.net:

  Beim Erstellen einer Anwendung, die die TIGL-Bibliothek verwendet, muss der include-Pfad
  des Compilers den Pfad zu tigl.h und tixi.h enthalten. Mit obigem Vorschlag also 

      "C:/TIVA/local/include"

  In Visual C++.net kann der Include-Pfad unter den "Projekteigenschaften->C/C++->
  Allgemein->Zus�tzliche Includeverzeichnisse" eingestellt werden.



  Der Linker muss als Pfad den Pfad zu den OpenCascade-Bibliotheken und der tixi.lib bzw. tigl.lib
  enthalten. Mit obigem Vorschlag w�re dies 

      "$(CASROOT)/win32/lib";C:/TIVA/local/lib"

  In Visual C++.net kann der Linker-Pfad unter den "Projekteigenschaften->Linker->
  Allgemein->Zus�tzliche Bibliotheksverzeichnisse" eingestellt werden.


  Folgende Bibliotheken m�ssen beim Erstellen einer TIGL-Anwendung hinzugelinkt werden:

      tigl.lib TIXI.lib libxml2.lib zdll.lib iconv.lib TKVrml.lib TKStl.lib TKBRep.lib TKIGES.lib TKBO.dll \
      TKShHealing.lib TKSTEP.lib TKXSBase.lib TKShapeSchema.lib FWOSPlugin.lib PTKernel.lib TKBool.lib \
      TKCAF.lib TKCDF.lib TKDraw.lib TKernel.lib TKFeat.lib TKFillet.lib TKG2d.lib TKG3d.lib TKGeomAlgo.lib \
      TKGeomBase.lib TKHLR.lib TKMath.lib TKOffset.lib TKPCAF.lib TKPrim.lib TKPShape.lib TKService.lib TKTopAlgo.lib \
      TKV2d.lib TKV3d.lib TKBO.lib

  Diese Bibliotheken k�nnen dem Linker unter "Projekteigenschaften->Linker->Eingabe->
  Zus�tzliche Abh�ngigkeiten" mitgeteilt werden.

  Dem Linker muss unter Umst�nden mitgeteilt werden, dass er verschiedene Bibliotheken nicht hinzu linken soll:

      libc;msvcrtd

  Im Wesentlichen wird dies wohl immer libc bzw. libcd (Debug-Version der libc) sein. Eingestellt
  werden kann dies unter "Projekteigenschaften->Linker->Eingabe->Bibliothek ignorieren".



  Ganz wichtig: Die hinzugelinkte Runtime-Bibliothek muss eine der Multithreaded-Dll Versionen sein.
  Andernfalls wird das Programm zwar gelinkt, aber es kommt sp�ter zu unerkl�rlichem Programmverhalten
  bzw. zu Programmabst�rzen. Einstellen kann man die Runtime-Bibliothek unter den "Projekteigenschaften->
  C/C++->Codeerstellung->Laufzeitbibliothek". G�ltig ist hier "/MD" oder "/MDd" (Debug-Version). 
  Grunds�tzlich muss die Einstellung der Runtime-Bibliothek bei allen Bibliotheken, die zu einem Programm
  hinzugelinkt werden, bei deren Erstellung die gleiche gewesen sein. D.h. beispielsweise, dass auch
  die TIXI-Bibliothek mit einer der Einstellungen "/MD" oder "/MDd" erstellt worden sein muss. Welche der
  beiden Einstellungen hierbei zum Einsatz kam ist belanglos. Wichtig ist nur, dass es sich um eine der
  multithreaded dll's handelt.


  Das mitgelieferte Beispielprogramm TIGLDemo enth�lt eine Visual C++.net und Visual C++ 6.0 Projektdatei, 
  die bis auf die anzupassenden Pfade f�r die includes und libs alle n�tigen Einstellungen enth�lt. Diese kann
  als Grundlage f�r eigene Projekdateien verwendet werden. Evtl. muss hier noch explizit zu den zus�tzlichen
  Bibliotheken in den Linker-Einstellungen noch die "tigl.lib" hinzugef�gt werden.



  