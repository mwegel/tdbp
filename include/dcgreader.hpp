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
// dcgreader.hpp
// Klasse DCGReader zum Einlesen einer separierten Prolog-DCG
////////////////////////////////////////////////////////////////////////////////

#ifndef __WEGEL_TDBP_DCGREADER_HPP__
#define __WEGEL_TDBP_DCGREADER_HPP__

#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <vector>
#include <map>
#include <cctype>
#include <new>
#include "globaltypes.hpp"

/// Automat zum Einlesen einer Prolog-DCG
/** Endlicher Automat, der entweder die Produktionsregeln oder das Lexikon
  * einer separierten Grammatik im Prolog-DCG-Format akzeptiert, und Regel
  * für Regel zur Weiterverarbeitung ausgibt.
  */
class DCGReader
{
	private:

	////////////////////////////////////////////////////////////////////////////
	// Typen

	/// Zeichenklassen, die der Automat unterscheidet
	enum CharType {ALPHA, SPACE, HYPHEN, GREATER, COMMA, DOT, QUOTE, ESCAPE,
	               OTHER, END, INVALID};

	/// Zustand des Automaten
	typedef int State;

	/// Map mit den Übergängen von einem Zustand aus
	typedef std::map<CharType,State> Transitions;

	/// Map mit Übergangs-Maps für alle Zustände
	typedef std::map<State,Transitions> DeltaMap;

	/// Menge von Zuständen
	typedef std::set<State> StateSet;

	public:

	////////////////////////////////////////////////////////////////////////////
	// Öffentliche Funktionen

	/// Konstruktor eines Automaten zum Lesen einer Grammatik
	/** Initialisiert den Automaten zum Lesen der Produktionsregeln der
	  * separierten Grammatik.
	    @param prologfile Dateiname der DCG
	  */
	DCGReader(const std::string prologfile)
	{
		lexicon = false;
		init_filestream(prologfile);
		init_grammar_reader();
	}

	/// Konstruktor eines Automaten zum Lesen von Lexikon oder Grammatik
	/** Initialisiert den Automaten wahlweise zum Lesen des Lexikons oder der
	  * Produktionsregeln der separierten Grammatik.
	    @param prologfile Dateiname der DCG
	    @param make_lexicon_reader Für Lexikon initialisieren ja/nein
	  */
	DCGReader(const std::string prologfile, const bool make_lexicon_reader)
	{
		lexicon = make_lexicon_reader;
		init_filestream(prologfile);
		if(make_lexicon_reader)
		{
			init_lexicon_reader();
		}
		else
		{
			init_grammar_reader();
		}
	}

	/// Destruktor
	/** Gibt den verwendeten Speicherplatz im Heap wieder frei.
	  */
	~DCGReader()
	{
		// Buffer löschen
		delete[] buffer;
	}

	/// Lässt den Grammatik-Automaten auf einer Zeichenkette laufen
	/** Startet den Automaten an der aktuellen Position im Input, liest eine
	  * Regel und gibt sie als Paar aus. Wenn keine weitere Regel zu finden
	  * ist, leere Ausgabe, bei unerwartetem Zeichen Abbruch.
	    @return Paar aus Symbol (LHS) und Liste von Symbolen (RHS)
	    @warning Nur nach Initialisierung eines Grammatik-Automaten nutzen
	  */
	std::pair<Symbol,SymbolList> run_grammar()
	{
		// Fange unpassende Aufrufe ab und gib leere Regel aus
		if(lexicon)
		{
			std::cerr << "Aufruf der Funktion run_grammar() nicht möglich: "
			          << "DCGReader wurde für Lexikon instantiiert, "
			          << "bitte run_lexicon() verwenden.\n";
			return std::pair<Symbol,SymbolList>(Symbol(""),SymbolList());
		}

		// Fange Aufrufe ab, wenn der Input fertig durchlaufen ist
		if(finished())
		{
			std::cerr << "Durchlauf des Automaten nicht möglich: "
			          << "Ende des Inputs bereits erreicht.\n";
			return std::pair<Symbol,SymbolList>(Symbol(""),SymbolList());
		}

		// Instantiiere Symbole, die gefüllt werden sollen
		Symbol lhs("");
		SymbolList rhs;
		rhs.push_back(Symbol(""));
		SymbolList::iterator rhs_pos = rhs.begin();

		// Beginne im Startzustand
		State q = start;

		// Schleife über die Zustände
		while(stop.find(q) == stop.end())
		{
			// Überspringe Prolog-Kommentare
			skip_comment();

			// Suche Übergang mit aktuellem Zeichen
			q = search_transition(q);

			// In einigen Zuständen besondere Behandlung des Inputs
			if(lhs_char.find(q) != lhs_char.end())
			{
				// Verkette Zeichen zum Symbol der linken Regelseite
				lhs += *curr_char;
			}
			else if(rhs_char.find(q) != rhs_char.end())
			{
				// Verkette Zeichen zu einem Symbol der rechten Regelseite
				*rhs_pos += *curr_char;
			}
			else if(next_rhs_symbol.find(q) != next_rhs_symbol.end())
			{
				// Beginne ein weiteres Symbol der rechten Regelseite
				rhs.push_back(std::string(""));
				++rhs_pos;
			}

			// Rücke ein Zeichen weiter
			next_char();
		}

		// Gib die gelesene Regel aus
		return std::pair<Symbol,SymbolList>(lhs,rhs);
	}

