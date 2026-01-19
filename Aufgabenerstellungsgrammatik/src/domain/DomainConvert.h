// ============================================================================
// File: src/domain/DomainConvert.h
// ============================================================================
#pragma once

#include <stdexcept>
#include "ir/IR.h"
#include "domain/Domain.h"

TaskD convertTask(const TaskIR& ir);
ProgramD convertProgram(const ProgramIR& ir);
