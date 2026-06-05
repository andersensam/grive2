/*
	webwrite: an GPL program to sync a local directory with Google Drive
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

#include <exception>
#include <string>
#include <sstream>
#include <type_traits>
#include <cstring>

#ifndef BOOST_THROW_EXCEPTION
#define BOOST_THROW_EXCEPTION(x) throw (x)
#endif

namespace gr {

class Backtrace ;

template <typename Tag, typename T>
struct ErrorInfo {
	T value;
	ErrorInfo(const T& v) : value(v) {}
};

/**	\defgroup	exception	Exception Classes
*/

/**	\brief	base class for exception in WebWrite
	\ingroup exception
	This class is the base class for all exception class in WebWrite.
	It allows us to catch all WebWrite exception with one catch clause.
*/
struct Exception : virtual public std::exception
{
	mutable std::string m_details;

	Exception( const std::string& msg = "" ) ;
	virtual ~Exception() throw() ;
	
	virtual const char* what() const throw() override ;
} ;

/// Exception informations
namespace expt
{
	struct BacktraceTag {};
	struct ApiFunctionTag {};
	struct ErrnoTag {};
	struct FileNameTag {};
	struct FileOpenModeTag {};
	struct FileHandleTag {};
	struct AtLineTag {};

	// back-trace information. should be present for all exceptions
	typedef ErrorInfo<BacktraceTag, Backtrace>	Backtrace_ ;
	typedef ErrorInfo<ApiFunctionTag, std::string> errinfo_api_function ;
	typedef ErrorInfo<ErrnoTag, int> errinfo_errno ;
	typedef ErrorInfo<FileNameTag, std::string> errinfo_file_name ;
	typedef ErrorInfo<FileOpenModeTag, std::string> errinfo_file_open_mode ;
	typedef ErrorInfo<FileHandleTag, void*> errinfo_file_handle ;
	typedef ErrorInfo<AtLineTag, int> errinfo_at_line ;
}

	template <class E, class Tag, class T>
	inline E const & operator<<( E const & x, const ErrorInfo<Tag,T>& info ) {
		if (!x.m_details.empty()) {
			x.m_details += "; ";
		}
		std::ostringstream ss;
		if constexpr (std::is_same_v<Tag, expt::ErrnoTag>) {
			ss << std::strerror(info.value);
		} else {
			ss << info.value;
		}
		x.m_details += ss.str();
		return x;
	}

} // end of namespace gr

namespace boost {
	template <typename Tag, typename T>
	using error_info = gr::ErrorInfo<Tag, T>;

	using gr::expt::errinfo_api_function;
	using gr::expt::errinfo_errno;
	using gr::expt::errinfo_file_name;
	using gr::expt::errinfo_file_open_mode;
	using gr::expt::errinfo_file_handle;
	using gr::expt::errinfo_at_line;
}


