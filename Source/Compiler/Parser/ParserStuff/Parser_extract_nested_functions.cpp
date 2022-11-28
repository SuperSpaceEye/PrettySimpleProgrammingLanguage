//
// Created by spaceeye on 28.11.22.
//

#include "../Parser.h"

TreeResult Parser::extract_nested_functions(ASTCreationResult &ast_result) {
    int level = 0;

    std::vector<int> start_levels{};
    std::vector<std::shared_ptr<BaseAction>> start_pointers;

    std::vector<std::pair<std::shared_ptr<BaseAction>, std::shared_ptr<BaseAction>>> root_pairs;

    TreeResult res;
    for (auto & root: ast_result.object_roots) {
        auto temp_root = std::make_shared<BaseAction>();
        temp_root->next_action = root;
        root = temp_root;

        std::shared_ptr<BaseAction> prev_root = root;

        while (root != nullptr) {
            if (root->act_type == ActionType::StartLogicBlock) {level++;}
            if (root->act_type == ActionType::EndLogicBlock)   {level--;}

            if (root->act_type == ActionType::FunctionDeclaration) {
                start_pointers.emplace_back(prev_root);
                start_levels.emplace_back(level);
            }

            if (root->act_type == ActionType::EndLogicBlock && level == start_levels.back()) {
                root_pairs.emplace_back(start_pointers.back(), root);
                start_pointers.pop_back();
                start_levels.pop_back();
            }

            prev_root = root;
            root = root->next_action;
        }
    }

    for (int i = root_pairs.size()-1; i >= 0; i--) {
        auto & [start, stop] = root_pairs[i];

        auto begin_root = start->next_action;
        start->next_action = stop->next_action;
        stop->next_action = nullptr;

        res.object_roots.emplace_back(begin_root);
    }

    int main_id = 0;
    for (auto & item: ast_result.reg.function_ids) {
        if (std::get<1>(item) == "main") {
            main_id = std::get<0>(item);
            break;
        }
    }

    res.main_idx = get_main_fn_idx(res, main_id);

    return res;
}

int Parser::get_main_fn_idx(TreeResult &t_res, int main_id) {
    for (int i = 0; i < t_res.object_roots.size(); i++) {
        auto root = t_res.object_roots[i];
        if (root->act_type == ActionType::FunctionDeclaration) {
            auto & fn_dec = *static_cast<FunctionDeclaration*>(root.get());
            if (fn_dec.fn_id == main_id) {
                return i;
            }
        }
    }
    throw std::logic_error("Shouldn't happen.");
}