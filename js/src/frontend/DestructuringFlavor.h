/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef frontend_DestructuringFlavor_h
#define frontend_DestructuringFlavor_h

namespace js {
namespace frontend {

enum DestructuringFlavor {
    // Destructuring into a declaration.
    DestructuringDeclaration,

    // Destructuring into a formal parameter, when the formal parameters
    // contain an expression that might be evaluated, and thus require
    // this destructuring to assign not into the innermost scope that
    // contains the function body's vars, but into its enclosing scope for
    // parameter expressions.
    DestructuringFormalParameterInVarScope,

    // Destructuring as part of an AssignmentExpression.
    DestructuringAssignment
};

} /* namespace frontend */
} /* namespace js */

#endif /* frontend_DestructuringFlavor_h */