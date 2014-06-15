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
// tdbp.hpp
// Klasse TDBParser implementiert einen Top-Down-Backtracking-Parser
////////////////////////////////////////////////////////////////////////////////

#ifndef __WEGEL_TDBP_HPP__
#define __WEGEL_TDBP_HPP__

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <stack>
#include <map>
#include <boost/unordered_map.hpp>
#include <boost/tokenizer.hpp>
#include "globaltypes.hpp"
#include "dcgreader.hpp"
#include "wishtree.hpp"

/// Top-Down-Backtracking-Parser
/** Parser, der einen String tokenisiert, dafür auf Grundlage einer separierten
  * Grammatik im Prolog-DCG-Format alle möglichen Parsebäume findet und sie im
  * Wishtree/Showtree-Format speichert.
  */
class TDBParser
{
	private:

	////////////////////////////////////////////////////////////////////////////
	// Typen

	/// Menge von Symbolen
	typedef std::set<Symbol> SymbolSet;

	/// Map mit Produktionsregeln Symbol -> Liste von rechten Regelseiten
	typedef boost::unordered_map<Symbol,SymbolListList> Grammar;

	/// Map mit Bottom-Up-Lexikonregeln Terminal -> Menge von Nichtterminalen
	typedef boost::unordered_map<Token,SymbolSet> Lexicon;

	/// Stapel mit Symbolinstanzen
	typedef std::stack<SymbolInstance> SymbolStack;

	/// Menge von Bäumen
	typedef std::set<std::string> TreeSet;

	public:

	////////////////////////////////////////////////////////////////////////////
	// Öffentliche Funktionen

	/// Konstruktor aus einer separierten Grammatik im Prolog-DCG-Format
	/** Liest Grammatik und Lexikon ein und erstellt daraus interne
	  * Repräsentationen.
	    @param grammarfile Dateiname der Grammatik
	    @param lexiconfile Dateiname des Lexikons
	  */
	TDBParser(const std::string grammarfile, const std::string lexiconfile)
	{
		init_grammar_prolog(grammarfile);
		init_lexicon_prolog(lexiconfile);
	}

	/// Parst einen Satz und speichert die Bäume in einer Datei
	/** Tokenisiert den Satz, parst ihn komplett mithilfe von Backtracking
	  * und speichert alle gefundenen Bäume in der angegebenen Datei.
	    @param sentence Satz
	    @param outfile Speicherziel für die Bäume
	  */
	void parse(const std::string sentence, const std::string outfile)
	{
		// Initialisiere Werte
		init_parse();

		// Tokenisiere Input
		TokenList input;
		boost::tokenizer<> tok(sentence);
		for(boost::tokenizer<>::iterator i = tok.begin(); i != tok.end(); ++i)
		{
			input.push_back(*i);
		}

		// Prüfe, ob alle Tokens im Lexikon stehen
		check_tokens(input);

		// Iteriere über Input
		TokenList::const_iterator input_pos = input.begin();

		// Instantiiere Startsymbol
		SymbolInstance start = new_si(Symbol("s"));

		// Instantiiere Stapel zu expandierender Symbole mit Startsymbol
		SymbolStack to_be_expanded;
		to_be_expanded.push(start);

		// Instantiiere den aufzubauenden Baum
		WishTree tree(start);

		// Parse rekursiv mit diesen Starteinstellungen
		parse_recursive(input_pos,input.end(),to_be_expanded,SymbolSet(),tree);

		// Speichere die gefundenen Bäume
		save_trees(outfile);
	}

	private:

	////////////////////////////////////////////////////////////////////////////
	// Daten

	Grammar		grammar;	///< Map mit den Produktionsregeln der Grammatik
	Lexicon		lexicon;	///< Map mit den Lexikonregeln
	TreeSet		trees;		///< Menge der gefundenen Parsebäume
	unsigned	si_count;	///< Anzahl der Symbolinstanzen

	////////////////////////////////////////////////////////////////////////////
	// Private Funktionen

	/// Baut die interne Grammatik aus dem Prolog-Format auf
	/** Nutzt die Klasse DCGReader als Automat zum Einlesen der
	  * Prolog-DCG-Regeln und fügt sie der internen Grammatik
	  * hinzu.
	    @param prologfile Dateiname der Grammatik
	  */
	void init_grammar_prolog(const std::string prologfile)
	{
		// Initialisiere Automat
		DCGReader grammar_reader(prologfile);
		while(!grammar_reader.finished())
		{
			// Lies eine Regel aus der Datei
			std::pair<Symbol,SymbolList> rule = grammar_reader.run_grammar();

			if(rule.first != Symbol(""))
			{
				// Füge die Regel der internen Grammatik hinzu
				Grammar::iterator i = grammar.find(rule.first);
				if(i == grammar.end())
				{
					// Noch keine Regel für das Symbol vorhanden, füge einen
					// neuen Eintrag mit der gefundenen Regel hinzu
					SymbolListList new_sll(1,rule.second);
					grammar[rule.first] = new_sll;
				}
				else
				{
					// Bereits Regeln für das Symbol vorhanden, füge die
					// gefundene dort ein
					i->second.push_back(rule.second);
				}
			}
		}
	}

