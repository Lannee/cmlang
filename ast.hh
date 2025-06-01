
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

class type;
class unit_type;
class integer_type;

static inline void __error(std::string_view message) 
    { std::cerr << "Error: " << message << std::endl; exit(1); }

static inline void __warning(std::string_view message) 
    { std::cerr << "Warning: " << message << std::endl; } 

class context {
public:

    context() : variables_(0) {}

    void set_global_variable(std::string_view name, const type* value);

    void set_local_variable(std::string_view name, const type* value);

    void assign_to_variable(std::string_view name, const type* value);

    const type *get_variable(std::string_view name);

    inline void new_scope();
    inline void pop_scope();

    void print_state() const;

    // ~context();

private:
    std::list<std::unordered_map<std::string, const type *>> variables_;
};

class expression {
public:
    virtual const type *value(context &ctx) const = 0;
    virtual ~expression() {}
};

enum type_kind {
    UNIT,
    INTEGER,
    STRING
};

std::string type_kind_to_string(type_kind kind);

class type : public expression {
public:
    virtual type_kind kind() const = 0;
    virtual std::string repr() const = 0;
    virtual integer_type to_integer_type() const = 0;

    virtual type *operator+ (type const &obj) const = 0;
    virtual type *operator- (type const &obj) const = 0;
    virtual type *operator* (type const &obj) const = 0;
    virtual type *operator/ (type const &obj) const = 0;
    virtual type *operator> (type const &obj) const = 0;
    virtual type *operator< (type const &obj) const = 0;
    virtual type *operator>=(type const &obj) const = 0;
    virtual type *operator<=(type const &obj) const = 0;
    virtual type *operator==(type const &obj) const = 0;
    virtual type *operator!=(type const &obj) const = 0;
};

class integer_type : public type {
public:

    integer_type(int64_t data) : data_(data) {}

    type_kind kind() const { return type_kind::INTEGER; };

    const type *value(context &_) const { return this; }

    std::string repr() const { return std::to_string(data_); }  
    integer_type to_integer_type() const override { return *this; }

    int64_t data__() const { return data_; }

    type *operator+ (type const &obj) const { return new integer_type(data_ + dynamic_cast<const integer_type &>(obj).data_); }
    type *operator- (type const &obj) const { return new integer_type(data_ - dynamic_cast<const integer_type &>(obj).data_); }
    type *operator* (type const &obj) const { return new integer_type(data_ * dynamic_cast<const integer_type &>(obj).data_); }
    type *operator/ (type const &obj) const { return new integer_type(data_ / dynamic_cast<const integer_type &>(obj).data_); }
    type *operator> (type const &obj) const { return new integer_type(data_ > dynamic_cast<const integer_type &>(obj).data_); }
    type *operator< (type const &obj) const { return new integer_type(data_ < dynamic_cast<const integer_type &>(obj).data_); }
    type *operator>=(type const &obj) const { return new integer_type(data_ >= dynamic_cast<const integer_type &>(obj).data_); }
    type *operator<=(type const &obj) const { return new integer_type(data_ <= dynamic_cast<const integer_type &>(obj).data_); }
    type *operator==(type const &obj) const { return new integer_type(data_ == dynamic_cast<const integer_type &>(obj).data_); }
    type *operator!=(type const &obj) const { return new integer_type(data_ != dynamic_cast<const integer_type &>(obj).data_); }
private:
    int64_t data_;
};

class unit_type : public type {
public:
    type_kind kind() const { return type_kind::UNIT; };
    const type *value(context &_) const { return this; }
    std::string repr() const { return "T"; } 
    integer_type to_integer_type() const override { return 0; }

