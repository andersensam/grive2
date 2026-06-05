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

#include "Exception.hh"
#include "FileSystem.hh"
#include "json/Val.hh"

#include <string>
#include <map>

namespace gr {

struct SimpleOptions {
	std::map<std::string, std::string> options;
	std::map<std::string, bool> flags;
	std::map<std::string, unsigned> uints;

	bool Has( const std::string& key ) const {
		return flags.count(key) || options.count(key) || uints.count(key);
	}
	
	std::string Str( const std::string& key ) const {
		auto it = options.find(key);
		return it != options.end() ? it->second : "";
	}
	
	bool Bool( const std::string& key ) const {
		auto it = flags.find(key);
		return it != flags.end() ? it->second : false;
	}
	
	unsigned Uint( const std::string& key ) const {
		auto it = uints.find(key);
		return it != uints.end() ? it->second : 0;
	}
};

class Config
{
public :
	using SimpleOptions = gr::SimpleOptions;
	struct Error : virtual Exception {} ;
	typedef ErrorInfo<struct FileTag, std::string>	File ;

	Config( const SimpleOptions& vm ) ;

	const fs::path Filename() const ;
	
	void Set( const std::string& key, const Val& value ) ;
	Val Get( const std::string& key ) const ;

	Val GetAll() const ;
	void Save() ;

private :
	Val Read( ) ;
	static fs::path GetPath( const fs::path& root_path ) ;

private :
	//! config file path
	fs::path	m_path;
	
	//! config values loaded from config file
	Val		m_file ;
	
	//! config values from command line
	Val		m_cmd ;
} ;
	
} // end of namespace
