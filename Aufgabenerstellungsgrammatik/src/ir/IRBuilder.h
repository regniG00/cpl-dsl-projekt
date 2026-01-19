// ============================================================================
// File: src/ir/IRBuilder.h
// ============================================================================
#pragma once

#include <any>
#include <string>

#include "antlr4-runtime.h"
#include "AufgabenerstellungsgrammatikBaseVisitor.h"
#include "AufgabenerstellungsgrammatikLexer.h"
#include "AufgabenerstellungsgrammatikParser.h"

#include "ir/IR.h"

class IRBuilder : public AufgabenerstellungsgrammatikBaseVisitor {
public:
    IRBuilder(const std::string& sourceText, antlr4::CommonTokenStream* tokenStream)
        : source(sourceText), tokens(tokenStream) {}

    std::any visitProg(AufgabenerstellungsgrammatikParser::ProgContext* ctx) override;
    std::any visitTask_definition(AufgabenerstellungsgrammatikParser::Task_definitionContext* ctx) override;

private:
    std::string source;
    antlr4::CommonTokenStream* tokens = nullptr;

    // ---- token-based reconstruction helpers ----
    std::string textJoin(antlr4::ParserRuleContext* ctx, bool keepNewlines) const;
    static bool isWordishToken(int tokenType);
    static bool isPunctToken(int tokenType);
    static bool isNoSpaceLeftToken(int tokenType);  // punctuation, ')', ']', '/', ',', etc.
    static bool isNoSpaceRightToken(int tokenType); // '(' , '[' , etc.

    static int parseIntStrict(const std::string& s);

    // ---- grammar-level helpers ----
    SentenceIR readSentence(AufgabenerstellungsgrammatikParser::SentenceContext* s) const;
    std::string readEndlessWords(AufgabenerstellungsgrammatikParser::Endless_wordsContext* ew) const;
};
