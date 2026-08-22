type node =
  { loc : Location.t
  }

type t =
  | Expr    of expr
  | Vardecl of vardecl
  | Proc    of proc
  | Blk     of blk

and expr =
  { node : node
  ; e    : Expr.t
  }

and vardecl =
  { node : node
  ; id   : Token.t
  ; ty   : Type.t
  ; e    : Expr.t
  }

and proc =
  { node   : node
  ; export : bool
  ; id     : Token.t
  ; params : (Token.t * Type.t) list
  ; rty    : Type.t
  ; body   : t
  }

and blk =
  { node : node
  ; stmts: t list
  }
