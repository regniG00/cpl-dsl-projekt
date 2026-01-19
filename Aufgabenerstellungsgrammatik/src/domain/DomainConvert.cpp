// ============================================================================
// File: src/domain/DomainConvert.cpp
// ============================================================================
#include "domain/DomainConvert.h"

TaskD convertTask(const TaskIR& ir) {
    if (ir.type == "RoF") {
        RoFTaskD t{ir.header, ir.rof};
        return t;
    }
    if (ir.type == "Umordnung") {
        SortingTaskD t{ir.header, ir.sorting};
        return t;
    }
    if (ir.type == "Zuordnung") {
        MatchingTaskD t{ir.header, ir.matching};
        return t;
    }
    if (ir.type == "Markierung") {
        if (!ir.marking) throw std::runtime_error("Markierung task missing payload");
        MarkingTaskD t{ir.header, *ir.marking};
        return t;
    }
    if (ir.type == "Lückentext" || ir.type == "Lueckentext") {
        if (!ir.cloze) throw std::runtime_error("Lueckentext task missing payload");
        ClozeTaskD t{ir.header, *ir.cloze};
        return t;
    }
    if (ir.type == "Textkorrektur") {
        if (!ir.correction) throw std::runtime_error("Textkorrektur task missing payload");
        CorrectionTaskD t{ir.header, *ir.correction};
        return t;
    }
    if (ir.type == "Auswahl") {
        ChoiceTaskD t{ir.header, ir.choice};
        return t;
    }

    throw std::runtime_error("Unknown task type in convertTask(): " + ir.type);
}

ProgramD convertProgram(const ProgramIR& ir) {
    ProgramD out;
    out.tasks.reserve(ir.tasks.size());
    for (const auto& t : ir.tasks) {
        if (t.type == "Unknown") {
            throw std::runtime_error("Cannot convert task with type=Unknown (header=" + t.header + ")");
        }
        out.tasks.push_back(convertTask(t));
    }
    return out;
}
