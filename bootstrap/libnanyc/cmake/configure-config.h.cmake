/*
** ny - https://nany.io
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

/* NOTE: this file is generated by cmake */


namespace ny {
namespace config {


//! The maximum number of nested namespaces
static constexpr uint32_t maxNamespaceDepth = 32;

//! Maximum number of parameters when declaring a function
static constexpr uint32_t maxFuncDeclParameterCount = 7;

//! Maximum length for a symbol name
static constexpr uint32_t maxSymbolNameLength = 64 - 1 /*zero-terminated*/;

//! Maxmimum number of pushed parameters for calling a function
static constexpr uint32_t maxPushedParameters = 32;

//! Remove redundant dbg info (line,offset) in opcode programs
static constexpr bool removeRedundantDbgOffset = true;

//! Size that should be added to any ny objects
// (reference counter)
static constexpr const uint32_t extraObjectSize = (uint32_t) sizeof(uint64_t);

//! Import the NSL
static constexpr bool importNSL = true;


} // namespace config
} // namespace ny
/* vim: set ft=cpp: */