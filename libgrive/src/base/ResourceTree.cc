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

#include "ResourceTree.hh"

#include "util/Destroy.hh"
#include "util/log/Log.hh"

#include <algorithm>
#include <cassert>

namespace gr {

using namespace details ;

ResourceTree::ResourceTree( const fs::path& rootFolder ) :
	m_root( new Resource( rootFolder ) )
{
	Insert( m_root ) ;
}

ResourceTree::ResourceTree( const ResourceTree& fs ) :
	m_root( 0 )
{
	for ( auto* r : fs.m_resources )
	{
		Resource *c = new Resource( *r ) ;
		if ( c->IsRoot() )
			m_root = c ;
		
		Insert( c ) ;
	}
	
	assert( m_root != 0 ) ;
}

ResourceTree::~ResourceTree( )
{
	Clear() ;
}

void ResourceTree::Clear()
{
	for ( auto* r : m_resources )
		delete r ;
	
	m_resources.clear() ;
	m_by_href.clear() ;
	m_last_md5_query.clear() ;
	m_last_size_query.clear() ;
	m_root = 0 ;
}

Resource* ResourceTree::Root()
{
	assert( m_root != 0 ) ;
	return m_root ;
}

const Resource* ResourceTree::Root() const
{
	assert( m_root != 0 ) ;
	return m_root ;
}

Resource* ResourceTree::FindByHref( const std::string& href )
{
	if ( href.empty() )
		return 0 ;

	auto i = m_by_href.find( href ) ;
	return i != m_by_href.end() ? i->second : 0 ;
}

const Resource* ResourceTree::FindByHref( const std::string& href ) const
{
	if ( href.empty() )
		return 0 ;

	auto i = m_by_href.find( href ) ;
	return i != m_by_href.end() ? i->second : 0 ;
}

MD5Range ResourceTree::FindByMD5( const std::string& md5 )
{
	m_last_md5_query.clear() ;
	if ( !md5.empty() )
	{
		for ( auto* r : m_resources )
		{
			if ( r->MD5() == md5 )
				m_last_md5_query.push_back( r ) ;
		}
	}
	return MD5Range( m_last_md5_query.begin(), m_last_md5_query.end() ) ;
}

SizeRange ResourceTree::FindBySize( u64_t size )
{
	m_last_size_query.clear() ;
	for ( auto* r : m_resources )
	{
		if ( r->Size() == size )
			m_last_size_query.push_back( r ) ;
	}
	return SizeRange( m_last_size_query.begin(), m_last_size_query.end() ) ;
}

///	Reinsert should be called when the ID/HREF/MD5 were updated
bool ResourceTree::ReInsert( Resource *coll )
{
	auto i = m_resources.find( coll ) ;
	if ( i != m_resources.end() )
	{
		for ( auto it = m_by_href.begin(); it != m_by_href.end(); )
		{
			if ( it->second == coll )
				it = m_by_href.erase( it ) ;
			else
				++it ;
		}
		if ( !coll->SelfHref().empty() )
			m_by_href[coll->SelfHref()] = coll ;
		return true ;
	}
	else
		return false ;
}

void ResourceTree::Insert( Resource *coll )
{
	if ( coll && m_resources.insert( coll ).second )
	{
		if ( !coll->SelfHref().empty() )
			m_by_href[coll->SelfHref()] = coll ;
	}
}

void ResourceTree::Erase( Resource *coll )
{
	if ( coll )
	{
		m_resources.erase( coll ) ;
		for ( auto it = m_by_href.begin(); it != m_by_href.end(); )
		{
			if ( it->second == coll )
				it = m_by_href.erase( it ) ;
			else
				++it ;
		}
	}
}

void ResourceTree::Update( Resource *coll, const Entry& e )
{
	assert( coll != 0 ) ;

	coll->FromRemote( e ) ;
	ReInsert( coll ) ;
}

ResourceTree::iterator ResourceTree::begin()
{
	return m_resources.begin() ;
}

ResourceTree::iterator ResourceTree::end()
{
	return m_resources.end() ;
}

} // end of namespace gr
