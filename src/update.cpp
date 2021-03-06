#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <iterator>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
using boost::asio::ip::tcp;

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>

boost::gregorian::date convertDATE(char const *time);

// returns true if the application needs to exit
bool informUserOfUpdate(std::string json)
{
	std::string version = DOVO_VERSION;
	std::string mustupdate = "false";
	std::string message = "";

	try
	{
		boost::property_tree::ptree pt;
		std::istringstream is(json);
		boost::property_tree::read_json(is, pt);

		version = pt.get_child("dovo.version").data();
		mustupdate = pt.get_child("dovo.mustupdate").data();
		message = pt.get_child("dovo.message").data();
	}
	catch (std::exception& e)
	{
		// std::cout << "Exception: " << e.what() << "\n";
	}


	boost::gregorian::date compiledate(convertDATE(__DATE__));
	boost::gregorian::date timelimit = compiledate + boost::gregorian::years(1);
	boost::gregorian::date today = boost::gregorian::day_clock::local_day();

	if(today > timelimit)
	{
		wxMessageBox(wxT("This software is outdated, please upgrade! Please see https://github.com/DraconPern/dovo"));
		return true;
	}

	if(version != DOVO_VERSION)
	{
		if(mustupdate == "true")
		{
			wxMessageBox(wxT("You must update to a new version.  Please see https://github.com/DraconPern/dovo"));
			return true;
		}
		else
		{
			wxMessageBox( wxT("There is an update available.  Please see https://github.com/DraconPern/dovo"));
		}
	}

	return false;
}

int getUpdateJSON(std::string &updateinfo);

void updateChecker()
{
	std::string updateinfo;
	getUpdateJSON(updateinfo);
	wxConfig::Get()->Write("/Settings/UpdateInfo", updateinfo.c_str());	
}

int getUpdateJSON(std::string &updateinfo)
{

	std::string potentialupdateinfo;
	try
	{
		boost::asio::io_service io_service;

		std::string host = "www.draconpern.com";
		std::string checkurl = "/software/dovo/";

		// Get a list of endpoints corresponding to the server name.
		tcp::resolver resolver(io_service);
		tcp::resolver::query query(host, "http");
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

		// Try each endpoint until we successfully establish a connection.
		tcp::socket socket(io_service);
		boost::asio::connect(socket, endpoint_iterator);

		// Form the request. We specify the "Connection: close" header so that the
		// server will close the socket after transmitting the response. This will
		// allow us to treat all data up until the EOF as the content.
		boost::asio::streambuf request;
		std::ostream request_stream(&request);
		request_stream << "GET " << checkurl << " HTTP/1.0\r\n";
		request_stream << "Host: " << host << "\r\n";
		request_stream << "Accept: */*\r\n";
		request_stream << "Connection: close\r\n\r\n";

		// Send the request.
		boost::asio::write(socket, request);

		// Read the response status line. The response streambuf will automatically
		// grow to accommodate the entire line. The growth may be limited by passing
		// a maximum size to the streambuf constructor.
		boost::asio::streambuf response;
		boost::asio::read_until(socket, response, "\r\n");

		// Check that response is OK.
		std::istream response_stream(&response);
		std::string http_version;
		response_stream >> http_version;
		unsigned int status_code;
		response_stream >> status_code;
		std::string status_message;
		std::getline(response_stream, status_message);
		if (!response_stream || http_version.substr(0, 5) != "HTTP/")
		{
			// std::cout << "Invalid response\n";
			return 1;
		}
		if (status_code != 200)
		{
			// std::cout << "Response returned with status code " << status_code << "\n";
			return 1;
		}

		// Read the response headers, which are terminated by a blank line.
		boost::asio::read_until(socket, response, "\r\n\r\n");

		// Process the response headers.
		std::string header;
		while (std::getline(response_stream, header) && header != "\r")
		{
			//std::cout << header << "\n";
		}

		// Get whatever content we already have
		if (response.size() > 0)
		{
			std::string s((std::istreambuf_iterator<char>(&response)), 
				std::istreambuf_iterator<char>());
			potentialupdateinfo += s;
		}

		// Read until EOF
		boost::system::error_code error;
		while (boost::asio::read(socket, response,
			boost::asio::transfer_at_least(1), error))
		{
			std::string s((std::istreambuf_iterator<char>(&response)), 
				std::istreambuf_iterator<char>());
			potentialupdateinfo += s;
		}

		if (error != boost::asio::error::eof)
			throw boost::system::system_error(error);
	}
	catch (std::exception& e)
	{
		// std::cout << "Exception: " << e.what() << "\n";
	}

	if(potentialupdateinfo.length() == 0)
		return 1;

	std::string marker = "<!-- updatecheck";
	int startmarker = potentialupdateinfo.find(marker);
	if(startmarker == std::string::npos)
		return 1;
	startmarker += marker.length();

	int stopmarker = potentialupdateinfo.find("-->", startmarker);
	if(stopmarker == std::string::npos)
		return 1;

	updateinfo = potentialupdateinfo.substr(startmarker, stopmarker - startmarker);

	return 0;
}


boost::gregorian::date convertDATE(char const *time) 
{ 
	char s_month[5];
	int month, day, year;
	struct tm t = {0};
	static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";

	sscanf(time, "%s %d %d", s_month, &day, &year);

	month = (strstr(month_names, s_month)-month_names)/3;

	t.tm_mon = month;
	t.tm_mday = day;
	t.tm_year = year - 1900;
	t.tm_isdst = -1;

	return boost::gregorian::date_from_tm(t);
}