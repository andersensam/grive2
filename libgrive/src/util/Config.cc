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

#include "Config.hh"

#include "util/File.hh"
#include "json/JsonWriter.hh"
#include "json/JsonParser.hh"

#include <iostream>
#include <iterator>

namespace gr {

const std::string	default_filename	= ".grive";
const char			*env_name			= "GR_CONFIG";
const std::string	default_root_folder = ".";

Config::Config( const SimpleOptions& vm )
{
	if ( vm.options.count( "id" ) > 0 )
		m_cmd.Add( "id",	Val( vm.Str("id") ) ) ;
	if ( vm.options.count( "secret" ) > 0 )
		m_cmd.Add( "secret",	Val( vm.Str("secret") ) ) ;
	m_cmd.Add( "new-rev",	Val( vm.Bool("new-rev") ) ) ;
	m_cmd.Add( "force",		Val( vm.Bool("force") ) ) ;
	m_cmd.Add( "path",		Val( vm.options.count("path") > 0
		? vm.Str("path")
		: default_root_folder ) ) ;
	m_cmd.Add( "dir",		Val( vm.options.count("dir") > 0
		? vm.Str("dir")
		: "" ) ) ;
	if ( vm.options.count( "ignore" ) > 0 )
		m_cmd.Add( "ignore",	Val( vm.Str("ignore") ) );
	m_cmd.Add( "no-remote-new", Val( vm.flags.count( "no-remote-new" ) > 0 || vm.flags.count( "upload-only" ) > 0 ) );
	m_cmd.Add( "upload-only", Val( vm.flags.count( "upload-only" ) > 0 ) );
	m_cmd.Add( "no-delete-remote", Val( vm.flags.count( "no-delete-remote" ) > 0 ) );
	
	m_path	= GetPath( fs::path(m_cmd["path"].Str()) ) ;
	m_file	= Read( ) ;
}

fs::path Config::GetPath( const fs::path& root_path )
{
	// config file will be (in order of preference)
	// value specified in environment string
	// value specified in defaultConfigFileName in path from commandline --path
	// value specified in defaultConfigFileName in current directory
	const char *env = ::getenv( env_name ) ;
	return root_path / (env ? env : default_filename) ;
}

const fs::path Config::Filename() const
{
	return m_path ;
}

void Config::Save( )
{
	gr::File file( m_path.string(), 0600 ) ;
	JsonWriter wr( &file ) ;
	m_file.Visit( &wr ) ;
}

void Config::Set( const std::string& key, const Val& value )
{
	m_file.Set( key, value ) ;
}

Val Config::Get( const std::string& key ) const
{
	return m_cmd.Has(key) ? m_cmd[key] : m_file[key] ;
}

Val Config::GetAll() const
{
	Val::Object obj		= m_file.AsObject() ;
	Val::Object cmd_obj	= m_cmd.AsObject() ;
	
	for ( Val::Object::iterator i = cmd_obj.begin() ; i != cmd_obj.end() ; ++i )
		obj[i->first] = i->second ;
	
	return Val( obj ) ;
}

Val Config::Read()
{
	try
	{
		gr::File file(m_path) ;
		return ParseJson( file ) ;
	}
	catch ( Exception& e )
	{
		return Val() ;
	}
}

} // end of namespace
