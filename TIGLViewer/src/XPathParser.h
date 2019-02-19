//
// Created by cfse on 1/29/19.
//

#ifndef TIGL_XPATHPARSER_H
#define TIGL_XPATHPARSER_H

#include <string>


namespace cpcr {

    /**
     * @brief Helper class to parse simple xpath.
     */
    class XPathParser {
    public:

        // functions to retrieve information from a xpath
        static std::string GetFirstNodeType(std::string xpath) ;
        static std::string GetLastNodeType(std::string xpath);
        static int GetFirstNodeIndex(std::string xpath);
        static int GetLastNodeIndex(std::string xpath);

        // function to transform a xpath
        static std::string AddNodeAtEnd(std::string baseXpath, std::string nodeToAdd);

        static std::string RemoveFirstNode(std::string xpath);
        static std::string RemoveLastNode(std::string xpath);



    protected:

        static std::string GetFirstNode(std::string xpath);
        static std::string GetLastNode(std::string xpath);
        // return the same string without the brackets and the content
        static std::string RemoveEndingBrackets(std::string string);
        static int GetIndexOfNode(std::string particle);

    };
}

#endif //TIGL_XPATHPARSER_H
