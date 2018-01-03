/*
 * File:   SFTPUtil.hpp
 * 
 * Author: Robert Tizzard
 *
 * Created on April 10, 2017, 2:34 PM
 * 
 * Copyright 2017.
 * 
 */

#ifndef SFTPUTIL_HPP
#define SFTPUTIL_HPP

// =============
// INCLUDE FILES
// =============

//
// C++ STL
//

#include <string>
#include <vector>
#include <fstream>

//
// Antik Classes
//

#include "CSSHSession.hpp"
#include "CSFTP.hpp"

namespace Antik {
    namespace SSH {

        void sftpGetFile(CSFTP &sftp, const std::string &srcFile, const std::string &dstFile);
        void sftpPutFile(CSFTP &sftp, const std::string &srcFile, const std::string &dstFile);
        int sftpGetDirectoryContents(CSFTP &sftp, const std::string &directoryPath, std::vector<CSFTP::FileAttributes> &directoryContents, bool recursive = false);

    } // namespace SSH
} // namespace Antik

#endif /* SFTPUTIL_HPP */