	/// Baut das interne Lexikon aus dem Prolog-Format auf
	/** Nutzt die Klasse DCGReader als Automat zum Einlesen der
	  * Prolog-DCG-Regeln und fügt sie dem internen Lexikon
	  * hinzu.
	    @param prologfile Dateiname des Lexikons
	  */
	void init_lexicon_prolog(const std::string prologfile)
	{
		// Initialisiere Automat
		DCGReader lexicon_reader(prologfile,true);
		while(!lexicon_reader.finished())
		{
			// Lies eine Regel aus der Datei
			std::pair<Token,Symbol> rule = lexicon_reader.run_lexicon();

			// Füge sie dem internen Lexikon hinzu
			Lexicon::iterator i = lexicon.find(rule.first);
			if(i == lexicon.end())
			{
				// Noch keine Regel für das Wort vorhanden, füge einen neuen
				// Eintrag mit der gefundenen Regel zum Lexikon hinzu
				SymbolSet new_ss;
				new_ss.insert(rule.second);
				lexicon[rule.first] = new_ss;
			}
			else
			{
				// Bereits Regeln für das Wort vorhanden, füge die gefundene
				// dort ein
				i->second.insert(rule.second);
			}
		}
	}

	/// Initialisiert Daten für Durchlauf des Parsers
	/** Setzt interne Daten auf Anfangswerte für einen Durchlauf des Parsers.
	  */
	void init_parse()
	{
		// Leere Menge gefundener Bäume
		trees = TreeSet();

		// IDs für die Symbolinstanzen beginnen bei 0
		si_count = 0;
	}

	/// Prüft, ob alle Tokens im Lexikon stehen
	/** Überprüft für jedes Token des Inputs, ob dafür Lexikonregeln
	  * existieren. Bricht im negativen Falle das Parsing ab.
	    @param tokens Liste der Input-Tokens
	  */
	void check_tokens(const TokenList tokens) const
	{
		// Iteriere über die Tokens
		for(TokenList::const_iterator t = tokens.begin(); t != tokens.end(); ++t)
		{
			// Schlage Token im Lexikon nach
			Lexicon::const_iterator l = lexicon.find(*t);
			if(l == lexicon.end())
			{
				// Token nicht im Lexikon, Abbruch
				std::cerr << "Unbekanntes Wort: "
				          << "Keine Lexikonregel für Terminal '"
				          << *t << "' gefunden.\n";
				exit(1);
			}
		}
	}

	/// Rekursiver Teil des Parsings
	/** Probiert für ein Symbol alle Möglichkeiten der Expansion rekursiv aus.
	    @param input_pos Iterator auf die aktuelle Position im Input
	    @param input_end Iterator auf das Ende des Inputs
	    @param to_be_expanded Stapel zu expandierender Symbole
	    @param left_expanding Menge der Symbole, die gerade linksexpandiert werden
	    @param tree Bisher aufgebauter Baum
	  */
	void parse_recursive(TokenList::const_iterator input_pos,
	                     TokenList::const_iterator input_end,
	                     SymbolStack to_be_expanded, SymbolSet left_expanding,
	                     WishTree tree)
	{
		// Nimm oberstes Symbol vom Stapel zu expandierender Symbole
		SymbolInstance si = to_be_expanded.top();
		to_be_expanded.pop();

		// Füge Symbol in die Menge von Symbolen ein, die gerade linksexpandiert
		// werden, und prüfe dabei, ob es schon enthalten ist
		bool no_left_recursion = left_expanding.insert(si.symbol).second;
		if(no_left_recursion)
		{
			// Suche Grammatikregeln für das Symbol
			Grammar::const_iterator rhs_list = grammar.find(si.symbol);
			if(rhs_list == grammar.end())
			{
				// Keine Produktionsregel gefunden, suche Lexikonregel für
				// das Symbol und das nachfolgende Wort des Inputs
				Lexicon::const_iterator lex_set = lexicon.find(*input_pos);
				if(lex_set->second.find(si.symbol) != lex_set->second.end())
				{
					// Passende Lexikonregel gefunden
					// Füge dem Baum den entsprechenden Ast hinzu
					WishTree new_tree(tree,si,*input_pos);
					// Rücke im Input weiter
					++input_pos;
					// Prüfe, ob Ende des Inputs erreicht ist
					if(input_pos == input_end)
					{
						// Prüfe, ob Stapel leer ist
						if(to_be_expanded.empty())
						{
							// Parsing erfolgreich! Speichere Baum
							trees.insert(new_tree.str());
						}
					}
					else
					{
						// Prüfe, ob Stapel noch voll ist
						if(!to_be_expanded.empty())
						{
							// Parse auf Grundlage der neuen Einstellungen
							parse_recursive(input_pos,input_end,to_be_expanded,SymbolSet(),new_tree);
						}
					}
				}
			}
			else
			{
				// Iteriere über die gefundenen Produktionsregeln
				for(SymbolListList::const_iterator rule = rhs_list->second.begin();
					rule != rhs_list->second.end(); ++rule)
				{
					// Betrachte die Regel und erzeuge aus der rechten Regelseite
					// eine Liste identifizierbarer Nichtterminale
					SymbolInstanceList si_list = make_si_list(*rule);
					// Lege die Nichtterminale auf den Stapel zu expandierender Symbole
					SymbolStack new_stack = push_list_to_stack(si_list,to_be_expanded);
					// Füge dem Baum die entsprechenden Äste hinzu
					WishTree new_tree(tree,si,si_list);
					// Parse auf Grundlage der neuen Einstellungen
					parse_recursive(input_pos,input_end,new_stack,left_expanding,new_tree);
				}
			}
		}
		else
		{
			// Linksrekursion entdeckt, Warnhinweis ausgeben und den aktuellen
			// Parsingversuch nicht weiterverfolgen
			std::cerr << "Warnung: Linksrekursion bei der Expansion des "
			          << "Symbols '" << si.symbol << "' entdeckt. "
			          << "Entsprechende Regeln werden ignoriert.\n";
		}

		// Alle Regeln für aktuelles Symbol ausprobiert, damit ist diese Instanz
		// der rekursiven Funktion beendet; der Ball geht wieder zur
		// nächsthöheren Instanz, die Alternativen für ihr Symbol probieren kann
	}