    type *operator+ (type const &obj) const { unsupported_operation(); }
    type *operator- (type const &obj) const { unsupported_operation(); }
    type *operator* (type const &obj) const { unsupported_operation(); }
    type *operator/ (type const &obj) const { unsupported_operation(); }
    type *operator> (type const &obj) const { unsupported_operation(); }
    type *operator< (type const &obj) const { unsupported_operation(); }
    type *operator>=(type const &obj) const { unsupported_operation(); }
    type *operator<=(type const &obj) const { unsupported_operation(); }
    type *operator==(type const &obj) const { unsupported_operation(); }
    type *operator!=(type const &obj) const { unsupported_operation(); }

private:
    void unsupported_operation() const { __error("Unsupported operation for unit type"); }

};

const unit_type UNIT__{};

class statement : public expression {
public:
    const type *value(context &ctx) const { execute(ctx); return &UNIT__; }
    virtual void execute(context &ctx) const = 0;
};

class print_function : public expression {
public:
    print_function(const std::vector<const expression *> *exprs) : exprs_(exprs) {}
    const type *value(context &ctx) const;
    ~print_function();

private:
    const std::vector<const expression *> *exprs_;
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
    const type *value(context &ctx) const;
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

    const type *value(context &_) const { return this; }

    std::string repr() const { return data_; } 
    integer_type to_integer_type() const override { return data_ != ""; }  

    
    type *operator+ (type const &obj) const { return new string_type(data_ + dynamic_cast<const string_type &>(obj).data_); }
    type *operator- (type const &obj) const { unsupported_operation(); }
    type *operator* (type const &obj) const { unsupported_operation(); }
    type *operator/ (type const &obj) const { unsupported_operation(); }
    type *operator> (type const &obj) const { return new integer_type(data_ > dynamic_cast<const string_type &>(obj).data_); }
    type *operator< (type const &obj) const { return new integer_type(data_ < dynamic_cast<const string_type &>(obj).data_); }
    type *operator>=(type const &obj) const { return new integer_type(data_ >= dynamic_cast<const string_type &>(obj).data_); }
    type *operator<=(type const &obj) const { return new integer_type(data_ <= dynamic_cast<const string_type &>(obj).data_); }
    type *operator==(type const &obj) const { return new integer_type(data_ == dynamic_cast<const string_type &>(obj).data_); }
    type *operator!=(type const &obj) const { return new integer_type(data_ != dynamic_cast<const string_type &>(obj).data_); }
    
private:
    void unsupported_operation() const { __error("Unsupported operation for string type"); }

private:
    std::string data_;
};

class expr_list : public expression {
public:
    expr_list(std::vector<const expression *> *exprs) : exprs_(exprs) {}

    const type *value(context &ctx) const;

private:
    const std::vector<const expression *> *exprs_;
};

class var_decl : public expression {
public:
    var_decl(std::string_view name, const expression *expr) : var_name_(name), expr_(expr) { }

    const type *value(context &ctx) const;

    ~var_decl();                                            

private:
    const std::string var_name_;
    const expression *expr_;
};

class essignment_expression : public expression {
public:
    essignment_expression(std::string_view name, const expression *expr) : var_name_(name), expr_(expr) { }

    const type *value(context &ctx) const;

    ~essignment_expression();                                            

private:
    const std::string var_name_;
    const expression *expr_;
};

class variable : public expression {
public:
    variable(std::string_view name) : name_(name) {}
    const type *value(context &ctx) const;

private:
    const std::string name_;
};


enum builtin_binop_kind {
    EQUAL,
    NOTEQUAL,
    GREATER,
    GREATEREQUAL,
    LESS,
    LESSEQUAL,
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE
};

std::string builtin_binop_kind_to_string(builtin_binop_kind kind);


class builtin_binop_function : public expression {
public:
    builtin_binop_function(builtin_binop_kind op, const expression *fst, const expression *snd) : op_(op), fst_(fst), snd_(snd) {}
    const type *value(context &ctx) const;
private:
    builtin_binop_kind op_;
    const expression *fst_;
    const expression *snd_;
};

class function_call : public expression {
public:
    function_call(std::string_view name, const std::vector<const expression *> *args) : name_(name), args_(args) {}
    const type *value(context &ctx) const;
    ~function_call();

private:
    const std::string name_; 
    const std::vector<const expression *> *args_;
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
