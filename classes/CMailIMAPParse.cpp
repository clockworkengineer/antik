#include "HOST.hpp"
/*
 * File:   CMailIMAPParse.cpp
 * 
 * Author: Robert Tizzard
 * 
 * Created on January 24, 2017, 2:33 PM
 *
 * Copyright 2016.
 *
 */

//
// Class: CMailIMAPParse
// 
// Description: A class to parse CMailIMAP command responses. It is designed 
// to expect syntactically correct commands from any server and not report any
// specific errors; but if any occur to report so through an exception exit 
// processing gracefully.
//
// NOTE: IMAP commands sent can be any in combination of case and this 
// is mirrored back in the response. So perform case-insensitive compares 
// for any commands in responses.
//
// Dependencies:   C11++     - Language standard features used.

//

// =================
// CLASS DEFINITIONS
// =================

#include "CMailIMAP.hpp"
#include "CMailIMAPParse.hpp"

// ====================
// CLASS IMPLEMENTATION
// ====================

//
// C++ STL definitions
//

#include <iostream>
#include <sstream>

// ===========================
// PRIVATE TYPES AND CONSTANTS
// ===========================

// ==========================
// PUBLIC TYPES AND CONSTANTS
// ==========================

// ========================
// PRIVATE STATIC VARIABLES
// ========================

//
// IMAP command code to parse response mapping table
//

std::unordered_map<int, CMailIMAPParse::ParseFunction> CMailIMAPParse::parseCommmandMap
{
    {static_cast<int> (Commands::LIST), parseLIST},
    {static_cast<int> (Commands::LSUB), parseLIST},
    {static_cast<int> (Commands::SEARCH), parseSEARCH},
    {static_cast<int> (Commands::SELECT), parseSELECT},
    {static_cast<int> (Commands::EXAMINE), parseSELECT},
    {static_cast<int> (Commands::STATUS), parseSTATUS},
    {static_cast<int> (Commands::EXPUNGE), parseEXPUNGE},
    {static_cast<int> (Commands::STORE), parseSTORE},
    {static_cast<int> (Commands::CAPABILITY), parseCAPABILITY},
    {static_cast<int> (Commands::FETCH), parseFETCH},
    {static_cast<int> (Commands::NOOP), parseNOOP},
    {static_cast<int> (Commands::IDLE), parseNOOP},
    {static_cast<int> (Commands::LOGOUT), parseLOGOUT}

};

//
// IMAP command string to internal enum code map table. Populated at runtime
// because command string constant objects need to be avaiable.
//

std::unordered_map<std::string, CMailIMAPParse::Commands> CMailIMAPParse::stringToCodeMap;

// =======================
// PUBLIC STATIC VARIABLES
// =======================

// ===============
// PRIVATE METHODS
// ===============

//
// Parse item/number pair in command response and add to response map. Note the current line is 
// updated to remove the pair and also this function is only used in FETCH command parse as the 
// response is processed over multiple lines and not line by line.
//

void CMailIMAPParse::parseNumber(const std::string& itemStr, FetchRespData& fetchData, std::string& lineStr) {
    int numberPos=0;
    std::string numberStr;
    lineStr = lineStr.substr(itemStr.length()+1);
    while(std::isdigit(lineStr[numberPos])) {
        numberStr.append(1,lineStr[numberPos++]);
    }
    lineStr = lineStr.substr(numberStr.length());
    fetchData.responseMap.insert({itemStr, numberStr});

}

//
// Parse item/string pair in response and add to response map.Note the current line is 
// updated to remove the pair and also this function is only used in FETCH command parse as the 
// response is processed over multiple lines and not line by line.

void CMailIMAPParse::parseString(const std::string& itemStr, FetchRespData& fetchData, std::string& lineStr) {
    std::string quotedString;
    lineStr = lineStr.substr(lineStr.find(itemStr) + itemStr.length() + 1);
    quotedString = "\"" + stringBetween(lineStr, '\"', '\"') + "\"";
    lineStr = lineStr.substr(quotedString.length());
    fetchData.responseMap.insert({itemStr, quotedString});

}

//
// Parse item list in response and add to response map.Note the current line is 
// updated to remove the pair and also this function is only used in FETCH 
// command parse as the response is processed over multiple lines and not line 
// by line.
//

