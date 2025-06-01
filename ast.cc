
#include <ostream>
#include <ranges>

#include "ast.hh"

namespace mlang {

void context::set_local_variable(std::string_view name, const expression* value) {
    variables_.front()[std::string(name)] = value;
}

const expression *context::get_variable(std::string_view name) {
    auto _name = std::string(name);
    for (auto scope : variables_)
        if(scope.count(_name)) return scope[_name];

    return nullptr;
}

inline void context::new_scope() { variables_.emplace_front(); }
inline void context::pop_scope() { variables_.pop_front()    ; }

// context::~context() {
//     for(auto &m : variables_)
//         for(auto [_, t] : m) delete t;
// }

const expression *print_function::value(context &ctx) const {
    for(const auto &expr : *exprs_)
        std::cout << expr->value(ctx)->as_type()->repr(); 
    return &UNIT__;
}

print_function::~print_function() {
    for(auto *e : *exprs_) delete e;
    delete exprs_;
}

const expression *function_call::value(context &ctx) const {
    std::cout << "f call" << std::endl;
    auto *var = name_ ? ctx.get_variable(*name_) : expr_;

    std::cout << "var : " << var << "var->args_names : " << var->args_names() << std::endl;

    for (const auto &e : *var->args_names())
        std::cout << "arg : \"" << e << "\"\n";

    if(!var)
        __error("usage of undefined symbol \"" + *name_ + "\"");

    if(ptrdiff_t num_args_diff = var->num_args() - args_->size())
        __error("too " + 
                std::string(num_args_diff > 0 ? "few" : "many") + 
                " arguments to call on " +
                std::string(name_ 
                                ? "function \"" + *name_ + "\""
                                : "anonymouse function"));

    if (var->num_args() != 0) {
        auto names = var->args_names();

        ctx.new_scope();
        
        for(size_t i = 0; i < var->num_args(); i++)
            ctx.set_local_variable((*names)[i], (*args_)[i]->value(ctx));

        auto *ret = var->value(ctx);
        ctx.pop_scope();

        return ret;
    }

    return var->value(ctx);;
}

function_call::~function_call() {
    for(auto *e : *args_) delete e;
    delete args_;
}

const string_type *tostr_function::value(context &ctx) const {    
    // return new string_type(arg_->value(ctx)->repr());
    return nullptr;
}

const integer_type *toint_function::value(context &ctx) const {  
    // auto *value = arg_->value(ctx);  
    // switch(value->kind()) {
    //     case UNIT   : return new integer_type(0);
    //     case INTEGER: return static_cast<const integer_type *>(value);
    //     case STRING : {
    //         try {
    //             return new integer_type(std::stoi(value->repr()));
    //         } catch(const std::exception& e) {
    //             __error("cannot conver str to type int");
    //         }
    //     }
    //     case STRUCT: __error("cannot conver struct to type int");
    // }
    return nullptr;
}

const expression *if_expression::value(context &ctx) const {
    if ((cond_->value(ctx))->as_type()->to_integer_type().data__() > 0)
        return then_->value(ctx);
    if (otherwise_) return otherwise_->value(ctx);
    return &UNIT__;
}

if_expression::~if_expression() {
    delete cond_;
    delete then_;
    delete otherwise_;
}

void until_statement::execute(context &ctx) const {
    while ((cond_->value(ctx))->as_type()->to_integer_type().data__() > 0)
        body_->value(ctx);
}

until_statement::~until_statement() {
    delete cond_;
    delete body_;
}

const expression *function_decl::value(context &ctx) const { 
    // if(ctx.get_variable(var_name_))
    //     __warning("redeclaration of variable \"" + var_name_ + "\"");

    ctx.set_local_variable(var_name_, expr_);
    return expr_;
}

function_decl::~function_decl() { delete expr_; }


const expression *expr_list::value(context &ctx) const {
    // Creating local scope
    ctx.new_scope();

    const expression *last_expr_value = &UNIT__;
    for(auto e : *exprs_)
        last_expr_value = e->value(ctx);
    
    ctx.pop_scope();

    return last_expr_value; 
}

const expression *comp_expression::value(context &ctx) const { 
    auto *l_value = l_->value(ctx);
    auto *r_value = r_->value(ctx);

    // if(l_value->kind() == UNIT || r_value->kind() == UNIT)
    //     __error("cannot execute compare operation on unit type");

    // switch (l_value->kind()) {
    // case UNIT:
    //     return &UNIT__;
    
    // default:
    //     break;
    // }

    // switch(op_) {
    //     case comp_kind::EQUAL:
    //         return l_value < r_value;
    //     case comp_kind::NOTEQUAL: 
    //         return l_value < r_value;
    //     case comp_kind::GREATER: 
    //         return l_value < r_value;
    //     case comp_kind::GREATEREQUAL: 
    //         return l_value < r_value;
    //     case comp_kind::LESS: 
    //         return l_value < r_value;
    //     case comp_kind::LESSEQUAL: 
    //         return l_value < r_value;    
    // }

    return nullptr;
}

}