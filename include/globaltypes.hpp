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
// globaltypes.hpp
// Definiert globale Typen für TDBParser und seine Hilfsklassen
////////////////////////////////////////////////////////////////////////////////

#ifndef __WEGEL_TDBP_GLOBALTYPES_HPP__
#define __WEGEL_TDBP_GLOBALTYPES_HPP__

#include <string>
#include <list>

/// Token des Inputs
typedef std::string Token;

/// Tokenisierter Input
typedef std::list<Token> TokenList;

/// Symbol als abstrakte Kategorie
typedef std::string Symbol;

/// Liste von Symbolen
typedef std::list<Symbol> SymbolList;

/// Liste von Listen von Symbolen
typedef std::list<SymbolList> SymbolListList;

/// Konkrete, identifizierbare Instanz eines Symbols
struct SymbolInstance
{
	Symbol	symbol;	///< Symbol
	int		id;		///< ID zur Identifikation

	/// Konstruktor für leere Instanz
	SymbolInstance()
	{
		symbol = Symbol("");
		id = -1;
	}

	/// Konstruktor aus Symbol und ID
	SymbolInstance(Symbol s, int i)
	{
		symbol = s;
		id = i;
	}

	/// Kopie-Konstruktor
	SymbolInstance(const SymbolInstance& rhs)
	{
		symbol = rhs.symbol;
		id = rhs.id;
	}

	/// Gleichheits-Operator
	bool operator==(const SymbolInstance& rhs) const
	{
		return id == rhs.id;
	}

	/// Kleiner-als-Operator
	bool operator<(const SymbolInstance& rhs) const
	{
		return id < rhs.id;
	}
};

/// Liste von Symbolinstanzen
typedef std::list<SymbolInstance> SymbolInstanceList;

#endif