	/// Liefert eine neue Symbolinstanz
	/** Erzeugt zu einem Symbol der Grammatik eine identifzierbare Instanz
	  * für den Stapel zu expandierender Symbole und den Baum.
	    @param s Symbol
	    @return Neue Instanz des Symbols
	  */
	SymbolInstance new_si(Symbol s)
	{
		SymbolInstance si(s,si_count);
		++si_count;
		return si;
	}

	/// Liefert zu einer Liste von Symbolen eine Liste von Symbolinstanzen
	/** Erzeugt zu jedem Symbol in einer Liste eine neue Instanz und liefert
	  * eine korrespondierende Liste.
	    @param s_list Liste von Symbolen
	    @return Liste von neuen Symbolinstanzen
	  */
	SymbolInstanceList make_si_list(SymbolList s_list)
	{
		// Instantiiere die Liste der Symbolinstanzen
		SymbolInstanceList si_list;
		// Iteriere über die Liste der Symbole
		for(SymbolList::const_iterator i = s_list.begin();
		    i != s_list.end(); ++i)
		{
			// Füge eine neue Instanz des Symbols zur Liste der
			// Symbolinstanzen hinzu
			si_list.push_back(new_si(*i));
		}
		// Gib die fertige Liste aus
		return si_list;
	}

	/// Legt Symbolinstanzen aus einer Liste auf einen Stapel
	/** Legt die Elemente einer Liste von Symbolinstanzen in umgekehrter
	  * Reihenfolge auf einen Stapel, sodass das erste Element zuoberst liegt.
	    @param si_list Liste von Symbolinstanzen
	    @param stack Stapel vorher
	    @return Stapel nachher
	  */
	SymbolStack push_list_to_stack(SymbolInstanceList si_list,SymbolStack stack)
	{
		// Iteriere in umgekehrter Reihenfolge über die Liste
		for(SymbolInstanceList::const_reverse_iterator i = si_list.rbegin();
		    i != si_list.rend(); ++i)
		{
			// Lege die Symbolinstanz auf den Stapel
			stack.push(*i);
		}
		// Gib den fertigen Stapel aus
		return stack;
	}

	/// Gibt die Menge gefundener Bäume aus
	/** Speichert alle gefundenen Bäume im Wishtree/Showtree-Format, getrennt
	  * mit Zeilenumbrüchen, in eine Textdatei.
	    @param outfile Dateiname für die Baum-Datei
	  */
	void save_trees(const std::string outfile) const
	{
		// Erzeuge einen Output-Filestream in die Datei
		std::ofstream outstream;
		outstream.open(outfile.c_str());
		if(outstream.is_open())
		{
			// Iteriere über die gefundenen Bäume
			for(TreeSet::const_iterator i = trees.begin(); i != trees.end(); ++i)
			{
				// Gib den Baum aus
				outstream << *i << "\n";
			}
		}
	}
};

#endif