void CMailIMAPParse::parseList(const std::string& itemStr, FetchRespData& fetchData, std::string& lineStr) {

    std::string listStr;
    lineStr = lineStr.substr(lineStr.find(itemStr) + itemStr.length() + 1);
    listStr = stringList(lineStr);
    lineStr = lineStr.substr(listStr.length());
    fetchData.responseMap.insert({itemStr, listStr});

}

//
// Parse item octet string in response and add to response map. This involves decoding 
// the octet string length and reading the string into a buffer, leaving line containing 
// the next part of the command response to be processed. Note: The command response before
// the octet string is used as the first item when inserting the octet string into the 
// response map to distinguish between multiple octet fetches that might occur.
//

void CMailIMAPParse::parseOctets(const std::string& itemStr, FetchRespData& fetchData, std::string& lineStr, std::istringstream& responseStream) {

    std::string octetStr, octectBuffer, commandLabel {lineStr};
    int numberOfOctets;

    if (commandLabel.back() == '\r') commandLabel.pop_back();

    lineStr = lineStr.substr(lineStr.find(itemStr) + itemStr.length());
    octetStr = stringBetween(lineStr, '{', '}');
    numberOfOctets = std::strtoull(octetStr.c_str(), nullptr, 10);
    lineStr = lineStr.substr(octetStr.length() + 2);
    octectBuffer.resize(numberOfOctets);
    responseStream.read(&octectBuffer[0], numberOfOctets);
    std::getline(responseStream, lineStr, '\n');
    fetchData.responseMap.insert({commandLabel, octectBuffer});

}

//
// Parse command response status and return response. At present an un-tagged BAD/NO gets 
// sent to std::cerr. Note: Any optional string that the server provides with a status is 
// stored away in the a response for use by the caller.
//

void CMailIMAPParse::parseStatus(const std::string& tagStr, const std::string& lineStr, BaseResponse& statusResponse) {

    if (stringEqual(lineStr, tagStr + " " + CMailIMAP::kOKStr)) {
        statusResponse.status = RespCode::OK;

    } else if (stringEqual(lineStr, tagStr + " " + CMailIMAP::kNOStr)) {
        statusResponse.status = RespCode::NO;
        statusResponse.errorMessageStr = lineStr;

    } else if (stringEqual(lineStr, tagStr + " " + CMailIMAP::kBADStr)) {
        statusResponse.status = RespCode::BAD;
        statusResponse.errorMessageStr = lineStr;

    } else if (stringEqual(lineStr, CMailIMAP::kUntaggedStr + " " + CMailIMAP::kBYEStr)) {
        if (!statusResponse.bBYESent) {
            statusResponse.bBYESent = true;
        }
        statusResponse.errorMessageStr = lineStr;

    } else if ((stringEqual(lineStr, CMailIMAP::kUntaggedStr + " " + CMailIMAP::kNOStr))
            || (stringEqual(lineStr, CMailIMAP::kUntaggedStr + " " + CMailIMAP::kBADStr))) {
        std::cerr << lineStr << std::endl;

    } else {
        throw Exception("error while parsing IMAP command ["+lineStr+"]");
    }

}

//
// Parse SELECT/EXAMINE Response. Note: The mailbox name is stringed from the 
// command line and used when decoding the response to find the mailbox access 
// privileges (READ-ONLY or READ-WRITE).
//

