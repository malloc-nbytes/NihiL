type t =
  | Integer    of integer
  | String     of string
  | Identifier of identifier
  | Binary     of binary

and integer    = Token.t
and string     = Token.t
and identifier = Token.t
and binary     = t * Token.t * t
