#include "OAuth2Test.hh"
#include "Assert.hh"
#include "protocol/OAuth2.hh"

#ifdef __APPLE__
#include "http/NSURLSessionAgent.hh"
typedef gr::http::NSURLSessionAgent TestHttpAgent;
#else
#include "http/CurlAgent.hh"
typedef gr::http::CurlAgent TestHttpAgent;
#endif

using namespace grut;
using namespace gr;

OAuth2Test::OAuth2Test( )
{
}

void OAuth2Test::TestDefaultPort( )
{
	TestHttpAgent http;
	OAuth2 token( &http, "client_id", "client_secret" ) ;
	std::string url = token.MakeAuthURL() ;
	
	CPPUNIT_ASSERT( url.find("https://accounts.google.com/o/oauth2/auth") == 0 ) ;
	CPPUNIT_ASSERT( url.find("redirect_uri=http%3A%2F%2Flocalhost:") != std::string::npos ) ;
}

void OAuth2Test::TestManualPort( )
{
	TestHttpAgent http;
	OAuth2 token( &http, "client_id", "client_secret" ) ;
	token.SetPort( 54321 ) ;
	std::string url = token.MakeAuthURL() ;
	
	CPPUNIT_ASSERT( url.find("https://accounts.google.com/o/oauth2/auth") == 0 ) ;
	CPPUNIT_ASSERT( url.find("redirect_uri=http%3A%2F%2Flocalhost:54321%2Fauth") != std::string::npos ) ;
}