CMailIMAPParse::BASERESPONSE CMailIMAPParse::parseSELECT(CMailIMAPParse::CommandData& commandData) {

    SELECTRESPONSE resp { new SelectResponse { commandData.commandCode } };
    
    // Extract mailbox name from command (stripping any quotes).

    resp->mailBoxNameStr = commandData.commandLineStr.substr(commandData.commandLineStr.find_last_of(' ') + 1);
    if (resp->mailBoxNameStr.back() == '\"') resp->mailBoxNameStr.pop_back();
    if (resp->mailBoxNameStr.front() == '\"') resp->mailBoxNameStr = resp->mailBoxNameStr.substr(1);

    for (std::string lineStr; std::getline(commandData.commandRespStream, lineStr, '\n');) {

        lineStr.pop_back(); // Remove linefeed

        if (stringEqual(lineStr, CMailIMAP::kUntaggedStr + " " + CMailIMAP::kOKStr + " [")) {
            lineStr = stringBetween(lineStr, '[', ']');
        }

        if (stringEqual(lineStr, CMailIMAP::kUntaggedStr + " " + CMailIMAP::kFLAGSStr)) {
            resp->responseMap.insert({CMailIMAP::kFLAGSStr, stringList(lineStr)});

        } else if (stringEqual(lineStr, CMailIMAP::kPERMANENTFLAGSStr)) {
            resp->responseMap.insert({CMailIMAP::kPERMANENTFLAGSStr, stringList(lineStr)});

        } else if (stringEqual(lineStr, CMailIMAP::kUIDVALIDITYStr)) {
            resp->responseMap.insert({CMailIMAP::kUIDVALIDITYStr, stringBetween(lineStr, ' ', ']')});

        } else if (stringEqual(lineStr, CMailIMAP::kUIDNEXTStr)) {
            resp->responseMap.insert({CMailIMAP::kUIDNEXTStr, stringBetween(lineStr, ' ', ']')});

        } else if (stringEqual(lineStr, CMailIMAP::kHIGHESTMODSEQStr)) {
            resp->responseMap.insert({CMailIMAP::kHIGHESTMODSEQStr, stringBetween(lineStr, ' ', ']')});

        } else if (stringEqual(lineStr, CMailIMAP::kUntaggedStr + " " + CMailIMAP::kCAPABILITYStr)) {
            lineStr = lineStr.substr((std::string(CMailIMAP::kUntaggedStr + " " + CMailIMAP::kCAPABILITYStr).length()) + 1);
            resp->responseMap.insert({CMailIMAP::kCAPABILITYStr, lineStr});

        } else if (lineStr.find(CMailIMAP::kUNSEENStr) == 0) {
            resp->responseMap.insert({CMailIMAP::kUNSEENStr, stringBetween(lineStr, ' ', ']')});

        } else if (lineStr.find(CMailIMAP::kEXISTSStr) != std::string::npos) {
            resp->responseMap.insert({CMailIMAP::kEXISTSStr, stringUntaggedNumber(lineStr)});

        } else if (lineStr.find(CMailIMAP::kRECENTStr) != std::string::npos) {
            resp->responseMap.insert({CMailIMAP::kRECENTStr, stringUntaggedNumber(lineStr)});

        } else {
            BaseResponse statusResponse{ resp->command, resp->status, resp->errorMessageStr, resp->bBYESent};
            parseStatus(commandData.tagStr, lineStr, statusResponse);
            resp->status = statusResponse.status;
            resp->errorMessageStr = statusResponse.errorMessageStr;
            resp->bBYESent = statusResponse.bBYESent;
            if (resp->status == RespCode::OK) {
                resp->mailBoxAccessStr = stringBetween(lineStr, '[', ']');
            }
        }

    }

    return (static_cast<BASERESPONSE> (std::move(resp)));

}

//
// Parse SEARCH Response.
//

CMailIMAPParse::BASERESPONSE CMailIMAPParse::parseSEARCH(CMailIMAPParse::CommandData& commandData) {

    SEARCHRESPONSE resp { new SearchResponse { commandData.commandCode  } };

    for (std::string lineStr; std::getline(commandData.commandRespStream, lineStr, '\n');) {

        lineStr.pop_back(); // Remove linefeed

        if (stringEqual(lineStr, CMailIMAP::kUntaggedStr + " " + CMailIMAP::kSEARCHStr)) {
            
            lineStr = lineStr.substr(std::string(CMailIMAP::kUntaggedStr + " " + CMailIMAP::kSEARCHStr).length());
            
            std::istringstream listStream(lineStr); // Read indexes/UIDs
            while (listStream.good()) {
                uint64_t index;
                listStream >> index;
                resp->indexes.push_back(index);
            }

        } else {
            BaseResponse statusResponse{ resp->command, resp->status, resp->errorMessageStr, resp->bBYESent};
            parseStatus(commandData.tagStr, lineStr, statusResponse);
            resp->status = statusResponse.status;
            resp->errorMessageStr = statusResponse.errorMessageStr;
            resp->bBYESent = statusResponse.bBYESent;
      }

    }

    return (static_cast<BASERESPONSE> (std::move(resp)));

}

//
// Parse LIST/LSUB Response.
//

