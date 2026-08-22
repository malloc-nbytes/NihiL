type expr =
  | Integer    of Token.t
  | String     of Token.t
  | Identifier of Token.t
  | Binary     of expr * Token.t * expr

type stmt_let =
  { id : Token.t
  ; ty : Type.t
  ; e : expr
  }

type stmt_proc =
  { export : bool
  ; id : Token.t
  ; params : (Token.t * Type.t) list
  ; rty : Type.t
  ; body : stmt
  }

type stmt =
  | Let of stmt_let
  | Proc of stmt_proc
