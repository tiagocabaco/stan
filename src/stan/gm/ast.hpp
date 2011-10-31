#ifndef __STAN__GM__AST_HPP__
#define __STAN__GM__AST_HPP__

#include <boost/lexical_cast.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_numeric.hpp>
#include <boost/spirit/include/classic_position_iterator.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/support_multi_pass.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/variant/static_visitor.hpp>


#include <iomanip>
#include <iostream>
#include <istream>
#include <sstream>
#include <string>
#include <utility>
#include <map>
#include <vector>

namespace stan {

  namespace gm {

    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;

    struct nil {
      /* placeholder val for boost::variant default ctors */
    };

    // components of abstract syntax tree 
    struct assignment;
    struct binary_op;
    struct distribution;
    struct double_var_decl;
    struct expression;
    struct for_statement;
    struct fun;
    struct index_op;
    struct inv_var_decl;
    struct matrix_var_decl;
    struct pos_ordered_var_decl;
    struct program;
    struct range;
    struct row_vector_var_decl;
    struct sample;
    struct simplex_var_decl;
    struct statement;
    struct statements;
    struct unary_op;
    struct var;
    struct var_decl;
    struct var_type;
    struct vector_var_decl;

    enum base_expr_type {
      INT_T,
      DOUBLE_T,
      VECTOR_T, // includes: SIMPLEX_T, POS_ORDERED_T
      ROW_VECTOR_T,
      MATRIX_T // includes: POS_SYM_DEF_MATRIX_T
    };

    class expr_type {
    private: 
      const base_expr_type base_type_;
      const unsigned int num_dims_;
    public:
      expr_type() 
	: base_type_(DOUBLE_T),
	  num_dims_(0) { 
      }
      expr_type(const base_expr_type base_type) 
	: base_type_(base_type),
	  num_dims_(0) {
      }
      expr_type(const base_expr_type base_type,
		unsigned int num_dims) 
	: base_type_(base_type),
	  num_dims_(num_dims) {
      }
      base_expr_type type() {
	return base_type_;
      }
      int num_dims() {
	return num_dims_;
      }
    };


    struct distribution {
      distribution() {
      }
      distribution(std::string& family,
		   std::vector<expression>& args)
	: family_(family),
	  args_(args) {
      }
      std::string family_;
      std::vector<expression> args_;
    };

    struct statements {
      statements() {
      }
      statements(std::vector<statement> stmts)
	: statements_(stmts) {
      }
      std::vector<statement> statements_;
    };

    struct expression {
      typedef boost::variant<nil, 
			     int,
			     double,
			     std::string, // for identifiers
			     boost::recursive_wrapper<fun>,
                             boost::recursive_wrapper<index_op>,
			     boost::recursive_wrapper<binary_op>,
			     boost::recursive_wrapper<unary_op> > 
      expression_t;

      expression()
	: expr_(nil()) {
      }

      template <typename Expr>
      expression(const Expr& expr)
	: expr_(expr) {
      }

      expression& operator+=(expression const& rhs);
      expression& operator-=(expression const& rhs);
      expression& operator*=(expression const& rhs);
      expression& operator/=(expression const& rhs);


      expression_t expr_;
    };

    struct is_nil_op : public boost::static_visitor<bool> {
      bool operator()(nil const& x) const { return true; }
      
      template <typename T>
      bool operator()(const T& x) const { return false; }
    };

    bool is_nil(const expression& e) {
      is_nil_op ino;
      return boost::apply_visitor(ino,e.expr_);
    }

    struct var {
      var() { } // req for FUSION_ADAPT
      var(std::string const& name,
	  std::vector<expression> const& dims) 
	: name_(name),
	  dims_(dims) {
      }
      std::string name_;
      std::vector<expression> dims_;
    };
    
    struct fun {
      fun() { }
      fun(std::string const& name,
	       std::vector<expression> const& args) 
	: name_(name),
	  args_(args) {
      }
      std::string name_;
      std::vector<expression> args_;
    };
    
    struct range {
      range() { } 
      range(expression const& low,
	    expression const& high)
	: low_(low),
	  high_(high) {
      }
      expression low_;
      expression high_;
    };

    struct int_var_decl {
      int_var_decl() { }
      int_var_decl(range const& range,
		   std::string const& name,
		   std::vector<expression> const& dims) 
	: name_(name),
	  range_(range),
	  dims_(dims) { 
      }
      std::string name_;
      range range_;
      std::vector<expression> dims_;
    };

    struct double_var_decl {
      double_var_decl() { }
      double_var_decl(range const& range,
		      std::string const& name,
		      std::vector<expression> const& dims)
	: name_(name),
	  range_(range),
	  dims_(dims) { 
      }
      std::string name_;
      range range_;
      std::vector<expression> dims_;
    };

    struct simplex_var_decl {
      simplex_var_decl() { }
      simplex_var_decl(expression const& K,
		       std::string const& name,
		       std::vector<expression> const& dims)
	: name_(name),
	  K_(K),
	  dims_(dims) { 
      }
      std::string name_;
      expression K_;
      std::vector<expression> dims_;
    };

    struct pos_ordered_var_decl {
      pos_ordered_var_decl() { }
      pos_ordered_var_decl(expression const& K,
			   std::string const& name,
			   std::vector<expression> const& dims)
	: name_(name),
	  K_(K),
	  dims_(dims) { 
      }
      std::string name_;
      expression K_;
      std::vector<expression> dims_;
    };