CMailIMAPParse::BASERESPONSE CMailIMAPParse::parseLIST(CMailIMAPParse::CommandData& commandData) {

    LISTRESPONSE resp { new ListResponse { commandData.commandCode  } };
    
    for (std::string lineStr; std::getline(commandData.commandRespStream, lineStr, '\n');) {

        ListRespData mailBoxEntry;

        lineStr.pop_back(); // Remove linefeed

        if ((stringEqual(lineStr, CMailIMAP::kUntaggedStr + " " + CMailIMAP::kLISTStr)) ||
            (stringEqual(lineStr, CMailIMAP::kUntaggedStr + " " + CMailIMAP::kLSUBStr))) {
            mailBoxEntry.attributesStr = stringList(lineStr);
            mailBoxEntry.hierDel = stringBetween(lineStr, '\"', '\"').front();
            if (lineStr.back() != '\"') {
                mailBoxEntry.mailBoxNameStr = lineStr.substr(lineStr.find_last_of(' '));
            } else {
                lineStr.pop_back();
                mailBoxEntry.mailBoxNameStr = lineStr.substr(lineStr.find_last_of('\"'));
                mailBoxEntry.mailBoxNameStr += '\"';
            }
            resp->mailBoxList.push_back(std::move(mailBoxEntry));

        } else {
            BaseResponse statusResponse{ resp->command, resp->status, resp->errorMessageStr, resp->bBYESent};
            parseStatus(commandData.tagStr, lineStr, statusResponse);
            resp->status = statusResponse.status;
            resp->errorMessageStr = statusResponse.errorMessageStr;
            resp->bBYESent = statusResponse.bBYESent;
     }

    }

    return (static_cast<BASERESPONSE> (std::move(resp)));

}

//
// Parse STATUS Response.
//

CMailIMAPParse::BASERESPONSE CMailIMAPParse::parseSTATUS(CMailIMAPParse::CommandData& commandData) {

    STATUSRESPONSE resp { new StatusResponse { commandData.commandCode  } };
    
    for (std::string lineStr; std::getline(commandData.commandRespStream, lineStr, '\n');) {

        lineStr.pop_back(); // Remove linefeed

        if (stringEqual(lineStr, CMailIMAP::kUntaggedStr + " " + CMailIMAP::kSTATUSStr)) {
            
            lineStr = lineStr.substr(std::string(CMailIMAP::kUntaggedStr + " " + CMailIMAP::kSTATUSStr).length()+1);

            resp->mailBoxNameStr = lineStr.substr(0, lineStr.find_first_of(' '));

            lineStr = stringBetween(lineStr, '(', ')');

            std::istringstream listStream(lineStr);
            std::string itemStr, valueStr;

            while (listStream.good()) {
                listStream >> itemStr >> valueStr;
                resp->responseMap.insert({itemStr, valueStr});
            }

        } else {
            BaseResponse statusResponse{ resp->command, resp->status, resp->errorMessageStr, resp->bBYESent};
            parseStatus(commandData.tagStr, lineStr, statusResponse);
            resp->status = statusResponse.status;
            resp->errorMessageStr = statusResponse.errorMessageStr;
            resp->bBYESent = statusResponse.bBYESent;
      }

    }

    return (static_cast<BASERESPONSE> (std::move(resp)));

}

//
// Parse EXPUNGE Response.
//

CMailIMAPParse::BASERESPONSE CMailIMAPParse::parseEXPUNGE(CMailIMAPParse::CommandData& commandData) {

    EXPUNGERESPONSE resp { new ExpungeResponse { commandData.commandCode } };
    
    for (std::string lineStr; std::getline(commandData.commandRespStream, lineStr, '\n');) {

        lineStr.pop_back(); // Remove linefeed

        if (lineStr.find(CMailIMAP::kEXISTSStr) != std::string::npos) {
            lineStr = stringUntaggedNumber(lineStr);
            resp->exists.push_back(std::strtoull(lineStr.c_str(), nullptr, 10));

        } else if (lineStr.find(CMailIMAP::kEXPUNGEStr) != std::string::npos) {
            lineStr = stringUntaggedNumber(lineStr);
            resp->expunged.push_back(std::strtoull(lineStr.c_str(), nullptr, 10));

        } else {
            BaseResponse statusResponse{ resp->command, resp->status, resp->errorMessageStr, resp->bBYESent};
            parseStatus(commandData.tagStr, lineStr, statusResponse);
            resp->status = statusResponse.status;
            resp->errorMessageStr = statusResponse.errorMessageStr;
            resp->bBYESent = statusResponse.bBYESent;
     }

    }

    return (static_cast<BASERESPONSE> (std::move(resp)));

}

