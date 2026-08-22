type node =
  { mutable type_ : Type.t option
  ; loc : Location.t
  }

type t =
  | Integer    of integer
  | String     of string_
  | Identifier of identifier
  | Binary     of binary

and integer =
  { node : node
  ; token : Token.t
  }

and string_ =
  { node : node
  ; s : Token.t
  }

and identifier =
  { node : node
  ; id : Token.t
  }

and binary =
  { node : node
  ; lhs : t
  ; op : Token.t
  ; rhs : t
  }
