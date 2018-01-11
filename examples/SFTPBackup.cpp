#include "HOST.hpp"
/*
 * File:   SFTPBackup.cpp
 * 
 * Author: Robert Tizzard
 * 
 * Created on October 24, 2017, 2:33 PM
 *
 * Copyright 2017.
 *
 */

//
// Program: SFTPBackup
//
// Description: Simple SFTP backup program that takes a local directory and backs it up
// to a specified SFTP server using account details provided.
//
// Dependencies: C11++, Classes (CSFTP, CSSHSession), Boost C++ Libraries.
//
// SFTPBackup
// Program Options:
//   --help                 Print help messages
//   -c [ --config ] arg    Config File Name
//   -s [ --server ] arg    FTP Server
//   -p [ --port ] arg      FTP Server port
//   -u [ --user ] arg      Account username
//   -p [ --password ] arg  User password
//   -r [ --remote ] arg    Remote server directory to restore
//   -l [ --local ] arg     Local Directory to backup

// =============
// INCLUDE FILES
// =============

//
// C++ STL
//

#include <iostream>

//
// Antik Classes
//

#include "SSHSessionUtil.hpp"
#include "SFTPUtil.hpp"
#include "FTPUtil.hpp"

using namespace Antik::SSH;

//
// Boost program options  & file system library
//

#include <boost/program_options.hpp>  
#include <boost/filesystem.hpp>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

// ======================
// LOCAL TYES/DEFINITIONS
// ======================

// Command line parameter data

struct ParamArgData {
    std::string userName;         // SSH account user name
    std::string userPassword;     // SSH account user name password
    std::string serverName;       // SSH server
    std::string serverPort;       // SSH server port
    std::string remoteDirectory;  // SSH remote directory to restore
    std::string localDirectory;   // Local directory to backup
    std::string configFileName;   // Configuration file name
};

// ===============
// LOCAL FUNCTIONS
// ===============

//
// Exit with error message/status
//

static void exitWithError(std::string errMsg) {

    // Display error and exit.

    std::cout.flush();
    std::cerr << errMsg << std::endl;
    exit(EXIT_FAILURE);

}

//
// Add options common to both command line and config file
//

static void addCommonOptions(po::options_description& commonOptions, ParamArgData& argData) {

    commonOptions.add_options()
            ("server,s", po::value<std::string>(&argData.serverName)->required(), "SSH Server name")
            ("port,o", po::value<std::string>(&argData.serverPort)->required(), "SSH Server port")
            ("user,u", po::value<std::string>(&argData.userName)->required(), "Account username")
            ("password,p", po::value<std::string>(&argData.userPassword)->required(), "User password")
            ("remote,r", po::value<std::string>(&argData.remoteDirectory)->required(), "Remote directory for backup")
            ("local,l", po::value<std::string>(&argData.localDirectory)->required(), "Local directory to backup");

}

//
// Read in and process command line arguments using boost.
//

static void procCmdLine(int argc, char** argv, ParamArgData &argData) {

    // Define and parse the program options

    po::options_description commandLine("Program Options");
    commandLine.add_options()
            ("help", "Print help messages")
            ("config,c", po::value<std::string>(&argData.configFileName), "Config File Name");

    addCommonOptions(commandLine, argData);

    po::options_description configFile("Config Files Options");

    addCommonOptions(configFile, argData);

    po::variables_map vm;

    try {

        // Process arguments

        po::store(po::parse_command_line(argc, argv, commandLine), vm);

        // Display options and exit with success

        if (vm.count("help")) {
            std::cout << "SFTPBackup" << std::endl << commandLine << std::endl;
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

    } catch (po::error& e) {
        std::cerr << "SFTPBackup Error: " << e.what() << std::endl << std::endl;
        std::cerr << commandLine << std::endl;
        exit(EXIT_FAILURE);
    }

}

// ============================
// ===== MAIN ENTRY POint =====
// ============================

int main(int argc, char** argv) {

    try {

        ParamArgData argData;
        CSSHSession sshSession;
        FileList locaFileList;
        FileList filesBackedUp;
 
        // Read in command line parameters and process

        procCmdLine(argc, argv, argData);

        std::cout << "SERVER [" << argData.serverName << "]" << std::endl;
        std::cout << "SERVER PORT [" << argData.serverPort << "]" << std::endl;
        std::cout << "USER [" << argData.userName << "]" << std::endl;
        std::cout << "LOCAL DIRECTORY [" << argData.localDirectory << "]" << std::endl;
        std::cout << "REMOTE DIRECTORY [" << argData.remoteDirectory << "]\n" << std::endl;
        
        // Set server and port
        
        sshSession.setServer(argData.serverName);
        sshSession.setPort(std::stoi(argData.serverPort));

        // Set account user name and password

        sshSession.setUser(argData.userName);
        sshSession.setUserPassword(argData.userPassword);

         // Connect to server

        sshSession.connect();

        // Verify the server's identity

        if (!verifyKnownServer(sshSession)) {
            throw std::runtime_error("Unable to verify server.");
        } else {
            std::cout << "Server verified..." << std::endl;
        }

        // Authenticate ourselves

        if (!userAuthorize(sshSession)) {
            throw std::runtime_error("Server unable to authorize client");
        } else {
            std::cout << "Client authorized..." << std::endl;
        }

        // Create, open SFTP session and initialise file mapper
        
        CSFTP sftpServer { sshSession };
        FileMapper fileMapper{ argData.localDirectory, argData.remoteDirectory};
        
        sftpServer.open();
        
        // Get local directory file list
        
        Antik::FTP::listLocalRecursive(argData.localDirectory, locaFileList);
        
        // Copy file list to SFTP Server

        if (!locaFileList.empty()) {
            filesBackedUp = putFiles(sftpServer, fileMapper, locaFileList);
        }

        // Signal success or failure
        
        if (!filesBackedUp.empty()) {
            for (auto file : filesBackedUp) {
                std::cout << "Sucessfully backed up [" << file << "]" << std::endl;
            }
        } else {
            std::cout << "Backup failed."<< std::endl;
        }

        // Disconnect 
        
        sftpServer.close();
        sshSession.disconnect();

    //
    // Catch any errors
    //    

    } catch (CSSHSession::Exception &e) {
        exitWithError(e.getMessage());
    } catch (CSFTP::Exception &e) {
        exitWithError(e.getMessage());
    } catch (std::runtime_error &e) {
        exitWithError(std::string("Standard exception occured: [") + e.what() + "]");
    }

    exit(EXIT_SUCCESS);

}