// ============================================================================
// File: src/domain/DomainJson.cpp
// JSON (SLIM-ish): omits empty optional fields
// ============================================================================
#include "domain/DomainJson.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <type_traits>

// -------------------------
// JSON helpers
// -------------------------
static void escapeJson(const std::string& in, std::ostream& os) {
    for (char c : in) {
        switch (c) {
        case '\"': os << "\\\""; break;
        case '\\': os << "\\\\"; break;
        case '\n': os << "\\n";  break;
        case '\r': os << "\\r";  break;
        case '\t': os << "\\t";  break;
        default:   os << c;      break;
        }
    }
}

static void writeStr(std::ostream& os, const std::string& s) {
    os << "\"";
    escapeJson(s, os);
    os << "\"";
}

static void writeStrField(std::ostream& os, const char* key, const std::string& val) {
    os << "\"" << key << "\": ";
    writeStr(os, val);
}

static void writeCharField(std::ostream& os, const char* key, char c) {
    os << "\"" << key << "\": ";
    std::string s(1, c);
    writeStr(os, s);
}

static void writeSentence(std::ostream& os, const SentenceIR& s) {
    os << "{ ";
    writeStrField(os, "text", s.text);
    os << ", ";
    writeCharField(os, "punctuation", s.punctuation);
    os << " }";
}

static void writeTaskPoints(std::ostream& os, const TaskPointsIR& p) {
    os << "{ ";
    os << "\"scoringMode\": ";
    if (p.scoringMode == ScoringModeIR::AllOrNothing) writeStr(os, "AllOrNothing");
    else writeStr(os, "PartialPerCorrect");
    if (p.pointsIfAllCorrect.has_value()) {
        os << ", \"pointsIfAllCorrect\": " << *p.pointsIfAllCorrect;
    }
    os << " }";
}

// ---------- RoF ----------
static void writeAnswer(std::ostream& os, const AnswerIR& a) {
    os << "{ \"isTrue\": " << (a.isTrue ? "true" : "false");
    if (a.reason.has_value()) {
        os << ", \"reason\": ";
        writeSentence(os, *a.reason);
    }
    os << " }";
}

static void writeRoFLine(std::ostream& os, const TrueFalseTaskIR& line) {
    os << "{ \"question\": ";
    writeSentence(os, line.question);
    os << ", \"answer\": ";
    writeAnswer(os, line.answer);
    os << " }";
}

// ---------- Sorting ----------
static void writeSortingLine(std::ostream& os, const SortingLineIR& line) {
    os << "{ \"question\": ";
    writeSentence(os, line.question);
    os << ", \"points\": ";
    writeTaskPoints(os, line.points);
    os << ", \"items\": [";
    for (size_t i = 0; i < line.items.size(); ++i) {
        writeStr(os, line.items[i]);
        if (i + 1 < line.items.size()) os << ", ";
    }
    os << "] }";
}

// ---------- Matching ----------
static void writeMatchingQuestion(std::ostream& os, const MatchingQuestionIR& q) {
    os << "{ ";
    writeStrField(os, "prefix", q.prefix);
    os << ", ";
    writeStrField(os, "slotA", q.slotA);
    os << ", ";
    writeStrField(os, "middle", q.middle);
    os << ", ";
    writeStrField(os, "slotB", q.slotB);
    os << ", ";
    writeCharField(os, "punctuation", q.punctuation);
    os << " }";
}

static void writeMatchingLine(std::ostream& os, const MatchingLineIR& line) {
    os << "{ \"question\": ";
    writeMatchingQuestion(os, line.question);
    os << ", \"points\": ";
    writeTaskPoints(os, line.points);
    os << ", \"pairs\": [";
    for (size_t i = 0; i < line.pairs.size(); ++i) {
        os << "{ ";
        writeStrField(os, "left", line.pairs[i].left);
        os << ", ";
        writeStrField(os, "right", line.pairs[i].right);
        os << " }";
        if (i + 1 < line.pairs.size()) os << ", ";
    }
    os << "] }";
}

