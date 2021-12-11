#include <iostream>
#include <fstream>
#include <vector>

#define SIMPLEDATA_VERSION "1.0"

#include "parser.hpp"
std::vector<storage> identifiers;

int errors = 0;
int main(int charc, char** charv)
{
    if (charc < 2)
    {
        std::cout << "simpledata: [ERROR]: No arguments provided!\n";
        return -1;
    }

    if (strcmp(charv[1], "-v") == 0)
    {
        std::cout << "SimpleData Version " << SIMPLEDATA_VERSION << "\n";
        return 0;
    }

    std::ifstream file(charv[1]);
    if (file == NULL)
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

            // Check for any reserved names
            if (identifier == "boolean" || identifier == "bool" || identifier == "float" || identifier == "int" || identifier == "integer" || identifier == "string" || identifier == "char" || identifier == "null" || identifier == "NULL")
            {
                error("Reserved identifier used on line " + std::to_string(current_line), errors);
                continue;
            }

            for (int i = 0; i < identifier.size(); i++)
            {
                if (identifier[i] == ' ')
                {
                    error("Expected non-whitespace character on line " + std::to_string(current_line) + ", column " + std::to_string(i + 1), errors);
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

                        /**
                         * Checking for comments after the value
                         * This is not supported in the API and currently I
                         * cannot be bothered to fix that however I may do so
                         * in a future version.
                        */
                        for (; i < value.size() && value[i] != '#'; i++);

                        if (value[i] == '#')
                        {
                            error("Comments after values are not supported. This will be fixed in the next version. Line " + std::to_string(current_line) + ".", errors);
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

                    // Once again, checking for comments after the value. This is simply a stopgap solution.
                    for (; end < value.size() && value[end] != '#'; end++);

                    if (value[end] == '#')
                    {
                        error("Comments after values are not supported. This will be fixed in the next version. Line " + std::to_string(current_line) + ".", errors);
                        continue;
                    }
                }
                break;
                default:
                {
                    // Checking for comments
                    int i = 0;
                    for (; i < value.size() && value[i] != '#'; i++);

                    if (value[i] == '#')
                    {
                        error("Comments after values are not supported. This will be fixed in the next version. Line " + std::to_string(current_line) + ".", errors);
                        continue;
                    }

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
            continue; // Only here for readability - not actually needed but has no negative impact on performance
    }
    file.close();
    std::cout << "simpledata: Finished with " << errors << " error\\s.\n";
}