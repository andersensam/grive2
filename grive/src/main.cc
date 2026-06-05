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

#include "util/Config.hh"
#include "util/ProgressBar.hh"

#include "base/Drive.hh"
#include "drive2/Syncer2.hh"

#ifdef __APPLE__
#include "http/NSURLSessionAgent.hh"
#else
#include "http/CurlAgent.hh"
#endif
#include "protocol/AuthAgent.hh"
#include "protocol/OAuth2.hh"
#include "json/Val.hh"

#include "bfd/Backtrace.hh"
#include "util/Exception.hh"
#include "util/log/Log.hh"
#include "util/log/CompositeLog.hh"
#include "util/log/DefaultLog.hh"


// initializing libgcrypt, must be done in executable
#include <gcrypt.h>

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <unistd.h>

const std::string default_id            = APP_ID ;
const std::string default_secret        = APP_SECRET ;

using namespace gr ;

// libgcrypt insist this to be done in application, not library
void InitGCrypt()
{
	if ( !gcry_check_version(GCRYPT_VERSION) )
		throw std::runtime_error( "libgcrypt version mismatch" ) ;

	// disable secure memory
	gcry_control(GCRYCTL_DISABLE_SECMEM, 0);

	// tell Libgcrypt that initialization has completed
	gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
}

void InitLog( const SimpleOptions& vm )
{
	std::unique_ptr<log::CompositeLog> comp_log( new log::CompositeLog ) ;
	std::unique_ptr<LogBase> def_log( new log::DefaultLog );
	LogBase* console_log = comp_log->Add( def_log ) ;

	if ( vm.options.count( "log" ) )
	{
		std::unique_ptr<LogBase> file_log( new log::DefaultLog( vm.Str("log") ) ) ;
		file_log->Enable( log::debug ) ;
		file_log->Enable( log::verbose ) ;
		file_log->Enable( log::info ) ;
		file_log->Enable( log::warning ) ;
		file_log->Enable( log::error ) ;
		file_log->Enable( log::critical ) ;

		// log grive version to log file
		file_log->Log( log::Fmt("grive version " VERSION " " __DATE__ " " __TIME__), log::verbose ) ;
		file_log->Log( log::Fmt("current time: %1%") % DateTime::Now(), log::verbose ) ;

		comp_log->Add( file_log ) ;
	}

	if ( vm.flags.count( "verbose" ) )
	{
		console_log->Enable( log::verbose ) ;
	}

	if ( vm.flags.count( "debug" ) )
	{
		console_log->Enable( log::verbose ) ;
		console_log->Enable( log::debug ) ;
	}
	LogBase::Inst( comp_log.release() ) ;
}

int Main( int argc, char **argv )
{
	InitGCrypt() ;

	SimpleOptions vm;
	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg == "-h" || arg == "--help") vm.flags["help"] = true;
		else if (arg == "-v" || arg == "--version") vm.flags["version"] = true;
		else if (arg == "-a" || arg == "--auth") vm.flags["auth"] = true;
		else if (arg == "--print-url") vm.flags["print-url"] = true;
		else if (arg == "-V" || arg == "--verbose") vm.flags["verbose"] = true;
		else if (arg == "--new-rev") vm.flags["new-rev"] = true;
		else if (arg == "-d" || arg == "--debug") { vm.flags["debug"] = true; vm.flags["verbose"] = true; }
		else if (arg == "-f" || arg == "--force") vm.flags["force"] = true;
		else if (arg == "-u" || arg == "--upload-only") vm.flags["upload-only"] = true;
		else if (arg == "-n" || arg == "--no-remote-new") vm.flags["no-remote-new"] = true;
		else if (arg == "--dry-run") vm.flags["dry-run"] = true;
		else if (arg == "-P" || arg == "--progress-bar") vm.flags["progress-bar"] = true;
		else if (arg == "-i" || arg == "--id") { if (i+1 < argc) vm.options["id"] = argv[++i]; }
		else if (arg == "-e" || arg == "--secret") { if (i+1 < argc) vm.options["secret"] = argv[++i]; }
		else if (arg == "-p" || arg == "--path") { if (i+1 < argc) vm.options["path"] = argv[++i]; }
		else if (arg == "-s" || arg == "--dir") { if (i+1 < argc) vm.options["dir"] = argv[++i]; }
		else if (arg == "--log-http") { if (i+1 < argc) vm.options["log-http"] = argv[++i]; }
		else if (arg == "-l" || arg == "--log") { if (i+1 < argc) vm.options["log"] = argv[++i]; }
		else if (arg == "-U" || arg == "--upload-speed") { if (i+1 < argc) vm.uints["upload-speed"] = std::atoi(argv[++i]); }
		else if (arg == "-D" || arg == "--download-speed") { if (i+1 < argc) vm.uints["download-speed"] = std::atoi(argv[++i]); }
	}

	// simple commands that doesn't require log or config
	if ( vm.flags.count("help") )
	{
		std::cout << "Grive sync client (C++17)\nOptions:\n"
			"  -h [ --help ]             Produce help message\n"
			"  -v [ --version ]          Display Grive version\n"
			"  -a [ --auth ]             Request authorization token\n"
			"  -i [ --id ] arg           Authentication ID\n"
			"  -e [ --secret ] arg       Authentication secret\n"
			"  --print-url               Only print url for request\n"
			"  -p [ --path ] arg         Path to working copy root\n"
			"  -s [ --dir ] arg          Single subdirectory to sync\n"
			"  -V [ --verbose ]          Verbose mode. Enable more messages than normal.\n"
			"  --log-http arg            Log all HTTP responses in this file for debugging.\n"
			"  --new-rev                 Create new revisions in server for updated files.\n"
			"  -d [ --debug ]            Enable debug level messages. Implies -v.\n"
			"  -l [ --log ] arg          Set log output filename.\n"
			"  -f [ --force ]            Force grive to always download a file from Google Drive\n"
			"  -u [ --upload-only ]      Do not download anything from Google Drive, only upload local changes\n"
			"  -n [ --no-remote-new ]    Download only files that are changed in Google Drive and already exist locally\n"
			"  --dry-run                 Only detect which files need to be uploaded/downloaded\n"
			"  -U [ --upload-speed ] arg Limit upload speed in kbytes per second\n"
			"  -D [ --download-speed ] arg Limit download speed in kbytes per second\n"
			"  -P [ --progress-bar ]     Enable progress bar for upload/download of files\n";
		return 0 ;
	}
	else if ( vm.flags.count( "version" ) )
	{
		std::cout
			<< "grive version " << VERSION << ' ' << __DATE__ << ' ' << __TIME__ << std::endl ;
		return 0 ;
	}

	// initialize logging
	InitLog( vm ) ;

	Config config( vm ) ;

	Log( "config file name %1%", config.Filename(), log::verbose );