// ---------- Cloze ----------
static void writeClozeSentence(std::ostream& os, const ClozeSentenceIR& s) {
    os << "{ \"punctuation\": ";
    std::string p(1, s.punctuation);
    writeStr(os, p);
    os << ", \"parts\": [";
    for (size_t i = 0; i < s.parts.size(); ++i) {
        const auto& part = s.parts[i];
        os << "{";
        bool wrote = false;
        if (!part.text.empty()) {
            writeStrField(os, "text", part.text);
            wrote = true;
        }
        if (part.blank.has_value()) {
            if (wrote) os << ", ";
            os << "\"blank\": { ";
            writeStrField(os, "solution", part.blank->solution);
            os << ", \"points\": " << part.blank->points;
            os << " }";
        }
        os << "}";
        if (i + 1 < s.parts.size()) os << ", ";
    }
    os << "] }";
}

static void writeClozeTask(std::ostream& os, const ClozeTaskIR& t) {
    os << "{ \"question\": ";
    writeSentence(os, t.question);
    os << ", \"sentences\": [";
    for (size_t i = 0; i < t.sentences.size(); ++i) {
        writeClozeSentence(os, t.sentences[i]);
        if (i + 1 < t.sentences.size()) os << ", ";
    }
    os << "] }";
}

// ---------- Marking ----------
static void writeMarkingSentence(std::ostream& os, const MarkingSentenceIR& s) {
    os << "{ \"punctuation\": ";
    std::string p(1, s.punctuation);
    writeStr(os, p);
    os << ", \"parts\": [";
    for (size_t i = 0; i < s.parts.size(); ++i) {
        const auto& part = s.parts[i];
        os << "{";
        bool wrote = false;
        if (!part.text.empty()) {
            writeStrField(os, "text", part.text);
            wrote = true;
        }
        if (part.mark.has_value()) {
            if (wrote) os << ", ";
            os << "\"mark\": { ";
            writeStrField(os, "markedText", part.mark->markedText);
            os << ", \"points\": " << part.mark->points;
            if (part.mark->correction.has_value()) {
                os << ", ";
                writeStrField(os, "correction", *part.mark->correction);
            }
            os << " }";
        }
        os << "}";
        if (i + 1 < s.parts.size()) os << ", ";
    }
    os << "] }";
}

static void writeMarkingTask(std::ostream& os, const MarkingTaskIR& t) {
    os << "{ \"question\": ";
    writeSentence(os, t.question);
    os << ", \"sentences\": [";
    for (size_t i = 0; i < t.sentences.size(); ++i) {
        writeMarkingSentence(os, t.sentences[i]);
        if (i + 1 < t.sentences.size()) os << ", ";
    }
    os << "] }";
}

// ---------- Correction ----------
static void writeCorrectionSentence(std::ostream& os, const CorrectionSentenceIR& s) {
    os << "{ \"punctuation\": ";
    std::string p(1, s.punctuation);
    writeStr(os, p);
    os << ", \"parts\": [";
    for (size_t i = 0; i < s.parts.size(); ++i) {
        const auto& part = s.parts[i];
        os << "{";
        bool wrote = false;
        if (!part.text.empty()) {
            writeStrField(os, "text", part.text);
            wrote = true;
        }
        if (part.corr.has_value()) {
            if (wrote) os << ", ";
            os << "\"correction\": { ";
            writeStrField(os, "wrong", part.corr->wrong);
            os << ", ";
            writeStrField(os, "correct", part.corr->correct);
            os << ", \"points\": " << part.corr->points;
            os << " }";
        }
        os << "}";
        if (i + 1 < s.parts.size()) os << ", ";
    }
    os << "] }";
}

static void writeCorrectionTask(std::ostream& os, const CorrectionTaskIR& t) {
    os << "{ \"question\": ";
    writeSentence(os, t.question);
    os << ", \"sentences\": [";
    for (size_t i = 0; i < t.sentences.size(); ++i) {
        writeCorrectionSentence(os, t.sentences[i]);
        if (i + 1 < t.sentences.size()) os << ", ";
    }
    os << "] }";
}

// ---------- Choice ----------
static void writeChoiceOption(std::ostream& os, const ChoiceOptionIR& o) {
    os << "{ ";
    writeStrField(os, "text", o.text);
    os << ", \"points\": " << o.points;
    os << ", \"isCorrect\": " << (o.isCorrect ? "true" : "false");
    os << " }";
}