	/// Lässt den Lexikon-Automaten auf einer Zeichenkette laufen
	/** Startet den Automaten an der aktuellen Position im Input, liest eine
	  * Regel und gibt sie als Paar aus. Wenn keine weitere Regel zu finden
	  * ist, leere Ausgabe, bei unerwartetem Zeichen Abbruch.
	    @return Paar aus Token (RHS) und Symbol (LHS)
	    @warning Nur nach Initialisierung eines Lexikon-Automaten nutzen
	  */
	std::pair<Token,Symbol> run_lexicon()
	{
		// Fange unpassende Aufrufe ab und gib leere Regel aus
		if(!lexicon)
		{
			std::cerr << "Aufruf der Funktion run_lexicon() nicht möglich: "
			          << "DCGReader wurde für Grammatik instantiiert, "
			          << "bitte run_grammar() verwenden.\n";
			return std::pair<Token,Symbol>(Token(""),Symbol(""));
		}

		// Fange Aufrufe ab, wenn der Input fertig durchlaufen ist
		if(finished())
		{
			std::cerr << "Durchlauf des Automaten nicht möglich: "
			          << "Ende des Inputs bereits erreicht.\n";
			return std::pair<Token,Symbol>(Token(""),Symbol(""));
		}

		// Instantiiere Symbole, die gefüllt werden sollen
		Symbol sym("");
		Token tok("");

		// Beginne im Startzustand
		State q = start;

		// Schleife über die Zustände
		while(stop.find(q) == stop.end())
		{
			// Überspringe Prolog-Kommentare
			skip_comment();

			// Suche Übergang mit aktuellem Zeichen
			q = search_transition(q);

			// In einigen Zuständen besondere Behandlung des Inputs
			if(lhs_char.find(q) != lhs_char.end())
			{
				// Verkette Zeichen zum Symbol
				sym += *curr_char;
			}
			else if(rhs_char.find(q) != rhs_char.end())
			{
				// Verkette Zeichen zum Token
				tok += *curr_char;
			}
			else if(rhs_char_esc.find(q) != rhs_char_esc.end())
			{
				// Entferne Escape-Zeichen und verkette Zeichen zum Token
				tok.erase(--tok.end());
				tok += *curr_char;
			}

			// Rücke ein Zeichen weiter
			next_char();
		}

		// Gib die gelesene Regel aus
		return std::pair<Token,Symbol>(tok,sym);
	}

	/// Gibt aus, ob der Automat am Ende des Inputs angelangt ist
	/** Prüft, ob die aktuelle Position des Automaten im Input das Ende der
	  * Datei ist.
	    @return Am Ende des Inputs ja/nein
	  */
	bool finished() const
	{
		return curr_char >= buffer + length;
	}

	private:

	////////////////////////////////////////////////////////////////////////////
	// Daten

