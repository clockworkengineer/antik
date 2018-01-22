#include "HOST.hpp"
/*
 * File:   SCPUtil.cpp(Work In Progress)
 * 
 * Author: Robert Tizzard
 *
 * Created on October 10, 2017, 2:34 PM
 * 
 * Copyright 2017.
 * 
 */

//
// Module: SCPUtil
//
// Description: SCP utility functions for the Antik class CSCP.
// Perform selective and  more powerful operations not available directly through
// single raw SCP commands. These functions are different from the FTP variants
// in that they use a FileMapper to convert paths and also deal in absolute paths
// and not the current working directory ( does not exist in SCP).
// 
// Dependencies: 
// 
// C11++              : Use of C11++ features.
// Antik classes      : CSCP
// Boost              : File system.
//

// =============
// INCLUDE FILES
// =============

//
// C++ STL
//

#include <iostream>
#include <system_error>

//
// SCP utility definitions
//

#include "SCPUtil.hpp"

//
// Boost file system
//

#include <boost/filesystem.hpp>

// =========
// NAMESPACE
// =========

namespace Antik {
    namespace SSH {

        // =======
        // IMPORTS
        // =======

        using namespace std;

        namespace fs = boost::filesystem;

        // ===============
        // LOCAL FUNCTIONS
        // ===============

        //
        // Break path into its component directories and create path structure on
        // remote FTP server.
        //
        
        static void makeRemotePath(CSCP &scpServer, const string &remotePath, const CSCP::FilePermissions permissions) {

            vector<string> pathComponents;
            fs::path currentPath{ string(1, '/') };

            boost::split(pathComponents, remotePath, boost::is_any_of(string(1, '/')));

            for (auto directory : pathComponents) {
                if (!directory.empty()) {
                    scpServer.pushDirectory(directory, permissions);
                }
            }


        }
        // ================
        // PUBLIC FUNCTIONS
        // ================

        //
        // Upload a file from remote SCP server assigning it the same permissions as the remote file.
        // SCP does not directly support file upload/download so this function is not part of the
        // CSCP class.
        //
        
        void getFile(CSSHSession &sshSession, const string &sourceFile, const string &destinationFile) {

            ofstream localFile;

            try {

                CSCP::FilePermissions filePermissions;
                
                int bytesRead{0},  pullStatus{0};
                size_t fileSize {0};
              
                CSCP scpServer { sshSession, SSH_SCP_READ, sourceFile };
                
                scpServer.open();

                if ((pullStatus = scpServer.pullRequest()) != SSH_SCP_REQUEST_NEWFILE) {
                    if (pullStatus == SSH_SCP_REQUEST_WARNING) {
                        throw CSCP::Exception(scpServer, __func__);
                    }

                }
                
                filePermissions = scpServer.requestFilePermissions();
                fileSize = scpServer.requestFileSize();
   
                scpServer.acceptRequest();
                
                if (!fs::exists(fs::path(destinationFile).parent_path())) {
                    fs::create_directories(fs::path(destinationFile).parent_path());
                }

                localFile.open(destinationFile, ios_base::out | ios_base::binary | ios_base::trunc);
                if (!localFile) {
                    throw system_error(errno, system_category());
                }

                for (;;) {
                    bytesRead = scpServer.read(scpServer.getIoBuffer().get(), scpServer.getIoBufferSize());
                    localFile.write(scpServer.getIoBuffer().get(), bytesRead);
                    fileSize -= bytesRead;
                    if (fileSize==0) {
                        break;
                    }
                }

                scpServer.close();

                localFile.close();

                fs::permissions(destinationFile, static_cast<fs::perms> (filePermissions));

            } catch (const CSCP::Exception &e) {
                throw;
            } catch (const system_error &e) {
                throw;
            }

        }
        
        //
        // Download a file to remote SCP server assigning it the same permissions as the local file. 
        // It will be created with the owner and group of the currently logged in SSH account.
        // SCP does not directly support file upload/download so this function is not part of the
        // CSCP class.
        //

        void putFile(CSSHSession &sshSession, const string &sourceFile, const string &destinationFile) {

            ifstream localFile;

            try {

                fs::file_status fileStatus;

                localFile.open(sourceFile, ios_base::in | ios_base::binary);
                if (!localFile) {
                    throw system_error(errno, system_category());
                }

                fileStatus = fs::status(fs::path(sourceFile).parent_path().string());
                
                CSCP scpServer { sshSession, SSH_SCP_WRITE | SSH_SCP_RECURSIVE,  "/" };
                
                scpServer.open();
                
                makeRemotePath(scpServer,fs::path(destinationFile).parent_path().string(), fileStatus.permissions());

                fileStatus = fs::status(fs::path(sourceFile));
                        
                scpServer.pushFile(fs::path(destinationFile).filename().string(), fs::file_size(sourceFile), fileStatus.permissions());
                
                for (;;) {

                    localFile.read(scpServer.getIoBuffer().get(), scpServer.getIoBufferSize());

                    if (localFile.gcount()) {
                        scpServer.write(scpServer.getIoBuffer().get(), localFile.gcount());
                    }

                    if (!localFile) break;

                }

                scpServer.close();           
                localFile.close();

            } catch (const CSCP::Exception &e) {
                throw;
            } catch (const system_error &e) {
                throw;
            }


        }

        //
        // Download all files passed in file list from server to the local directory passed in; recreating any server directory
        // structure in situ. If safe == true then the file is downloaded to a filename with a postfix then the file is renamed
        // to its correct value on success. Returns a list of successfully downloaded files and directories created in the local
        // directory.
        //
        
        FileList getFiles(CSCP &scpServer, FileMapper &fileMapper, const FileList &remoteFileList, FileCompletionFn completionFn, bool safe, char postFix) {

            FileList successList;

            try {


                // On exception report and return with files that where successfully downloaded.

            } catch (const CSCP::Exception &e) {
                cerr << e.getMessage() << endl;
            } catch (const boost::filesystem::filesystem_error & e) {
                cerr << string("BOOST file system exception occured: [") + e.what() + "]" << endl;
            } catch (const exception &e) {
                cerr << string("Standard exception occured: [") + e.what() + "]" << endl;
            }

            return (successList);

        }
        
        //
        // Take local directory, file list and upload all files to server;  recreating 
        // any local directory structure in situ on the server. Returns a list of successfully 
        // uploaded files and directories created.If safe == true then the file is uploaded to a 
        // filename with a postfix then the file is renamed to its correct value on success.
        // 

        FileList putFiles(CSCP &scpServer, FileMapper &fileMapper, const FileList &localFileList, FileCompletionFn completionFn, bool safe, char postFix) {

            FileList successList;

            try {

     

           // On exception report and return with files that where successfully uploaded.

            } catch (const CSCP::Exception &e) {
                cerr << e.getMessage() << endl;
            } catch (const boost::filesystem::filesystem_error & e) {
                cerr << string("BOOST file system exception occured: [") + e.what() + "]" << endl;
            } catch (const exception &e) {
                cerr << string("Standard exception occured: [") + e.what() + "]" << endl;
            }

            return (successList);

        }

    } // namespace SSH

} // namespace Antik

