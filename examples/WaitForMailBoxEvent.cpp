#include "HOST.hpp"
/*
 * File:   WaitForMailBoxEvent.cpp
 * 
 * Author: Robert Tizzard
 * 
 * Created on October 24, 2016, 2:33 PM
 *
 * Copyright 2016.
 *
 */

//
// Program: WaitForMailBoxEvent
//
// Description: Log on to a IMAP server and wait for a status change in a specified
// mailbox. By default it will use IDLE but polling every time period is also supported.
// This is not directly useful but may be applied to other situations where the functionality
// is needed.All parameters and their meaning are obtained by running the program with the 
// parameter --help.
//
// WaitForMailBoxEvent Example Application
// Program Options:
//   --help                Print help messages
//   -c [ --config ] arg   Config File Name
//   -s [ --server ] arg   IMAP Server URL and port
//   -u [ --user ] arg     Account username
//   -p [ --password ] arg User password
//   -m [ --mailbox ] arg  Mailbox name
//   -l [ --poll ]         Check status using NOOP
//
// Dependencies: C11++, Classes (CMailIMAP, CMailIMAPParse),
//               Linux, Boost C++ Libraries.
//

// =============
// INCLUDE FILES
// =============

//
// C++ STL definitions
//

#include <iostream>
#include <chrono>
#include <thread>

//
// Antikythera Classes
//

#include "CMailIMAP.hpp"
#include "CMailIMAPParse.hpp"

using namespace Antik;

//
// Boost program options  & file system library definitions
//

#include <boost/program_options.hpp> 
#include <boost/filesystem.hpp>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

// ======================
// LOCAL TYES/DEFINITIONS
// ======================

//
// Command line parameter data
//

struct ParamArgData {
    std::string userNameStr;            // Email account user name
    std::string userPasswordStr;        // Email account user name password
    std::string serverURLStr;           // SMTP server URL
    std::string mailBoxNameStr;         // Mailbox name
    std::string configFileNameStr;      // Configuration file name
    bool bPolls=false;                  // ==true then use NOOP
};

//
// Polling period between NOOP in seconds;
//

const int kPollPeriod = 15;

// ===============
// LOCAL FUNCTIONS
// ===============


//
// Exit with error message/status
//

void exitWithError(std::string errMsgStr) {

    // Closedown email, display error and exit.

    CMailIMAP::closedown();
    std::cerr << errMsgStr << std::endl;
    exit(EXIT_FAILURE);

}

//
// Add options common to both command line and config file
//

void addCommonOptions(po::options_description& commonOptions, ParamArgData& argData) {
    
    commonOptions.add_options()
            ("server,s", po::value<std::string>(&argData.serverURLStr)->required(), "IMAP Server URL and port")
            ("user,u", po::value<std::string>(&argData.userNameStr)->required(), "Account username")
            ("password,p", po::value<std::string>(&argData.userPasswordStr)->required(), "User password")
            ("mailbox,m", po::value<std::string>(&argData.mailBoxNameStr)->required(), "Mailbox name")
            ("poll,l", "Check status using NOOP");

}

//
// Read in and process command line arguments using boost.
//

void procCmdLine(int argc, char** argv, ParamArgData &argData) {

    // Define and parse the program options

    po::options_description commandLine("Program Options");
    commandLine.add_options()
            ("help", "Print help messages")
            ("config,c", po::value<std::string>(&argData.configFileNameStr)->required(), "Config File Name");

    addCommonOptions(commandLine, argData);
    
    po::options_description configFile("Config Files Options");
  
    addCommonOptions(configFile, argData);
       
    po::variables_map vm;

    try {

        // Process arguments

        po::store(po::parse_command_line(argc, argv, commandLine), vm);

        // Display options and exit with success

        if (vm.count("help")) {
            std::cout << "WaitForMailBoxEvent Example Application" << std::endl << commandLine << std::endl;
            exit(EXIT_SUCCESS);
        }

        if (vm.count("config")) {
            if (fs::exists(vm["config"].as<std::string>().c_str())) {
                std::ifstream ifs{vm["config"].as<std::string>().c_str()};
                if (ifs) {
                    po::store(po::parse_config_file(ifs, configFile), vm);
                }
            } else {
                throw po::error("Specified config file does not exist.");
            }
        }

        po::notify(vm);

        // Check status with polling loop
        
        if (vm.count("poll")) {
            argData.bPolls = true;
        }

    } catch (po::error& e) {
        std::cerr << "WaitForMailBoxEvent Error: " << e.what() << std::endl << std::endl;
        std::cerr << commandLine << std::endl;
        exit(EXIT_FAILURE);
    }

}