	bool lexicon;				///< Instanz ist Lexikon-Reader, ja/nein
	std::string filename;		///< Dateiname der eingelesenen Prolog-Datei
	unsigned length;			///< Länge der Datei in Zeichen
	char* buffer;				///< Buffer, in den die Datei eingelesen wird
	const char* curr_char;		///< Zeichen, bei dem der Automat aktuell steht
	unsigned linecount;			///< Zeilennummer des aktuellen Zeichens
	unsigned colcount;			///< Spaltennummer des aktuellen Zeichens

	DeltaMap delta;				///< Map mit den Übergängen des Automaten
	State start;				///< Startzustand des Automaten
	StateSet stop;				///< Zustände, bei denen der Automat anhält
	StateSet lhs_char;			///< Zustände, b. d. LHS-Symbol gelesen wird
	StateSet rhs_char;			///< Zustände, b. d. RHS-Symbol gelesen wird
	StateSet rhs_char_esc;		///< Zustände mit escaptem Zeichen der RHS
	StateSet next_rhs_symbol;	///< Zustände mit einem weiteren RHS-Symbol

	////////////////////////////////////////////////////////////////////////////
	// Private Funktionen

	/// Liest die Prolog-Datei in einen Buffer
	/** Öffnet die Prolog-DCG und liest sie in einen Buffer. Beginnt beim
	  * ersten Zeichen.
	    @param prologfile Dateiname der DCG
	  */
	void init_filestream(const std::string prologfile)
	{
		// Speichere Dateinamen intern für Fehlermeldungen
		filename = prologfile;

		// Erzeuge einen Input-Filestream aus der Datei
		std::ifstream dcg_stream(prologfile.c_str());
		if(dcg_stream)
		{
			// Ermittle Länge der Datei
			dcg_stream.seekg(0,dcg_stream.end);
			length = dcg_stream.tellg();
			dcg_stream.seekg(0,dcg_stream.beg);

			// Stelle einen Buffer entsprechender Länge bereit
			buffer = new (std::nothrow) char[length+1];
			if(buffer == 0)
			{
				std::cerr << "Speicherfehler. Datei '" << prologfile
				          << "' konnte nicht eingelesen werden.\n";
				exit(1);
			}

			// Lies die Datei ein
			dcg_stream.read(buffer,length);

			// Aktuelles Zeichen ist das erste Zeichen der Datei
			curr_char = buffer;
			linecount = 1;
			colcount = 1;
		}
		else
		{
			// Stream fehlgeschlagen, Fehlermeldung und Abbruch
			std::cerr << "Datei '" << prologfile
			          << "' konnte nicht geoeffnet werden.\n";
			exit(1);
		}
	}

	/// Initialisiert den Automaten zum Lesen einer Grammatik
	/** Initialisiert die Zustände und Übergänge des Automaten, der den Teil
	  * der separierten Grammatik mit den Produktionsregeln akzeptiert.
	  */
	void init_grammar_reader()
	{
		// Definiere für die einzelnen Zustände, welche Aliasse sie haben oder
		// welchen Mengen sie angehören, sowie ihre Übergänge

		// 0: Nichts oder nur Whitespace gelesen
		start = 0;
		Transitions t0;
		t0[SPACE] = 0;
		t0[ALPHA] = 1;
		t0[END] = 12;
		delta[0] = t0;

		// 1: Buchstabe der linken Regelseite gelesen
		lhs_char.insert(1);
		Transitions t1;
		t1[ALPHA] = 1;
		t1[SPACE] = 2;
		t1[HYPHEN] = 3;
		delta[1] = t1;

		// 2: Whitespace nach der linken Regelseite gelesen
		Transitions t2;
		t2[SPACE] = 2;
		t2[HYPHEN] = 3;
		delta[2] = t2;

		// 3: Ersten Strich des Pfeils gelesen
		Transitions t3;
		t3[HYPHEN] = 4;
		delta[3] = t3;

		// 4: Zweiten Strich des Pfeils gelesen
		Transitions t4;
		t4[GREATER] = 5;
		delta[4] = t4;

		// 5: Pfeil vollständig gelesen
		Transitions t5;
		t5[SPACE] = 6;
		t5[ALPHA] = 7;
		delta[5] = t5;
		/* Diese beiden wären zusammenfassbar */
		// 6: Whitespace nach Regelpfeil gelesen
		Transitions t6;
		t6[SPACE] = 6;
		t6[ALPHA] = 7;
		delta[6] = t6;

		// 7: Buchstabe der rechten Regelseite gelesen
		rhs_char.insert(7);
		Transitions t7;
		t7[ALPHA] = 7;
		t7[SPACE] = 8;
		t7[COMMA] = 9;
		t7[DOT] = 11;
		delta[7] = t7;

		// 8: Whitespace nach Symbol der rechten Regelseite gelesen
		Transitions t8;
		t8[SPACE] = 8;
		t8[COMMA] = 9;
		t8[DOT] = 11;
		delta[8] = t8;

		// 9: Komma gelesen
		next_rhs_symbol.insert(9);
		Transitions t9;
		t9[SPACE] = 10;
		t9[ALPHA] = 7;
		delta[9] = t9;

		// 10: Whitespace nach Komma gelesen
		Transitions t10;
		t10[SPACE] = 10;
		t10[ALPHA] = 7;
		delta[10] = t10;

		// 11: Punkt gelesen
		Transitions t11;
		t11[SPACE] = 12;
		t11[END] = 12;
		delta[11] = t11;

		// 12: Gar nichts oder vollständige Regel gelesen
		stop.insert(12);
	}

