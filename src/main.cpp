#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

#define THROW(error) throw std::runtime_error("\n\t" + std::string(error) + "\n\tfrom '" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "'\n\tin " + std::string(__PRETTY_FUNCTION__));

template <class A, class B>
using map = std::unordered_map<A, B>;

const static string HRML_LABEL_CHARS = "azertyuiopqsdfghjklmwxcvbnAZERTYUIOPQSDFGHJKLMWXCVBN1234567890-.!_&$%";
const static string HRML_QUERY_CHARS = "azertyuiopqsdfghjklmwxcvbnAZERTYUIOPQSDFGHJKLMWXCVBN1234567890-_";

const static char HRML_OPEN_TOKEN = '<';
const static char HRML_CLOSE_TOKEN = '>';
const static char HRML_SLASH_TOKEN = '/';
const static char HRML_EQUAL_TOKEN = '=';
const static char HRML_QUOTE_TOKEN = '"';
const static char HRML_SPACE_TOKEN = ' ';

const static char HRML_QUERY_NAME = '.';
const static char HRML_QUERY_PROP = '~';

namespace lexer {
enum TYPE { OPEN, SLASH, CLOSE, LABEL, EQUAL, QUOTE };

struct Token {
    TYPE type;
    string value;
};

string to_string(Token token) {
    switch (token.type) {
        case lexer::OPEN: return "OPEN";
        case lexer::SLASH: return "SLASH";
        case lexer::CLOSE: return "CLOSE";
        case lexer::LABEL: return "LABEL[" + token.value + "]";
        case lexer::EQUAL: return "EQUAL";
        case lexer::QUOTE: return "QUOTE";
        default: THROW("unexpected token type");
    }
}

vector<Token> lexer_HRML(string content) {
    vector<Token> tokens = {};
    Token token;

    string buffer = "";
    int i = 0;

    while (i < content.length()) {
        // Simple char, fill buffer
        if (HRML_LABEL_CHARS.find(content[i]) != string::npos) {
            buffer += content[i];
            i++;
            continue;
        }

        // Else, we have a token (extract buffer)
        if (buffer != "") {
            token.type = LABEL;
            token.value = buffer;
            tokens.push_back(token);
            buffer = "";
        }

        // Parse token
        switch (content[i]) {
            case HRML_OPEN_TOKEN: {
                token.type = OPEN;
                token.value = "";
                tokens.push_back(token);
            } break;
            case HRML_CLOSE_TOKEN: {
                token.type = CLOSE;
                token.value = "";
                tokens.push_back(token);
            } break;
            case HRML_SLASH_TOKEN: {
                token.type = SLASH;
                token.value = "";
                tokens.push_back(token);
            } break;
            case HRML_EQUAL_TOKEN: {
                token.type = EQUAL;
                token.value = "";
                tokens.push_back(token);
            } break;
            case HRML_QUOTE_TOKEN: {
                token.type = QUOTE;
                token.value = "";
                tokens.push_back(token);
            } break;
            case HRML_SPACE_TOKEN: break;
            default: string err = "unexpected char encountered ('" + std::string{content[i]} + "' at pos " + std::to_string(i) + ")"; THROW(err);
        }

        i++;
    }
    return tokens;
}

}  // namespace lexer

namespace parser {

enum TYPE { OPEN, CLOSE };

struct Token {
    TYPE type;
    string name;
    map<string, string> props = {};
};

string to_string(Token token) {
    switch (token.type) {
        case parser::OPEN: return "OPEN[" + token.name + "]";
        case parser::CLOSE: return "CLOSE[" + token.name + "]";
        default: THROW("unexpected token type");
    }
}

void next(vector<lexer::Token> tokens, int &index) {
    index++;
    if (index >= tokens.size()) THROW("unexpected end of file");
}

Token parser_close(vector<lexer::Token> tokens, int &index, Token &token) {
    return token;
}

Token parser_props(vector<lexer::Token> tokens, int &index, Token &token) {
    while (tokens[index].type != lexer::CLOSE) {
        if (tokens[index].type != lexer::LABEL) THROW("unexpected token encountered");
        string label = tokens[index].value;
        next(tokens, index);

        if (tokens[index].type != lexer::EQUAL) THROW("unexpected token encountered");
        next(tokens, index);

        if (tokens[index].type != lexer::QUOTE) THROW("unexpected token encountered");
        next(tokens, index);

        if (tokens[index].type != lexer::LABEL) THROW("unexpected token encountered");
        string value = tokens[index].value;
        next(tokens, index);

        if (tokens[index].type != lexer::QUOTE) THROW("unexpected token encountered");
        next(tokens, index);

        token.props[label] = value;
    }

    return parser_close(tokens, index, token);
}

Token parser_name(vector<lexer::Token> tokens, int &index, Token &token) {
    token.name = tokens[index].value;

    next(tokens, index);

    return parser_props(tokens, index, token);
}

Token parser_slash(vector<lexer::Token> tokens, int &index, Token &token) {
    next(tokens, index);

    switch (tokens[index].type) {
        case lexer::LABEL: return parser_name(tokens, index, token);
        default: THROW("unexpected token encountered");
    }
}

Token parser_open(vector<lexer::Token> tokens, int &index, Token &token) {
    next(tokens, index);

    switch (tokens[index].type) {
        case lexer::SLASH: {
            token.type = CLOSE;
            return parser_slash(tokens, index, token);
        };
        case lexer::LABEL: {
            token.type = OPEN;
            return parser_name(tokens, index, token);
        };
        default: THROW("unexpected token encountered");
    }
}

vector<Token> parser_HRML(vector<lexer::Token> tokens) {
    int index = 0;
    vector<Token> parsed = {};

    while (index < tokens.size()) {
        if (tokens[index].type != lexer::OPEN) {
            string err = "unexpected start of token (got '" + to_string(tokens[index]) + "' instead of 'OPEN' at " + std::to_string(index) + ")";
            THROW(err);
        }

        Token token;

        parsed.push_back(parser_open(tokens, index, token));

        if (index < tokens.size()) index++;
    }

    return parsed;
}

}  // namespace parser

