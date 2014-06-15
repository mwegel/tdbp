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
// wishtree.hpp
// Klasse WishTree implementiert einen Parsebaum
////////////////////////////////////////////////////////////////////////////////

#ifndef __WEGEL_TDBP_WISHTREE_HPP__
#define __WEGEL_TDBP_WISHTREE_HPP__

#include <sstream>
#include <string>
#include <map>
#include "globaltypes.hpp"

/// Parsebaum
/** Baum, dem Knoten hinzugefügt werden können und der als String im
  * Wishtree/Showtree-Format ausgegeben werden kann.
  */
class WishTree
{
	private:

	////////////////////////////////////////////////////////////////////////
	// Typen

	/// Äste als Map Symbolinstanz -> Liste von Symbolinstanzen
	typedef std::map<SymbolInstance,SymbolInstanceList> Edges;

	/// Einzelner Ast als Paar Symbolinstanz, Liste von Symbolinstanzen
	typedef std::pair<SymbolInstance,SymbolInstanceList> Edge;

	public:

	////////////////////////////////////////////////////////////////////////
	// Öffentliche Funktionen

	/// Konstruktor eines leeren Baumes
	/** Initialisiert einen Baum ohne Knoten und Äste, der aber sein
	  * künftiges Startsymbol bereits kennt.
	    @param si Symbolinstanz, die den obersten Knoten bilden wird
	  */
	WishTree(SymbolInstance si)
	{
		start = si;
	}

	/// Konstruktor, der einem Baum Nichtterminale hinzufügt
	/** Initialisiert einen Baum auf Grundlage eines bestehenden Baumes.
	  * Unter einer Symbolinstanz werden dabei Äste mit weiteren
	  * Symbolinstanzen hinzugefügt.
	    @param old_tree Bestehender Baum
	    @param si Symbolinstanz, unter der Äste eingefügt werden
	    @param si_list Liste der Symbolinstanzen, zu denen die Äste führen
	  */
	WishTree(WishTree old_tree, SymbolInstance si, SymbolInstanceList si_list)
	{
		edges = old_tree.edges;
		edges.insert(Edge(si,si_list));
		start = old_tree.start;
	}

	/// Konstruktor, der einem Baum ein Terminal hinzufügt
	/** Initialisiert einen Baum auf Grundlage eines bestehenden Baumes.
	  * Unter einer Symbolinstanz wird dabei ein Ast mit einem Terminal
	  * hinzugefügt.
	    @param old_tree Bestehender Baum
	    @param si Symbolinstanz, unter der der Ast eingefügt wird
	    @param t Terminal, zu dem der Ast führt
	  */
	WishTree(WishTree old_tree, SymbolInstance si, Token t)
	{
		edges = old_tree.edges;
		SymbolInstanceList si_list;
		si_list.push_back(SymbolInstance('-'+t,-1));
		edges.insert(Edge(si,si_list));
		start = old_tree.start;
	}

	/// Gibt den Baum als String aus
	/** Gibt den Baum als String im Wishtree/Showtree-Format aus.
	    @return Baum als String
	  */
	std::string str()
	{
		return str_recursive(start);
	}

	private:

	////////////////////////////////////////////////////////////////////////
	// Daten

	Edges edges;			///< Äste des Baumes
	SymbolInstance start;	///< Oberster Knoten des Baumes

	////////////////////////////////////////////////////////////////////////
	// Private Funktionen

	/// Baut den String rekursiv auf
	/** Erstellt rekursiv den String zum Teilbaum, der unter einer bestimmten
	  * Symbolinstanz hängt.
	    @param si Symbolinstanz, deren Teilbaum ausgegeben wird
	    @return Teilbaum als String
	  */
	std::string str_recursive(SymbolInstance si)
	{
		std::stringstream s;
		// Gib aus: Symbol des obersten Knotens, Klammer auf
		s << si.symbol << "/[";
		// Durchsuche die Tochterknoten
		SymbolInstanceList& children = edges[si];
		SymbolInstanceList::const_iterator c = children.begin();
		if(c->id == -1)
		{
			// Tochterknoten ist ein Terminal, gib es aus
			s << c->symbol;
		}
		else
		{
			// Gib die Teilbäume der Tochterknoten rekursiv aus
			s << str_recursive(*c);
			for(++c; c != children.end(); ++c)
			{
				s << ',' << str_recursive(*c);
			}
		}
		// Gib aus: Klammer zu
		s << ']';
		return s.str();
	}
};

#endif
