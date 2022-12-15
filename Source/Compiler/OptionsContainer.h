//
// Created by spaceeye on 06.12.22.
//

#ifndef PRETTYSIMPLEPROGRAMMINGLANGUAGE_OPTIONSCONTAINER_H
#define PRETTYSIMPLEPROGRAMMINGLANGUAGE_OPTIONSCONTAINER_H

struct Options {
    struct OptimizationOptions {
        //TODO
        //batches separate pop commands into one command if they are directly connected
        bool batch_push_commands = true;
        bool trim_unreachable = true;
    };
    OptimizationOptions optimization{};

    struct OtherOptions {
        //for function arguments
        bool allow_implicit_cast_for_builtin_fns = true;
        bool allow_implicit_cast_for_user_defined_fns = true;
    };
    OtherOptions other{};

    struct WarningOptions {
        bool alert_on_builtin_fns_implicit_cast = false;
        bool alert_on_user_defined_fns_implicit_cast = false;
        //if, while, for, etc.
        bool alert_on_implicit_cast_in_structure_arguments = false;
    };
    WarningOptions warning{};

    //debug options
    struct DebugOptions {
        bool show_transpiler_output = false;
        bool show_parser_output = true;
        bool show_optimized_tree = false;
        bool show_compiler_output = false;
    };
    DebugOptions debug{};
};

#endif //PRETTYSIMPLEPROGRAMMINGLANGUAGE_OPTIONSCONTAINER_H
