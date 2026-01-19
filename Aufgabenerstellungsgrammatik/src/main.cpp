#include <iostream>
#include <fstream>
#include <string>
#include <any>
#include <sstream>

#include "antlr4-runtime.h"
#include "AufgabenerstellungsgrammatikLexer.h"
#include "AufgabenerstellungsgrammatikParser.h"

#include "ir/IRBuilder.h"
#include "ir/IR.h"

#include "domain/Domain.h"
#include "domain/DomainConvert.h"
#include "domain/DomainJson.h"

using namespace antlr4;

#include "antlr4-runtime.h"
#include <iostream>
#include <string>

static void dumpTokens(antlr4::CommonTokenStream& tokens) {
    tokens.fill();
    for (auto* t : tokens.getTokens()) {
        std::string txt = t->getText();

        // newlines sichtbar machen (ASCII-safe)
        for (char& c : txt) {
            if (c == '\n') c = '#'; // oder lass es ganz weg
        }

        std::cout
            << "[" << t->getTokenIndex() << "] "
            << "type=" << t->getType()
            << " line=" << t->getLine()
            << ":" << t->getCharPositionInLine()
            << " start=" << t->getStartIndex()
            << " stop=" << t->getStopIndex()
            << " text='" << txt << "'\n";
    }
}


int main(int argc, char* argv[]) {
    std::cerr << "[aufgaben_dsl] started\n";

    // Usage: aufgaben_dsl <input.dsl.txt> <output.json>
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0]
                  << " <input.dsl.txt> <output.json>\n";
        return 1;
    }

    const std::string inputPath  = argv[1];
    const std::string outputPath = argv[2];

    // ------------------------------------------------------------
    // 1) Read input
    // ------------------------------------------------------------
    std::ifstream inFile(inputPath);
    if (!inFile) {
        std::cerr << "Konnte Eingabedatei nicht öffnen: " << inputPath << "\n";
        return 1;
    }

    std::ostringstream buffer;
    buffer << inFile.rdbuf();
    std::string input = buffer.str();

    if (input.empty()) {
        std::cerr << "Eingabedatei ist leer: " << inputPath << "\n";
        return 1;
    }

    // ------------------------------------------------------------
    // 2) ANTLR parse
    // ------------------------------------------------------------
    ANTLRInputStream inputStream(input);
    AufgabenerstellungsgrammatikLexer lexer(&inputStream);
    CommonTokenStream tokens(&lexer);
    
    // DEBUG: lexer output
    dumpTokens(tokens);

    AufgabenerstellungsgrammatikParser parser(&tokens);

    auto* progCtx = parser.prog();
    if (parser.getNumberOfSyntaxErrors() > 0) {
        std::cerr << "Syntaxfehler in Datei: " << inputPath << "\n";
        return 1;
    }

    // ------------------------------------------------------------
    // 3) ParseTree -> IR
    // ------------------------------------------------------------
    IRBuilder builder(input, &tokens);
    std::any progAny = builder.visitProg(progCtx);
    ProgramIR progIR = std::any_cast<ProgramIR>(progAny);

    // ------------------------------------------------------------
    // 4) IR -> Domain (typed)
    // ------------------------------------------------------------
    ProgramD progD;
    try {
        progD = convertProgram(progIR);
    } catch (const std::exception& ex) {
        std::cerr << "Fehler beim Konvertieren IR -> Domain: " << ex.what() << "\n";
        return 1;
    }

    // ------------------------------------------------------------
    // 5) Domain -> JSON (SLIM, no empty fields)
    // ------------------------------------------------------------
    try {
        writeDomainToFile(progD, outputPath);
    } catch (const std::exception& ex) {
        std::cerr << "Fehler beim Schreiben der Domain-JSON: " << ex.what() << "\n";
        return 1;
    }

    std::cerr << "Domain JSON geschrieben: " << outputPath << "\n";
    return 0;
}
