#include <iostream>
#include <fstream>
#include <vector>

#include "parser.hpp"
std::vector<storage> identifiers;

int errors = 0;
int main(int charc, char** charv)
{
    if (charc < 2 || (charc >= 2 && strcmp(charv[1], "-h") == 0))
    {
        std::cout << "Usage:\n";
        std::cout << "\t" << charv[0] << " -hv\n\n";

        std::cout << "\t- The -h argument displays this message,\n";
        std::cout << "\t- and the -v argument prints the version number of the parser\n\n";

        std::cout << "\t" << charv[0] << " filename\n\n";

        std::cout << "\t- The filename passed must be a valid SimpleData file\n";
        std::cout << "\t- with a .simpdat extension and will be the file parsed\n\n";

        std::cout << "\t" << charv[0] << " filename language\n\n";

        std::cout << "\t- The language passed will govern what identifier restrictions\n";
        std::cout << "\t- will be enforced\n";
        std::cout << "\t- Language-specific keywords will be looked for and flagged\n";

        return 0;
    }

    if (strcmp(charv[1], "-v") == 0)
    {
        std::cout << "SimpleData Version " << SIMPLEDATA_VERSION << "\n";
        return 0;
    }

    // Add elements to the "restricted::supported" array
    restricted_init();

    // Language-specific identifier restrictions
    if (charc >= 3) restricted::input = charv[2];

    // Identifying the language inputted - spits out warning if inputted language is not supported
    for (int i = 0; i < restricted::supported.size(); i++)
    {
        for (int j = 0; j < restricted::supported[i].size(); j++)
        {
            if (restricted::input == restricted::supported[i][j])
            {
                switch (i)
                {
                    case 0:
                        restricted::lang = "C++";
                        std::cout << "simpledata: [INFO]: Enforcing C++ identifier restrictions\n";
                    break;
                    case 1:
                        restricted::lang = "Python";
                        std::cout << "simpledata: [INFO]: Enforcing Python identifier restrictions\n";
                    break;
                }
            }
        }
    }
    if (restricted::input != "" && restricted::lang == "Default") std::cout << "simpledata: [WARNING]: Inputted language unknown, default restrictions will be enforced\n";
    else if (restricted::lang == "Default") std::cout << "simpledata: [INFO]: Enforcing default identifier restrictions\n";

    std::ifstream file(charv[1]);
    if (!file.is_open())
    {
        file.close();
        std::cout << "simpledata: [ERROR]: File invalid!\n";
        return -1;
    }

    // Check that the file is a valid SimpleData file
    int ext_start;
    std::string file_ext;

    for (ext_start = strlen(charv[1]); ext_start >= 0 && charv[1][ext_start] != '.'; ext_start--);

    // Checking that the file has an extension
    if (charv[1][ext_start] != '.')
    {
        std::cout << "simpledata: ERROR: File passed must be a valid SimpleData file\n";
        file.close();

        return -1;
    }
    for (int i = ext_start; i < strlen(charv[1]); i++) file_ext += charv[1][i];

    if (file_ext != ".simpdat")
    {
        std::cout << "simpledata: ERROR: File passed must be a valid SimpleData file\n";
        file.close();

        return -1;
    }

    // Best way to keep track of lines
    for (int current_line = 1; !file.eof(); current_line++)
    {
        std::string line;
        std::getline(file, line);

        // Ignore comments and blank lines
        if (line[0] != '#' && line != "" && line != "\n") {
            if (line[0] == ':')
            {
                error("Expected identifier on line " + std::to_string(current_line), errors);
                continue;
            }
            std::string identifier, value;

            // Parse the identifier and the value
            int value_start;
            for (int i = 0; line[i] != ':'; i++)
            {
                identifier += line[i];
                value_start = i;
            }

            remove_leading(identifier);
            remove_trailing(identifier);

            if (identifier == "null" || identifier == "NULL")
            {
                error("Reserved identifier used on line " + std::to_string(current_line), errors);
                continue;
            }

            // Check for any reserved names
            if (restricted::lang == "C++")
            {
                for (int i = 0; i < restricted::cpp.size(); i++)
                {
                    if (identifier == restricted::cpp[i])
                    {
                        error("Reserved identifier used on line " + std::to_string(current_line), errors);
                        goto outside;
                    }
                }
            }
            else if (restricted::lang == "Python")
            {
                for (int i = 0; i < restricted::python.size(); i++)
                {
                    if (identifier == restricted::python[i])
                    {
                        error("Reserved identifier used on line " + std::to_string(current_line), errors);
                        goto outside;
                    }
                }
            }
            else
            {
                for (int i = 0; i < restricted::dfault.size(); i++)
                {
                    if (identifier == restricted::dfault[i])
                    {
                        error("Reserved identifier used on line " + std::to_string(current_line), errors);
                        goto outside;
                    }
                }
            }

            for (int i = 0; i < identifier.size(); i++)
            {
                if (identifier[i] == ' ')
                {
                    error("Expected non-whitespace character on line " + std::to_string(current_line) + ", column " + std::to_string(i + 1), errors);
                    continue;
                }
                else if (identifier[i] == '+')
                {
                    error("Invalid character in identifier, \'+\' not allowed on line " + std::to_string(current_line) + ", column " + std::to_string(i + 1), errors);
                    continue;
                }
            }

            // Copy the line to the value starting from the end of the identifier + 2
            for (int i = value_start + 2; i < line.size(); i++) value += line[i];

            // Remove leading and trailing whitespace from the value
            remove_leading(value);
            remove_trailing(value);

            // Making sure that the value is valid
            switch(value[0])
            {
                case '"':
                {
                    if (value[value.size() - 1] != '"')
                    {
                        int i = 1;

                        // Scan the value for an ending quote (")
                        for (; i < value.size() && value[i] != '"'; i++);

                        // Assuming i == value.size() - 1
                        if (value[i] != '"')
                        {
                            error("Invalid value on line " + std::to_string(current_line) + ", expected closing '\"'", errors);
                            continue;
                        }
                    }
                }
                break;
                case '\'':
                {
                    int end;
                    for (end = 1; end < value.size() && value[end] != '\''; end++);

                    // The ending ' is recorded in the iteration
                    if (value[end] != '\'')
                    {
                        error("Invalid value on line " + std::to_string(current_line) + ", expected closing '\''", errors);
                    }
                    else if ((end - 1) > 1)
                    {
                        // This only looks for how many characters are inside the value so we have to minus the "end" variable by one
                        error("Invalid value on line " + std::to_string(current_line) + ", extra characters in a one-character type", errors);
                        continue;
                    }
                }
                break;
                case '[':
                {
                    // Getting the elements of the list and put them into a vector to be checked
                    std::vector<std::string> elements;
                    for (int i = 1; i < value.size();)
                    {
                        int j = i;
                        std::string element = "";
                        for (; j < value.size() && value[j] != ','; j++)
                        {
                            element += value[j];
                        }

                        remove_leading(element);
                        remove_trailing(element);

                        elements.push_back(element);

                        i = j + 1;
                    }

                    int i;
                    for (i = 0; i < elements.size(); i++)
                    {
                        // Checking all the individual elements of the list
                        switch (elements[i][0])
                        {
                            case '\"':
                            {
                                int j = 1;
                                for (; j < elements[i].size() && elements[i][j] != '\"'; j++);

                                if (elements[i][j] != '\"')
                                {
                                    error("Invalid value on line " + std::to_string(current_line) + ", element #" + std::to_string(i + 1) + ". expected closing '\"'", errors);
                                }

                                // Exit the array if the value ends
                                if (j < elements[i].size())
                                {
                                    for (; j < elements[i].size(); j++)
                                    {
                                        if (elements[i][j] == ']') goto outside;
                                    }
                                }
                            }
                            break;
                            case '\'':
                            {
                                int j = 1;
                                for (; j < elements[i].size() && elements[i][j] != '\''; j++);

                                if (elements[i][j] != '\'')
                                {
                                    error("Invalid value on line " + std::to_string(current_line) + ", element #" + std::to_string(i + 1) + ". expected closing '\''", errors);
                                }
                                else if (j != 2)
                                {
                                    error("Invalid value on line " + std::to_string(current_line) + ", element #" + std::to_string(i + 1) + ". extra characters in one-character type", errors);
                                }

                                if (j < elements[i].size())
                                {
                                    for (; j < elements[i].size(); j++)
                                    {
                                        if (elements[i][j] == ']') goto outside;
                                    }
                                }
                            }
                            break;
                            default:
                            {
                                int j;
                                if (elements[i][0] >= '0' && elements[i][0] <= '9')
                                {
                                    int decimal_points = 0;
                                    for (j = 1; j < elements[i].size() && elements[i][j] != ']'; j++)
                                    {
                                        if (elements[i][j] == '.')
                                        {
                                            decimal_points++;
                                        }
                                        else if (!(elements[i][j] >= '0' && elements[i][j] <= '9'))
                                        {
                                            error("Invalid value on line " + std::to_string(current_line) + ", element #" + std::to_string(i + 1) + ". expected a numeric value or '.'", errors);
                                        }
                                    }
                                    if (decimal_points > 1)
                                    {
                                        error("Invalid value on line " + std::to_string(current_line) + ", element #" + std::to_string(i + 1) + ". excessive decimal points", errors);
                                    }
                                }
                                else
                                {
                                    std::string actual_value = "";
                                    for (j = 0; j < elements[i].size(); j++)
                                    {
                                        if (elements[i][j] == ']') break;
                                        else actual_value += elements[i][j];
                                    }

                                    if (actual_value != "true" && actual_value != "false" && actual_value != "null" && actual_value != "NULL")
                                    {   
                                        error("Invalid value on line " + std::to_string(current_line) + ", element #" + std::to_string(i + 1) + ". type cannot be identified", errors);
                                    }
                                    if (elements[i][j] == ']') goto outside;
                                }
                            }
                        }
                    }

                    if (i == elements.size())
                    {
                        error("Invalud value on line " + std::to_string(current_line) + ", unterminated array", errors);
                    }
                }
                break;
                default:
                {
                    // Checking for comments
                    int i = 0;
                    for (; i < value.size() && value[i] != '#'; i++);

                    if (value[0] >= '0' && value[0] <= '9')
                    {
                        int decimal_points = 0;
                        for (int i = 1; i < value.size(); i++)
                        {
                            if (value[i] == '.')
                            {
                                decimal_points++;
                            }
                            else if (!(value[i] >= '0' && value[i] <= '9'))
                            {
                                error("Invalid value on line " + std::to_string(current_line) + ", column " + std::to_string(i + 1) + ", expected numeric value or '.'", errors);
                                goto outside;
                            }

                            if (decimal_points > 1)
                            {
                                error("Invalid value on line " + std::to_string(current_line) + ", excessive decimal points", errors);
                                goto outside;
                            }
                        }
                        break;
                    }

                    if (value != "true" && value != "false" && value != "null" && value != "NULL")
                    {
                        error("Invalid value on line " + std::to_string(current_line) + ", type cannot be identified", errors);
                        continue;
                    }
                }
                break;
            }

            // Checking for identifier uniqueness
            int index = 0;
            for (; index < identifiers.size(); index++)
            {
                if (identifier == identifiers[index].identifier)
                {
                    error("Variable \"" + identifier + "\" on line " + std::to_string(current_line) + " predefined on line " + std::to_string(identifiers[index].line), errors);
                    break;
                }
            }
            
            // If the identifier is unique, put it into the identifiers list
            if (index == identifiers.size())
            {
                storage buffer = {identifier, current_line};
                identifiers.push_back(buffer);
            }
        }
        outside:
            continue;
    }
    file.close();
    std::cout << "simpledata: Finished with " << errors << " error\\s.\n";
}