	/// Initialisiert den Automaten zum Lesen eines Lexikons
	/** Initialisiert die Zustände und Übergänge des Automaten, der den Teil
	  * der separierten Grammatik mit den Lexikonregeln akzeptiert.
	  */
	void init_lexicon_reader()
	{
		// Definiere für die einzelnen Zustände, welche Aliasse sie haben oder
		// welchen Mengen sie angehören, sowie ihre Übergänge

		// 0: Nichts oder nur Whitespace gelesen
		start = 0;
		Transitions t0;
		t0[SPACE] = 0;
		t0[ALPHA] = 1;
		t0[END] = 14;
		delta[0] = t0;

		// 1: Buchstabe der linken Regelseite gelesen
		lhs_char.insert(1);
		Transitions t1;
		t1[ALPHA] = 1;
		t1[SPACE] = 2;
		t1[HYPHEN] = 3;
		delta[1] = t1;

		// 2: Whitespace nach der linken Regelseite gelesen
		Transitions t2;
		t2[SPACE] = 2;
		t2[HYPHEN] = 3;
		delta[2] = t2;

		// 3: Ersten Strich des Pfeils gelesen
		Transitions t3;
		t3[HYPHEN] = 4;
		delta[3] = t3;

		// 4: Zweiten Strich des Pfeils gelesen
		Transitions t4;
		t4[GREATER] = 5;
		delta[4] = t4;

		// 5: Pfeil vollständig gelesen
		Transitions t5;
		t5[SPACE] = 6;
		t5[QUOTE] = 7;
		t5[ALPHA] = 8;
		delta[5] = t5;
		/* Diese beiden wären zusammenfassbar */
		// 6: Whitespace nach Regelpfeil gelesen
		Transitions t6;
		t6[SPACE] = 6;
		t6[QUOTE] = 7;
		t6[ALPHA] = 8;
		delta[6] = t6;

		// 7: Öffnendes Anführungszeichen gelesen
		Transitions t7;
		t7[ALPHA] = 8;
		t7[OTHER] = 8;
		t7[ESCAPE] = 9;
		delta[7] = t7;

		// 8: Zeichen eines Tokens gelesen
		rhs_char.insert(8);
		Transitions t8;
		t8[ALPHA] = 8;
		t8[OTHER] = 8;
		t8[ESCAPE] = 9;
		t8[QUOTE] = 11;
		t8[DOT] = 13;
		delta[8] = t8;

		// 9: Escape-Zeichen im Token gelesen
		rhs_char.insert(9);
		Transitions t9;
		t9[ALPHA] = 8;
		t9[OTHER] = 8;
		t9[ESCAPE] = 9;
		t9[QUOTE] = 10;
		delta[9] = t9;

		// 10: Anführungszeichen nach Escape gelesen
		rhs_char_esc.insert(10);
		Transitions t10;
		t10[ALPHA] = 8;
		t10[OTHER] = 8;
		t10[ESCAPE] = 9;
		t10[QUOTE] = 11;
		delta[10] = t10;

		// 11: Schließendes Anführungszeichen gelesen
		Transitions t11;
		t11[SPACE] = 12;
		t11[DOT] = 13;
		delta[11] = t11;
		/* Diese beiden wären zusammenfassbar */
		// 12: Whitespace nach rechter Regelseite gelesen
		Transitions t12;
		t12[SPACE] = 12;
		t12[DOT] = 13;
		delta[12] = t12;

		// 13: Punkt gelesen
		Transitions t13;
		t13[SPACE] = 14;
		t13[END] = 14;
		delta[13] = t13;

		// 14: Gar nichts oder vollständige Regel gelesen
		stop.insert(14);
	}

