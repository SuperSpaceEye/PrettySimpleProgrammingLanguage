//
// Created by spaceeye on 10.12.22.
//

#include "../Parser.h"

void Parser::optimize_tree(TreeResult &tres, const Options &options) {
    if (options.optimization.trim_unreachable) {trim_unreachable(tres);}
}

void Parser::trim_unreachable(TreeResult &tres) {
    for (const auto& root: tres.object_roots) {
        auto node = root;
        recursive_trim_unreachable(node);
    }
}

bool Parser::recursive_trim_unreachable(std::shared_ptr<BaseAction> node) {
    while (node != nullptr) {
        switch (node->act_type) {
            case ActionType::ForLoop: {
                auto for_loop_act = *static_cast<ForLoop*>(node.get());
                recursive_trim_unreachable(for_loop_act.body);
            }
                break;
            case ActionType::WhileLoop: {
                auto while_act = *static_cast<WhileLoop*>(node.get());
                recursive_trim_unreachable(while_act.body);
            }
                break;
            case ActionType::IfStatement: {
                auto  if_act = *static_cast<IfStatement*>(node.get());

                auto true_res = recursive_trim_unreachable(if_act.true_branch);
                bool false_res = false;
                if (if_act.false_branch != nullptr) {
                    false_res = recursive_trim_unreachable(if_act.false_branch);
                }

                if (true_res && false_res) {
                    if_act.next_action = nullptr;
                    return true;
                }
            }
                break;
            case ActionType::ReturnCall: {
                node->next_action = nullptr;
                return true;
            }
        }
        node = node->next_action;
    }
    return false;
}
