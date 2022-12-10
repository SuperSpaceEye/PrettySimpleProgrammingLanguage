//
// Created by spaceeye on 10.12.22.
//

#ifndef PRETTYSIMPLEPROGRAMMINGLANGUAGE_RESULTS_H
#define PRETTYSIMPLEPROGRAMMINGLANGUAGE_RESULTS_H

#include "IdRegister.h"
#include "Action/Actions.h"

#include <memory>
#include <vector>

struct ASTCreationResult {
    std::vector<std::shared_ptr<BaseAction>> object_roots;
    IdRegister reg;
};
struct TreeResult {
    std::vector<std::shared_ptr<BaseAction>> object_roots;
    int main_idx;
};
#endif //PRETTYSIMPLEPROGRAMMINGLANGUAGE_RESULTS_H
