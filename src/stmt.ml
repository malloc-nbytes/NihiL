type t =
  | Expr    of expr
  | Vardecl of vardecl
  | Proc    of proc

and expr = Expr.t

and vardecl =
  { id : Token.t
  ; ty : Type.t
  ; e  : Expr.t
  }

and proc =
  { export : bool
  ; id     : Token.t
  ; params : (Token.t * Type.t) list
  ; rty    : Type.t
  ; body   : t
  }