#ifdef __APPLE__
	std::unique_ptr<http::Agent> http( new http::NSURLSessionAgent );
#else
	std::unique_ptr<http::Agent> http( new http::CurlAgent );
#endif
	if ( vm.options.count( "log-http" ) )
		http->SetLog( new http::ResponseLog( vm.Str("log-http"), ".txt" ) );

	std::unique_ptr<ProgressBar> pb;
	if ( vm.flags.count( "progress-bar" ) )
	{
		pb.reset( new ProgressBar() );
		http->SetProgressReporter( pb.get() );
	}

	if ( vm.flags.count( "auth" ) )
	{
		std::string id = vm.options.count( "id" ) > 0
                        ? vm.Str("id")
                        : default_id ;
		std::string secret = vm.options.count( "secret" ) > 0
                        ? vm.Str("secret")
                        : default_secret ;

		OAuth2 token( http.get(), id, secret ) ;

		if ( vm.flags.count("print-url") )
		{
			std::cout << token.MakeAuthURL() << std::endl ;
			return 0 ;
		}

		std::cout
			<< "-----------------------\n"
			<< "Please open this URL in your browser to authenticate Grive2:\n\n"
			<< token.MakeAuthURL()
			<< std::endl ;

		if ( !token.GetCode() )
		{
			std::cout << "Authentication failed\n";
			return -1;
		}

		// save to config
		config.Set( "id", Val( id ) ) ;
		config.Set( "secret", Val( secret ) ) ;
		config.Set( "refresh_token", Val( token.RefreshToken() ) ) ;
		config.Save() ;
	}

	std::string refresh_token ;
	std::string id ;
	std::string secret ;
	try
	{
		refresh_token = config.Get("refresh_token").Str() ;
		id = config.Get("id").Str() ;
		secret = config.Get("secret").Str() ;
	}
	catch ( Exception& e )
	{
		Log(
			"Please run grive with the \"-a\" option if this is the "
			"first time you're accessing your Google Drive!",
			log::critical ) ;

		return -1;
	}

	OAuth2 token( http.get(), refresh_token, id, secret ) ;
	AuthAgent agent( token, http.get() ) ;
	v2::Syncer2 syncer( &agent );

	if ( vm.uints.count( "upload-speed" ) > 0 )
		agent.SetUploadSpeed( vm.Uint("upload-speed") * 1000 );
	if ( vm.uints.count( "download-speed" ) > 0 )
		agent.SetDownloadSpeed( vm.Uint("download-speed") * 1000 );

	DateTime startTime = DateTime::Now();
	Log( "\nStarted at %1%", startTime, log::info );
	Drive drive( &syncer, config.GetAll() );
	drive.DetectChanges() ;

	if ( vm.flags.count( "dry-run" ) == 0 )
	{
		// The progress bar should just be enabled when actual file transfers take place
		if ( pb )
			pb->setShowProgressBar( true ) ;
		drive.Update() ;
		if ( pb )
			pb->setShowProgressBar( false ) ;

		drive.SaveState() ;
	}
	else
		drive.DryRun() ;

	config.Save() ;
	DateTime endTime = DateTime::Now();
	Log(
		"Finished at %1% (%2% seconds)",
		endTime,
		endTime.Sec() - startTime.Sec(),
		log::info );
	return 0 ;
}

int main( int argc, char **argv )
{
	try
	{
		return Main( argc, argv ) ;
	}
	catch ( Exception& e )
	{
		Log( "exception: %1%", e.what(), log::critical ) ;
	}
	catch ( std::exception& e )
	{
		Log( "exception: %1%", e.what(), log::critical ) ;
	}
	catch ( ... )
	{
		Log( "unexpected exception", log::critical ) ;
	}
	return -1 ;
}