    struct vector_var_decl {
      vector_var_decl() { }
      vector_var_decl(expression const& M,
		      std::string const& name,
		      std::vector<expression> const& dims)
	: name_(name),
	  M_(M),
	  dims_(dims) { 
      }
      std::string name_;
      expression M_;
      std::vector<expression> dims_;
    };

    struct row_vector_var_decl {
      row_vector_var_decl() { }
      row_vector_var_decl(expression const& N,
			  std::string const& name,
			  std::vector<expression> const& dims)
	: name_(name),
	  N_(N),
	  dims_(dims) { 
      }
      std::string name_;
      expression N_;
      std::vector<expression> dims_;
    };

    struct matrix_var_decl {
      matrix_var_decl() { }
      matrix_var_decl(expression const& M,
		      expression const& N,
		      std::string const& name,
		      std::vector<expression> const& dims)
	: name_(name),
	  M_(M),
	  N_(N),
	  dims_(dims) { 
      }
      std::string name_;
      expression M_;
      expression N_;
      std::vector<expression> dims_;
    };

    struct cov_matrix_var_decl {
      cov_matrix_var_decl() { }
      cov_matrix_var_decl(expression const& K,
			   std::string const& name,
			   std::vector<expression> const& dims)
	: name_(name),
	  K_(K),
	  dims_(dims) {
      }
      std::string name_;
      expression K_;
      std::vector<expression> dims_;
    };

    struct corr_matrix_var_decl {
      corr_matrix_var_decl() { }
      corr_matrix_var_decl(expression const& K,
			   std::string const& name,
			   std::vector<expression> const& dims)
	: name_(name),
	  K_(K),
	  dims_(dims) {
      }
      std::string name_;
      expression K_;
      std::vector<expression> dims_;
    };

    struct var_decl {
      typedef boost::variant<nil, // just for default constructor
			     boost::recursive_wrapper<int_var_decl>,
			     boost::recursive_wrapper<double_var_decl>,
			     boost::recursive_wrapper<vector_var_decl>,
			     boost::recursive_wrapper<row_vector_var_decl>,
			     boost::recursive_wrapper<matrix_var_decl>,
			     boost::recursive_wrapper<simplex_var_decl>,
			     boost::recursive_wrapper<pos_ordered_var_decl>,
			     boost::recursive_wrapper<cov_matrix_var_decl>,
			     boost::recursive_wrapper<corr_matrix_var_decl> >
      type;

      var_decl() : decl_(nil()) { }

      template <typename Decl>
      var_decl(Decl const& decl) : decl_(decl) { }

      type decl_;
    };

    struct statement {
      typedef boost::variant<nil,
			     boost::recursive_wrapper<assignment>,
			     boost::recursive_wrapper<sample>,
			     boost::recursive_wrapper<statements>,
			     boost::recursive_wrapper<for_statement> >
      type;

      statement()
	: statement_(nil()) {
      }

      template <typename Statement>
      statement(Statement const& statement)
	: statement_(statement) {
      }

      type statement_;
    };

    struct for_statement {
      std::string variable_;
      range range_;
      statement statement_;
      for_statement() {
      }
      for_statement(std::string& variable,
		    range& range,
		    statement& stmt)
	: variable_(variable),
	  range_(range),
	  statement_(stmt) {
      }
    };

    struct program {
      program() { }
      program(std::vector<var_decl> const& data_decl,
	      std::vector<var_decl> const& parameter_decl,
	      std::vector<var_decl> const& derived_decl,
	      statement const& stmt) 
	: data_decl_(data_decl),
	  parameter_decl_(parameter_decl),
	  derived_decl_(derived_decl),
	  statement_(stmt) {
      }
      std::vector<var_decl> data_decl_;
      std::vector<var_decl> parameter_decl_;
      std::vector<var_decl> derived_decl_;
      statement statement_;
    };

    struct binary_op {
      binary_op(char op,
		expression const& left,
		expression const& right)
        : op(op), 
	  left(left), 
	  right(right) {
      }

      char op;
      expression left;
      expression right;
    };

    struct unary_op {
      unary_op(char op,
	       expression const& subject)
        : op(op), 
	  subject(subject) {
      }

      char op;
      expression subject;
    };

    struct index_op {
      index_op() { }
      index_op(const expression& expr,
	       const std::vector<std::vector<expression> >& dimss)
	: expr_(expr),
	  dimss_(dimss) {
      }
      expression expr_;
      std::vector<std::vector<expression> > dimss_;
    };

    struct sample {
      sample() {
      }
      sample(var& v,
	     distribution& dist) 
	: v_(v),
	  dist_(dist) {
      }
      var v_;
      distribution dist_;
    };

    struct assignment {
      assignment() {
      }
      assignment(var& var,
		 expression& expr)
	: var_(var),
	  expr_(expr) {
      }
      var var_;
      expression expr_;
    };
      
    expression& expression::operator+=(expression const& rhs) {
      expr_ = binary_op('+', expr_, rhs);
      return *this;
    }

    expression& expression::operator-=(expression const& rhs) {
      expr_ = binary_op('-', expr_, rhs);
      return *this;
    }

    expression& expression::operator*=(expression const& rhs) {
      expr_ = binary_op('*', expr_, rhs);
      return *this;
    }

    expression& expression::operator/=(expression const& rhs) {
      expr_ = binary_op('/', expr_, rhs);
      return *this;
    }

  }
}

#endif
