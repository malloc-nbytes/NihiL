type node =
  { loc : Location.t
  ; ty : Type.t
  }

type t =
  | Integer of integer
  | String of string_
  | Identifier of identifier
  | Binary of binary
  | Unary of unary

and integer =
  { node : node
  ; i : Token.t
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

and unary =
  { node : node
  ; op : Token.t
  ; rhs : t
  }

let get_location = function
  | Integer e -> e.node.loc
  | String e -> e.node.loc
  | Identifier e -> e.node.loc
  | Binary e -> e.node.loc
  | Unary e -> e.node.loc

let get_type = function
  | Integer e -> e.node.ty
  | String e -> e.node.ty
  | Identifier e -> e.node.ty
  | Binary e -> e.node.ty
  | Unary e -> e.node.ty
