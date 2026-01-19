// ============================================================================
// File: src/domain/Domain.h
// Typed Domain model mirrors rich IR (no blobs)
// ============================================================================
#pragma once

#include <string>
#include <variant>
#include <vector>

#include "ir/IR.h"

// Task wrappers (header + payload)
struct RoFTaskD {
    std::string header;
    std::vector<TrueFalseTaskIR> lines;
};

struct SortingTaskD {
    std::string header;
    std::vector<SortingLineIR> lines;
};

struct MatchingTaskD {
    std::string header;
    std::vector<MatchingLineIR> lines;
};

struct MarkingTaskD {
    std::string header;
    MarkingTaskIR task;
};

struct ClozeTaskD {
    std::string header;
    ClozeTaskIR task;
};

struct CorrectionTaskD {
    std::string header;
    CorrectionTaskIR task;
};

struct ChoiceTaskD {
    std::string header;
    std::vector<ChoiceLineIR> lines;
};

using TaskD = std::variant<
    RoFTaskD,
    SortingTaskD,
    MatchingTaskD,
    MarkingTaskD,
    ClozeTaskD,
    CorrectionTaskD,
    ChoiceTaskD
>;

struct ProgramD {
    std::vector<TaskD> tasks;
};

inline const char* taskKind(const TaskD& t) {
    return std::visit([](const auto& x) -> const char* {
        using T = std::decay_t<decltype(x)>;
        if constexpr (std::is_same_v<T, RoFTaskD>) return "RoF";
        if constexpr (std::is_same_v<T, SortingTaskD>) return "Umordnung";
        if constexpr (std::is_same_v<T, MatchingTaskD>) return "Zuordnung";
        if constexpr (std::is_same_v<T, MarkingTaskD>) return "Markierung";
        if constexpr (std::is_same_v<T, ClozeTaskD>) return "Lueckentext";
        if constexpr (std::is_same_v<T, CorrectionTaskD>) return "Textkorrektur";
        if constexpr (std::is_same_v<T, ChoiceTaskD>) return "Auswahl";
        return "Unknown";
    }, t);
}