namespace HRML {

struct Element {
    string name;
    map<string, string> props = {};
    vector<Element> childs = {};
};

string to_string(Element elem) {
    string s = "\n<" + elem.name;
    for (auto &it : elem.props) s += " " + it.first + "=\"" + it.second + "\"";
    s += ">";

    for (auto child : elem.childs) s += to_string(child);

    s += "\n</" + elem.name + ">";

    return s;
}

void next(vector<parser::Token> tokens, int &index) {
    index++;
    if (index >= tokens.size()) THROW("unexpected end of tokens");
}

Element visitor_HRML(vector<parser::Token> tokens, int &index) {
    if (tokens[index].type != parser::OPEN) THROW("unexpected start of tokens");

    Element elem;

    elem.name = tokens[index].name;
    elem.props = tokens[index].props;

    next(tokens, index);

    while (tokens[index].type != parser::CLOSE && tokens[index].name != elem.name) {
        elem.childs.push_back(visitor_HRML(tokens, index));
        next(tokens, index);
    }

    return elem;
}

Element generator_HRML(vector<parser::Token> tokens) {
    int index = 0;

    Element root;
    root.name = "_root_";
    root.childs = {};

    while (index < tokens.size()) {
        root.childs.push_back(visitor_HRML(tokens, index));
        index++;
    }

    return root;
}

vector<string> query_parser(string query) {
    vector<string> tokens = {};

    string buffer = string(1, HRML_QUERY_NAME);  // queries always start with an element selector

    for (int i = 0; i < query.length(); i++) {
        if (HRML_QUERY_CHARS.find(query[i]) != string::npos) {
            buffer += query[i];
        } else {
            switch (query[i]) {
                case HRML_QUERY_NAME:
                case HRML_QUERY_PROP: {
                    // push last label
                    tokens.push_back(buffer);
                    buffer = "";

                    // push current token
                    buffer += query[i];
                } break;
                default: THROW("unexpected query token")
            }
        }
    }

    // push back last label
    tokens.push_back(buffer);

    return tokens;
}

string query_search(vector<string> tokens, Element elem, int index) {
    if (index >= tokens.size()) THROW("out of bounds")

    string token = tokens[index];
    if (!token.length()) THROW("unexpected empty query token")

    char selector = token[0];
    string label = token.substr(1);

    switch (selector) {
        case HRML_QUERY_NAME: {
            for (Element child : elem.childs) {
                if (child.name == label) return query_search(tokens, child, index + 1);
            }

            // string err = "query label '" + label + "' was not found in the object " + to_string(elem);
            // THROW(err);

            return "Not Found!";

        } break;
        case HRML_QUERY_PROP: {
            for (auto prop : elem.props) {
                if (prop.first == label) return prop.second;
            }

            // string err = "query prop '" + label + "' was not found in the object";
            // THROW(err);

            return "Not Found!";
        } break;
        default: break;
    }

    THROW("unexpected encountered query token");
}

string query_HRML(string query, Element elem) {
    vector<string> tokens = query_parser(query);
    string found = query_search(tokens, elem, 0);

    return found;
}

}  // namespace HRML

int main() {
    int nb_lines, nb_queries;
    cin >> nb_lines >> nb_queries;

    vector<string> lines(nb_lines + 1);
    vector<string> queries(nb_queries);

    for (int i = 0; i < lines.size(); i++) getline(cin, lines[i]);
    for (int i = 0; i < queries.size(); i++) getline(cin, queries[i]);

    cout << "\nLINES: ";
    for (auto line : lines) cout << line + " ";

    cout << "\nQUERIES: ";
    for (auto query : queries) cout << query + " ";

    string content = "";
    for (string line : lines) content += line;

    cout << "\nCONTENT: " + content;

    vector<lexer::Token> lexer_tokens = lexer::lexer_HRML(content);

    cout << "\nLEXER: ";
    for (auto token : lexer_tokens) cout << lexer::to_string(token) << " ";

    vector<parser::Token> parser_tokens = parser::parser_HRML(lexer_tokens);

    cout << "\nPARSER: ";
    for (auto token : parser_tokens) cout << parser::to_string(token) << " ";

    HRML::Element root = HRML::generator_HRML(parser_tokens);

    cout << "\nHRML Output: ";
    cout << HRML::to_string(root);

    // cout << "\nQUERIES results: ";
    // for (auto query : queries) cout << "\n" << query << " = " << HRML::query_HRML(query, root);

    for (auto query : queries) cout << HRML::query_HRML(query, root) << endl;

    return 0;
}
