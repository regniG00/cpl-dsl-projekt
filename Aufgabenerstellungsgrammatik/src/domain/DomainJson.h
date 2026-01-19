// ============================================================================
// File: src/domain/DomainJson.h
// ============================================================================
#pragma once

#include <string>
#include "domain/Domain.h"

std::string domainToJson(const ProgramD& prog);
std::string prettyJsonDomain(const std::string& src);
void writeDomainToFile(const ProgramD& prog, const std::string& path);
