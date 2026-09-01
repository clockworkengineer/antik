#include "CProgramOptions.hpp"
#include "CommonUtil.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace Antik::Util
{
    CProgramOptions::CProgramOptions(const std::string &programDescription)
        : m_description(programDescription)
    {
    }

    void CProgramOptions::addOption(const std::string &flagShort, const std::string &flagLong, const std::string &description, bool hasValue, const std::string &defaultValue)
    {
        Option opt;
        opt.flagShort = flagShort;
        opt.flagLong = flagLong;
        opt.description = description;
        opt.hasValue = hasValue;
        opt.defaultValue = defaultValue;
        opt.value = defaultValue;
        opt.isSet = false;
        m_options.push_back(opt);
    }

    bool CProgramOptions::parse(int argc, char **argv)
    {
        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];
            for (auto &opt : m_options)
            {
                if ((!opt.flagShort.empty() && arg == "-" + opt.flagShort) ||
                    (!opt.flagLong.empty() && arg == "--" + opt.flagLong))
                {
                    opt.isSet = true;
                    if (opt.hasValue && i + 1 < argc)
                    {
                        opt.value = argv[++i];
                    }
                    break;
                }
            }
        }
        return true;
    }

    bool CProgramOptions::loadConfigFile(const std::string &configFilePath)
    {
        std::ifstream configFile(configFilePath);
        if (!configFile.is_open()) return false;
        std::string line;
        while (std::getline(configFile, line))
        {
            line = StringUtil::trim(line);
            if (line.empty() || line.front() == '#') continue;
            auto pos = line.find('=');
            if (pos != std::string::npos)
            {
                std::string key = StringUtil::trim(line.substr(0, pos));
                std::string val = StringUtil::trim(line.substr(pos + 1));
                for (auto &opt : m_options)
                {
                    if (opt.flagLong == key || opt.flagShort == key)
                    {
                        opt.value = val;
                        opt.isSet = true;
                        break;
                    }
                }
            }
        }
        return true;
    }

    bool CProgramOptions::count(const std::string &name) const
    {
        for (const auto &opt : m_options)
        {
            if ((opt.flagLong == name || opt.flagShort == name) && opt.isSet)
            {
                return true;
            }
        }
        return false;
    }

    std::string CProgramOptions::get(const std::string &name) const
    {
        for (const auto &opt : m_options)
        {
            if (opt.flagLong == name || opt.flagShort == name)
            {
                return opt.value;
            }
        }
        return "";
    }

    int CProgramOptions::getInt(const std::string &name) const
    {
        std::string val = get(name);
        if (val.empty()) return 0;
        try {
            return std::stoi(val);
        } catch (...) {
            return 0;
        }
    }

    void CProgramOptions::printHelp(std::ostream &os) const
    {
        if (!m_description.empty())
        {
            os << m_description << "\n\n";
        }
        os << "Options:\n";
        for (const auto &opt : m_options)
        {
            os << "  ";
            if (!opt.flagShort.empty()) os << "-" << opt.flagShort << ", ";
            if (!opt.flagLong.empty()) os << "--" << opt.flagLong << "\t";
            os << opt.description;
            if (!opt.defaultValue.empty()) os << " (default: " << opt.defaultValue << ")";
            os << "\n";
        }
    }
} // namespace Antik::Util
