type node =
  { loc : Location.t
  }

type t =
  | Proc of proc
  | Let of let_
  | Expr of expr
  | Block of block
  | Return of return_
  | If of if_

and proc =
  { node : node
  ; export : bool
  ; id : Token.t
  ; params : (Token.t * Type.t) list
  ; rty : Type.t
  ; blk : t
  }

and let_ =
  { node : node
  ; id : Token.t
  ; ty : Type.t
  ; e : Expr.t
  }

and expr =
  { node : node
  ; e : Expr.t
  }

and block =
  { node : node
  ; stmts : t list
  }

and return_ =
  { node : node
  ; e : Expr.t option
  }

and if_ =
  { node : node
  ; cond : Expr.t
  ; then_ : t
  ; else_ : t option
  }