//
// Parse command response and return pointer to parsed data.
//

CMailIMAPParse::COMMANDRESPONSE parseCommandResponse(const std::string& commandStr, 
                             const std::string& commandResponseStr) {

    CMailIMAPParse::COMMANDRESPONSE parsedResponse;

    try {
        parsedResponse = CMailIMAPParse::parseResponse(commandResponseStr);
    } catch (CMailIMAPParse::Exception &e) {
        std::cerr << "RESPONSE IN ERRROR: [" << commandResponseStr << "]" << std::endl;
        throw (e);
    }

    if (parsedResponse) {
        if (parsedResponse->bBYESent) {
            throw CMailIMAP::Exception("Received BYE from server: " + parsedResponse->errorMessageStr);
        } else if (parsedResponse->status != CMailIMAPParse::RespCode::OK) {
            throw CMailIMAP::Exception(commandStr + ": " + parsedResponse->errorMessageStr);
        }
    } else {
        throw CMailIMAPParse::Exception("nullptr parsed response returned.");
    }

    return (parsedResponse);

}

//
// Send command to IMAP server. At present it checks for any errors and just exits.
//

std::string sendCommand(CMailIMAP& imap, const std::string& mailBoxNameStr, 
                           const std::string& commandStr) {

    std::string commandResponseStr;

    try {
        commandResponseStr = imap.sendCommand(commandStr);
    } catch (CMailIMAP::Exception &e) {
        std::cerr << "IMAP ERROR: Need to reconnect to server" << std::endl;
        throw (e);
    }

    return (commandResponseStr);

}

// ============================
// ===== MAIN ENTRY POINT =====
// ============================

int main(int argc, char** argv) {

    try {

        ParamArgData argData;
        CMailIMAP imap;
        std::string parsedResponseStr, commandStr;
        CMailIMAPParse::COMMANDRESPONSE  parsedResponse;

        // Read in command line parameters and process

        procCmdLine(argc, argv, argData);

        // Initialise CMailIMAP internals

        CMailIMAP::init();

        // Set mail account user name and password
        
        imap.setServer(argData.serverURLStr);
        imap.setUserAndPassword(argData.userNameStr, argData.userPasswordStr);
        
        // Connect
 
        std::cout << "Connecting to server [" << argData.serverURLStr << "]" << std::endl;
               
        imap.connect();

        std::cout << "Connected." << std::endl;
        
        // SELECT mailbox

        commandStr = "SELECT " + argData.mailBoxNameStr;
        parsedResponseStr = sendCommand(imap, argData.mailBoxNameStr, commandStr);
        parsedResponse = parseCommandResponse(commandStr, parsedResponseStr);
        
        // IDLE

        std::cout << "Waiting on mailbox [" << argData.mailBoxNameStr << "]" << std::endl;

        if (!argData.bPolls) {
            commandStr = "IDLE";
            parsedResponseStr = sendCommand(imap, argData.mailBoxNameStr, commandStr);
            parsedResponse = parseCommandResponse(commandStr, parsedResponseStr);
        } else {
            while(true) {
                std::cout << "Polling [" << argData.mailBoxNameStr << "]" << std::endl;
                commandStr = "NOOP";
                parsedResponseStr = sendCommand(imap, argData.mailBoxNameStr, commandStr);
                parsedResponse = parseCommandResponse(commandStr, parsedResponseStr);
                if (parsedResponse->responseMap.size() >= 1) break;
                std::this_thread::sleep_for(std::chrono::seconds(kPollPeriod));
            }
        }
        
        // Display any response
        
        for (auto& resp : parsedResponse->responseMap) {
            std::cout << resp.first << " = " << resp.second << std::endl;
        }
        
        std::cout << "Disconnecting from server [" << argData.serverURLStr << "]" << std::endl;

        imap.disconnect();


    //
    // Catch any errors
    //    

    } catch (CMailIMAP::Exception &e) {
        exitWithError(e.what());
    } catch (CMailIMAPParse::Exception &e) {
        exitWithError(e.what());
    } catch (const fs::filesystem_error & e) {
        exitWithError(std::string("BOOST file system exception occured: [") + e.what() + "]");
    } catch (std::exception & e) {
        exitWithError(std::string("Standard exception occured: [") + e.what() + "]");
    }

    // IMAP closedown
    
    CMailIMAP::closedown();

    exit(EXIT_SUCCESS);

}

