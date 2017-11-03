#include "HOST.hpp"
/*
 * File:   FTPUtil.cpp
 * 
 * Author: Robert Tizzard
 *
 * Created on October 10, 2017, 2:34 PM
 * 
 * Copyright 2017.
 * 
 */

//
// Module: FTPUtil
//
// Description: FTP utility functions for the Antik class CFTP.
// Perform selective and  more powerful operations not available directly through
// single raw FTP commands. Any generated exceptions are not handled but passed back
// up the call stack.
// 
// Dependencies: 
// 
// C11++              : Use of C11++ features.
// Boost              : File system, string, iterators.
//

// =============
// INCLUDE FILES
// =============

//
// FTP utility definitions
//

#include "FTPUtil.hpp"
#include <iostream>
//
// Boost file system, string and iterators
//

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/range/iterator_range.hpp>

// =========
// NAMESPACE
// =========

namespace Antik {
    namespace FTP {

        // =======
        // IMPORTS
        // =======

        using namespace std;

        namespace fs = boost::filesystem;

        // ===============
        // LOCAL FUNCTIONS
        // ===============

        // ================
        // PUBLIC FUNCTIONS
        // ================

        //
        // Recursively parse a local directory and produce a list of files.
        //

        void listLocalRecursive(const string &localDirectory, vector<string> &fileList) {

            for (auto directoryEntry : fs::recursive_directory_iterator{localDirectory}) {
                fileList.push_back(directoryEntry.path().string());
            }

        }

        //
        // Recursively parse a remote server path passed in and pass back a list of directories/files found.
        //

        void listRemoteRecursive(CFTP &ftpServer, const string &remoteDirectory, vector<string> &fileList) {

            vector<string> serverFileList;
            uint16_t statusCode;

            statusCode = ftpServer.listFiles(remoteDirectory, serverFileList);
            if (statusCode == 226) {
                for (auto file : serverFileList) {
                    if (remoteDirectory != file) {
                        listRemoteRecursive(ftpServer, file, fileList);
                        fileList.push_back(file);
                    }
                }
            }
        }

        //
        // Break path into its component directories and create path structure on
        // remote FTP server. Note: This done relative to the server currently set
        // working directory and no errors are reported. To test for success/failure
        // use CFTP::isDirectory() after call to see if it has been created.
        //
        
        void makeRemotePath (CFTP &ftpServer, const string &remotePath) {
            
            vector<string> pathComponents;
            
            boost::split(pathComponents, remotePath, boost::is_any_of("/"));
            
            for (auto directory : pathComponents) {
                if (!directory.empty()) {
                    ftpServer.makeDirectory(directory);
                    ftpServer.changeWorkingDirectory(directory);
                }
            } 
            
        }
        
        //
        // Download all files passed in file list from server to the local path passed in; recreating any server directory
        // structure in situ. If safe == true then the file is downloaded to a filename with a postfix then the file is renamed
        // to its correct value on success. Returns a list of successfully downloaded files and directories created.
        //

        vector<string> getFiles(CFTP &ftpServer, const string &localDirectory, const vector<string> &fileList, bool safe, char postFix) {

            uint16_t statusCode;
            vector<string> successList;

            for (auto file : fileList) {
                fs::path destination{ localDirectory};
                destination /= file;
                if (!fs::exists(destination.parent_path())) {
                    fs::create_directories(destination.parent_path());
                }
                if (!ftpServer.isDirectory(file)) {
                    std::string destinationFileName{ destination.string() + postFix};
                    if (!safe) destinationFileName.pop_back();
                    statusCode = ftpServer.getFile(file, destinationFileName);
                    if (statusCode == 226) {
                        if (safe) fs::rename(destinationFileName, destination.string());
                        successList.push_back(file); // File get ok so add to success
                    }
                } else {
                    successList.push_back(file); // File directory created so add to success
                }

            }

            return (successList);

        }

        //
        // Take local folder, file list and upload all files to server;  recreating 
        // any local directory structure in situ on the server. Returns a list of successfully 
        // uploaded files and directories created.If safe == true then the file is uploaded to a 
        // filename with a postfix then the file is renamed to its correct value on success.
        // All files/directories are placed/created relative to the server current working directory.
        //

        vector<string> putFiles(CFTP &ftpServer, const string &localFolder, const vector<string> &fileList, bool safe, char postFix) {

            vector<string> successList;
            size_t localPathLength { 0 };
            std::string currentWorkingDirectory;

            // Find beginning of file name
            
            localPathLength = localFolder.rfind('/');
            if (localPathLength == std::string::npos) {
                localPathLength = 0;
            }
           
            // Save current working directory
            
            ftpServer.getCurrentWoringDirectory(currentWorkingDirectory);

            // Process file/directory list
            
            for (auto file : fileList) {

                fs::path filePath{ file};

                if (fs::exists(filePath)) {

                    string remoteDirectory;
                    bool transferFile { false };

                    // Create remote path and set file to be transfered flag
                    
                    if (fs::is_directory(filePath)) {
                        remoteDirectory = filePath.string();
                    } else if (fs::is_regular_file(filePath)) {
                        remoteDirectory = filePath.parent_path().string();
                        transferFile = true;
                    } else {
                        continue; // Not valid for transfer NEXT FILE!
                    }
                    
                    remoteDirectory = remoteDirectory.substr(localPathLength);
                    
                    // Make sure that no paths are root based
                    
                    if (remoteDirectory[0] == '/') {
                        remoteDirectory = remoteDirectory.substr(1);
                    }

                    // Set current working directory and create any remote path needed
                    
                    ftpServer.changeWorkingDirectory(currentWorkingDirectory);
                    
                    if (!remoteDirectory.empty()) {
                        if (!ftpServer.isDirectory(remoteDirectory)) {
                            makeRemotePath(ftpServer, remoteDirectory);
                            successList.push_back(currentWorkingDirectory  + "/" + remoteDirectory);
                        } else {
                            ftpServer.changeWorkingDirectory(remoteDirectory);
                        }
                    }
                    
                    // Transfer file
                    
                    if (transferFile) {
                        std::string destinationFileName{ filePath.filename().string() + postFix};
                        if (!safe) destinationFileName.pop_back();
                        if (ftpServer.putFile(destinationFileName, filePath.string()) == 226) {
                            if (safe) ftpServer.renameFile(destinationFileName, filePath.filename().string());
                            if (remoteDirectory.empty()) {
                                successList.push_back(currentWorkingDirectory + "/" + filePath.filename().string());
                            } else {
                                successList.push_back(currentWorkingDirectory + "/" + remoteDirectory + "/" + filePath.filename().string());                            
                            }
                        }
                    }

                }

            }

            // Restore saved current working directory
            
            ftpServer.changeWorkingDirectory(currentWorkingDirectory);

            return (successList);

        }

    } // namespace FTP

} // namespace Antik

