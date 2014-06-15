////////////////////////////////////////////////////////////////////////////////
// Matthias Wegel, Oktober 2013
//
// Getestete Compiler:
// Microsoft 32bit C/C++-Optimierungscompiler Version 16.00.30319.01
//   mit Boost Version 1.54.0
//   unter Microsoft Windows XP Professional 32bit Version 5.1.2600
// g++ Version 4.6.3-1ubuntu5
//   mit Boost Version 1.48.0.2
//   unter Ubuntu 12.04.2 LTS, Precise Pangolin
// g++ Version 4.7.3-1ubuntu10
//   mit Boost Version 1.49.0.1
//   unter Ubuntu 13.04 64bit
//
// main.cpp
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include "../include/tdbp.hpp"

int main(int argc, const char* argv[])
{
	if(argc != 5)
	{
		// Keine gültige Anzahl von Parametern
		// Verwendungsinformation ausgeben
		std::cerr << "Top-Down-Backtracking-Parser\n\n"
		<< "Verwendung: tdbp <Grammatik> <Lexikon> <Satz> <Baum-Ziel>\n"
		<< "<Grammatik>: eine Prolog-DCG-Datei mit Produktionsregeln\n"
		<< "<Lexikon>: eine Prolog-DCG-Datei mit Lexikonregeln\n"
		<< "<Satz>: ein String, der tokenisiert und geparst werden soll\n"
		<< "<Baum-Ziel>: Speicherort fuer Textdatei mit Baeumen\n";
		// Programm beenden
		exit(1);
	}

	// Erzeuge Instanz des Mustererkenners auf Basis von Grammatik und Lexikon
	TDBParser parser(argv[1],argv[2]);

	// Parse den Satz
	parser.parse(argv[3],argv[4]);
}