	/// Liefert zu einem Zeichen die interne Zeichenklasse
	/** Ordnet ein Zeichen der DCG einer der internen Zeichenklassen zu.
	    @param c Zeichen
	    @return Zeichenklasse
	  */
	CharType classify(const char c) const
	{
		CharType c_type = INVALID;

		if(finished())		c_type = END;
		else if(isalpha(c))	c_type = ALPHA;
		else if(isspace(c))	c_type = SPACE;
		else if(c == '-')	c_type = HYPHEN;
		else if(c == '>')	c_type = GREATER;
		else if(c == ',')	c_type = COMMA;
		else if(c == '.')	c_type = DOT;
		else if(c == '\'')	c_type = QUOTE;
		else if(c == '\\')	c_type = ESCAPE;
		else if(isgraph(c))	c_type = OTHER;

		return c_type;
	}

	/// Überspringt einen Prolog-Kommentar
	/** Prüft, ob das aktuelle Zeichen der DCG einen Kommentar markiert.
	  * Überspringt diesen im positiven Falle bis zum folgenden Zeilenumbruch.
	  */
	void skip_comment()
	{
		if(*curr_char == '%')
		{
			while(*curr_char != '\n')
			{
				++curr_char;
			}
			// Aktuelles Zeichen ist jetzt beabsichtigterweise
			// der Zeilenumbruch: Kommentare erlaubt Prolog nur dort,
			// wo auch Whitespace stehen darf
		}
	}

	/// Sucht einen Übergang von einem Zustand aus
	/** Sucht von einem Zustand aus einen Übergang mit dem aktuellen Zeichen
	  * und gibt den so erreichten Zustand aus. Bei nicht gefundenem Übergang
	  * Abbruch wegen unerwartetem Zeichen in der DCG.
	    @param q Zustand vorher
	    @return Zustand nachher
	  */
	State search_transition(State q)
	{
		// Suche Übergang mit aktuellem Zeichen
		CharType c_type = classify(*curr_char);
		Transitions& trans = delta[q];
		Transitions::const_iterator t = trans.find(c_type);
		if(t != trans.end())
		{
			// Gib den gefundenen Übergang aus
			return t->second;
		}
		else
		{
			// Kein Übergang gefunden: unerwartetes Zeichen

			// Gib eine Fehlermeldung aus
			std::cerr << "Einlesen der Datei '" << filename
					  << "' fehlgeschlagen: ";
			if(*curr_char == '\0')
			{
				std::cerr << "unerwartetes Dateiende.\n";
				std::cerr << "Zustand " << q << ", Zeile " << linecount
						  << ", Zeichen " << colcount << ".\n";
			}
			else if(*curr_char == '\n')
			{
				std::cerr << "unerwarteter Zeilenumbruch, Zeile "
						  << linecount << ".\n";
			}
			else
			{
				std::cerr << "unerwartetes Zeichen '" << *curr_char
						  << "', Zeile " << linecount
						  << ", Zeichen " << colcount << ".\n";
			}

			// Abbruch
			exit(1);
		}
	}

	/// Rückt im Input ein Zeichen vor
	/** Rückt im Input ein Zeichen weiter und aktualisiert Zeilen- und
	  * Spaltennummer.
	  */
	void next_char()
	{
		if(*curr_char == '\n')
		{
			++linecount;
			colcount = 1;
		}
		else
		{
			++colcount;
		}
		++curr_char;
	}
};

#endif
