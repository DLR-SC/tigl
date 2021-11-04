/*
* Copyright (C) 2021 German Aerospace Center (DLR/SC)
*
* Created: 2021-11-02 Martin Siggel <Martin.Siggel@dlr.de>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef VERSION_H
#define VERSION_H

#include <string>
#include <cctype>
#include <regex>
#include <exception>

/**
 * @brief This class implements semver 2.0.0 with one modification:
 *
 * For backwards compatibility we allow to ommit the patch version.
 * A missing patch version is assumed to have the patch version number
 * of 0.
 */
class Version
{
public:
    explicit Version(const std::string& str)
        : m_major(0),
          m_minor(0),
          m_patch(0)
    {
        parse(str);
    }

    int vMajor() const
    {
        return m_major;
    }

    int vMinor() const
    {
        return m_minor;
    }

    int vPatch() const
    {
        return m_patch;
    }

    std::string vLabel() const
    {
        return m_label;
    }

    std::string vBuild() const
    {
        return m_build;
    }

    bool operator< (const Version& other) const
    {
        if (vMajor() < other.vMajor()) {
            return true;
        }
        else if (vMajor() > other.vMajor()) {
            return false;
        }

        // major Versions are same
        if (vMinor() < other.vMinor()) {
            return true;
        }
        else if (vMinor() > other.vMinor()) {
            return false;
        }

        // major and minor are same
        if (vPatch() < other.vPatch()) {
            return true;
        }
        else if (vPatch() > other.vPatch()) {
            return false;
        }

        if (comparePreleases(vLabel(), other.vLabel())) {
            return true;
        }
        else if (comparePreleases(other.vLabel(), vLabel())) {
            return false;
        }

        // TODO: compare builds right now

        return false;
    }

    bool operator> (const Version& other) const
    {
        return other < *this;
    }

    bool operator== (const Version& other) const
    {
        return !(*this < other) && !(*this > other);
    }

private:

    int m_major, m_minor, m_patch;
    std::string m_label, m_build;

    void parse(const std::string& str)
    {
        // Copied and modified from: https://semver.org/#is-there-a-suggested-regular-expression-regex-to-check-a-semver-string
        const auto expr = std::regex(R"(^(0|[1-9]\d*)\.(0|[1-9]\d*)(\.(0|[1-9]\d*))?(?:-((?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*))"
                                     R"((?:\.(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?(?:\+([0-9a-zA-Z-]+(?:\.[0-9a-zA-Z-]+)*))?$)");
        auto results = std::match_results<std::string::const_iterator>();

        if (std::regex_search(str, results, expr)) {
            m_major = atof(results[1].str().c_str());
            m_minor = atof(results[2].str().c_str());
            m_patch = atof(results[4].str().c_str());
            m_label = results[5];
            m_build = results[6];
        }
        else {
            throw std::invalid_argument("Invalid format of version string '" + str + "'." );
        }
    }

    // Returns true, if pr1 < pr2
    static bool comparePreleases(const std::string& pr1, const std::string& pr2)
    {
        // pre-release version is always lower than release version
        if (!pr1.empty() && pr2.empty()) {
            return true;
        }
        else if (pr1.empty() && !pr2.empty()) {
            return false;
        }

        auto pr1fields = split(pr1, '.');
        auto pr2fields = split(pr2, '.');

        auto nSameFields = std::min(pr1fields.size(), pr2fields.size());

        for (unsigned int i = 0; i < nSameFields; ++i) {
            if (comparePreleaseField(pr1fields[i], pr2fields[i])) {
                return true;
            }
            else if (comparePreleaseField(pr2fields[i], pr1fields[i])) {
                return false;
            }
        }

        // all until nSameField are same, only field size matterss
        return pr1fields.size() < pr2fields.size();

    }

    static bool comparePreleaseField(const std::string& pr1, const std::string& pr2)
    {

        if (isNumber(pr1) && isNumber(pr2)) {
            return std::stoi(pr1) < std::stoi(pr2);
        }
        else if (isNumber(pr1) && !isNumber(pr2)) {
            return true;
        }
        else {
            return lexicographicalCompare(pr1.begin(), pr1.end(), pr2.begin(), pr2.end());
        }

        return false;
    }

    static bool isNumber(const std::string& str)
    {
        auto iter = str.begin();
        while (iter != str.end() && std::isdigit(*iter)) ++iter;
        return !str.empty() && iter == str.end();
    }

    static std::vector<std::string> split(const std::string& s, char delimiter)
    {
       std::vector<std::string> tokens;
       std::string token;
       std::istringstream tokenStream(s);
       while (std::getline(tokenStream, token, delimiter))
       {
          tokens.push_back(token);
       }
       return tokens;
    }

    template<class InputIt1, class InputIt2>
    static bool lexicographicalCompare(InputIt1 first1, InputIt1 last1,
                                 InputIt2 first2, InputIt2 last2)
    {
        for ( ; (first1 != last1) && (first2 != last2); ++first1, (void) ++first2 ) {
            if (*first1 < *first2) return true;
            if (*first2 < *first1) return false;
        }
        return (first1 == last1) && (first2 != last2);
    }

};

#endif // VERSION_H
