//
// Created by spaceeye on 10.12.22.
//

#ifndef PRETTYSIMPLEPROGRAMMINGLANGUAGE_TREEOPTIMIZER_H
#define PRETTYSIMPLEPROGRAMMINGLANGUAGE_TREEOPTIMIZER_H


#include "ValidateScope.h"
#include "IdRegister.h"
#include "CommonFNS.h"
#include "../OptionsContainer.h"
#include "BuiltinFunctions.h"
#include "Action/Actions.h"
#include "../Transpiler/Transpiler.h"
#include "Results.h"

#include <iostream>
#include <exception>
#include <memory>

class TreeOptimizer {
    static void trim_unreachable(TreeResult & tres);
    static bool recursive_trim_unreachable(std::shared_ptr<BaseAction> node);
public:
    static void optimize_tree(TreeResult &tres, const Options &options);
};


#endif //PRETTYSIMPLEPROGRAMMINGLANGUAGE_TREEOPTIMIZER_H
