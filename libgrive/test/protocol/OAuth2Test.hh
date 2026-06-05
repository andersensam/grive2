#pragma once

#include "protocol/OAuth2.hh"
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace grut {

class OAuth2Test : public CppUnit::TestFixture
{
public :
	OAuth2Test( ) ;

	CPPUNIT_TEST_SUITE( OAuth2Test ) ;
		CPPUNIT_TEST( TestDefaultPort ) ;
		CPPUNIT_TEST( TestManualPort ) ;
	CPPUNIT_TEST_SUITE_END();

private :
	void TestDefaultPort( );
	void TestManualPort( );
} ;

} // end of namespace
