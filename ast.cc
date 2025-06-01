
#include <ostream>

#include "ast.hh"

namespace mlang {

void context::set_global_variable(std::string_view name, const type* value) {
    variables_.back()[std::string(name)] = value;
}

void context::set_local_variable(std::string_view name, const type* value) {
    variables_.front()[std::string(name)] = value;
}

void context::assign_to_variable(std::string_view name, const type* value) {
    auto _name = std::string(name);
    for (auto &scope : variables_) {
        if(scope.count(_name)) scope[_name] = value;
    }
}

const type *context::get_variable(std::string_view name) {
    auto _name = std::string(name);
    for (auto &scope : variables_) {
        if(scope.count(_name)) return scope[_name];
    }

    return nullptr;
}

inline void context::new_scope() { variables_.emplace_front(); }
inline void context::pop_scope() { variables_.pop_front();     }

void context::print_state() const {
    for(auto scope : variables_) {
        std::cout << "scope" << std::endl;
        for (auto &[n, t] : scope) {
            std::cout << "name: " << n << " value: " << (t ? t->repr() : "null") << std::endl;
        }
    }
}

// context::~context() {
//     for(auto &m : variables_)
//         for(auto [_, t] : m) delete t;
// }

std::string type_kind_to_string(type_kind kind) {
    switch (kind) {
        case UNIT    : return "unit";
        case INTEGER : return "integer";
        case STRING  : return "string";
        default      : return "undefined";
    }
}

std::string builtin_binop_kind_to_string(builtin_binop_kind kind) {
    switch(kind) {
        case builtin_binop_kind::EQUAL : return "equal";
        case builtin_binop_kind::NOTEQUAL : return "not equal";
        case builtin_binop_kind::GREATER : return "greater";
        case builtin_binop_kind::GREATEREQUAL : return "reater or equal";
        case builtin_binop_kind::LESS : return "less";
        case builtin_binop_kind::LESSEQUAL : return "less or equal";
    }
}

const type *print_function::value(context &ctx) const {
    for(const auto &expr : *exprs_)
        std::cout << expr->value(ctx)->repr(); 
    std::cout << std::endl;
    return &UNIT__;
}

print_function::~print_function() {
    for(auto *e : *exprs_) delete e;
    delete exprs_;
}

const type *function_call::value(context &ctx) const {
    return &UNIT__;
}

function_call::~function_call() {
    for(auto *e : *args_) delete e;
    delete args_;
}

const string_type *tostr_function::value(context &ctx) const {    
    return new string_type(arg_->value(ctx)->repr());
}

const integer_type *toint_function::value(context &ctx) const {  
    auto *value = arg_->value(ctx);  
    switch(value->kind()) {
        case UNIT   : return new integer_type(0);
        case INTEGER: return static_cast<const integer_type *>(value);
        case STRING : {
            try {
                return new integer_type(std::stoi(value->repr()));
            } catch(const std::exception& e) {
                __error("cannot conver str to type int");
            }
        }
        default: __error("unsupported convertion to type int");
    }
}

const type *if_expression::value(context &ctx) const {
    if (cond_->value(ctx)->to_integer_type().data__() > 0)
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
    while (cond_->value(ctx)->to_integer_type().data__() > 0)
        body_->value(ctx);
}

until_statement::~until_statement() {
    delete cond_;
    delete body_;
}

const type *var_decl::value(context &ctx) const { 
    auto *value = expr_->value(ctx);

    if(ctx.get_variable(var_name_))
        __warning("redeclaration of variable \"" + var_name_ + "\"");

    ctx.set_local_variable(var_name_, value);
    return value;
}

var_decl::~var_decl() { delete expr_; }

const type *essignment_expression::value(context &ctx) const { 
    auto *value = expr_->value(ctx);

    auto var = ctx.get_variable(var_name_);
    if(!var)
        __error("undefined symbol \"" + var_name_ + "\"");

    ctx.assign_to_variable(var_name_, value);
    return value;
}

essignment_expression::~essignment_expression() { delete expr_; }

const type *variable::value(context &ctx) const {
    auto *var = ctx.get_variable(name_);
    if(!var)
        __error("usage of undefined symbol \"" + name_ + "\"");

    return var->value(ctx);
}


const type *expr_list::value(context &ctx) const {
    // Creating local scope
    ctx.new_scope();

    const type *last_expr_value = &UNIT__;
    for(auto e : *exprs_)
        last_expr_value = e->value(ctx);
    
    ctx.pop_scope();

    return last_expr_value; 
}

const type *builtin_binop_function::value(context &ctx) const { 
    auto *fst_value = fst_->value(ctx);
    auto *snd_value = snd_->value(ctx);

    auto kind = fst_value->kind();

    if(kind != snd_value->kind())
        __error("Cannot performe " + builtin_binop_kind_to_string(op_) +
                " operation on type " + type_kind_to_string(kind) +
                " and type " + type_kind_to_string(snd_value->kind()));

    if(kind == UNIT || fst_value->kind() == UNIT)
        __error("Cannot performe " + builtin_binop_kind_to_string(op_) + " operation on unit type");

    switch (op_) {
        case NOTEQUAL     : return *fst_value != *snd_value;
        case EQUAL        : return *fst_value == *snd_value;
        case GREATER      : return *fst_value > *snd_value;
        case GREATEREQUAL : return *fst_value >= *snd_value;
        case LESS         : return *fst_value < *snd_value;
        case LESSEQUAL    : return *fst_value <= *snd_value;
        case PLUS         : return *fst_value + *snd_value;
        case MINUS        : return *fst_value - *snd_value;
        case MULTIPLY     : return *fst_value * *snd_value;
        case DIVIDE       : return *fst_value / *snd_value;
    }

    return nullptr;
}

}