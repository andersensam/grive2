/*
	grive: an GPL program to sync a local directory with Google Drive
	Copyright (C) 2012  Wan Wai Ho

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation version 2
	of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#pragma once

#include "util/Exception.hh"

#include <iterator>

#include <iosfwd>
#include <string>
#include <vector>
#include <utility>

namespace gr { namespace xml {

class NodeSet ;

class Node
{
private :
	class	Impl ;
	typedef std::vector<Impl*>	ImplVec ;
	
public :
	class iterator ;

	typedef boost::error_info<struct DupAttr, std::string>	DupAttr_ ;

public :
	Node() ;
	Node( const Node& node ) ;
	~Node() ;

	static Node Element( const std::string& name ) ;
	static Node Text( const std::string& name ) ;
	
	Node& operator=( const Node& node ) ;
	void Swap( Node& node ) ;
	
	Node AddElement( const std::string& name ) ;
	Node AddText( const std::string& text ) ;
	void AddNode( const Node& node ) ;
	void AddNode( iterator first, iterator last ) ;
	void AddAttribute( const std::string& name, const std::string& val ) ;

	NodeSet operator[]( const std::string& name ) const ;
	operator std::string() const ;
	bool operator==( const std::string& value ) const ;
	
	const std::string& Name() const ;
	std::string Value() const ;
	
	// read-only access to the reference counter. for checking.
	std::size_t RefCount() const ;
	
	enum Type { element, attr, text } ;
	Type GetType() const ;

	static bool IsCompatible( Type parent, Type child ) ;
	static std::ostream& PrintChar( std::ostream& os, char c ) ;
	static std::ostream& PrintString( std::ostream& os, const std::string& s ) ;

	iterator begin() const ;
	iterator end() const ;
	std::size_t size() const ;
	NodeSet Children() const ;
	
	NodeSet Attr() const ;
	std::string Attr( const std::string& attr ) const ;
	bool HasAttr( const std::string& attr ) const ;
	
private :
	explicit Node( Impl *impl ) ;

	typedef std::pair<ImplVec::iterator, ImplVec::iterator> Range ;
	
private :
	Impl *m_ptr ;
} ;

class Node::iterator
{
public :
	using iterator_category = std::random_access_iterator_tag;
	using value_type = Node;
	using difference_type = std::ptrdiff_t;
	using pointer = Node*;
	using reference = Node;

	iterator( ) ;
	explicit iterator( ImplVec::const_iterator i ) ;		

	reference operator*() const ;

	iterator& operator++() ;
	iterator operator++(int) ;
	iterator& operator--() ;
	iterator operator--(int) ;

	iterator& operator+=(difference_type n) ;
	iterator operator+(difference_type n) const ;
	iterator& operator-=(difference_type n) ;
	iterator operator-(difference_type n) const ;
	difference_type operator-(const iterator& other) const ;

	bool operator==(const iterator& other) const ;
	bool operator!=(const iterator& other) const ;
	bool operator<(const iterator& other) const ;
	bool operator<=(const iterator& other) const ;
	bool operator>(const iterator& other) const ;
	bool operator>=(const iterator& other) const ;

private :
	ImplVec::const_iterator m_it ;
} ;

std::ostream& operator<<( std::ostream& os, const Node& node ) ;

} } // end of namespace
