// ============================================================================
// File: src/ir/IR.h
// Rich IR for new grammar (segments + points separated)
// ============================================================================
#pragma once

#include <optional>
#include <string>
#include <variant>
#include <vector>

// ----------------------------
// Common primitives
// ----------------------------
struct SentenceIR {
    std::string text;   // without trailing punctuation
    char punctuation = '.'; // '.', '!', '?'
};

enum class ScoringModeIR {
    PartialPerCorrect, // default if no pointsIfAllCorrect given
    AllOrNothing
};

struct TaskPointsIR {
    std::optional<int> pointsIfAllCorrect;
    ScoringModeIR scoringMode = ScoringModeIR::PartialPerCorrect;
};

// ----------------------------
// RoF (True/False)
// ----------------------------
struct AnswerIR {
    bool isTrue = false;
    std::optional<SentenceIR> reason; // only when false + reason
};

struct TrueFalseTaskIR {
    SentenceIR question;
    AnswerIR answer;
};

// ----------------------------
// Sorting
// ----------------------------
struct SortingLineIR {
    SentenceIR question;
    TaskPointsIR points;
    std::vector<std::string> items; // item values (word / number)
};

// ----------------------------
// Matching
// ----------------------------
struct MatchingQuestionIR {
    std::string prefix;   // endless_words
    std::string slotA;    // word in (...)
    std::string middle;   // endless_words
    std::string slotB;    // word in (...)
    char punctuation = '!';
};

struct MatchingItemIR {
    std::string left;
    std::string right;
};

struct MatchingLineIR {
    MatchingQuestionIR question;
    TaskPointsIR points;
    std::vector<MatchingItemIR> pairs;
};

// ----------------------------
// Cloze (fill in blanks): (word,points)
// ----------------------------
struct ClozeBlankIR {
    std::string solution;
    int points = 1;
};

struct ClozePartIR {
    std::string text; // plain text chunk (may be empty)
    std::optional<ClozeBlankIR> blank;
};

struct ClozeSentenceIR {
    std::vector<ClozePartIR> parts;
    char punctuation = '.';
};

struct ClozeTaskIR {
    SentenceIR question;
    std::vector<ClozeSentenceIR> sentences;
};

// ----------------------------
// Marking: ( ... )[points] or ( ... )[correction,points]
// ----------------------------
struct MarkedSpanIR {
    std::string markedText;
    std::optional<std::string> correction;
    int points = 1;
};

struct MarkingPartIR {
    std::string text; // plain text
    std::optional<MarkedSpanIR> mark;
};

struct MarkingSentenceIR {
    std::vector<MarkingPartIR> parts;
    char punctuation = '.';
};

struct MarkingTaskIR {
    SentenceIR question;
    std::vector<MarkingSentenceIR> sentences;
};

// ----------------------------
// Correction: (wrong)[correct,points]
// ----------------------------
struct CorrectionSpanIR {
    std::string wrong;
    std::string correct;
    int points = 1;
};

struct CorrectionPartIR {
    std::string text;
    std::optional<CorrectionSpanIR> corr;
};

struct CorrectionSentenceIR {
    std::vector<CorrectionPartIR> parts;
    char punctuation = '.';
};

struct CorrectionTaskIR {
    SentenceIR question;
    std::vector<CorrectionSentenceIR> sentences;
};

// ----------------------------
// Choice (single/multiple)
// ----------------------------
struct ChoiceOptionIR {
    std::string text;
    int points = 0;
    bool isCorrect = false;
};

struct ChoiceLineIR {
    SentenceIR question;
    std::vector<ChoiceOptionIR> options;
};

// ----------------------------
// Program/Tasks
// ----------------------------
struct TaskIR {
    std::string header;
    std::string type; // "RoF","Umordnung","Zuordnung","Markierung","Lückentext","Textkorrektur","Auswahl"

    std::vector<TrueFalseTaskIR> rof;
    std::vector<SortingLineIR> sorting;
    std::vector<MatchingLineIR> matching;

    std::optional<MarkingTaskIR> marking;
    std::optional<ClozeTaskIR> cloze;
    std::optional<CorrectionTaskIR> correction;

    std::vector<ChoiceLineIR> choice;
};

struct ProgramIR {
    std::vector<TaskIR> tasks;
};