static void writeChoiceLine(std::ostream& os, const ChoiceLineIR& line) {
    os << "{ \"question\": ";
    writeSentence(os, line.question);
    os << ", \"options\": [";
    for (size_t i = 0; i < line.options.size(); ++i) {
        writeChoiceOption(os, line.options[i]);
        if (i + 1 < line.options.size()) os << ", ";
    }
    os << "] }";
}

static void writeChoiceTask(std::ostream& os, const std::vector<ChoiceLineIR>& lines) {
    os << "{ \"lines\": [";
    for (size_t i = 0; i < lines.size(); ++i) {
        writeChoiceLine(os, lines[i]);
        if (i + 1 < lines.size()) os << ", ";
    }
    os << "] }";
}

// -------------------------
// Program / Task dispatch
// -------------------------
static void writeTask(std::ostream& os, const TaskD& t) {
    std::visit([&](const auto& x) {
        using T = std::decay_t<decltype(x)>;

        os << "{ ";
        os << "\"type\": ";
        writeStr(os, taskKind(t));
        os << ", ";
        writeStrField(os, "header", x.header);

        if constexpr (std::is_same_v<T, RoFTaskD>) {
            os << ", \"lines\": [";
            for (size_t i = 0; i < x.lines.size(); ++i) {
                writeRoFLine(os, x.lines[i]);
                if (i + 1 < x.lines.size()) os << ", ";
            }
            os << "]";
        }
        else if constexpr (std::is_same_v<T, SortingTaskD>) {
            os << ", \"lines\": [";
            for (size_t i = 0; i < x.lines.size(); ++i) {
                writeSortingLine(os, x.lines[i]);
                if (i + 1 < x.lines.size()) os << ", ";
            }
            os << "]";
        }
        else if constexpr (std::is_same_v<T, MatchingTaskD>) {
            os << ", \"lines\": [";
            for (size_t i = 0; i < x.lines.size(); ++i) {
                writeMatchingLine(os, x.lines[i]);
                if (i + 1 < x.lines.size()) os << ", ";
            }
            os << "]";
        }
        else if constexpr (std::is_same_v<T, MarkingTaskD>) {
            os << ", \"task\": ";
            writeMarkingTask(os, x.task);
        }
        else if constexpr (std::is_same_v<T, ClozeTaskD>) {
            os << ", \"task\": ";
            writeClozeTask(os, x.task);
        }
        else if constexpr (std::is_same_v<T, CorrectionTaskD>) {
            os << ", \"task\": ";
            writeCorrectionTask(os, x.task);
        }
        else if constexpr (std::is_same_v<T, ChoiceTaskD>) {
            os << ", \"task\": ";
            writeChoiceTask(os, x.lines);
        }

        os << " }";
    }, t);
}

std::string domainToJson(const ProgramD& prog) {
    std::ostringstream os;
    os << "{ \"type\": \"Program\", \"tasks\": [";
    for (size_t i = 0; i < prog.tasks.size(); ++i) {
        writeTask(os, prog.tasks[i]);
        if (i + 1 < prog.tasks.size()) os << ", ";
    }
    os << "] }";
    return os.str();
}

std::string prettyJsonDomain(const std::string& src) {
    std::ostringstream out;
    int indent = 0;
    bool inString = false;

    for (size_t i = 0; i < src.size(); ++i) {
        char c = src[i];

        if (c == '\"') {
            out << c;
            if (i == 0 || src[i - 1] != '\\') inString = !inString;
        }
        else if (!inString && (c == '{' || c == '[')) {
            out << c << "\n";
            indent++;
            out << std::string(indent * 2, ' ');
        }
        else if (!inString && (c == '}' || c == ']')) {
            out << "\n";
            indent--;
            out << std::string(indent * 2, ' ') << c;
        }
        else if (!inString && c == ',') {
            out << c << "\n" << std::string(indent * 2, ' ');
        }
        else {
            out << c;
        }
    }

    return out.str();
}

void writeDomainToFile(const ProgramD& prog, const std::string& path) {
    namespace fs = std::filesystem;

    fs::path p(path);
    if (p.has_parent_path()) fs::create_directories(p.parent_path());

    std::ofstream out(path);
    if (!out) throw std::runtime_error("Could not open output file: " + path);

    out << prettyJsonDomain(domainToJson(prog));
}

