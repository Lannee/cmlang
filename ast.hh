
#include <unordered_map>
#include <list>
#include <vector>
#include <string>
#include <cstdint>
#include <optional>
#include <cassert>
#include <cstring>
#include <iostream>

namespace mlang {

static inline void __error(std::string_view message) 
    { std::cerr << "Error: " << message << std::endl; exit(1); }

static inline void __warning(std::string_view message) 
    { std::cerr << "Warning: " << message << std::endl; } 

class type;
class unit_type;
class integer_type;
class expression;

class context {
public:

    context() : variables_(0) {}

    void set_local_variable(std::string_view name, const expression* value);

    const expression *get_variable(std::string_view name);

    inline void new_scope();
    inline void pop_scope();

    void print_state() const;

    // ~context();

private:
    std::list<std::unordered_map<std::string, const expression *>> variables_;
};

class expression {
public:
    expression() : args_names_(new std::vector<std::string>(0)) {}

    virtual const expression *value(context &ctx) const = 0;

    const type *as_type() const { return (const type *)this; }

    void set_args_names(std::vector<std::string> *names) { args_names_ = names; }
    
    std::size_t num_args() const { return args_names_->size(); }
    std::vector<std::string> *args_names() const { return args_names_; }
    virtual ~expression() {}
private:
    std::vector<std::string> *args_names_;
};

enum type_kind {
    UNIT,
    INTEGER,
    STRING,
    FUNCTION,
    STRUCT
};

class type : public expression {
public:   
    virtual type_kind kind() const = 0;
    virtual std::string repr() const = 0;
    virtual integer_type to_integer_type() const = 0;
};

class integer_type : public type {
public:

    integer_type(uint32_t data) : data_(data) {}

    type_kind kind() const { return type_kind::INTEGER; };

    std::string repr() const { return std::to_string(data_); }  
    integer_type to_integer_type() const override { return *this; }

    const expression *value(context &_) const override { return this; }

    uint32_t data__() const { return data_; }
private:
    uint32_t data_;
};

class unit_type : public type {
public:
    type_kind kind() const { return type_kind::UNIT; };
    std::string repr() const { return "T"; } 
    integer_type to_integer_type() const override { return 0; } 

    const expression *value(context &_) const override { return this; }
};

const unit_type UNIT__{};

class statement : public expression {
public:
    statement() : expression() {}

    const expression *value(context &ctx) const { execute(ctx); return &UNIT__; }
    virtual void execute(context &ctx) const = 0;
};

class print_function : public expression {
public:
    print_function(const std::vector<expression *> *exprs) : exprs_(exprs) {}
    const expression *value(context &ctx) const;
    ~print_function();

private:
    const std::vector<expression *> *exprs_;
};

class until_statement : public statement {
public:
    until_statement(const expression *cond, const expression *body) : cond_(cond), body_(body) {}
    void execute(context &ctx) const;
    ~until_statement();

private:
    const expression *cond_;
    const expression *body_;
};


class if_expression : public expression {
public:
    if_expression(const expression *cond, const expression *then, const expression *otherwise) : cond_(cond), then_(then), otherwise_(otherwise) {}
    const expression *value(context &ctx) const;
    ~if_expression();

private:
    const expression *cond_;
    const expression *then_;
    const expression *otherwise_;
};

class string_type : public type {
public:

    string_type(std::string_view data) : data_(data) {}

    type_kind kind() const { return type_kind::STRING; };

    std::string repr() const { return data_; } 
    integer_type to_integer_type() const override { return data_ != ""; }  

    string_type operator+(string_type const& obj) { return string_type(data_ + obj.data_); }

    const expression *value(context &_) const override { return this; }
private:
    std::string data_;
};

class expr_list : public expression {
public:
    expr_list(std::vector<expression *> *exprs) : exprs_(exprs) {}

    const expression *value(context &ctx) const;

private:
    const std::vector<expression *> *exprs_;
};

class function_decl : public expression {
public:
    function_decl(std::string_view name, 
                  const expression *expr)
        : var_name_(name), expr_(expr) {}

    const expression *value(context &ctx) const;

    ~function_decl();                                            

private:
    const std::string var_name_;
    const expression *expr_;
};


enum comp_kind {
    EQUAL,
    NOTEQUAL,
    GREATER,
    GREATEREQUAL,
    LESS,
    LESSEQUAL
};

class comp_expression : public expression {
public:
    comp_expression(const expression *l, comp_kind op, const expression *r) : l_(l), op_(op), r_(r) {}
    const expression *value(context &ctx) const;
private:
    const expression *r_;
    comp_kind op_;
    const expression *l_;
};

class function_call : public expression {
public:
    function_call(std::string_view name, const std::vector<expression *> *args) : name_(name), args_(args) {}
    function_call(const expression *expr, const std::vector<expression *> *args) : name_(std::nullopt), expr_(expr), args_(args) {}
    const expression *value(context &ctx) const;
    ~function_call();

private:
    const std::optional<std::string> name_; 
    const expression *expr_;
    const std::vector<expression *> *args_;
};


class tostr_function : public expression {
public:
    tostr_function(const expression *arg) : arg_(arg) {}
    const string_type *value(context &ctx) const;
    ~tostr_function() { delete arg_; }

private:
    const expression *arg_;
};

class toint_function : public expression {
public:
    toint_function(const expression *arg) : arg_(arg) {}
    const integer_type *value(context &ctx) const;
    ~toint_function() { delete arg_; }

private:
    const expression *arg_;
};

}
