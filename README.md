# HRML ENGINE

## Introduction

HRML engine is a small `XML-like` engine for parsing and generation.

It is based on the HackerRank "Attribute Parser" problem:

<https://www.hackerrank.com/challenges/attribute-parser/problem>

## 1. Functionnalities

### 1.1. Parsing

For instance, let's say we want to parse the following xml document:

```xml
<tag1 value = "HelloWorld">
<tag2 name = "Name1">
</tag2>
</tag1>
```

**STEP 1**: INPUT

  *Code*

  ```c++
  std::string content = "<tag1 value = "HelloWorld"><tag2 name = "Name1"></tag2></tag1>"
  ```

  *Result*

  ```text
  <tag1 value = "HelloWorld"><tag2 name = "Name1"></tag2></tag1>
  ```
  
**STEP 2**: LEXER

  *Code*

  ```c++
  vector<lexer::Token> lexer_tokens = lexer::lexer_HRML(content);
  ```

  *Result*

  ```text
  OPEN LABEL[tag1] LABEL[value] EQUAL QUOTE LABEL[HelloWorld] QUOTE CLOSE OPEN LABEL[tag2] LABEL[name] EQUAL QUOTE LABEL[Name1] QUOTE CLOSE OPEN SLASH LABEL[tag2] CLOSE OPEN SLASH LABEL[tag1] CLOSE
  ```

**STEP 3**: PARSER

*Code*

  ```c++
  vector<parser::Token> parser_tokens = parser::parser_HRML(lexer_tokens);
  ```

  *Result*

  ```text
  OPEN[tag1] OPEN[tag2] CLOSE[tag2] CLOSE[tag1]
  ```

**STEP 4**: GENERATOR

  *Code*

  ```c++
  HRML::Element root = HRML::generator_HRML(parser_tokens);
  ```

  *Result*

  ```json
  { 
    "name": "_root_", 
    "props": {}, 
    "childs": [
      {
        "name": "tag1",
        "props": { "value": "HelloWorld" },
        "childs": [
          {
            "name": "tag2",
            "props": { "name": "Name1" },
            "childs": []
          }
        ]
      }
    ]
  }
  ```

### 1.2. Printing

  *Code*

  ```c++
  string print = HRML::to_string(root);
  ```

  *Result*

  ```text
  <_root_>
  <tag1 value="HelloWorld">
  <tag2 name="Name1">
  </tag2>
  </tag1>
  </_root_>
  ```

### 1.3. Queries

  *Code*

  ```c++
  std::string query = "tag1.tag2~name"
  std::string result = HRML::query_HRML(query, root)
  ```

  *Result*

  ```text
  Name1
  ```