//
// Parse STORE Response.
//

CMailIMAPParse::BASERESPONSE CMailIMAPParse::parseSTORE(CMailIMAPParse::CommandData& commandData) {

    STORERESPONSE resp { new StoreResponse { commandData.commandCode } };
    
    for (std::string lineStr; std::getline(commandData.commandRespStream, lineStr, '\n');) {

        StoreRespData storeData;

        lineStr.pop_back(); // Remove linefeed

        if (lineStr.find(CMailIMAP::kFETCHStr) != std::string::npos) {
            storeData.index = std::strtoull(stringUntaggedNumber(lineStr).c_str(), nullptr, 10);
            storeData.flagsListStr = stringList(stringList(lineStr).substr(1));
            resp->storeList.push_back(std::move(storeData));

        } else {
            BaseResponse statusResponse{ resp->command, resp->status, resp->errorMessageStr, resp->bBYESent};
            parseStatus(commandData.tagStr, lineStr, statusResponse);
            resp->status = statusResponse.status;
            resp->errorMessageStr = statusResponse.errorMessageStr;
            resp->bBYESent = statusResponse.bBYESent;
     }

    }

    return (static_cast<BASERESPONSE> (std::move(resp)));

}

//
// Parse CAPABILITY Response.
//

CMailIMAPParse::BASERESPONSE CMailIMAPParse::parseCAPABILITY(CMailIMAPParse::CommandData& commandData) {

    CAPABILITYRESPONSE resp { new CapabilityResponse { commandData.commandCode } };
    
    for (std::string lineStr; std::getline(commandData.commandRespStream, lineStr, '\n');) {

        lineStr.pop_back(); // Remove linefeed

        if (stringEqual(lineStr, CMailIMAP::kUntaggedStr + " " + CMailIMAP::kCAPABILITYStr)) {        
            resp->capabilitiesStr = lineStr.substr(std::string(CMailIMAP::kUntaggedStr + " " + CMailIMAP::kCAPABILITYStr).length() + 1);

        } else {
            BaseResponse statusResponse{ resp->command, resp->status, resp->errorMessageStr, resp->bBYESent};
            parseStatus(commandData.tagStr, lineStr, statusResponse);
            resp->status = statusResponse.status;
            resp->errorMessageStr = statusResponse.errorMessageStr;
            resp->bBYESent = statusResponse.bBYESent;
       }

    }

    return (static_cast<BASERESPONSE> (std::move(resp)));

}

//
// Parse NOOP/IDLE Response.
//

CMailIMAPParse::BASERESPONSE CMailIMAPParse::parseNOOP(CMailIMAPParse::CommandData& commandData) {

    NOOPRESPONSE resp { new NoOpResponse { commandData.commandCode } };
    
    for (std::string lineStr; std::getline(commandData.commandRespStream, lineStr, '\n');) {

        lineStr.pop_back(); // Remove linefeed

        if (lineStr.find(CMailIMAP::kUntaggedStr) == 0) {
            resp->rawResponse.push_back(std::move(lineStr));
        } else {
            BaseResponse statusResponse{ resp->command, resp->status, resp->errorMessageStr, resp->bBYESent};
            parseStatus(commandData.tagStr, lineStr, statusResponse);
            resp->status = statusResponse.status;
            resp->errorMessageStr = statusResponse.errorMessageStr;
            resp->bBYESent = statusResponse.bBYESent;
       }

    }

    return (static_cast<BASERESPONSE> (std::move(resp)));

}

//
// Parse FETCH Response
//

