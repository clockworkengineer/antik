#ifndef CPROGRAMOPTIONS_HPP
#define CPROGRAMOPTIONS_HPP

#include <string>
#include <vector>
#include <iostream>

namespace Antik::Util
{
    //
    // Native C++17 Command Line & Config Option Parser (Replaces boost::program_options)
    //
    class CProgramOptions
    {
    public:
        struct Option
        {
            std::string flagShort;
            std::string flagLong;
            std::string description;
            bool hasValue{false};
            std::string value;
            std::string defaultValue;
            bool isSet{false};
        };

        explicit CProgramOptions(const std::string &programDescription = "");

        void addOption(const std::string &flagShort, const std::string &flagLong, const std::string &description, bool hasValue = false, const std::string &defaultValue = "");
        bool parse(int argc, char **argv);
        bool loadConfigFile(const std::string &configFilePath);

        bool count(const std::string &name) const;
        std::string get(const std::string &name) const;
        int getInt(const std::string &name) const;
        void printHelp(std::ostream &os = std::cout) const;

    private:
        std::string m_description;
        std::vector<Option> m_options;
    };
} // namespace Antik::Util

#endif /* CPROGRAMOPTIONS_HPP */
