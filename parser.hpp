#pragma once
#include <string>
#include <iostream>

struct storage
{
    std::string identifier;
    int line;
};

void error(const std::string err, int& counter)
{
    counter++;
    std::cout << "simpledata: [ERROR]: " << err << "\n";
}

void remove_leading(std::string& string)
{
    std::string buffer;

    // Find when the leading whitespace ends
    int leading_end = 0;
    for (; leading_end < string.size() && string[leading_end] == ' '; leading_end++);

    for (int i = leading_end; i < string.size(); i++) buffer += string[i];
    string = buffer;
}

void remove_trailing(std::string& string)
{
    std::string buffer;

    // Find where the trailing whitespace starts
    int end = string.size() - 1;
    for (; string[end] == ' ' && end >= 0; end--);

    // Copy the string to the destination
    for (int i = 0; i != end + 1; i++) buffer += string[i];
    string = buffer;
}