CMailIMAPParse::BASERESPONSE CMailIMAPParse::parseFETCH(CMailIMAPParse::CommandData& commandData) {

    FETCHRESPONSE resp { new FetchResponse { commandData.commandCode } };
    
    for (std::string lineStr; std::getline(commandData.commandRespStream, lineStr, '\n');) {

        FetchRespData fetchData;

        lineStr.pop_back(); // Add back '\n' as '\r\n' will be part of octet count
        
        int lineLength = lineStr.length()+CMailIMAP::kEOLStr.length();
        
        if (lineStr.find(CMailIMAP::kFETCHStr + " (") != std::string::npos) {

            fetchData.index = std::strtoull(stringUntaggedNumber(lineStr).c_str(), nullptr, 10);
            lineStr = lineStr.substr(lineStr.find_first_of('(') + 1);

            bool endOfFetch = false;

            do {

                if (stringEqual(lineStr, CMailIMAP::kBODYSTRUCTUREStr + " ")) {
                    parseList(CMailIMAP::kBODYSTRUCTUREStr, fetchData, lineStr);
                } else if (stringEqual(lineStr, CMailIMAP::kENVELOPEStr + " ")) {
                    parseList(CMailIMAP::kENVELOPEStr, fetchData, lineStr);
                } else if (stringEqual(lineStr, CMailIMAP::kFLAGSStr + " ")) {
                    parseList(CMailIMAP::kFLAGSStr, fetchData, lineStr);
                } else if (stringEqual(lineStr, CMailIMAP::kBODYStr + " ")) {
                    parseList(CMailIMAP::kBODYStr, fetchData, lineStr);
                } else if (stringEqual(lineStr, CMailIMAP::kINTERNALDATEStr + " ")) {
                    parseString(CMailIMAP::kINTERNALDATEStr, fetchData, lineStr);
                } else if (stringEqual(lineStr, CMailIMAP::kRFC822SIZEStr + " ")) {
                    parseNumber(CMailIMAP::kRFC822SIZEStr, fetchData, lineStr);
                } else if (stringEqual(lineStr, CMailIMAP::kUIDStr + " ")) {
                    parseNumber(CMailIMAP::kUIDStr, fetchData, lineStr);
                } else if (stringEqual(lineStr, CMailIMAP::kRFC822HEADERStr + " ")) {
                    parseOctets(CMailIMAP::kRFC822HEADERStr, fetchData, lineStr, commandData.commandRespStream);
                } else if (stringEqual(lineStr, CMailIMAP::kBODYStr + "[")) {
                    parseOctets(CMailIMAP::kBODYStr, fetchData, lineStr, commandData.commandRespStream);
                } else if (stringEqual(lineStr, CMailIMAP::kRFC822Str + " ")) {
                    parseOctets(CMailIMAP::kRFC822Str, fetchData, lineStr, commandData.commandRespStream);
                } else {
                    lineStr.clear(); // Error so force exception
                }

                // Still data to process
                
                if (lineStr.length() != 0) {
                    lineStr = lineStr.substr(lineStr.find_first_not_of(' '));   // Next non space.
                    if (lineStr[0] == ')') { // End of FETCH List
                        endOfFetch = true;
                    } else if (lineStr.length() == CMailIMAP::kEOLStr.length() - 1) { // No data left on line
                        std::getline(commandData.commandRespStream, lineStr, '\n');   // Move to next
                    }
                } else {
                    commandData.commandRespStream.seekg(-lineLength, std::ios_base::end); // Rewind read to get line
                    std::getline(commandData.commandRespStream, lineStr, '\n');
                    throw Exception("error while parsing FETCH command ["+lineStr+"]");
                }
          

            } while (!endOfFetch);

            resp->fetchList.push_back(std::move(fetchData));

        } else {
            BaseResponse statusResponse{ resp->command, resp->status, resp->errorMessageStr, resp->bBYESent};
            parseStatus(commandData.tagStr, lineStr, statusResponse);
            resp->status = statusResponse.status;
            resp->errorMessageStr = statusResponse.errorMessageStr;
            resp->bBYESent = statusResponse.bBYESent;
     }

    }

    return (static_cast<BASERESPONSE> (std::move(resp)));

}

//
// Parse LOGOUT Response
//

CMailIMAPParse::BASERESPONSE CMailIMAPParse::parseLOGOUT(CMailIMAPParse::CommandData& commandData) {

    LOGOUTRESPONSE resp { new LogOutResponse { commandData.commandCode } };
    
    for (std::string lineStr; std::getline(commandData.commandRespStream, lineStr, '\n');) {

        lineStr.pop_back(); // Remove linefeed

        if (stringEqual(lineStr, CMailIMAP::kUntaggedStr + " " + CMailIMAP::kBYEStr)) {
            resp->rawResponse.push_back(std::move(lineStr));
            resp->bBYESent = true;
            continue;
        } else {
             BaseResponse statusResponse{ resp->command, resp->status, resp->errorMessageStr, resp->bBYESent};
            parseStatus(commandData.tagStr, lineStr, statusResponse);
            resp->status = statusResponse.status;
            resp->errorMessageStr = statusResponse.errorMessageStr;
            resp->bBYESent = statusResponse.bBYESent;
       }
    }

    return (static_cast<BASERESPONSE> (std::move(resp)));

}

