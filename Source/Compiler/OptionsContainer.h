//
// Created by spaceeye on 06.12.22.
//

#ifndef PRETTYSIMPLEPROGRAMMINGLANGUAGE_OPTIONSCONTAINER_H
#define PRETTYSIMPLEPROGRAMMINGLANGUAGE_OPTIONSCONTAINER_H

struct Options {
    struct OptimizationOptions {
        //TODO
        //batches separate pop commands into one command if they are directly connected
        bool batch_byte_words = true;
        bool trim_unreachable = true;
    };
    OptimizationOptions optimization{};

    struct OtherOptions {
        //TODO
        //for function arguments
        bool allow_implicit_cast_of_simple_types_for_builtin_fns = true;
        bool allow_implicit_cast_of_simple_types_for_user_defined_fns = false;
    };

    struct WarningOptions {
        //TODO
    };
    WarningOptions warning{};

    //debug options
    struct DebugOptions {
        bool show_transpiler_output = false;
        bool show_parser_output = false;
        bool show_optimized_tree = false;
        bool show_compiler_output = false;
    };
    DebugOptions debug{};
};

#endif //PRETTYSIMPLEPROGRAMMINGLANGUAGE_OPTIONSCONTAINER_H