//
// Default Parse Response
//

CMailIMAPParse::BASERESPONSE CMailIMAPParse::parseDefault(CMailIMAPParse::CommandData& commandData) {

    BASERESPONSE resp { new BaseResponse { commandData.commandCode } };
    
    for (std::string lineStr; std::getline(commandData.commandRespStream, lineStr, '\n');) {
        
        lineStr.pop_back(); // Remove linefeed
        
        BaseResponse statusResponse { resp->command, resp->status, resp->errorMessageStr, resp->bBYESent};    
        parseStatus(commandData.tagStr, lineStr, statusResponse);    
        resp->status = statusResponse.status;
        resp->errorMessageStr = statusResponse.errorMessageStr;
        resp->bBYESent = statusResponse.bBYESent;
        
    }

    return (static_cast<BASERESPONSE> (std::move(resp)));

}

// ==============
// PUBLIC METHODS
// ==============

//
// Convert any lowercase characters in string to upper.
//

inline std::string CMailIMAPParse::stringToUpper(const std::string& lineStr) {

    std::string upperCaseStr(lineStr);
    for (auto &c : upperCaseStr) c = std::toupper(c);
    return (upperCaseStr);

}

//
// Perform case-insensitive string compare (return true strings are equal, false otherwise)
//

inline bool CMailIMAPParse::stringEqual(const std::string& lineStr, const std::string& compareStr) {

    int cnt01 = 0;
    if (lineStr.length() < compareStr.length()) return (false);
    for (auto &c : compareStr) if (std::toupper(c) != std::toupper(lineStr[cnt01++])) return (false);
    return (true);

}

//
// Extract the contents between two delimeters in command response line.
//

inline std::string CMailIMAPParse::stringBetween(const std::string& lineStr, const char first, const char last) {
    int firstDel = lineStr.find_first_of(first);
    int lastDel = lineStr.find_first_of(last, firstDel+1);
    return (lineStr.substr(firstDel + 1, (lastDel - firstDel - 1)));
}

//
// Extract number that may follow an un-tagged command response.
//

inline std::string CMailIMAPParse::stringUntaggedNumber(const std::string& lineStr) {

    int startNumber = lineStr.find_first_not_of(' ', 1);
    int endNumber = lineStr.find_first_of(' ', startNumber);
    return (lineStr.substr(startNumber, endNumber - startNumber));

}

//
// Extract tag from command response line.
//

inline std::string CMailIMAPParse::stringTag(const std::string& lineStr) {
    return (lineStr.substr(0, lineStr.find_first_of(' ')));
}

//
// Extract command string from command line. If the command has the UID 
// prefix then this is skipped over.
//

inline std::string CMailIMAPParse::stringCommand(const std::string& lineStr) {

    int startOfCommand = lineStr.find_first_of(' ') + 1;
    int endOfCommand = lineStr.find_first_of(' ', startOfCommand);

    if (stringEqual(lineStr.substr(startOfCommand, endOfCommand - startOfCommand), CMailIMAP::kUIDStr)) {
        startOfCommand = lineStr.find_first_of(' ', startOfCommand) + 1;
        endOfCommand = lineStr.find_first_of(' ', startOfCommand);
    }

    return (stringToUpper(lineStr.substr(startOfCommand, endOfCommand - startOfCommand)));

}

//
// Extract list  from command response line. Note: only check until 
// the end of line; the first character in linsStr is the start 
// of the list ie. a '('.
//

inline std::string CMailIMAPParse::stringList(const std::string& lineStr) {

    int bracketCount = 0, startPosition=0, currentIndex = 0, lineLength = lineStr.length();
    
    startPosition = lineStr.find_first_of('(');
    lineLength -= startPosition;
    
    currentIndex = startPosition;
    do {
        if (lineStr[currentIndex] == '(') bracketCount++;
        if (lineStr[currentIndex] == ')') bracketCount--;
        currentIndex++;
    } while (bracketCount && (--lineLength > 0));

    return (lineStr.substr(startPosition, currentIndex-startPosition));

}

//
// Parse Command Response. The response string is one long string containing "\r\n"s to
// signal end of lines. The string is read line by line converting the response to a istringstream 
// and using getline() and '\n' to signal the end of line character (except FETCH which is dealt 
// with differently as it has to cater for octet strings that can span multiple lines.
//

CMailIMAPParse::BASERESPONSE CMailIMAPParse::parseResponse(const std::string & commandResponseStr) {

    std::istringstream responseStream(commandResponseStr);
    std::string commandLineStr;
 
    if (stringToCodeMap.empty()) {
        stringToCodeMap=loadStringToCodeMap();
    }

    std::getline(responseStream, commandLineStr, '\n');
    commandLineStr.pop_back();

    ParseFunction parseFn;
    CommandData commandData{ stringTag(commandLineStr), stringToCodeMap[stringCommand(commandLineStr)], commandLineStr, responseStream};

    parseFn = parseCommmandMap[static_cast<int>(commandData.commandCode)];
    if (!parseFn) {
        parseCommmandMap[static_cast<int>(commandData.commandCode)] = parseDefault;
        parseFn = parseDefault;
    }

    return (parseFn(commandData));

}

//
// Return string for IMAP command code
//

std::string CMailIMAPParse::commandCodeString(CMailIMAPParse::Commands commandCode) {
    
    if (stringToCodeMap.empty()) {
        stringToCodeMap=loadStringToCodeMap();
    }
    
    for (auto commandEntry : stringToCodeMap) {
        if (commandEntry.second == commandCode) {
            return (commandEntry.first);
        }
    }

    Exception("commandCodeString() : Invalid command code.");

    return (""); // Never reached.

}

//
// Load IMAP command string to internal enum code map table
//

std::unordered_map<std::string, CMailIMAPParse::Commands> CMailIMAPParse::loadStringToCodeMap() {

    std::unordered_map<std::string, CMailIMAPParse::Commands> map 
    {
        { CMailIMAP::kSTARTTLSStr, Commands::STARTTLS},
        { CMailIMAP::kAUTHENTICATEStr, Commands::AUTHENTICATE},
        { CMailIMAP::kLOGINStr, Commands::LOGIN},
        { CMailIMAP::kCAPABILITYStr, Commands::CAPABILITY},
        { CMailIMAP::kSELECTStr, Commands::SELECT},
        { CMailIMAP::kEXAMINEStr, Commands::EXAMINE},
        { CMailIMAP::kCREATEStr, Commands::CREATE},
        { CMailIMAP::kDELETEStr, Commands::DELETE},
        { CMailIMAP::kRENAMEStr, Commands::RENAME},
        { CMailIMAP::kSUBSCRIBEStr, Commands::SUBSCRIBE},
        { CMailIMAP::kUNSUBSCRIBEStr, Commands::UNSUBSCRIBE},
        { CMailIMAP::kLISTStr, Commands::LIST},
        { CMailIMAP::kLSUBStr, Commands::LSUB},
        { CMailIMAP::kSTATUSStr, Commands::STATUS},
        { CMailIMAP::kAPPENDStr, Commands::APPEND},
        { CMailIMAP::kCHECKStr, Commands::CHECK},
        { CMailIMAP::kCLOSEStr, Commands::CLOSE},
        { CMailIMAP::kEXPUNGEStr, Commands::EXPUNGE},
        { CMailIMAP::kSEARCHStr, Commands::SEARCH},
        { CMailIMAP::kFETCHStr, Commands::FETCH},
        { CMailIMAP::kSTOREStr, Commands::STORE},
        { CMailIMAP::kCOPYStr, Commands::COPY},
        { CMailIMAP::kUIDStr, Commands::UID},
        { CMailIMAP::kNOOPStr, Commands::NOOP},
        { CMailIMAP::kLOGOUTStr, Commands::LOGOUT},
        { CMailIMAP::kIDLEStr, Commands::IDLE}
    
    };

    return (map);

}

//
// Main CMailIMAPParse object constructor. 
//

CMailIMAPParse::CMailIMAPParse() {

}

//
// CMailIMAPParse Destructor
//

CMailIMAPParse::~CMailIMAPParse() {

}